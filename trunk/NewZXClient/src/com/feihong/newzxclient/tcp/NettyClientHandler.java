/*
 * Copyright 2012 The Netty Project
 *
 * The Netty Project licenses this file to you under the Apache License,
 * version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.feihong.newzxclient.tcp;

import android.app.Activity;
import android.content.Intent;
import android.text.format.Time;
import android.util.Log;
import android.widget.Toast;

import com.feihong.newzxclient.activity.ActivityManager;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;
import com.google.protobuf.InvalidProtocolBufferException;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.EventLoop;
import io.netty.util.AttributeKey;
import zaoxun.Msg;

import io.netty.buffer.ByteBuf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * ReceiveHandler implementation for the echo client.  It initiates the ping-pong
 * traffic between the echo client and server by sending the first message to
 * the server.
 */
public class NettyClientHandler extends ChannelInboundHandlerAdapter {

	//private final NettyClient client;

/*    public NettyClientHandler(NettyClient client) {
        this.client = client;
    }*/
    
    private final String SERVER_CONNECTION = "android.com.feihong.broadcastreceiver.action.SERVERCONNECTION";

    private static final Logger logger = Logger.getLogger(
            NettyClientHandler.class.getName());

    private ChannelHandlerContext mContext;

    private OnReadListener mListener;
    
    //private AttributeKey<Long> START_TIME = new AttributeKey<Long>("START_TIME");

    public interface OnReadListener {
        public void read(Object msg) throws InvalidProtocolBufferException;
    }
    
/*    public void NettyClientHandler() {
    	
    }*/

    public void setOnReadListener(OnReadListener listener) {
        mListener = listener;
    }

    protected void sendMsg(Msg.CommonMessage msg) {
        if (mContext != null) {
            byte[] result = msg.toByteArray();

            ByteBuf buf = Unpooled.buffer(4 + result.length);
            buf.writeInt(result.length);
            buf.writeBytes(result);

            //System.out.println("send Msg success!");
            mContext.writeAndFlush(buf);
        }
    }

    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {

    }

    @Override
    public void channelRegistered(ChannelHandlerContext ctx) throws Exception {

        //�����������ԣ���¼���ӿ�ʼ��ʱ��
        //ctx.attr(START_TIME).set(System.currentTimeMillis());
        mContext = ctx;
        Loger.print("channelRegistered! " + String.valueOf(ctx.channel().isOpen()));
    }

    @Override
    public void channelUnregistered(ChannelHandlerContext ctx) throws Exception {
    	 Loger.print("channelUnregistered! " + String.valueOf(ctx.channel().isOpen()));
    	 ZXConfig.setLastRecvHBTime(0);
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        final Channel channel = ctx.channel();
         //if(channel.isOpen()){
        	 ZXConfig.setconnStatus(1);
        	 
            // �ܵ� ��Ծ��  �����ǿ���״̬
             Intent intent = new Intent();
             /*  ����Intent�����action����  */
             intent.setAction(SERVER_CONNECTION);
            /* ΪIntent������Ӹ�����Ϣ */
            intent.putExtra("status", "1");
            /* �����㲥 */
           Activity currentActivity = ActivityManager.instance().getCurrentActivity();
           currentActivity.sendBroadcast(intent);

           Loger.print("channelActive! " + String.valueOf(ctx.channel().isOpen()));
         //}

         super.channelActive(ctx);
    }

    @Override
    public void channelInactive(final ChannelHandlerContext ctx) throws Exception {
        Channel channel = ctx.channel();
        //if(!channel.isOpen()) {
        	ZXConfig.setconnStatus(0);
        	
            // �ܵ��ӻ�Ծ״̬  ת��  ����Ծ״̬ ����
            Intent intent = new Intent();
             /*  ����Intent�����action����  */
            intent.setAction(SERVER_CONNECTION);
            /* ΪIntent������Ӹ�����Ϣ */
            intent.putExtra("status", "0");
            /* �����㲥 */
            Activity currentActivity = ActivityManager.instance().getCurrentActivity();
            currentActivity.sendBroadcast(intent);

            Loger.print("channelInactive! " + String.valueOf(ctx.channel().isOpen()));
         
            ctx.channel().close();
            
            /*       
            final EventLoop loop = ctx.channel().eventLoop();
            loop.schedule(new Runnable() {
                @Override
                public void run() {
                	System.out.println("Reconnecting to: " + ctx.channel().remoteAddress());
                    try {
						client.connect();
					} 
					catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
                }
             }, 5, TimeUnit.SECONDS);
          }
          */

        super.channelInactive(ctx);
     }

	@Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        if (mListener != null) {
        	mListener.read(msg);
        }

        return;
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) throws Exception {
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
    	Loger.print("exceptionCaught() : " + cause.getMessage());
        ctx.channel().close();
        ctx.close();
    }
    
}
