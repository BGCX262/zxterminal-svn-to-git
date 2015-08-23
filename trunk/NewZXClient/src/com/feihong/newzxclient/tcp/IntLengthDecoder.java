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

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.CorruptedFrameException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.util.List;

import zaoxun.Msg;

import android.text.format.Time;

import com.feihong.newzxclient.config.ZXConfig;
import com.google.protobuf.InvalidProtocolBufferException;

/**
 * Decodes the binary representation of a {@link BigInteger} prepended
 * with a magic number ('F' or 0x46) and a 32-bit integer length prefix into a
 * {@link BigInteger} instance.  For example, { 'F', 0, 0, 0, 1, 42 } will be
 * decoded into new BigInteger("42").
 */
public class IntLengthDecoder extends ByteToMessageDecoder {

	//private String mLogFileName;
    
    //private boolean flag = false;
    
	public void writeLog(String text){
/*        try {
            FileOutputStream stream = new FileOutputStream(mLogFileName, true);
            Time time = new Time();
            time.setToNow();
            String log = time.format("%Y-%m-%d %H:%M:%S  ") + text +" \r\n";
            byte[] buffer = log.getBytes();
            stream.write(buffer);
            stream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }*/
    }
	
    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) { 	
        // Wait until the length prefix is available.
        if (in.readableBytes() < 4) {
        	//String strError = String.format("Read length data error, in.readableBytes(): %d, length: %d", in.readableBytes(), 4);
        	//writeLog(strError);
            return;
        }

        in.markReaderIndex();

        // Wait until the whole data is available.
        int dataLength = in.readInt();
        if (in.readableBytes() < dataLength) {
        	//String strError = String.format("Read length data error, in.readableBytes(): %d, length: %d", in.readableBytes(), dataLength + 4);
        	//writeLog(strError);
        	
            in.resetReaderIndex();
            return;
        }

        // Convert the received data into a new BigInteger.
        byte[] decoded = new byte[dataLength];
        in.readBytes(decoded);

        
		try {
			Msg.CommonMessage commonMessage = Msg.CommonMessage.parseFrom(decoded);
			out.add(commonMessage);
		} catch (InvalidProtocolBufferException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
    }
}
