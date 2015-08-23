package com.feihong.newzxclient.util;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public class PrintUtils {

    public static String printBytes(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bytes.length; i++) {
            sb.append("[" + i + "]:" + bytes[i] + ",");
        }
        return sb.substring(0, sb.length() - 1);
    }
}
