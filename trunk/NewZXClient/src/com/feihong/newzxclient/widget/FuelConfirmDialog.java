package com.feihong.newzxclient.widget;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.activity.LaunchActivity;
import com.feihong.newzxclient.activity.MainActivity;
import com.feihong.newzxclient.util.DisplayUtils;

public class FuelConfirmDialog extends BaseDialog {
	private TextView content;
	
	private MainActivity m_mainActivity;

	public FuelConfirmDialog(Context context) {
		
		
		super(context);
		setOwnerActivity((Activity)context);
		
		m_mainActivity = (MainActivity)context;
		
		// TODO Auto-generated constructor stub
		setTitle(R.string.fuel_confirm);
		setContentView(R.layout.fuel_confirm_dialog);
		setWidth((int) (DisplayUtils.getWidthPixels() * 0.6f));
		content = (TextView)findViewById(R.id.fuel_tx);
		Button bt_fuel_yes = (Button)findViewById(R.id.fuel_yes);
		Button bt_fuel_no = (Button)findViewById(R.id.fuel_no);
		//Button bt_cancel = (Button)findViewById(R.id.cancel_quit);
		bt_fuel_yes.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub

				m_mainActivity.FuelConfirm(true);
		
			}
		});
		
		bt_fuel_no.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub

				m_mainActivity.FuelConfirm(false);
				
			}
		});
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
}
