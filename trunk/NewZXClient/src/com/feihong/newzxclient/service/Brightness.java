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
    /** 可调节的最小亮度值 */  
    public static final int MIN_BRIGHTNESS = 0;  
    /** 可调节的最大亮度值 */  
    public static final int MAX_BRIGHTNESS = 255;  
    
    public static final int BRIGHT_DAY = 255;
    public static final int BRIGHT_NIGHT = 0;
    
    /** 
     * 获得当前系统的亮度模式 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 为自动调节屏幕亮度 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 为手动调节屏幕亮度 
     */  
    public int getBrightnessMode(Activity activity) {  
        int brightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;  
        try {  
            brightnessMode = Settings.System.getInt(activity.getContentResolver(),  
                Settings.System.SCREEN_BRIGHTNESS_MODE);  
        } catch (Exception e) {  
        	Loger.print("获得当前屏幕的亮度模式失败：" + e.getMessage());  
        }  
        return brightnessMode;  
    }  
  
    /** 
     * 设置当前系统的亮度模式 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 为自动调节屏幕亮度 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 为手动调节屏幕亮度 
     */  
    public void setBrightnessMode(Activity activity, int brightnessMode) {  
        try {  
            Settings.System.putInt(activity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, brightnessMode);  
        } catch (Exception e) {  
        	Loger.print("设置当前屏幕的亮度模式失败：" + e.getMessage());  
        }  
    }  
  
    /** 
     * 获得当前系统的亮度值： 0~255 
     */  
    public int getSysScreenBrightness(Activity activity) {  
        int screenBrightness = MAX_BRIGHTNESS;  
        try {  
            screenBrightness = Settings.System.getInt(activity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS);  
        } catch (Exception e) {  
        	Loger.print("获得当前系统的亮度值失败：" + e.getMessage());  
        }  
        return screenBrightness;  
    }  
  
    /** 
     * 设置当前系统的亮度值:0~255 
     */  
    public void setSysScreenBrightness(Activity activity, int brightness) {  
        try {  
            ContentResolver resolver = activity.getContentResolver();  
            Uri uri = Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS);  
            Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS, brightness);  
            resolver.notifyChange(uri, null); // 实时通知改变  
        } catch (Exception e) {  
        	Loger.print("设置当前系统的亮度值失败：" + e.getMessage());  
        }  
    }  
  
    /** 
     * 设置屏幕亮度，这会反映到真实屏幕上 
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
