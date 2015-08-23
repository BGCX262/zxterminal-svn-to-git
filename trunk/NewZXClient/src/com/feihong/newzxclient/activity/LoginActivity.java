package com.feihong.newzxclient.activity;

import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.bean.Constant.LoadStatus;
import com.feihong.newzxclient.callback.OnResponseRegisterListener;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.service.Brightness;
import com.feihong.newzxclient.tcp.NettyUtils;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;
import zaoxun.Msg;

import java.io.IOException;
import java.util.Calendar;
import java.util.Date;
import java.util.Timer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public class LoginActivity extends BaseActivity implements View.OnClickListener
        , View.OnLongClickListener, OnResponseRegisterListener {

    private static final int SUCCESS = 0;
    private ImageView login_imgeView;
    private TextView displayText;
    private  ImageView mClearIv;
    private  String LoginNum = "";
    ProgressDialog mProgressDialog;
    
    private static final int MSG_LOGIN_FINISH = 0x001;
    private static final int MSG_LOGIN_FINISH_INTERVAL = 10000;
    private static final int MSG_CHECK_BRIGHT_LOGIN = 10011;
    private static final int MSG_AUTO_LOGIN = 10012;
    
    private Timer mBrightTimer = null;
    private int mCurrentBright;
    
    private Timer mAutoLoginTimer = null;
    
    private Brightness bright;
    public static LoginActivity mActivity = null;
    
    //int is_logined = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.login_layout);
        initView();
        setListener();
        
        ZXConfig.setLoadStatus(LoadStatus.IDLE);
        
        mProgressDialog =  new ProgressDialog(LoginActivity.this);
        mProgressDialog.setTitle("提示信息");
        mProgressDialog.setMessage("正在登陆中，请稍后......");
        //设置ProgressDialog的显示样式，ProgressDialog.STYLE_SPINNER代表的是圆形进度条
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        //设置弹出框不能被取消
        mProgressDialog.setCancelable(false);   
        
        mActivity = this;
        //bright = new Brightness();
        //InitBrightTimer();
        
        mAutoLoginTimer = new Timer(true);
        // 每20s执行一次
        mAutoLoginTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            	//auto login
                mHandler.sendEmptyMessage(MSG_AUTO_LOGIN);
            }
        }, 30 * 1000,  1000 * 30);
    }
    
    @Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		
		//is_logined = 0;
		ZXConfig.setLoginStatus(0);
	}
    
    private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
                case MSG_LOGIN_FINISH:
                	Loger.print("Process MSG_LOGIN_FINISH: " + String.valueOf(msg.arg1));
                	
                	if(msg.arg1 == 0) {
                		//is_logined = 0;
                		ZXConfig.setLoginStatus(0);
                	}
                	else if(msg.arg1 == 1) {
                		//is_logined = 2;
                		ZXConfig.setLoginStatus(2);
                	}
                	
                    if(mProgressDialog.isShowing()) {
                    	mProgressDialog.dismiss();
                    }
                    mHandler.removeMessages(MSG_LOGIN_FINISH);
                    
                    if(msg.arg1 == 1) {                    	
                    	Loger.print("Login success: Enter launch window");
                    	
                    	Intent intent = new Intent(LoginActivity.this, LaunchActivity.class);
                        startActivity(intent);
                        
                        finish();
                    }
                    
                    break;
                    
                case MSG_CHECK_BRIGHT_LOGIN:
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
                	
                    mHandler.removeMessages(MSG_CHECK_BRIGHT_LOGIN);
                    break;
                    
                case MSG_AUTO_LOGIN:
                	int status = ZXConfig.getLoginStatus();
                	if(status == 0) {
                		SharedPreferences settings = getSharedPreferences("login_setting", 0);
                        int is_login = settings.getInt("is_login", 0);
                        String device_id = settings.getString("device_id", "");
                        Loger.print("Auto login: " + String.valueOf(is_login) + " " + device_id);
                        if(is_login == 1 && !device_id.equalsIgnoreCase("")) {
                        	requestRegister(device_id);
                        }
                	}
 
                    mHandler.removeMessages(MSG_AUTO_LOGIN);
                	break;
                    
			default:
				break;
			}
			super.handleMessage(msg);
		}

	};

	private void CheckBright() {
    	mHandler.sendEmptyMessage(MSG_CHECK_BRIGHT_LOGIN);
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
	
    private void initView() {
        displayText = (TextView) this.findViewById(R.id.useraccount_edittext);
        /*displayText.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                hideSoftInputMode((EditText)view);

            }
        });*/
        login_imgeView = (ImageView) this.findViewById(R.id.ok_imageview);
        mClearIv = (ImageView)this.findViewById(R.id.clear_num_imageview);

    }

    private void setListener() {
        findViewById(R.id.zero_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.one_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.two_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.three_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.four_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.five_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.six_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.seven_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.eight_imageview).setOnClickListener(mNumClickListener);
        findViewById(R.id.nine_imageview).setOnClickListener(mNumClickListener);

        login_imgeView.setOnClickListener(this);
        mClearIv.setOnClickListener(this);
        mClearIv.setOnLongClickListener(this);

    }

    private View.OnClickListener mNumClickListener =  new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            displayText.setText(displayText.getText().toString() + v.getTag());
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.clear_num_imageview:
                clearEvent();
                
                break;
                
            case R.id.ok_imageview:
            	String number = displayText.getText().toString().trim();
            	//if(checkPhoneNum(number)) {
        		LoginNum = number;
        		
        		try {
                    loginEvent();
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            	//}

                break;
            default:

                break;
        }

    }

    private void clearEvent(){
        String content = displayText.getText().toString();
        if(content.length() > 0){
            displayText.setText(content.substring(0, content.length()-1));
        }
    }

    private void loginEvent() throws IOException, InterruptedException {
    	mProgressDialog.show();
        requestRegister(LoginNum);
        //}
    }

    private void requestRegister(String login_num) {
    	
    	//mHandler.sendEmptyMessageDelayed(MSG_LOGIN_FINISH, MSG_LOGIN_FINISH_INTERVAL);
    	Message msg = new Message();
    	msg.what = MSG_LOGIN_FINISH;
    	msg.arg1 = 0;
    	mHandler.sendMessageDelayed(msg, MSG_LOGIN_FINISH_INTERVAL);
    	
        //if (checkPhoneNum(LoginNum)) {
    	
    	if(login_num.equalsIgnoreCase("")) {
    		SharedPreferences settings = getSharedPreferences("login_setting", 0);
            int is_login = settings.getInt("is_login", 0);
            String device_id = settings.getString("device_id", "");
            ZXConfig.setDeviceId(device_id);
    	}
    	else {
    		ZXConfig.setDeviceId(login_num);
    	}
    	
        //Intent intent = new Intent(LoginActivity.this,LaunchActivity.class);
        //startActivity(intent);
    	String deviceID = ZXConfig.getDeviceId();
    	if(!TextUtils.isEmpty(deviceID)) {
    		Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
            builder.setType(Msg.MessageType.LOGIN_REQUEST);
            Msg.LoginRequest.Builder request = Msg.LoginRequest.newBuilder();
            request.setDeviceId(login_num);
            builder.setLoginRequest(request);
            NettyUtils.getInstance().sendMsg(builder.build());
            //PromptUtils.showProgressDialog(this, "请求中，请稍候…", true);
            
            //is_logined = 1;
            ZXConfig.setLoginStatus(1);
    	} 
    }

    /**
     * 验证输入手机号码
     */
    private boolean checkPhoneNum(String str) {
        String regex = "^[0,1]+[3,5]+\\d{9}$";
        // String regex = "^\\d{1,18}$";
        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(str);
        if (!m.matches()) {
            PromptUtils.showToast("请输入正确手机号", Toast.LENGTH_SHORT);
            return false;
        }
        return true;
    }


    /**
     * @param editText
     */
    private void hideSoftInputMode(EditText editText) {
        ((InputMethodManager)getSystemService(INPUT_METHOD_SERVICE))
                .hideSoftInputFromWindow(editText.getWindowToken(),
                        InputMethodManager.HIDE_NOT_ALWAYS);
    }

    @Override
    public boolean onLongClick(View v) {
        boolean result = false;
        switch (v.getId()) {
            case R.id.clear_num_imageview:
                displayText.setText("");
                result = true;
                break;
        }
        return result;
    }


    @Override
    public void responseRegister(final Msg.LoginResponse msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //PromptUtils.dismissProgressDialog();
                
                if (msg != null && msg.getResult() == SUCCESS) {
                    //PromptUtils.showToast("登陆成功", Toast.LENGTH_SHORT);
                    ZXConfig.setDeviceId(msg.getDeviceId());
                    ZXConfig.setCarNo(msg.getCarNo());
                    ZXConfig.setCarType(msg.getType());
                    ZXConfig.setRunningStatus(msg.getRunStatus());
                    ZXConfig.setMaterialID(msg.getMaterialId());
                    ZXConfig.setMaterialName(msg.getMaterialName());
                    
                    //save login config
                    SharedPreferences settings = getSharedPreferences("login_setting", 0);
                    SharedPreferences.Editor editor = settings.edit();
                    editor.putInt("is_login", 1);
                    if(!msg.getDeviceId().equalsIgnoreCase("")) {
                    	editor.putString("device_id", msg.getDeviceId());
                    }
                    
                    editor.commit();
                    
                    Loger.print("Save Auto login config:" + String.valueOf(1) + " " + msg.getDeviceId());
             
                    Message msg = new Message();
                	msg.what = MSG_LOGIN_FINISH;
                	msg.arg1 = 1;
                	mHandler.sendMessage(msg);
                    
                    /*Intent intent = new Intent(LoginActivity.this, LaunchActivity.class);
                    startActivity(intent);
                    
                    finish();*/
                    
                } else {
                    PromptUtils.showToast("登陆失败", Toast.LENGTH_SHORT);
                       
                    Message msg = new Message();
                	msg.what = MSG_LOGIN_FINISH;
                	msg.arg1 = 0;
                	mHandler.sendMessage(msg);
                }
            }
        });
    }
}
