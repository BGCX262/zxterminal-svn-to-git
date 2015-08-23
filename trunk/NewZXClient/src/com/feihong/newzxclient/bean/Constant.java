package com.feihong.newzxclient.bean;

import com.feihong.newzxclient.R;

public class Constant {

	public static final String HOST = "183.224.148.30";  
	//public static final String HOST = "60.160.233.25";  
	public static final int PORT = 5555;    
	
public static enum LoadStatus {
		IDLE(0),
		WAIT(1),
		LOADED(2);
		
		private int nCode ;
		private LoadStatus( int _nCode) {
	           this.nCode = _nCode;
	       }
	 
	       @Override
	       public String toString() {
	           return String.valueOf (this.nCode);
	       }
	}
}
