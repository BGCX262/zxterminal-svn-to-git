package com.feihong.newzxclient.util;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Util {
	
	

	/**
	 * 验证输入手机号码
	 * 
	 * @param 待验证的字符串
	 * @return 如果是符合格式的字符串,返回 <b>true </b>,否则为 <b>false </b>
	 */
	public static boolean IsHandset(String str) {
		String regex = "^[0,1]+[3,5]+\\d{9}$";
		// String regex = "^\\d{1,18}$";
		Pattern p = Pattern.compile(regex);
		Matcher m = p.matcher(str);
		return m.matches();
	}
	
	
}
