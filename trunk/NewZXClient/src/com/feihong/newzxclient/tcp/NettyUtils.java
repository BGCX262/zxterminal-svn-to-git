
package com.feihong.newzxclient.tcp;

import android.util.Log;

import com.feihong.newzxclient.bean.Constant;
import com.feihong.newzxclient.util.Loger;
import com.google.protobuf.InvalidProtocolBufferException;
import io.netty.buffer.ByteBuf;
import zaoxun.Msg;

/**
 * Created with IntelliJ IDEA.
 * User: Administrator
 * Date: 13-11-11
 * Time: ����4:32
 * To change this template use File | Settings | File Templates.
 */

public class NettyUtils {

    private static NettyUtils sInstance;
    private NettyClient mNettyReadClient;
    
    private double mSendNum = 0.0;
    
    private NettyUtils() {
    }


/**
     * ����
     * @return  NetUtils
     */

    public static NettyUtils getInstance() {
        if (sInstance == null) {
            synchronized (NettyUtils.class) {
                if (sInstance == null) {
                    sInstance = new NettyUtils();
                }
            }
        }
        return sInstance;
    }
    
    public Boolean IsConnected() {
    	return mNettyReadClient.IsConencted();
    }

/**
     * ��ʼ����
     */

    public void open(final NettyClientHandler.OnReadListener listener) throws Exception {
        new Thread(new Runnable() {
            @Override
            public void run() {
                mNettyReadClient = new NettyClient(Constant.HOST, Constant.PORT);
                try {
                    mNettyReadClient.connect();
                    mNettyReadClient.setOnReadListener(listener);
                } catch (Exception e) {
                    e.printStackTrace();
                    Loger.print("NettyUtils.open() " + e.getMessage());                }
            }
        }).start();
    }

/**
     * ����Socket�Ͽ�����
     * @return  true - �Ͽ�
     */

    public boolean isSendSocketShutdown() {
        return mNettyReadClient == null || mNettyReadClient.isShutDown();
    }

/**
     * �Ͽ�����
     */

    public void close() {
        if (mNettyReadClient != null) {
            mNettyReadClient.close();
        }
    }

    public synchronized void sendMsg(Msg.CommonMessage msg) {
        if (mNettyReadClient != null && !isSendSocketShutdown()) {
            mNettyReadClient.sendMsg(msg);
            
            //mSendNum += 4 + msg.getSerializedSize();
            //Loger.print("Network traffic statistic[Send]: " + String.valueOf(mSendNum));
        }
    }
}