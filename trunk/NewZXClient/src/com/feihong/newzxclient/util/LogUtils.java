package com.feihong.newzxclient.util;

import android.util.Log;

public class LogUtils {

	public final static boolean isDebug=true;
	private final static String DEBUG_LOG="client";
	public static void logDebug(String tag,String msg){
		if(isDebug){
			Log.d(tag, msg);
		}
	}
	
	public static void logDebug(String msg){
		if(isDebug){
			Log.d(DEBUG_LOG, msg);
		}
	}
	
	
}
