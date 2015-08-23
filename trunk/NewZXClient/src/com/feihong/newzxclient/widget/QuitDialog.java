package com.feihong.newzxclient.widget;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.activity.LaunchActivity;
import com.feihong.newzxclient.activity.MainActivity;
import com.feihong.newzxclient.util.DisplayUtils;

public class QuitDialog extends BaseDialog {
	private TextView content;
	
	private MainActivity m_mainActivity;
	
	private QuitDialog mQuitDlg;

	public QuitDialog(Context context) {
		
		
		super(context);
		setOwnerActivity((Activity)context);
		
		m_mainActivity = (MainActivity)context;
		
		// TODO Auto-generated constructor stub
		setTitle(R.string.exit);
		setContentView(R.layout.quitdialog);
		setWidth((int) (DisplayUtils.getWidthPixels() * 0.6f));
		content = (TextView)findViewById(R.id.qd_tx);
		Button bt_normal_quit = (Button)findViewById(R.id.normal_quit);
		Button bt_non_normal_quit = (Button)findViewById(R.id.non_normal_quit);
		//Button bt_cancel = (Button)findViewById(R.id.cancel_quit);
		bt_normal_quit.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub

/*				m_mainActivity.NotifyNormalExit();
				
				getOwnerActivity().finish();
				
				Intent intent = new Intent();
		        intent.setClass(v.getContext(), LaunchActivity.class);
		        getOwnerActivity().startActivity(intent);*/
		        
				//getOwnerActivity().finish();
		        m_mainActivity.ExitAll();
		        
			}
		});
		bt_non_normal_quit.setOnClickListener(new android.view.View.OnClickListener() {
			
			@SuppressWarnings("deprecation")
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub

/*				m_mainActivity.NotifyAbnormalExit();
				
				getOwnerActivity().finish();
				
				Intent intent = new Intent();
		        intent.setClass(v.getContext(), LaunchActivity.class);
		        getOwnerActivity().startActivity(intent);*/
		        
				//getOwnerActivity().dismissDialog(0);
				//getOwnerActivity().finish();
				
				//((BaseDialog)v.get .getParent()).dismiss();
				
				mQuitDlg.dismiss();
			}
		});
		
		mQuitDlg = this;
/*		
		receiver = new MyReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction("android.intent.action.MY_RECEIVER");
		//×¢²á¼àÌýÆ÷
		registerReceiver(receiver, filter);*/
	}
	public void setText(String s) {
		if(content!=null){
			content.setText(s);
		}
	}
	public void setText(int res) {
		if(content!=null){
			content.setText(res);
		}
	}
	
/*	@Override
	protected void onDestroy(){
		//×¢Ïú¼àÌýÆ÷
		unregisterReceiver(receiver);
		super.onDestroy();
	}*/

}
