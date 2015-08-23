package com.feihong.newzxclient.service;

import com.feihong.newzxclient.util.Loger;

import android.app.Activity;
import android.content.ContentResolver;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;
import android.view.WindowManager;

public class Brightness {
	private static final String TAG = "Brightness";
    /** �ɵ��ڵ���С����ֵ */  
    public static final int MIN_BRIGHTNESS = 0;  
    /** �ɵ��ڵ��������ֵ */  
    public static final int MAX_BRIGHTNESS = 255;  
    
    public static final int BRIGHT_DAY = 255;
    public static final int BRIGHT_NIGHT = 0;
    
    /** 
     * ��õ�ǰϵͳ������ģʽ 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 Ϊ�Զ�������Ļ���� 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 Ϊ�ֶ�������Ļ���� 
     */  
    public int getBrightnessMode(Activity activity) {  
        int brightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;  
        try {  
            brightnessMode = Settings.System.getInt(activity.getContentResolver(),  
                Settings.System.SCREEN_BRIGHTNESS_MODE);  
        } catch (Exception e) {  
        	Loger.print("��õ�ǰ��Ļ������ģʽʧ�ܣ�" + e.getMessage());  
        }  
        return brightnessMode;  
    }  
  
    /** 
     * ���õ�ǰϵͳ������ģʽ 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 Ϊ�Զ�������Ļ���� 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 Ϊ�ֶ�������Ļ���� 
     */  
    public void setBrightnessMode(Activity activity, int brightnessMode) {  
        try {  
            Settings.System.putInt(activity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, brightnessMode);  
        } catch (Exception e) {  
        	Loger.print("���õ�ǰ��Ļ������ģʽʧ�ܣ�" + e.getMessage());  
        }  
    }  
  
    /** 
     * ��õ�ǰϵͳ������ֵ�� 0~255 
     */  
    public int getSysScreenBrightness(Activity activity) {  
        int screenBrightness = MAX_BRIGHTNESS;  
        try {  
            screenBrightness = Settings.System.getInt(activity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS);  
        } catch (Exception e) {  
        	Loger.print("��õ�ǰϵͳ������ֵʧ�ܣ�" + e.getMessage());  
        }  
        return screenBrightness;  
    }  
  
    /** 
     * ���õ�ǰϵͳ������ֵ:0~255 
     */  
    public void setSysScreenBrightness(Activity activity, int brightness) {  
        try {  
            ContentResolver resolver = activity.getContentResolver();  
            Uri uri = Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS);  
            Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS, brightness);  
            resolver.notifyChange(uri, null); // ʵʱ֪ͨ�ı�  
        } catch (Exception e) {  
        	Loger.print("���õ�ǰϵͳ������ֵʧ�ܣ�" + e.getMessage());  
        }  
    }  
  
    /** 
     * ������Ļ���ȣ���ᷴӳ����ʵ��Ļ�� 
     *  
     * @param activity 
     * @param brightness 
     */  
    public void setActScreenBrightness(final Activity activity, final int brightness) {  
        final WindowManager.LayoutParams lp = activity.getWindow().getAttributes();  
        lp.screenBrightness = brightness / (float) MAX_BRIGHTNESS;  
        activity.getWindow().setAttributes(lp);  
        
        ContentResolver resolver = activity.getContentResolver();  
        Uri uri = android.provider.Settings.System
				.getUriFor("screen_brightness");
		android.provider.Settings.System.putInt(resolver, "screen_brightness",
				brightness);
		resolver.notifyChange(uri, null);
    }  

}
