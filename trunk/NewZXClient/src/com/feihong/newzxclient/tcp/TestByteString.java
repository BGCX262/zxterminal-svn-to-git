package com.feihong.newzxclient.tcp;

import com.google.protobuf.ByteString;

import java.io.*;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public class TestByteString {
    public static void main(String[] args) throws IOException {
        FileInputStream fis = new FileInputStream("d:\\20131208161720.mp4");
        byte[] buffer = new byte[512 * 1024];
        int len = fis.read(buffer);
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        bos.write(buffer, 0, len);
        ByteString byteString = ByteString.copyFrom(bos.toByteArray());
        File outFile = new File("d:\\test.mp4");
        if (!outFile.exists()) {
            outFile.createNewFile();
        }
        FileOutputStream fos = new FileOutputStream(outFile);
        fos.write(byteString.toByteArray());
        fos.flush();
        fos.close();
    }

}
