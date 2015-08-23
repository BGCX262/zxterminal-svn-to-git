package com.feihong.newzxclient.activity;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.Date;
import java.util.Timer;

import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.format.Time;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.callback.OnResponseCheckListener;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.service.Brightness;
import com.feihong.newzxclient.tcp.NettyUtils;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;
import com.feihong.newzxclient.util.SoundsUtils;

import zaoxun.Msg;
import zaoxun.Msg.CarType;
import zaoxun.Msg.RunningStatus;

public class LaunchActivity extends BaseActivity implements OnResponseCheckListener {

    private static final int SUCCESS = 0;
	private ImageView mCheckIb;
	ProgressDialog mProgressDialog;
	
	private static final int MSG_START_FINISH = 0x001;
    private static final int MSG_START_FINISH_INTERVAL = 10000;
    private static final int MSG_CHECK_BRIGHT_LAUNCH = 10001;
    private static final int MSG_AUTO_LAUNCH = 10002;
    
    private String mLogFileName;
    
    private Timer mBrightTimer = null;
    private int mCurrentBright;
    
    private Timer mAutoLaunchTimer = null;
    
    private Brightness bright;
    public static LaunchActivity mActivity = null;
    
    int is_launched = 0;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.launch);
        
        initWidget();
        
        mProgressDialog =  new ProgressDialog(LaunchActivity.this);
        mProgressDialog.setTitle("提示信息");
        mProgressDialog.setMessage("正在请求中，请稍后......");
        //设置ProgressDialog的显示样式，ProgressDialog.STYLE_SPINNER代表的是圆形进度条
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        //设置弹出框不能被取消
        mProgressDialog.setCancelable(false);
        
        mActivity = this;
        //bright = new Brightness();
        //InitBrightTimer();
        
        mAutoLaunchTimer = new Timer(true);
        // 每20s执行一次
        mAutoLaunchTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            	//auto launch
                mHandler.sendEmptyMessage(MSG_AUTO_LAUNCH);
            }
        }, 30 * 1000,  1000 * 30);
	}
	
	//Override
	protected void onNewIntent(Intent intent) {
		// TODO Auto-generated method stub
		super.onNewIntent(intent);
		//退出
        if ((Intent.FLAG_ACTIVITY_CLEAR_TOP & intent.getFlags()) != 0) {
               finish();
        }
	}
	
	private Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
                case MSG_START_FINISH:
                	PromptUtils.dismissProgressDialog();
                	
                	if(msg.arg1 == 0) {
                		is_launched = 0;
                	}
                	else if(msg.arg1 == 1) {
                		is_launched = 2;
                	}
                	
                	mHandler.removeMessages(MSG_START_FINISH);
                    
                    break;
                    
                case MSG_CHECK_BRIGHT_LAUNCH:
                	Loger.print("MSG_CHECK_BRIGHT, System brightness: " + String.valueOf(bright.getSysScreenBrightness(mActivity)));
                	
                	if (mCurrentBright == Brightness.BRIGHT_NIGHT) {
                		bright.setActScreenBrightness(mActivity, Brightness.BRIGHT_DAY);
                    	mCurrentBright = Brightness.BRIGHT_DAY;
                	}
                	else {
                		bright.setActScreenBrightness(mActivity, Brightness.BRIGHT_NIGHT);
                    	mCurrentBright = Brightness.BRIGHT_NIGHT;
                	}
                	
                	Loger.print("MSG_CHECK_BRIGHT, Brightness mode: " + String.valueOf(bright.getBrightnessMode(mActivity)));
                	Loger.print("MSG_CHECK_BRIGHT, System brightness: " + String.valueOf(bright.getSysScreenBrightness(mActivity)));
                	
                    mHandler.removeMessages(MSG_CHECK_BRIGHT_LAUNCH);
                    break;
                    
                case MSG_AUTO_LAUNCH:
                	if(is_launched == 0) {
                		if (ZXConfig.getRunningStatus() == RunningStatus.RUNNING) {
                        	SharedPreferences settings = getSharedPreferences("login_setting", 0);
                            int is_login = settings.getInt("is_login", 0);
                            String device_id = settings.getString("device_id", "");
                            Loger.print("Auto launch: is_login(" + String.valueOf(is_login) + "), device_id(" + device_id + ")");
                            if(is_login == 1 && !device_id.equalsIgnoreCase("")) {
                            	requestCheck();
                            }
                        }
                	}

                	mHandler.removeMessages(MSG_AUTO_LAUNCH);
                    break;
			default:
				break;
			}
			super.handleMessage(msg);
		}

	};

	private void CheckBright() {
    	mHandler.sendEmptyMessage(MSG_CHECK_BRIGHT_LAUNCH);
    }
	
	private void InitBrightTimer() {
    	//create timer
        mBrightTimer = new Timer(true);
        
        int mode = bright.getBrightnessMode(mActivity);
        bright.setBrightnessMode(mActivity, 0);
        int value = bright.getSysScreenBrightness(mActivity);
        
        Calendar calEnviron = Calendar.getInstance();
        int hour = calEnviron.get(Calendar.HOUR_OF_DAY);
        if(hour < 6 || hour >= 18) {
        	bright.setActScreenBrightness(this, Brightness.BRIGHT_NIGHT);
        	mCurrentBright = Brightness.BRIGHT_NIGHT;
        	
        	// 每天的06:00.am开始执行
            calEnviron.set(Calendar.HOUR_OF_DAY, 6);
        }
        else {
        	bright.setActScreenBrightness(this, Brightness.BRIGHT_DAY);
        	mCurrentBright = Brightness.BRIGHT_DAY;
        	
        	// 每天的18:00.am开始执行
            calEnviron.set(Calendar.HOUR_OF_DAY, 18);
        }
        
        calEnviron.set(Calendar.MINUTE, 00);
        // date为制定时间
        Date dateSetter = new Date();
        dateSetter = calEnviron.getTime();
        // nowDate为当前时间
        Date nowDateSetter = new Date();
        // 所得时间差为，距现在待触发时间的间隔
        long intervalEnviron = dateSetter.getTime() - nowDateSetter.getTime();
        if (intervalEnviron < 0) {
			 calEnviron.add(Calendar.DAY_OF_MONTH, 1);
			 dateSetter = calEnviron.getTime();
			 intervalEnviron = dateSetter.getTime() - nowDateSetter.getTime();
        }
        
        // 每12小时执行一次
        mBrightTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            	CheckBright();
            }
        }, intervalEnviron,  1 * 1000 * 60 * 60 * 12);
    }
	
	private void initWidget() {
		mCheckIb = (ImageView)findViewById(R.id.check_ib);
		if (ZXConfig.getRunningStatus() == RunningStatus.RUNNING) {
			mCheckIb.setImageResource(R.drawable.check_btn_selector);
			mCheckIb.setEnabled(true);
		}
		else {
			mCheckIb.setImageResource(R.drawable.check_disable);
			mCheckIb.setEnabled(false);
		}
		
		mCheckIb.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				//soundsUtils.playSound(soundsUtils.CHECKING);
				PromptUtils.showProgressDialog(mActivity, "请求中，请稍候…", false, false);
				requestCheck();
			}
		});
	}

    private void requestCheck() {
        Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.CHECK_REQUEST);
        Msg.CheckRequest.Builder request = Msg.CheckRequest.newBuilder();       
        request.setDeviceId(ZXConfig.getDeviceId());
        builder.setCheckRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
        
        Message msg = new Message();
    	msg.what = MSG_START_FINISH;
    	msg.arg1 = 0;
    	mHandler.sendMessageDelayed(msg, MSG_START_FINISH_INTERVAL);
        
        is_launched = 1;
    }
    
    @Override
    public void responseCheck(Msg.CheckResponse msg) {   	
        if (msg != null && msg.getStatus() == SUCCESS) {
        	ZXConfig.setDriverID(msg.getDriverId());
        	ZXConfig.setDriverName(msg.getDriverName());
        	ZXConfig.setSpeedlimit(msg.getSpeedLimit());
        	
        	if (ZXConfig.getCarType() != CarType.OIL_CAR) {
        		Intent intent = new Intent(this, MainActivity.class);
                startActivity(intent);
        	}
        	else {
        		Intent intent = new Intent(this, FuelMainActivity.class);
                startActivity(intent);
        	}
        	
        	Message msg1 = new Message();
        	msg1.what = MSG_START_FINISH;
        	msg1.arg1 = 1;
        	mHandler.sendMessage(msg1);
            
            //finish();
        } 
        else {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    PromptUtils.showToast("点检失败，请重试", Toast.LENGTH_SHORT);
                    //requestCheck();
                    
                    Message msg1 = new Message();
                    msg1.what = MSG_START_FINISH;
                    msg1.arg1 = 0;
                	mHandler.sendMessage(msg1);
                }
            });
        }
    }
    
    @Override
    public void responseDeviceStatusRequest(final Msg.DeviceStatusRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (ZXConfig.getRunningStatus() == RunningStatus.RUNNING) {
        			mCheckIb.setImageResource(R.drawable.check_btn_selector);
        			mCheckIb.setEnabled(true);
        		}
        		else {
        			mCheckIb.setImageResource(R.drawable.check_disable);
        			mCheckIb.setEnabled(false);
        		}
            	
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.DEVICE_STATUS_RESPONSE);
                Msg.DeviceStatusResponse.Builder request = Msg.DeviceStatusResponse.newBuilder();
                request.setDeviceId(msg.getDeviceId());
                request.setResult(0);
                request.setUpdateTime(msg.getUpdateTime());
                builder.setDeviceStatusResponse(request);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    }
    
    @Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
    	
		initWidget();
		
		is_launched = 0;
	}
}
