package com.feihong.newzxclient.util;

/**
 * Created with IntelliJ IDEA.
 * User: Administrator
 * Date: 13-11-27
 * Time: обнГ11:02
 * To change this template use File | Settings | File Templates.
 */
public class NumUtils {
    public static int bytesToInt(byte b[]) {
        return    b[3] & 0xff
                | (b[2] & 0xff) << 8
                | (b[1] & 0xff) << 16
                | (b[0] & 0xff) << 24;
    }

    public static byte[] intToBytes(int i) {
        byte[] result = new byte[4];
        result[0] = (byte)((i >> 24) & 0xFF);
        result[1] = (byte)((i >> 16) & 0xFF);
        result[2] = (byte)((i >> 8) & 0xFF);
        result[3] = (byte)(i & 0xFF);
        return result;
    }
}
