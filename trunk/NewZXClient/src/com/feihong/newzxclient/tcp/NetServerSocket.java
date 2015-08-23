package com.feihong.newzxclient.tcp;

import android.text.format.Time;
import android.util.Log;
import com.feihong.newzxclient.bean.Constant;
import com.google.protobuf.ByteString;
import zaoxun.Msg;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class NetServerSocket {
    private ServerSocket mServerSocket;
    private ExecutorService executorService;
    private final int POOL_SIZE = 3;

    public NetServerSocket() throws IOException{
        mServerSocket = new ServerSocket(Constant.PORT);
        executorService = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors()*POOL_SIZE);
        System.out.println("服务启动");
    }

    public void service(){
        while(true){
            Socket socket=null;
            try {
                socket= mServerSocket.accept();
                //executorService.execute(new SnedHandler(socket));
                executorService.execute(new ReceiveHandler(socket));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        new NetServerSocket().service();
    }

    private class SnedHandler implements Runnable{
        private Socket socket;
        public SnedHandler(Socket socket){
            this.socket = socket;
        }

        public void run(){
            try {
                System.out.println("New connection accepted "+socket.getInetAddress()+":"+socket.getPort());
                OutputStream os = socket.getOutputStream();
                while(true) {
                    int cmd = 1011;
                    Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                    builder.setType(Msg.MessageType.SPEED_LIMIT);
                    Msg.SpeedLimitMessage.Builder msg = Msg.SpeedLimitMessage.newBuilder();
                    msg.setSpeed(48.0f);
                    builder.setSpeedLimit(msg);

                    byte[] b = new byte[4];
                    b[0] = (byte)((cmd >> 24) & 0xFF);
                    b[1] = (byte)((cmd >> 16) & 0xFF);
                    b[2] = (byte)((cmd >> 8) & 0xFF);
                    b[3] = (byte)(cmd & 0xFF);
                    ByteArrayOutputStream bos = new ByteArrayOutputStream();
                    bos.write(b);
                    bos.write(builder.build().toByteArray());
                    os.write(bos.toByteArray());
                    System.out.println("sendSpeed speed");
                    Thread.sleep(1000);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally{
                try {
                    if(socket!=null)
                        socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class ReceiveHandler implements Runnable{
        private Socket socket;
        public ReceiveHandler(Socket socket){
            this.socket = socket;
        }

        public final int readInt(InputStream in) throws IOException {
            int ch1 = in.read();
            int ch2 = in.read();
            int ch3 = in.read();
            int ch4 = in.read();
            if ((ch1 | ch2 | ch3 | ch4) < 0)
                throw new EOFException();
            return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
        }

        public void run(){
            try {
                System.out.println("New connection accepted "+socket.getInetAddress()+":"+socket.getPort());
                DataInputStream dis = new DataInputStream(socket.getInputStream());
                while(true) {
                    int length = dis.readInt();
                    System.out.println("消息长度："+length);
                    int readLength = 0;
                    ByteArrayOutputStream bos = new ByteArrayOutputStream();
                    while(readLength < length) {
                        byte[] buffer = new byte[512 * 1024];
                        int len = dis.read(buffer);
                        bos.write(buffer, 0, len);
                        readLength += len;
                        System.out.println("已读取长度：" + readLength);
                    }
                    logBytes(bos.toByteArray());
                    Msg.CommonMessage msg = Msg.CommonMessage.parseFrom(bos.toByteArray());
                    System.out.println("msg type:" + msg.getType());
                    switch (msg.getType()) {
                        case LOGIN_REQUEST:
                            System.out.println("收到注册数据");
                            break;
                        case CHECK_REQUEST:
                            System.out.println("收到点检数据");
                            break;
                        case OIL_QUANTITY_REQUEST:
                            System.out.println("收到油量数据");
                            break;
                        case SHORT_MESSAGE:
                            System.out.println("收到短信息数据");
                            break;
                        case GPS_DATA:
                            System.out.println("收到GPS数据");
                            break;
                        case AUDIO_FILE_DATA:
                            Msg.AudioFileData audioMsg = msg.getAudioFileData();
                            System.out.println("接受到音频文件>> deviceId:" + audioMsg.getDeviceId()+ " fileName:" + audioMsg.getFileName()+ " length:"+audioMsg.getDataLen());
                            ByteString byteString = audioMsg.getData();
                            //logData(byteString);
                            File file = new File("D:\\" + audioMsg.getFileName());
                            if (!file.exists()) {
                                file.createNewFile();
                            }
                            FileOutputStream fos = new FileOutputStream(file);
                            fos.write(byteString.toByteArray());
                            fos.flush();
                            fos.close();
                            System.out.println("写入文件成功!");
                            break;
                        case PRODUCTION_REQUEST:
                        default:
                            break;
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }finally{
                try {
                    if(socket!=null)
                        socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void logData(ByteString byteString) {
        byte[] bytes = byteString.toByteArray();
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(b+",");
        }
        System.out.println(sb.toString());
    }

    private void logBytes(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (int i =0; i < bytes.length; i++) {
            sb.append("b["+i+"]:"+bytes[i]+",");
        }
        writeLog(sb.toString());
    }

    public void writeLog(String text){
        try {
            FileOutputStream stream = new FileOutputStream("f:\\xh.log", true);
            byte[] buffer = text.getBytes();
            stream.write(buffer);
            stream.flush();
            stream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}

