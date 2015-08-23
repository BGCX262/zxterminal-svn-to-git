package com.feihong.newzxclient.tcp;

import android.app.Activity;
import android.text.format.Time;
import com.feihong.newzxclient.activity.ActivityManager;
import com.feihong.newzxclient.bean.Constant;
import com.feihong.newzxclient.callback.OnResponseCheckListener;
import com.feihong.newzxclient.callback.OnResponseMessageListener;
import com.feihong.newzxclient.callback.OnResponseRegisterListener;
import com.feihong.newzxclient.util.NumUtils;
import com.google.protobuf.ByteString;
import zaoxun.Msg;

import java.io.*;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created with IntelliJ IDEA.
 * User: Administrator
 * Date: 13-11-11
 * Time: 上午4:32
 * To change this template use File | Settings | File Templates.
 */
public class NetUtils {

    private static final int POOL_SIZE = 2;

    private static NetUtils sInstance;

    private ExecutorService mThreadPool;

    private ReceiveThread mReceiveThread;
    private SendThread mSendThread;
    private Socket mSocket;
    
    

    private NetUtils() {
        mThreadPool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors() * POOL_SIZE);
    }

    /**
     * 单例
     * @return  NetUtils
     */
    public static NetUtils getInstance() {
        if (sInstance == null) {
            synchronized (NetUtils.class) {
                if (sInstance == null) {
                    sInstance = new NetUtils();
                }
            }
        }
        return sInstance;
    }

    private void reconnect() {
        synchronized (mSocket) {
            if (mSocket != null && mSocket.isConnected()) {
                try {
                    mSocket.close();
                    mSocket = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            try {
                mSocket = new Socket(Constant.HOST, Constant.PORT);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 开始连接
     */
    public void open() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    mSocket = new Socket(Constant.HOST, Constant.PORT);
                    if (isReceiveSocketShutdown()) {
                        mReceiveThread = new ReceiveThread(mSocket);
                    }
                    if (isSendSocketShutdown()) {
                        mSendThread = new SendThread(mSocket);
                    }
                    mThreadPool.execute(mReceiveThread);
                    mThreadPool.execute(mSendThread);

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    /**
     * 接收Socket断开连接
     * @return  true - 断开
     */
    public boolean isReceiveSocketShutdown() {
        return mReceiveThread == null || mReceiveThread.mShutDown || mReceiveThread.mSocket == null;
    }

    /**
     * 发送Socket断开连接
     * @return  true - 断开
     */
    public boolean isSendSocketShutdown() {
        return mSendThread == null || mSendThread.mShutDown || mSendThread.mSocket == null;
    }

    /**
     * 断开连接
     */
    public void close() {
        if (mReceiveThread != null) {
            mReceiveThread.shutdown();
        }

        if (mSendThread != null) {
            mSendThread.shutdown();
        }

        if (mSocket != null && mSocket.isConnected()) {
            try {
                mSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 发送消息
     * @param msg  msg
     */
    public void sendMsg(Msg.CommonMessage msg) {
        if (mSendThread != null) {
            mSendThread.sendMsg(msg);
        }
    }

    private class ReceiveThread implements Runnable {

        private Socket mSocket;
        public ReceiveThread(Socket socket) {
            mSocket = socket;
        }

        private boolean mShutDown = true;

        private void shutdown() {
            mShutDown = true;
        }

        @Override
        public void run() {
                mShutDown = false;
                while(!mShutDown) {
                    try {
                        DataInputStream dis = new DataInputStream(mSocket.getInputStream());
                        int length = dis.readInt();
                        byte[]  data = new byte[length];
                        dis.read(data);
                        handleMessage(Msg.CommonMessage.parseFrom(data));
                    } catch (IOException e) {
                        e.printStackTrace();
                        reconnect();
                    }
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
    }

    private void handleMessage(Msg.CommonMessage msg) {
        Activity currentActivity = ActivityManager.instance().getCurrentActivity();
        System.out.println("msg type:" + msg.getType());
        switch (msg.getType()) {
            case LOGIN_RESPONSE:
                if (currentActivity instanceof OnResponseRegisterListener) {
                    ((OnResponseRegisterListener)currentActivity).responseRegister(msg.getLoginResponse());
                }
                break;
            case CHECK_RESPONSE:
                if (currentActivity instanceof OnResponseCheckListener) {
                    ((OnResponseCheckListener)currentActivity).responseCheck(msg.getCheckResponse());
                }
                break;
            case OIL_QUANTITY_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener)currentActivity).responseOilQuantity(msg.getOilQuantityResponse());
                    System.out.println(" deviceId:" + msg.getOilQuantityResponse().getDeviceId() + " oilQuantity:" + msg.getOilQuantityResponse().getQuantity());
                }
                break;
            case SPEED_LIMIT:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener)currentActivity).responseSpeedLimit(msg.getSpeedLimit());
                    System.out.println(" deviceId:" + msg.getSpeedLimit().getDeviceId() + " speed:" + msg.getSpeedLimit().getSpeed());
                }
                break;
            case SHORT_MESSAGE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener)currentActivity).responseShortMessage(msg.getShortMessage());
                    System.out.println(" deviceId:" + msg.getShortMessage().getDeviceId() + " short msg :" + msg.getShortMessage().getContent());
                }
                break;
            case GPS_DATA:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener)currentActivity).responseGPS(msg.getGpsData());
                }
                break;
            case AUDIO_FILE_DATA:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener)currentActivity).responseAudioFile(msg.getAudioFileData());
                    System.out.println(" deviceId:" + msg.getAudioFileData().getDeviceId() + " file:" + msg.getAudioFileData().getFileName());
                }
                break;
            case PRODUCTION_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    System.out.println(" deviceId:" + msg.getProductionResponse().getDeviceId() + " duty:" +msg.getProductionResponse().getPeriod()+" producation:" + msg.getProductionResponse().getQuantity());
                    ((OnResponseMessageListener)currentActivity).responseProduction(msg.getProductionResponse());

                }
            default:
                break;
        }
    }

    private class SendThread implements Runnable {

        private boolean mShutDown = true;
        private Socket mSocket;
        public SendThread(Socket socket) {
            mSocket = socket;
        }

        public void sendAudioMsg(byte[] bytes) {
            try {
                synchronized (mSocket) {
                    System.out.println("消息长度:" + bytes.length + " 字节");
                    OutputStream dos = mSocket.getOutputStream();
                    dos.write(NumUtils.intToBytes(bytes.length));
                    dos.write(bytes);
                    logBytes(bytes);
                    dos.flush();
                    System.out.println("消息发送成功!");
                }
            } catch (IOException e) {
                e.printStackTrace();
                reconnect();
            }
        }
        private void sendMsg(Msg.CommonMessage msg) {
            try {
                synchronized (mSocket) {
                    byte[] result = msg.toByteArray();
                    logBytes(result);
                    System.out.println("消息长度:" + result.length + " 字节");
                    DataOutputStream dos = new DataOutputStream(mSocket.getOutputStream());
                    dos.writeInt(result.length);
                    dos.write(result);
                    dos.flush();
                    System.out.println("消息发送成功!");
                }
            } catch (IOException e) {
                System.out.println("send Msg error:"+ e.getMessage());
                e.printStackTrace();
                reconnect();
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
                FileOutputStream stream = new FileOutputStream("/sdcard/xh2.log", true);
                Time time = new Time();
                time.setToNow();
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

        private void shutdown() {
            mShutDown = true;
        }

        @Override
        public void run() {
            try {
                mShutDown = false;
                while(!mShutDown) {
                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mShutDown = true;
        }
    }
}
