package com.feihong.newzxclient.widget;

import com.feihong.newzxclient.R;

import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Handler;
import android.os.Message;
import android.view.Gravity;
import android.view.MotionEvent;
import android.widget.TextView;

public class MessageDialog extends BaseDialog{
     TextView content;
     public static int DIMISS = 100;
     Handler dismissHandler = new Handler(){

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			MessageDialog.this.dismiss();
		}
    	 
     };
	 public MessageDialog(Context context) {
		super(context);
		content = new TextView(context);
		content.setTextSize(40);
		setTitle(R.string.message_dialog_title);
		content.setTextColor(Color.BLACK);
		//content.setBackgroundColor(Color.GRAY);
		content.setGravity(Gravity.CENTER);
		content.setPadding(10, 40, 10, 40);
		setContentView(content);
		// TODO Auto-generated constructor stub
	}
	 public void setMessage(String s){
		 content.setText(s);
	 }
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		dismiss();
		return true;
	}
	@Override
	public void dismiss() {
		// TODO Auto-generated method stub
		super.dismiss();
		dismissHandler.removeMessages(DIMISS);
	}
	@Override
	public void show() {
		// TODO Auto-generated method stub
		super.show();
		//dismissHandler.sendEmptyMessageDelayed(DIMISS, 15000);
	}
	 
    
}
