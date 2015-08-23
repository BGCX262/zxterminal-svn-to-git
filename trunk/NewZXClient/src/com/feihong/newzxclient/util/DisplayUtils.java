package com.feihong.newzxclient.util;

import android.content.Context;
import android.util.DisplayMetrics;


/**
 * 显示相关总汇
 * @author hao.xiong
 * @version 1.0.0
 */
public class DisplayUtils {
    private static DisplayMetrics sDisplayMetrics;

    private static final float ROUND_DIFFERENCE = 0.5f;

    /**
     * 初始化操作
     * @param context context
     */
    public static void init(Context context) {
        sDisplayMetrics = context.getResources().getDisplayMetrics();
    }

    /**
     * 获取屏幕宽度 单位：像素
     * @return 屏幕宽度
     */
    public static int getWidthPixels() {
        return sDisplayMetrics.widthPixels;
    }

    /**
     * 获取屏幕高度 单位：像素
     * @return 屏幕高度
     */
    public static int getHeightPixels() {
        return sDisplayMetrics.heightPixels;
    }

    /**
     * 获取屏幕宽度 单位：像素
     * @return 屏幕宽度
     */
    public static float getDensity() {
        return sDisplayMetrics.density;
    }

    /**
     * dp 转 px
     * @param dp dp值
     * @return 转换后的像素值
     */
    public static int dp2px(int dp) {
        return (int)(dp * sDisplayMetrics.density + ROUND_DIFFERENCE);
    }

    /**
     * px 转 dp
     * @param px px值
     * @return 转换后的dp值
     */
    public static int px2dp(int px) {
        return (int)(px / sDisplayMetrics.density + ROUND_DIFFERENCE);
    }
}
