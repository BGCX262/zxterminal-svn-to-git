package com.feihong.newzxclient.widget;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.activity.LaunchActivity;
import com.feihong.newzxclient.activity.MainActivity;
import com.feihong.newzxclient.util.DisplayUtils;

public class CompleteRejectDialog extends BaseDialog {
	private TextView content;
	
	private MainActivity m_mainActivity;

	public String mEdipperID = "";
	public String mCarID = "";
	
	public CompleteRejectDialog(Context context) {
		super(context);
		setOwnerActivity((Activity)context);
		
		m_mainActivity = (MainActivity)context;
		
		// TODO Auto-generated constructor stub
		setTitle(R.string.please_select);
		setContentView(R.layout.complete_reject_dialog);
		setWidth((int) (DisplayUtils.getWidthPixels() * 0.6f));
		content = (TextView)findViewById(R.id.complete_reject_tx);
		Button bt_complete_load = (Button)findViewById(R.id.load_complete);
		Button bt_reject_load = (Button)findViewById(R.id.reject_load);
		//Button bt_cancel = (Button)findViewById(R.id.cancel_quit);
		
		bt_complete_load.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				//getOwnerActivity().finish();
				
				//Intent intent = new Intent();
		        //intent.setClass(v.getContext(), LaunchActivity.class);
		        //getOwnerActivity().startActivity(intent);
		        
				m_mainActivity.NotifyLoadComplete(mCarID); 
				dismiss();
			}
		});
		
		bt_reject_load.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				//getOwnerActivity().finish();
				
				//Intent intent = new Intent();
		        //intent.setClass(v.getContext(), LaunchActivity.class);
		        //getOwnerActivity().startActivity(intent);
				
				m_mainActivity.RejectLoad(mCarID);
				dismiss();
			}
		});
	}
	
	public void setText(String s){
		if(content!=null){
			content.setText(s);
		}
	}
	
	public void setText(int res){
		if(content!=null){
			content.setText(res);
		}
	}
	
	public void setEdipperID(String s) {
		mEdipperID = s;
	}
	
	public void setCarID(String s) {
		mCarID = s;
	}
}