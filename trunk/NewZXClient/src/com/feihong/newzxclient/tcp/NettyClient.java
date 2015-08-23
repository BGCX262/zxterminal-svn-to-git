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

import android.text.TextUtils;

import com.feihong.newzxclient.config.ZXConfig;
import com.google.protobuf.InvalidProtocolBufferException;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import zaoxun.Msg;

/**
 * Sends one message when a connection is open and echoes back any received
 * data to the server.  Simply put, the echo client initiates the ping-pong
 * traffic between the echo client and server by sending the first message to
 * the server.
 */
public class NettyClient {

    private final String host;
    private final int port;

    private EventLoopGroup mGroup;
    private Channel mChannel;
    private NettyClientHandler mHandler;

    private NettyClientHandler.OnReadListener mListener;

    public void setOnReadListener(NettyClientHandler.OnReadListener listener) {
        mListener = listener;
        mHandler.setOnReadListener(listener);
    }

    public NettyClient(String host, int port) {
        this.host = host;
        this.port = port;
    }

    private void requestLogin() {
    	String deviceID = ZXConfig.getDeviceId();
    	if(!TextUtils.isEmpty(deviceID)) {
    		Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
            builder.setType(Msg.MessageType.LOGIN_REQUEST);
            Msg.LoginRequest.Builder request = Msg.LoginRequest.newBuilder();
            request.setDeviceId(deviceID);
            builder.setLoginRequest(request);
            NettyUtils.getInstance().sendMsg(builder.build());
    	}
    }
    
    @SuppressWarnings("unchecked")
	public void connect() throws Exception {
        mGroup = new NioEventLoopGroup();
        Bootstrap b = new Bootstrap();
        b.group(mGroup)
                .channel(NioSocketChannel.class)
                .option(ChannelOption.TCP_NODELAY, true)
                .handler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    public void initChannel(SocketChannel ch) throws Exception {
                    	ch.pipeline().addLast("decoder", new IntLengthDecoder());
                        ch.pipeline().addLast(
                                new NettyClientHandler());
                    }
                });

        ChannelFuture future = b.connect(host, port).sync();
        future.addListeners(new ChannelFutureListener()
        {
            public void operationComplete(final ChannelFuture future)
                throws Exception
            {
            	requestLogin();
            }
        });

        mChannel = future.channel();

        mHandler = mChannel.pipeline().get(NettyClientHandler.class);

    }

    public boolean isShutDown() {
        return mGroup == null || mGroup.isShutdown() || mGroup.isShuttingDown();
    }
    
    public synchronized boolean IsConencted() {
    	//synchronized (mSocket) {
    		if (mChannel == null) {
    			return false;
            }	
            else {
            	return mChannel.isOpen();
            }
    	//}
    }

    public void sendMsg(Msg.CommonMessage msg) {
        if (mHandler != null) {
            mHandler.sendMsg(msg);
        }
    }

    public synchronized void close() {
        if (mChannel != null) {
            mChannel.close();
        }
        if (!isShutDown()) {
            mGroup.shutdownGracefully();
        }
    }
}
