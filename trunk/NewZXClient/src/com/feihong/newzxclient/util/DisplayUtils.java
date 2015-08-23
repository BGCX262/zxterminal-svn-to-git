package com.feihong.newzxclient.util;

import android.content.Context;
import android.util.DisplayMetrics;


/**
 * ��ʾ����ܻ�
 * @author hao.xiong
 * @version 1.0.0
 */
public class DisplayUtils {
    private static DisplayMetrics sDisplayMetrics;

    private static final float ROUND_DIFFERENCE = 0.5f;

    /**
     * ��ʼ������
     * @param context context
     */
    public static void init(Context context) {
        sDisplayMetrics = context.getResources().getDisplayMetrics();
    }

    /**
     * ��ȡ��Ļ��� ��λ������
     * @return ��Ļ���
     */
    public static int getWidthPixels() {
        return sDisplayMetrics.widthPixels;
    }

    /**
     * ��ȡ��Ļ�߶� ��λ������
     * @return ��Ļ�߶�
     */
    public static int getHeightPixels() {
        return sDisplayMetrics.heightPixels;
    }

    /**
     * ��ȡ��Ļ��� ��λ������
     * @return ��Ļ���
     */
    public static float getDensity() {
        return sDisplayMetrics.density;
    }

    /**
     * dp ת px
     * @param dp dpֵ
     * @return ת���������ֵ
     */
    public static int dp2px(int dp) {
        return (int)(dp * sDisplayMetrics.density + ROUND_DIFFERENCE);
    }

    /**
     * px ת dp
     * @param px pxֵ
     * @return ת�����dpֵ
     */
    public static int px2dp(int px) {
        return (int)(px / sDisplayMetrics.density + ROUND_DIFFERENCE);
    }
}
