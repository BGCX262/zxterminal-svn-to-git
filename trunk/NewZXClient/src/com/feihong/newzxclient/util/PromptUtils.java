package com.feihong.newzxclient.util;

import android.app.ProgressDialog;
import android.content.Context;
import android.widget.Toast;


/**
 * ��ʾUtil�ܻ�
 *
 * @author hao.xiong
 * @version 1.0.0
 */
public class PromptUtils {

    private static Context sContext; //Application ��context
    private static Toast sToast;
    private static ProgressDialog sProgressDialog;
    /**
     * ��ʼ��
     *
     * @param context context
     */
    public static void init(Context context) {
        sContext = context;
        sToast = Toast.makeText(context, "", Toast.LENGTH_SHORT);
    }

    /**
     * ��ʾһ��Toast��ʾ
     *
     * @param message  ��ʾ��Ϣ
     * @param duration ��ʾʱ��
     */
    public static void showToast(String message, int duration) {
        sToast.setText(message);
        sToast.setDuration(duration);
        sToast.show();
    }

    /**
     * ��ʾһ��Toast��ʾ
     * @param resId    ��ʾ��Ϣ��ԴId
     * @param duration ��ʾʱ��
     */
    public static void showToast(int resId, int duration) {
        showToast(sContext.getString(resId), duration);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context �����Ķ���
     * @param resId   ��ʾ��Ϣ��ԴID
     */
    public static synchronized void showProgressDialog(Context context, int resId) {
        showProgressDialog(context, context.getResources().getString(resId), true);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context �����Ķ���
     * @param message ��ʾ��Ϣ
     */
    public static synchronized void showProgressDialog(Context context, String message) {
        showProgressDialog(context, message, true, true);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context                      �����Ķ���
     * @param resId                        ��ʾ��Ϣ��ԴID
     * @param canceledOnTouchOutsideEnable �Ƿ��������Ի�������ĵط����رնԻ���
     */
    public static synchronized void showProgressDialog(Context context, int resId, boolean canceledOnTouchOutsideEnable) {
        showProgressDialog(context, context.getResources().getString(resId), canceledOnTouchOutsideEnable, true);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context                      �����Ķ���
     * @param message                      ��ʾ��Ϣ
     * @param canceledOnTouchOutsideEnable �Ƿ��������Ի�������ĵط����رնԻ���
     */
    public static synchronized void showProgressDialog(Context context, String message, boolean canceledOnTouchOutsideEnable) {
        showProgressDialog(context, message, canceledOnTouchOutsideEnable, true);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context                      �����Ķ���
     * @param resId                        ��ʾ��Ϣ��ԴID
     * @param canceledOnTouchOutsideEnable �Ƿ��������Ի�������ĵط����رնԻ���
     * @param cancel                       ���back��ʱ���Ƿ�رնԻ���
     */
    public static synchronized void showProgressDialog(Context context, int resId, boolean canceledOnTouchOutsideEnable, boolean cancel) {
        if (context == null) {
            throw new IllegalArgumentException("context must not be null!!");
        }
        showProgressDialog(context, context.getString(resId), canceledOnTouchOutsideEnable, cancel);
    }

    /**
     * ��ʾһ�����ȶԻ���
     *
     * @param context                      �����Ķ���
     * @param message                      ��ʾ��Ϣ
     * @param canceledOnTouchOutsideEnable �Ƿ��������Ի�������ĵط����رնԻ���
     * @param cancel                       ���back��ʱ���Ƿ�رնԻ���
     */
    public static synchronized void showProgressDialog(Context context, String message, boolean canceledOnTouchOutsideEnable, boolean cancel) {
        if (context == null) {
            throw new IllegalArgumentException("context must not be null!!");
        }
        try {
            if (sProgressDialog != null) {
                sProgressDialog.dismiss();
                sProgressDialog = null;
            }

            sProgressDialog = new ProgressDialog(context);
            sProgressDialog.setCanceledOnTouchOutside(canceledOnTouchOutsideEnable);
            sProgressDialog.setCancelable(cancel);
            sProgressDialog.setMessage(message);
            sProgressDialog.show();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    /**
     * �رնԻ���
     */
    public static void dismissProgressDialog() {
        try {
            if (sProgressDialog != null) {
                sProgressDialog.dismiss();
                sProgressDialog = null;
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    /**
     * ���ȶԻ����Ƿ�����ʾ��
     *
     * @return true - ��ʾ��
     */
    public static boolean isProgressDialogShowing() {
        return sProgressDialog != null && sProgressDialog.isShowing();
    }
}
