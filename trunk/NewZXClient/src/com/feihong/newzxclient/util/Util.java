package com.feihong.newzxclient.util;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Util {
	
	

	/**
	 * ��֤�����ֻ�����
	 * 
	 * @param ����֤���ַ���
	 * @return ����Ƿ��ϸ�ʽ���ַ���,���� <b>true </b>,����Ϊ <b>false </b>
	 */
	public static boolean IsHandset(String str) {
		String regex = "^[0,1]+[3,5]+\\d{9}$";
		// String regex = "^\\d{1,18}$";
		Pattern p = Pattern.compile(regex);
		Matcher m = p.matcher(str);
		return m.matches();
	}
	
	
}
