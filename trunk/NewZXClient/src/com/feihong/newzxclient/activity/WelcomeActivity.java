package com.feihong.newzxclient.activity;

import android.content.Intent;
import android.os.Build;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.widget.ImageView;
import android.widget.Toast;

import com.feihong.newzxclient.R;

/**
 * @author song
 */
public class WelcomeActivity extends BaseActivity {

	private ImageView welcome_page;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.welcome_layout);
		initView();
		createAnimation();
	}
	
	public void initView() {
		
		welcome_page = (ImageView) this.findViewById(R.id.welcome_page);
		
		//closeBar();
	}
	
	private void createAnimation() {
		// TODO Auto-generated method stub
		AlphaAnimation alpAni = new AlphaAnimation(0.0f, 1.0f);
		alpAni.setDuration(2000);
		welcome_page.startAnimation(alpAni);
		alpAni.setAnimationListener(new MyAnimationListener());
	}
	
	@Override
	protected void onStart() {
		super.onStart();
	}
	
	 private void closeBar() {
         try{
			 //需要root 权限 
			 Build.VERSION_CODES vc = new Build.VERSION_CODES();
			 Build.VERSION vr = new Build.VERSION();
			 String ProcID = "79"; 
			
/*			 if(VERSION.SDK_INT >= VERSION_CODES.ICE_CREAM_SANDWICH) {
			     ProcID = "42"; //ICS AND NEWER
			 }*/
			
			 //需要root 权限 
			 Process proc = Runtime.getRuntime().exec(new String[]{"su","-c","service call activity "+ ProcID +" s16 com.android.systemui"}); //WAS 79
			 proc.waitFor();
			
         }catch(Exception ex){
             Toast.makeText(getApplicationContext(), ex.getMessage(), Toast.LENGTH_LONG).show();
         }
	}
	 
	 private void showBar() {
        try {
            Process proc = Runtime.getRuntime().exec(new String[]{
                    "am","startservice","-n","com.android.systemui/.SystemUIService"});
            proc.waitFor();
        } 
        catch (Exception e) {
            e.printStackTrace();
        }
	 }
	 
	private class MyAnimationListener implements AnimationListener {

		@Override
		public void onAnimationEnd(Animation animation) {
			Intent intent = new Intent();
			intent.setClass(WelcomeActivity.this, LoginActivity.class);
			startActivity(intent);
			finish();
		}

		@Override
		public void onAnimationRepeat(Animation animation) {

		}

		@Override
		public void onAnimationStart(Animation animation) {

		}
	}

}