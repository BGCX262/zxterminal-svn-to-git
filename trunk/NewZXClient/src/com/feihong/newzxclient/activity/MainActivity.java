package com.feihong.newzxclient.activity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.location.LocationManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.text.format.Time;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.GridLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.bean.Constant.LoadStatus;
import com.feihong.newzxclient.callback.OnResponseMessageListener;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.js.JSDataProvider;
import com.feihong.newzxclient.service.SyncData;
import com.feihong.newzxclient.tcp.NettyUtils;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;
import com.feihong.newzxclient.util.RecordUtils;
import com.feihong.newzxclient.widget.*;
import com.google.protobuf.ByteString;
import zaoxun.Msg;
import zaoxun.Msg.EDipperInfo;
import zaoxun.Msg.Material;
import zaoxun.Msg.EDipperInfo.Builder;
import zaoxun.Msg.*;

import java.io.*;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import org.json.JSONObject;

import com.feihong.newzxclient.activity.LaunchActivity;

import android.provider.Settings;  
import android.provider.Settings.SettingNotFoundException;  
import android.util.Log;
import android.view.View;  
import android.view.Window;  
import android.view.WindowManager; 

import com.androidquery.AQuery;
import com.androidquery.callback.AjaxCallback;
import com.androidquery.callback.AjaxStatus;
import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;

/**
 * 主页面
 * @author song
 * @time 2013.6.20
 */
public class MainActivity extends BaseActivity implements OnClickListener, OnResponseMessageListener {

	private static double OIL_FULL = 200;
	private ImageView welcomeImageView = null;
	
	private FrameLayout mWebviewlayout;
	private DrawLineWebView mWebView;
	
	private MessageDialog mMessageDialog;
	
	private static final int LOAD_URI = 100;
	private static final int QUIT = 99;

    private Msg.GPSData mCurrentGPSData;
    private JSDataProvider mJsDataProvider;
    private QueryProductionDialog mQueryProductionialog;
    private ChooseMaterialDialog mChooseMaterialDialog;
    private LoadRequestDialog mLoadRequestDialog;
    
    private ProductionInfoDialog mProductionInfoDialog;
    private FuelConfirmDialog mFuelConfirmDialog;
    //private String[] mMaterials = new String[20];

    private static final int PERIOD_DUTY = 1;
    private static final int PERIOD_DAY = 2;
    private static final int PERIOD_MONTH = 3;

    private static final String SERVER_ADDRESS = "http://183.224.148.30:8888/";
    
	private ImageView netIbt;//data material
	private ImageView gprsIbt;//wifi material
	private TextView mMaterialTextView;//dust or mine
	private TextView mDate;//date
	private TextView mTime;//time
	private TextView mLimitedSpeed;//speed limited
	private TextView mSpeed;//speed
	private TextView lat_tv,lon_tv;//纬度、经度
	private TextView mOutPut;
	//private OilView mOilStatus;
	private TextView mUnits;
	private ImageButton mRecode;//recode
	private ImageView mPlay;//play sound
	//private TextView mDistance;//distance
	private ImageView mQuit;//quit
	
	private ImageButton mLoadButton;
	private ImageButton mUnloadButton;
	
	private QuitDialog mQuitDialog;
	private CompleteRejectDialog mCompleteRejectDialog;

    private RecordUtils mRecordUtils;
    private GridLayout mRootGrid;

	private long start = 0L, end = 0L;
    private long mLastRecordTime;
	
	private int mi = 0;
	
	private  static final int TIME_RECEIVER = 1;
	private  static final int NET_RECEIVER = 2;

	private static final int NETWORK_WIFI = 4;
	private static final int NETWORK_MOBILE = 5;
	private static final int NETWORK_BOTH = 6;
	private static final int NETWORK_NONE = 7;

    private static final int MSG_SUCCESS = 0;
    private static final int MSG_FAILURE = 1;

    private ServerConnectionReceiver mSCReceiver;
    
    private Timer mBrightTimer = null;
    private int mCurrentBright;
    private static final int BRIGHT_DAY = 255;
    private static final int BRIGHT_NIGHT = 0;
    
    private ArrayList<EDipperInfo> listEDippers = new ArrayList<EDipperInfo>();
    private ArrayList<LoadRequest> listRequest = new ArrayList<LoadRequest>();

    private boolean soundEnabled = true;
    
    public static MainActivity mActivity = null;
    
    private String mLogFileName;
    
    private static final int REQUEST_TIMEOUT_INTERVAL = 10 * 1000;

    private static final int MSG_REQUEST_OIL = 0x001;
    private static final int MSG_LOAD_REQUEST = 0x002;
    private static final int MSG_UNLOAD_REQUEST = 0x003;
    private static final int MSG_LOAD_CANCEL = 0x004;
    private static final int MSG_GET_EDIPPERS = 0x005;
    private static final int MSG_LOAD_REJECT = 0x006;
    private static final int MSG_LOAD_COMPLETE = 0x007;
    private static final int MSG_MATERIAL_REQUEST = 0x008;
    private static final int MSG_PRODUCTION_REQUEST = 0x009;
    private static final int MSG_CHECK_BRIGHT = 0x00A;
    
    private static final int MSG_RECORD_TIMEOUT = 0x00B;
    private static final int MSG_RECORD_STOP = 0x00C;
        
    /////////////////////////////
    ArrayList<String>  mAudioQuque;
    ScheduledExecutorService mScheduler = Executors.newScheduledThreadPool(3);
    @SuppressWarnings("rawtypes")
	ScheduledFuture mSendCommandDataScheduleHandler;
	@SuppressWarnings("rawtypes")
	ScheduledFuture mPlayAudioScheduleHandler;
	
	Handler mSendCommandDataHandler = new Handler();
	SendCommandDataTask mSendCommandDataTask;
	Handler mPlayAudioDataHandler = new Handler();
	PlayAudioDataTask mPlayAudioDataTask;
	
	AQuery 	mAqueryCommand  = null;
	AQuery 	mAqueryAudio   	= null;
	AQuery 	mAqueryUploadAudio  = null;
	
	boolean 	isPlay; 
	boolean 	isSendCommand;
	
	MediaPlayer 	mMediaPlayer;
	
	///////Control Utils
	@SuppressLint("NewApi")
    // 用来判断gps是否打开
    private boolean isGPSOpen() {
        boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled(
                getContentResolver(), LocationManager.GPS_PROVIDER);
        if (gpsEnabled) {
            return true;
        } else {
            return false;
        }
    }
	 
	private void toggleGPS() {

		Intent gpsIntent = new Intent();
	    gpsIntent.setClassName("com.android.settings",
	        		"com.android.settings.widget.SettingsAppWidgetProvider");
	        		gpsIntent.addCategory("android.intent.category.ALTERNATIVE");
	        		gpsIntent.setData(Uri.parse("custom:3"));
	    try {
	        PendingIntent.getBroadcast(this, 0, gpsIntent, 0).send();
	    } catch (CanceledException e) {
	        e.printStackTrace();
	    }
	}
	
	private void openGPSSettings() {       
        //获取GPS现在的状态（打开或是关闭状态）
	      boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled( getContentResolver(), LocationManager.GPS_PROVIDER );
	      if(gpsEnabled)
	      {
	
	      //关闭GPS
	       Settings.Secure.setLocationProviderEnabled( getContentResolver(), LocationManager.GPS_PROVIDER, false );
	      }
	      else
	      {
	       //打开GPS 
	       Settings.Secure.setLocationProviderEnabled( getContentResolver(), LocationManager.GPS_PROVIDER, true);
	      }
   }
	
	private void ToggleGPS(boolean flag) {       
		Settings.Secure.setLocationProviderEnabled( getContentResolver(), LocationManager.GPS_PROVIDER, flag );
   }
	
	private void method3() {
		//3.第三种方法（手动设置）

		LocationManager alm = (LocationManager)this.getSystemService(Context.LOCATION_SERVICE);

		if (alm.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER))

		{

		Toast.makeText(this, "GPS模块正常", Toast.LENGTH_SHORT).show();

		}

		Toast.makeText(this, "请开启GPS！", Toast.LENGTH_SHORT).show();

		Intent intent = new Intent(Settings.ACTION_SECURITY_SETTINGS);

		startActivityForResult(intent,0); //此为设置完成后返回到获取界面
	}
	
	/**   
	 * 移动网络开关   
	 */   
	private void toggleMobileData(Context context, boolean enabled) {    
	    ConnectivityManager conMgr = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);    
	    Class<?> conMgrClass = null; // ConnectivityManager类    
	    Field iConMgrField = null; // ConnectivityManager类中的字段    
	    Object iConMgr = null; // IConnectivityManager类的引用    
	    Class<?> iConMgrClass = null; // IConnectivityManager类    
	    Method setMobileDataEnabledMethod = null; // setMobileDataEnabled方法    
	    try {     
	        // 取得ConnectivityManager类     
	    conMgrClass = Class.forName(conMgr.getClass().getName());     
	    // 取得ConnectivityManager类中的对象mService     
	    iConMgrField = conMgrClass.getDeclaredField("mService");     
	    // 设置mService可访问    
	        iConMgrField.setAccessible(true);     
	    // 取得mService的实例化类IConnectivityManager     
	    iConMgr = iConMgrField.get(conMgr);     
	    // 取得IConnectivityManager类     
	    iConMgrClass = Class.forName(iConMgr.getClass().getName());     
	    // 取得IConnectivityManager类中的setMobileDataEnabled(boolean)方法     
	    setMobileDataEnabledMethod = iConMgrClass.getDeclaredMethod("setMobileDataEnabled", Boolean.TYPE);     
	    // 设置setMobileDataEnabled方法可访问     
	    setMobileDataEnabledMethod.setAccessible(true);     
	    // 调用setMobileDataEnabled方法     
	    setMobileDataEnabledMethod.invoke(iConMgr, enabled);    
	    } catch (ClassNotFoundException e) {     
	        e.printStackTrace();    
	    } catch (NoSuchFieldException e) {     
	        e.printStackTrace();    
	    } catch (SecurityException e) {     
	        e.printStackTrace();    
	    } catch (NoSuchMethodException e) {     
	        e.printStackTrace();    
	    } catch (IllegalArgumentException e) {     
	        e.printStackTrace();    
	    } catch (IllegalAccessException e) {     
	        e.printStackTrace();    
	    } catch (InvocationTargetException e) {     
	        e.printStackTrace();    
	    }   
	}  
	
	/**  
	* 设置数据使用状态 需要 签名 系统 可以根据 变化  
	* @param true 为可用 false为 不可用  
	* @return 0为 成功 -1为失败  
	*/   
	private int setMobileDataEnabled(Context context, boolean flag) {   
		ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);   
		Method setMobileDataEnabl;   
		try {   
			setMobileDataEnabl = cm.getClass().getDeclaredMethod(   
			"setMobileDataEnabled", boolean.class);   
			setMobileDataEnabl.invoke(cm, flag);   
			return 0;   
		} catch (Exception e) {   
			e.printStackTrace();   
			return -1;   
		}   
	} 
	
	/** 
     * WIFI网络开关  */  
    private void toggleWiFi(Context context, boolean enabled) {  
        WifiManager wm = (WifiManager) context  
                .getSystemService(Context.WIFI_SERVICE);  
        wm.setWifiEnabled(enabled);  
    }  

    private void SetDriverName(int id, String driverName) {
    	ZXConfig.setDriverID(id);
    	ZXConfig.setDriverName(driverName);
    	
    	mUnits.setText(ZXConfig.getDriverName());
    }
    
    private void SyncDateTime(String dateTime) {
    	Loger.print("SyncDateTime()-date:" + dateTime);
        ArrayList<String> envlist = new ArrayList<String>();
        Map<String, String> env = System.getenv();
        for (String envName : env.keySet()) {
            envlist.add(envName + "=" + env.get(envName));
        }
        String[] envp = (String[]) envlist.toArray(new String[0]);
        String command;
        command = "date -s " + dateTime;
        try {
            Runtime.getRuntime().exec(new String[] { "su", "-c", command }, envp);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Loger.print("SyncDateTime() -date error:" + e.getMessage());
        }
    }

	class SendCommandDataTask implements Runnable{
		public void run(){
			sendCommandData();
		}
	}
	
	class PlayAudioDataTask implements Runnable{
		public void run(){
			
			//Loger.print("PlayAudioDataTask::run()"); 

			if(isPlay == true || mAudioQuque.size() <= 0) 
				return;

			int lastIndex = mAudioQuque.size() - 1;
			String last = mAudioQuque.get(lastIndex);
			mAudioQuque.remove(lastIndex);

			mMediaPlayer = MediaPlayer.create(MainActivity.this, Uri.parse(last));
			if(mMediaPlayer == null) {
				Loger.print("Create Media Player error, null value return.");
				mAudioQuque.add(last);
				return;
			}
			try {
				isPlay = true;
				mMediaPlayer.setOnCompletionListener(new OnCompletionListener() {        
			        
		            @Override        
		            public void onCompletion(MediaPlayer player) {  
		            	
		            	isPlay = false;
		            	mMediaPlayer.release();
						Loger.print("Release Media Player success.");
		            }        
		        });
				mMediaPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {        
			        
		            @Override        
		            public boolean onError(MediaPlayer mp, int what, int extra) {  
		            	
		            	isPlay = false;
		            	mMediaPlayer.release();
						Loger.print("Release play Media Player error, onError");
		            	return false;
		            }        
		        });
				
				mMediaPlayer.start();
			}
			catch (IllegalStateException e) {
				Loger.print("Start Media Player error, IllegalStateException: " + e.getMessage());
				mAudioQuque.add(last);
			}
		}
	}
	
	void sendCommandData()	{
		if(isSendCommand == true) return;
		
		String url = SERVER_ADDRESS + "audio/fetch";
		Map<String, Object> params = new HashMap<String, Object>();
	    params.put("from", "web");
	    params.put("to", ZXConfig.getDeviceId());

	    isSendCommand = true;

	    mAqueryCommand.ajax(url, params, JSONObject.class, new AjaxCallback<JSONObject>() {
	        public void callback(String url, JSONObject json, AjaxStatus status) {
	    	    isSendCommand = false;
	    	    
	        	if(json == null){ 
	        		return;
	        	}
	        	
	        	SyncData response = null;
				try {
					response = SyncData.parseJsonObject(json.toString());
					if(response.status == null 
							|| response.status.length() <= 0 
							|| response.status.contentEquals("error") == true )
					{
						Loger.print("sendCommandData(): error happen"); 
						return;
					}
					else if(response.status.contentEquals("none") == true) {
						Loger.print("sendCommandData(): none audio returned"); 
						return;
					}
					
					int j = response.audios.size();  
					for(int i=0; i<response.audios.size(); i++) {
						String name = response.audios.get(i).path;
						//writeLog("Receive audio:" + name);
						if(!mAudioQuque.contains(name)) {
							mAudioQuque.add(name);
						}			
					}
					Loger.print("sendCommandData(): add audio ok"); 
					
				} catch (JsonSyntaxException e) {
					e.printStackTrace();
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
				}
	        }
	    });
	}
	
	/////////////////////////////////////////////
    
    private static final String TAG = "Brightness";
    /** 可调节的最小亮度值 */  
    public static final int MIN_BRIGHTNESS = 0;  
    /** 可调节的最大亮度值 */  
    public static final int MAX_BRIGHTNESS = 255;  
    
    /** 
     * 获得当前系统的亮度模式 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 为自动调节屏幕亮度 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 为手动调节屏幕亮度 
     */  
    public int getBrightnessMode() {  
        int brightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;  
        try {  
            brightnessMode = Settings.System.getInt(this.getContentResolver(),  
                Settings.System.SCREEN_BRIGHTNESS_MODE);  
        } catch (Exception e) {  
        	Loger.print("获得当前屏幕的亮度模式失败：" + e.getMessage());  
        }  
        return brightnessMode;  
    }  
  
    /** 
     * 设置当前系统的亮度模式 
     * SCREEN_BRIGHTNESS_MODE_AUTOMATIC=1 为自动调节屏幕亮度 
     * SCREEN_BRIGHTNESS_MODE_MANUAL=0 为手动调节屏幕亮度 
     */  
    public void setBrightnessMode(int brightnessMode) {  
        try {  
            Settings.System.putInt(this.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, brightnessMode);  
        } catch (Exception e) {  
        	Loger.print("设置当前屏幕的亮度模式失败：" + e.getMessage());  
        }  
    }  
  
    /** 
     * 获得当前系统的亮度值： 0~255 
     */  
    public int getSysScreenBrightness() {  
        int screenBrightness = MAX_BRIGHTNESS;  
        try {  
            screenBrightness = Settings.System.getInt(this.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS);  
        } catch (Exception e) {  
        	Loger.print("获得当前系统的亮度值失败："+ e.getMessage());  
        }  
        return screenBrightness;  
    }  
  
    /** 
     * 设置当前系统的亮度值:0~255 
     */  
    public void setSysScreenBrightness(int brightness) {  
        try {  
            ContentResolver resolver = this.getContentResolver();  
            Uri uri = Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS);  
            Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS, brightness);  
            resolver.notifyChange(uri, null); // 实时通知改变  
        } catch (Exception e) {  
            Loger.print("设置当前系统的亮度值失败：" + e.getMessage());  
        }  
    }  
  
    /** 
     * 设置屏幕亮度，这会反映到真实屏幕上 
     *  
     * @param activity 
     * @param brightness 
     */  
    public void setActScreenBrightness(final Activity activity, final int brightness) {  
        final WindowManager.LayoutParams lp = activity.getWindow().getAttributes();  
        lp.screenBrightness = brightness / (float) MAX_BRIGHTNESS;  
        activity.getWindow().setAttributes(lp);  
        
        ContentResolver resolver = this.getContentResolver();  
        Uri uri = android.provider.Settings.System
				.getUriFor("screen_brightness");
		android.provider.Settings.System.putInt(resolver, "screen_brightness",
				brightness);
		resolver.notifyChange(uri, null);
    }  
      
    private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
		
			final int arg = msg.arg1;
			
			switch (msg.what) {
                case MSG_REQUEST_OIL:
                    requestOil();
                    mHandler.removeMessages(MSG_REQUEST_OIL);
                    
                    break;
                
                case MSG_LOAD_REQUEST:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_LOAD_REQUEST);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                
                case MSG_UNLOAD_REQUEST:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_UNLOAD_REQUEST);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                    
                case MSG_GET_EDIPPERS:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_GET_EDIPPERS);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                    
                case MSG_LOAD_CANCEL:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_LOAD_CANCEL);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                    
                case MSG_LOAD_REJECT:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_LOAD_REJECT);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                
                case MSG_LOAD_COMPLETE:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_LOAD_COMPLETE);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                    
                case MSG_MATERIAL_REQUEST:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_MATERIAL_REQUEST);

                	if (mChooseMaterialDialog != null && mChooseMaterialDialog.isShowing()) {
                        mChooseMaterialDialog.dismiss();
                        mChooseMaterialDialog = null;
                    }
                	
                    mChooseMaterialDialog = new ChooseMaterialDialog(MainActivity.this, new ChooseMaterialDialog.OnMaterialItemSelectedListener() {
                        @Override
                        public void onMaterialItemSelected(Material material) {
                            mMaterialTextView.setText(material.getChineseName());
                            ZXConfig.setMaterialID(material.getMaterialId());
                            ZXConfig.setMaterialName(material.getChineseName());
                        }
                    });
                    
                    mChooseMaterialDialog.setDataSet(ZXConfig.getMaterialList());
                    mChooseMaterialDialog.setSelectItem(ZXConfig.getMaterialName());
                    mChooseMaterialDialog.show();
                    
                    if(arg == 1)	{
                    	//soundsUtils.playSound(soundsUtils.MATERIALUPDATE);
                    }
                    else {
                    	NettyUtils.getInstance().close();
                    }
                    
                    soundsUtils.playSound(soundsUtils.MATERIAL);
                      
                    break;
                    
                case MSG_PRODUCTION_REQUEST:
                	PromptUtils.dismissProgressDialog();
                    mHandler.removeMessages(MSG_PRODUCTION_REQUEST);
                    
                    if(arg == 0) {
                    	NettyUtils.getInstance().close();
                    }
                    
                    break;
                    
                case MSG_CHECK_BRIGHT:
                	Loger.print("MSG_CHECK_BRIGHT, System brightness: " + String.valueOf(mActivity.getSysScreenBrightness()));
                	
                	if (mCurrentBright == BRIGHT_NIGHT) {
                		mActivity.setActScreenBrightness(mActivity, BRIGHT_DAY);
                    	mCurrentBright = BRIGHT_DAY;
                	}
                	else {
                		mActivity.setActScreenBrightness(mActivity, BRIGHT_NIGHT);
                    	mCurrentBright = BRIGHT_NIGHT;
                	}
                	
                	Loger.print("MSG_CHECK_BRIGHT, Brightness mode: " + String.valueOf(mActivity.getBrightnessMode()));
                	Loger.print("MSG_CHECK_BRIGHT, System brightness: " + String.valueOf(mActivity.getSysScreenBrightness()));
                	
                    mHandler.removeMessages(MSG_CHECK_BRIGHT);
                    break;
                    
                case MSG_RECORD_TIMEOUT:
                	mRecode.setImageResource(R.drawable.chatoff);

                    long s = System.currentTimeMillis() - mLastRecordTime;
                    if(s < 1000) {

                        PromptUtils.showToast(R.string.try_again, Toast.LENGTH_LONG); //语音指令太短
                        if(TextUtils.isEmpty(mRecordUtils.stopRecorder())) {
                            mRecordUtils.deleteEncodeFile();
                        }
                    }

                    //end
                    mRecordUtils.playBeep();
                    String recordPath = mRecordUtils.stopRecorder();
                    if(!TextUtils.isEmpty(recordPath)) {
                        new UploadRecordTask().execute(recordPath);
                    }
                    
                    mHandler.removeMessages(MSG_RECORD_STOP); 
                    
                	break;
                  
                case MSG_RECORD_STOP:
                	mHandler.removeMessages(MSG_RECORD_TIMEOUT);
                	break;
			default:
				break;
			}
			super.handleMessage(msg);
		}

	};

    private void requestOil() {
        Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.OIL_QUANTITY_REQUEST);
        Msg.OilQuantityRequest.Builder request = Msg.OilQuantityRequest.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        builder.setOilQuantityRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    private void CheckBright() {
    	mHandler.sendEmptyMessage(MSG_CHECK_BRIGHT);
    }

    private void InitBrightTimer() {
    	//create timer
        mBrightTimer = new Timer(true);
        
        int mode = getBrightnessMode();
        setBrightnessMode(0);
        int value = getSysScreenBrightness();
        
        Calendar calEnviron = Calendar.getInstance();
        int hour = calEnviron.get(Calendar.HOUR_OF_DAY);
        if(hour < 6 || hour >= 18) {
        	setActScreenBrightness(this, BRIGHT_NIGHT);
        	mCurrentBright = BRIGHT_NIGHT;
        	
        	// 每天的06:00.am开始执行
            calEnviron.set(Calendar.HOUR_OF_DAY, 6);
        }
        else {
        	setActScreenBrightness(this, BRIGHT_DAY);
        	mCurrentBright = BRIGHT_DAY;
        	
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
    
    public void EnableUI(boolean flag) {
    	CarType type = ZXConfig.getCarType();
 		if (type == CarType.MINE_CAR) {
 			mLoadButton.setEnabled(flag);
 	    	mUnloadButton.setEnabled(flag);
 		}

 		mMaterialTextView.setEnabled(flag);
    	mRecode.setEnabled(flag);
    	mRootGrid.setEnabled(flag);
    }
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		//String temp = "{"audios":[{"path":"http://183.224.148.30:8888/media/audio/2014/07/05/1404567059-53b7fe1336eaa.mp3","to":"13700000001","audio_id":"1","from":"web"}],"status":"ok"}";

		//Gson gson = new Gson();
		//SyncData data = gson.fromJson(tmp, SyncData.class);
		
		//init log
        mLogFileName = ZXConfig.getLogPath("mainact");
/*        File file = new File(mLogFileName);
        if(!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }*/
        
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.activity_main);
		mMessageDialog = new MessageDialog(this);
        mJsDataProvider = new JSDataProvider();
		initView();
		initData();
        mRecordUtils = new RecordUtils(this, false);

        mSCReceiver = new ServerConnectionReceiver();
        //startService(new Intent(this, TimeService.class));
        registerReceiver(mSCReceiver, new IntentFilter("android.com.feihong.broadcastreceiver.action.SERVERCONNECTION"));
        
        CarType type = ZXConfig.getCarType();
		if (type == CarType.E_DIPPER) {
			new Thread(new Runnable() {
	            @Override
	            public void run() {
	                try {
						Thread.sleep(1000);
						
						RequestMaterials();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
	                
	            }
	        }).start();
		} 
		
		mAudioQuque = new ArrayList<String>();
		mAqueryCommand   	= new AQuery(getApplicationContext());
		mAqueryAudio   		= new AQuery(getApplicationContext());
		mAqueryUploadAudio   		= new AQuery(getApplicationContext());
		
		//1.
		mSendCommandDataTask= new SendCommandDataTask();
		mPlayAudioDataTask	= new PlayAudioDataTask();

		mSendCommandDataScheduleHandler = mScheduler.scheduleAtFixedRate(new Runnable(){
			public void run(){
				mSendCommandDataHandler.post(mSendCommandDataTask);
			}
		}, 1, 5, TimeUnit.SECONDS); //查询语音
		
		mPlayAudioScheduleHandler = mScheduler.scheduleAtFixedRate(new Runnable(){
			public void run(){
				mPlayAudioDataHandler.post(mPlayAudioDataTask);
			}
		}, 5, 5, TimeUnit.SECONDS); // 播放语音
		
		mActivity = this;
		//InitBrightTimer();

		soundsUtils.playSound(soundsUtils.WELCOME);
    }

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
        regeditReceiver(NET_RECEIVER);
		regeditReceiver(TIME_RECEIVER);
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		unregisterReceiver(mTimeReceiver);
        unregisterReceiver(mConnetcReceiver);
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
        mRecordUtils.release();
        
        mWebviewlayout.removeView(mWebView);
        mWebView.removeAllViews();
        mWebView.destroy();

        unregisterReceiver(mSCReceiver);
        
        if(mAqueryCommand != null)
			mAqueryCommand.ajaxCancel();
		if(mAqueryAudio != null)
			mAqueryAudio.ajaxCancel();
		if(mAqueryUploadAudio != null)
			mAqueryUploadAudio.ajaxCancel();
    }

	private void initData() {
		//showLimitedSpeed(80);
		showSpeed(0f);
		refreshDate();
		refreshNet();
		mRecode.setImageResource(R.drawable.chatoff);
		mPlay.setImageResource(R.drawable.speakeron);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		return true;
	}
	
	public void SetConnectStatus(boolean flag) {
		if(flag) {
			netIbt.setImageResource(R.drawable.serveron);
			
			EnableUI(true);
		}
		else {
			netIbt.setImageResource(R.drawable.serveroff);
			EnableUI(false);
		}		
	}

	public void initView() {
		lat_tv = (TextView) this.findViewById(R.id.lat_tv);
		lon_tv = (TextView) this.findViewById(R.id.lon_tv);
		
		mWebviewlayout = (FrameLayout) findViewById(R.id.webviewFrameLayout);  
		mWebView = (DrawLineWebView) this.findViewById(R.id.webview);
		mMaterialTextView = (TextView)this.findViewById(R.id.tv_material);
        mQuit = (ImageView)findViewById(R.id.quit);
        mRootGrid = (GridLayout)findViewById(R.id.rootGrid);
        mQuitDialog = new QuitDialog(this);
        mCompleteRejectDialog = new CompleteRejectDialog(this);
        mQuit.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				showQuit(getString(R.string.real_quit));
				//soundsUtils.playSound(soundsUtils.QUITSTATUS);
				soundsUtils.playSound(soundsUtils.REBOOT);
			}
		});
        
        mMaterialTextView.setText(ZXConfig.getMaterialName());
		mMaterialTextView.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
            	
            	
            	PromptUtils.showProgressDialog(mActivity, "请求中，请稍候…", false, false);
            	RequestMaterials();
            }
        });
	
		mLimitedSpeed = (TextView)this.findViewById(R.id.limitedspeed);
		mLimitedSpeed.setText(String.valueOf((int)ZXConfig.getSpeedlimit()));
		
		mSpeed = (TextView)this.findViewById(R.id.speed_tv);
		mDate = (TextView)this.findViewById(R.id.date);
        mTime = (TextView)this.findViewById(R.id.time);
        
        netIbt = (ImageView)findViewById(R.id.net);
        
    
		gprsIbt = (ImageView) findViewById(R.id.gprs);
		mUnits = (TextView)findViewById(R.id.units);
		mUnits.setText(ZXConfig.getDriverName());
		
		mOutPut = (TextView)findViewById(R.id.output);
		mOutPut.setText(ZXConfig.getCarNo());
        
        
		mRecode = (ImageButton)findViewById(R.id.recode);
		mPlay = (ImageView)findViewById(R.id.play);
		mRecode.setOnClickListener(this);
		mPlay.setOnClickListener(this);
		//mOilStatus = (OilView)findViewById(R.id.oil_view);
		//mDistance = (TextView)findViewById(R.id.distance);
		//mQuit = (ImageButton)findViewById(R.id.quit);
		//mQuit.setOnTouchListener(this);
		
		CarType type = ZXConfig.getCarType();
		if (type == CarType.MINE_CAR) {
			InitMineCarView();
			
			mMaterialTextView.setEnabled(false);
			
		}
		else {
			InitEdipperView();
		}
		
		mOutPut.setOnClickListener(this);
		
		mProductionInfoDialog = new ProductionInfoDialog(this);
		mFuelConfirmDialog = new FuelConfirmDialog(this);
		
		WebSettings webSettings = mWebView.getSettings();
		webSettings.setSavePassword(false);

		webSettings.setSaveFormData(false);
		webSettings.setJavaScriptEnabled(true);
		webSettings.setSupportZoom(false);
		mWebView.setWebChromeClient(new MyWebChromeClient());

		//mWebView.loadUrl("http://mapv2.zaoxuntec.cn/index.php?a=mobile&id=18270420104");
		mWebView.loadUrl("http://183.224.148.30:8887/index.php?a=mobile&id=18270420104");

        mWebView.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                mWebView.triggerDrawing();
                mWebView.clearCache();
                return false;
            }
        });

        /*
                          绑定Java对象到WebView，这样可以让JS与Java通信(JS访问Java方法)
                          第一个参数是自定义类对象，映射成JS对象
                          第二个参数是第一个参数的JS别名
                          调用示例：
            mWebView.loadUrl("javascript:window.stub.jsMethod('param')");
         */
        mJsDataProvider.SetLatitude(0);
        mJsDataProvider.SetLongtitude(0);
        mJsDataProvider.SetRotation(0);
        mWebView.addJavascriptInterface(mJsDataProvider, "JSDataProvider");
        
        if (ZXConfig.getconnStatus() == 1) {
        	SetConnectStatus(true);
        }
        else {
        	SetConnectStatus(false);
        }
	}
	
	private void RequestMaterials() {
    	Message message = new Message();
    	message.what = MSG_MATERIAL_REQUEST;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
		//send material query
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.MATERIALS_REQUEST);
        Msg.MaterialsRequest.Builder request = Msg.MaterialsRequest.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        builder.setMaterialsRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
	}
	
	public void showQuit(String s){
		if(mQuitDialog.isShowing()){
			mQuitDialog.dismiss();
		}
		mQuitDialog.setText(s);
		mQuitDialog.show();
	}
	
	public void ExitAll(){
		//停止mService 
		//getMainService().stopService(MainActivity.this);
		getMainService().Stop();
		
		Intent intent = new Intent(); 
		intent.setClass(this, LaunchActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);  //注意本行的FLAG设置
		startActivity(intent);
		finish();//关掉自己
		
		ActivityManager.instance().finishActivities();
	}
	
	public void showCompleteReject(String s, String edipperID, String carID, String carNo){
		if(mCompleteRejectDialog.isShowing()){
			mCompleteRejectDialog.dismiss();
		}
		mCompleteRejectDialog.setText("是否已完成装载" + carNo + "操作?");
		mCompleteRejectDialog.setEdipperID(edipperID);
		mCompleteRejectDialog.setCarID(carID);
		
		mCompleteRejectDialog.show();
	}

    @Override
    protected void onMainserviceResumed() {
        super.onMainserviceResumed();
        lat_tv.setText(getMainService().getLatitude() + "");
        lon_tv.setText(getMainService().getLogitude() + "");
    }

    @Override
    public void responseOilQuantity(Msg.OilQuantityResponse msg) {
     if (msg != null) {
         double oilQuantity = msg.getQuantity();
         double point = oilQuantity/OIL_FULL;
         refreshOil(point);
     }
    }

    @Override
    public void responseProduction(Msg.ProductionResponse msg) {
    	if (msg != null && mOutPut != null) {
            mOutPut.setText(msg.getQuantity()+"");
        }
    }

    @Override
    public void responseSpeedLimit(final Msg.SpeedLimitMessage msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (msg != null) {
                    showLimitedSpeed((int)msg.getSpeed());
                }
            }
        });
    }
    
    @Override
    public void responseCommandRequest(final Msg.CommandRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (msg != null) {
            		soundsUtils.playSound(soundsUtils.NOTIFY);
            		
            		Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                    builder.setType(Msg.MessageType.COMMAND_RESPONSE);
                    Msg.CommandResponse.Builder request = Msg.CommandResponse.newBuilder();
                    request.setDeviceId(msg.getDeviceId());
                    request.setResult(0);
                    request.setCmdId(msg.getCmdId());
                    builder.setCommandResponse(request);
                    NettyUtils.getInstance().sendMsg(builder.build());
                    
            		if(mMessageDialog.isShowing()) {
                    	mMessageDialog.dismiss();
                    }
            		
                	String s = msg.getContent();
                	int nType = msg.getType();
                	if(nType == 1 || nType == 2) {
                		mMessageDialog.setMessage(s);
                        mMessageDialog.show();
                	}
                	else {
                		if(s.length() < 3) {
                			return;
                		}
                		
                		String first = s.substring(0, 1);
                		String end = s.substring(s.length() - 1, s.length());
                		if(first.equalsIgnoreCase("#") && end.equalsIgnoreCase("#")) {
                			String cmd = s.substring(1, s.length() - 1);
                			switch(nType) {
                        	case 3:
                        		if(cmd.equalsIgnoreCase("reconnect")) {
                        			NettyUtils.getInstance().close();
                        		}
                        		
                        		break;
                        		
                        	case 4:
								if(cmd.equalsIgnoreCase("light")) {
									mActivity.setActScreenBrightness(mActivity, BRIGHT_DAY);                    			
                        		}
								else if(cmd.equalsIgnoreCase("dark")) {
									mActivity.setActScreenBrightness(mActivity, BRIGHT_NIGHT);
								}
                        		
                        		break;
                         		
                        	case 5:
                        		if(cmd.equalsIgnoreCase("gpson")) {
                        			ToggleGPS(true);
                        		}
                        		else if(cmd.equalsIgnoreCase("gpsoff")) {
                        			ToggleGPS(false);
                        		}

                        		break;
                        	
                        	case 6:
                        		if(cmd.equalsIgnoreCase("mobiledata")) {
                        			toggleMobileData(getApplicationContext(), false);
                        			
                        			try {
										Thread.sleep(30 * 1000);
									} catch (InterruptedException e) {
										// TODO Auto-generated catch block
										e.printStackTrace();
									}
                        			
                        			toggleMobileData(getApplicationContext(), true);
                        		}

                        		break;
                        		
                        	case 7:

                        		if(cmd.equalsIgnoreCase("wifi")) {
                        			toggleWiFi(getApplicationContext(), false);
                        			
                        			try {
										Thread.sleep(30 * 1000);
									} catch (InterruptedException e) {
										// TODO Auto-generated catch block
										e.printStackTrace();
									}
                        			
                        			toggleWiFi(getApplicationContext(), true);
                        		}
                        		
                        		break;
                        	case 8:
                        		int nPos = cmd.indexOf("|");
                        		if(nPos != -1) {
                        			String driverID = cmd.substring(0, nPos);
                        			int nDriverID = Integer.parseInt(driverID);
                            		String driverName = cmd.substring(nPos + 1, cmd.length());
                            		
                            		SetDriverName(nDriverID, driverName);
                        		}

                        		break;
                        		
                        	case 9:
                        		/*
                        		 
                        		设置系统时间：
                        		boolean isSuc = SystemClock.setCurrentTimeMillis(curMs);//需要Root权限
                        		 
                        		设置系统时区：
                        		AlarmManager mAlarmManager = (AlarmManager)getSystemService(Context.ALARM_SERVICE);
                        		mAlarmManager.setTimeZone("GMT+08:00");
                        		
                        		*/
                        		
                        		String dateTime = cmd.substring(0, 8);
                        		dateTime += ".";
                        		dateTime += cmd.substring(8, 6);
                        		SyncDateTime(dateTime);

                        		break;
                        	
                        	case 10:
                        		if(cmd.equalsIgnoreCase("kill")) {
                        			ExitAll();
                        		}
                        		
                        		break;
                        		
                        	default:
                        		break;
                        		
                        	}
                			
                		}
                		
                	}
                }
            }
        });
    }
    
    @Override
    public void responseProductionInfoResponse(final Msg.ProductionInfoResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (msg != null) {
            		ZXConfig.setTimes(msg.getTimes());
            		ZXConfig.setDistance(msg.getDistance());
            		ZXConfig.setTon(msg.getTon());
            		ZXConfig.setFill(msg.getFill());
            		ZXConfig.setTemp(msg.getTemp());
            		
            		mProductionInfoDialog.UpdateData();
    				mProductionInfoDialog.show();

    				//mHandler.sendEmptyMessage(MSG_PRODUCTION_REQUEST);
    				
    				Message message = new Message();
    		    	message.what = MSG_PRODUCTION_REQUEST;
    		    	message.arg1 = 1;
    		    	mHandler.sendMessage(message);
                }
            }
        });
    }

    @Override
    public void responseGPS(final Msg.GPSData msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (msg != null) {
                    mCurrentGPSData = msg;
                    mJsDataProvider.SetLatitude(msg.getLatitude() != null ? Double.valueOf(msg.getLatitude()) : 0);
                    mJsDataProvider.SetLongtitude(msg.getLongitude() != null ? Double.valueOf(msg.getLongitude()) : 0);
                    mJsDataProvider.SetSpeed(msg.getSpeed() != null ? Double.valueOf(msg.getSpeed()) : 0);
                    mJsDataProvider.SetRotation(msg.getDirection() != null ? Double.valueOf(msg.getDirection()) : 0);
                    
                    if (ZXConfig.getCarType() != CarType.E_DIPPER) {
                    	Double dLimitedSpeed = Double.valueOf((String) mLimitedSpeed.getText());
                    	if(dLimitedSpeed != 0 && Double.valueOf(msg.getSpeed()) >= dLimitedSpeed) {
                        	soundsUtils.playSound(soundsUtils.OVERSPEED);
                        }
                    }
                    
                    refreshGPS(msg);
                }
            }
        });
    }

    @Override
    public void responseAudioFile(Msg.AudioFileData msg) {

    }

    @Override
    public void responseShortMessage(Msg.ShortMessage msg) {
        if (msg != null) {
        	if(mMessageDialog.isShowing()) {
            	mMessageDialog.dismiss();
            }
        	
            String s = msg.getContent();
            mMessageDialog.setMessage(s);
            mMessageDialog.show();
        }
    }
    
    @Override
    public void responseDirection(double direction) {
    	mJsDataProvider.SetRotation(direction);
    }
    
    public void RequestLoad(String edipperID) {
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_LOAD_REQUEST, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message message = new Message();
    	message.what = MSG_LOAD_REQUEST;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.LOAD_REQUEST);
        Msg.LoadRequest.Builder request = Msg.LoadRequest.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        request.setCarNo(ZXConfig.getCarNo());
        request.setEdipperDeviceId(edipperID);
        builder.setLoadRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    @Override
    public void responseGetEdippersResponse(final Msg.GetEDippersResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	//mHandler.sendEmptyMessage(MSG_GET_EDIPPERS);
            	
            	Message message = new Message();
            	message.what = MSG_GET_EDIPPERS;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	EDipperInfo[] edippers = new EDipperInfo[2];
            	ArrayList<EDipperInfo> edipperList = new ArrayList<EDipperInfo>();
            	for (int i = 0; i < msg.getEdippersCount(); i ++) {
            		//edippers[i] = msg.getEdippers(i);
            		edipperList.add(msg.getEdippers(i));
            	}
            	
            	if (mLoadRequestDialog != null && mLoadRequestDialog.isShowing()) {
            		mLoadRequestDialog.dismiss();
            		mLoadRequestDialog = null;
                }
            	mLoadRequestDialog = new LoadRequestDialog(MainActivity.this, new LoadRequestDialog.OnEdipperItemSelectedListener() {
                    @Override
                    public void onEdipperItemSelected(EDipperInfo edipper) {
                        //mMaterialTextView.setText(material);
                    	String edipperID = edipper.getDeviceId();
                    	ZXConfig.setEdipperId(edipperID);
                    	RequestLoad(edipperID);
                    	
                    	soundsUtils.playSound(soundsUtils.SUBMIT);
                    }
                });
            	mLoadRequestDialog.setDataSet(edipperList);
                //mChooseMaterialDialog.setSelectItem(mMaterialTextView.getText().toString());
            	mLoadRequestDialog.show();
            	
            	soundsUtils.playSound(soundsUtils.CHOOSELOADER);
            }
        });
    	
    }
    
    @Override
    public void responseLoadRequest(final Msg.LoadRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	soundsUtils.playSound(soundsUtils.NOTIFY);
            	Loger.print("responseLoadRequest");
            	
            	//delete duplicate request
            	int len= listRequest.size();
            	for(int i = 0; i < len; i ++) {
            		Msg.LoadRequest request = listRequest.get(i);
            		String strID = request.getDeviceId();
            		if(msg.getDeviceId().equalsIgnoreCase(strID)) {
            			listRequest.remove(i);
            			--len;
            		    --i;
            		}
            	}
            	
            	listRequest.add(msg);
            	InitEdipperView();
            	
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.LOAD_RESPONSE);
                Msg.LoadResponse.Builder response = Msg.LoadResponse.newBuilder();
                response.setDeviceId(msg.getDeviceId());
                response.setEdipperDeviceId(ZXConfig.getDeviceId());
                response.setMaterialId(ZXConfig.getMaterialID());
                response.setMaterialName(ZXConfig.getMaterialName());
                response.setResult(1);
                builder.setLoadResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    	
    }
    
    @Override
    public void responseLoadResponse(final Msg.LoadResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	Loger.print("responseLoadResponse");
            	//mHandler.sendEmptyMessage(MSG_LOAD_REQUEST);
            	
            	Message message = new Message();
            	message.what = MSG_LOAD_REQUEST;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	mLoadButton.setImageResource(R.drawable.load_waiting);
            	
            	//更新物料显示
            	ZXConfig.setMaterialID(msg.getMaterialId());
            	ZXConfig.setMaterialName(msg.getMaterialName());
            	mMaterialTextView.setText(ZXConfig.getMaterialName());
            	
            	ZXConfig.setLoadStatus(LoadStatus.WAIT);
             }
        });
    }
    
    @Override
    public void responseLoadCompleteRequest(final Msg.LoadCompleteRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	soundsUtils.playSound(soundsUtils.LOADDONE);
            	Loger.print("responseLoadCompleteRequest");
            	
            	mLoadButton.setImageResource(R.drawable.load_btn_selector);
            	ZXConfig.setLoadStatus(LoadStatus.LOADED);       	
            	mLoadButton.setEnabled(false);
            	mUnloadButton.setEnabled(true);
            	
            	//send load complete response
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.LOAD_COMPLETE_RESPONSE);
                Msg.LoadCompleteResponse.Builder response = Msg.LoadCompleteResponse.newBuilder();
                response.setCarDeviceId(msg.getCarDeviceId());
                response.setEdipperDeviceId(msg.getEdipperDeviceId());
                response.setResult(1);
                builder.setLoadCompleteResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    }
    
    @Override
    public void responseLoadCompleteResponse(final Msg.LoadCompleteResponse msg)	{
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {   	
            	Message message = new Message();
            	message.what = MSG_LOAD_COMPLETE;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	Loger.print("responseLoadCompleteResponse");
            	
            	for (int i = 0; i < listRequest.size(); i ++) {
            		String strID = listRequest.get(i).getDeviceId();
            		String strCarID = msg.getCarDeviceId();
            		if (strID.equalsIgnoreCase(strCarID)) {
            			listRequest.remove(i);
            			break;
            		}
                }
                InitEdipperView();
            }
        });
    }
    
    @Override
    public void responseManualLoadCompleteRequest(final Msg.ManualLoadCompleteRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	//soundsUtils.playSound(soundsUtils.LOADDONE);
            	Loger.print("responseManualLoadCompleteRequest");
            	
            	CarType type = ZXConfig.getCarType();
        		if (type == CarType.MINE_CAR) {
        			mLoadButton.setImageResource(R.drawable.load_btn_selector);
                	ZXConfig.setLoadStatus(LoadStatus.LOADED);       	
                	mLoadButton.setEnabled(false);
                	mUnloadButton.setEnabled(true);
        		} 
        		else if(type == CarType.E_DIPPER) {
        			for (int i = 0; i < listRequest.size(); i ++) {
                		String strID = listRequest.get(i).getDeviceId();
                		String strCarID = msg.getCarDeviceId();
                		if (strID.equalsIgnoreCase(strCarID)) {
                			listRequest.remove(i);
                			break;
                		}
                    }
                    InitEdipperView();
        		}
 
            	//send load complete response
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.MANUAL_LOAD_COMPLETE_RESPONSE);
                Msg.ManualLoadCompleteResponse.Builder response = Msg.ManualLoadCompleteResponse.newBuilder();
                response.setCarDeviceId(msg.getCarDeviceId());
                response.setEdipperDeviceId(msg.getEdipperDeviceId());
                response.setResult(1);
                builder.setManualLoadCompleteResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    }
    
    @Override
    public void responseUnloadResponse(Msg.UnloadResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	//mHandler.sendEmptyMessage(MSG_UNLOAD_REQUEST);
            	
            	Message message = new Message();
            	message.what = MSG_UNLOAD_REQUEST;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	ZXConfig.setLoadStatus(LoadStatus.IDLE);       
                mLoadButton.setEnabled(true);
            	mUnloadButton.setEnabled(false);
            }
        });
    }
    
    @Override
    public void responseManualUnloadRequest(final Msg.ManualUnloadRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	
            	ZXConfig.setLoadStatus(LoadStatus.IDLE);       
                mLoadButton.setEnabled(true);
            	mUnloadButton.setEnabled(false);
            	
            	//send load complete response
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.MANUAL_UNLOAD_RESPONSE);
                Msg.ManualUnloadResponse.Builder response = Msg.ManualUnloadResponse.newBuilder();
                response.setDeviceId(msg.getDeviceId());
                response.setEdipperDeviceId(msg.getEdipperDeviceId());
                response.setResult(1);
                builder.setManualUnloadResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    }
    
    @Override
    public void responseMaterialsNotification(final Msg.MaterialsNotification msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	
                //mChooseMaterialDialog.setDataSet(strArr);
            }
        });
    }
    
    public void responseCancelLoadRequest(final Msg.CancelLoadRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.CANCEL_LOAD_RESPONSE);
                Msg.CancelLoadResponse.Builder response = Msg.CancelLoadResponse.newBuilder();
                String carDeviceID = msg.getCarDeviceId();
                String edipperDeviceID = msg.getEdipperDeviceId();
                response.setCarDeviceId(carDeviceID);
                response.setEdipperDeviceId(edipperDeviceID);
                response.setResult(1);
                builder.setCancelLoadResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
                
                int len= listRequest.size();
                for(int i = 0; i < len; i ++) {
                	String id = listRequest.get(i).getDeviceId();
                	if(id.equalsIgnoreCase(carDeviceID) ) {
                		listRequest.remove(i);
                		--len;
            		    --i;
                	}
                }
                
                InitEdipperView();
            }
        });
    }
    
    public void responseCancelLoadResponse(final Msg.CancelLoadResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {            	
            	//mHandler.sendEmptyMessage(MSG_LOAD_CANCEL);
            	
            	Message message = new Message();
            	message.what = MSG_LOAD_CANCEL;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	if (msg.getResult() == 1) {
            		mLoadButton.setImageResource(R.drawable.load_btn_selector);
            	}
            	
            	ZXConfig.setLoadStatus(LoadStatus.IDLE);
            	mLoadButton.setEnabled(true);
            	mUnloadButton.setEnabled(false);
            }
        });
    }
    
    public void responseRejectCancelRequest(final Msg.RejectLoadRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	soundsUtils.playSound(soundsUtils.CANCELLOAD);
            	
            	
            	ZXConfig.setLoadStatus(LoadStatus.IDLE);
            	mLoadButton.setEnabled(true);
            	mUnloadButton.setEnabled(false);

            	mLoadButton.setImageResource(R.drawable.load_btn_selector);
            	
            	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.REJECT_LOAD_RESPONSE);
                Msg.RejectLoadResponse.Builder response = Msg.RejectLoadResponse.newBuilder();
                response.setEdipperDeviceId(msg.getEdipperDeviceId());
                response.setCarDeviceId(msg.getCarDeviceId());
                response.setResult(0);
                builder.setRejectLoadResponse(response);
                NettyUtils.getInstance().sendMsg(builder.build());
            }
        });
    }
    public void responseRejectCancelResponse(final Msg.RejectLoadResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	//mHandler.sendEmptyMessage(MSG_LOAD_REJECT);
            	
            	Message message = new Message();
            	message.what = MSG_LOAD_REJECT;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            	
            	for (int i = 0; i < listRequest.size(); i ++) {
            		String strID = listRequest.get(i).getDeviceId();
            		String strCarID = msg.getCarDeviceId();
            		if (strID.equalsIgnoreCase(strCarID)) {
            			listRequest.remove(i);
            			break;
            		}
            	}
            	
            	InitEdipperView();
            	
            	ZXConfig.setLoadStatus(LoadStatus.IDLE);
            } 
        });
    }
    
    public void responseDeviceStatusRequest(final Msg.DeviceStatusRequest msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	RunningStatus status = (RunningStatus)msg.getStatus();
            	ZXConfig.setRunningStatus(status);
            	
            	SetConnectStatus(true);
            	
            	String strError = String.format("responseDeviceStatusRequest(), RunningStatus: %s", ZXConfig.getRunningStatus().toString());
            	//writeLog(strError);
            	
            	if(status == RunningStatus.CLOSED) {
            		//Intent intent = new Intent(this, LaunchActivity.class);
                    //startActivity(intent);                 
                    finish();
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
    
    public void responseAntiCollisionNotification(final Msg.AntiCollisionNotification msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	String sWarning = "";
            	if (msg.getAngleCount() == 0) {
            		sWarning = "[]";
            	}
            	else {
            		for(int i = 0; i < msg.getAngleCount(); i ++) {
                		if (i == 0) {
                			sWarning += "[";
                		}
                		
                		sWarning += String.valueOf(msg.getAngle(i));
                		
                		if (i != msg.getAngleCount() - 1) {
                			sWarning += ",";
                		}
                		
                		if (i == msg.getAngleCount() - 1) {
                			sWarning += "]";
                		}
                	}
            	}
            	
            	mJsDataProvider.SetWarning(sWarning);
            } 
        });
    }
    
    public void responseMaterialsResponse(final Msg.MaterialsResponse msg) {
    	runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	Message message = new Message();
            	message.what = MSG_MATERIAL_REQUEST;
            	message.arg1 = 1;
            	mHandler.sendMessage(message);
            }
        }); 
    }
    
    private void CancelLoadRequest() {
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_LOAD_CANCEL, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message message = new Message();
    	message.what = MSG_LOAD_CANCEL;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.CANCEL_LOAD_REQUEST);
        Msg.CancelLoadRequest.Builder request = Msg.CancelLoadRequest.newBuilder();
        request.setCarDeviceId(ZXConfig.getDeviceId());
        request.setEdipperDeviceId(ZXConfig.getEdipperId());
        builder.setCancelLoadRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    private void GetNearEdippers(String deviceID) {
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_GET_EDIPPERS, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message message = new Message();
    	message.what = MSG_GET_EDIPPERS;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.GET_EDIPPERS_REQUEST);
        Msg.GetEDippersRequest.Builder request = Msg.GetEDippersRequest.newBuilder();
        request.setDeviceId(deviceID);
        builder.setGetEdippersRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    public void InitMineCarView() {
    	mRootGrid.removeAllViews();
 
    	mLoadButton = new ImageButton(this);  
    	mLoadButton.setImageResource(R.drawable.load_btn_selector);
    	mLoadButton.setBackgroundColor(0);
    	mLoadButton.setOnClickListener(new OnClickListener() {
     	   @Override
     	   public void onClick(View arg0) {
     		   LoadStatus status = ZXConfig.getLoadStatus();
     		   if (status == LoadStatus.WAIT) {
     			   // cancel load request
     			  soundsUtils.playSound(soundsUtils.CANCELLOAD);
     			  
     			  CancelLoadRequest();
     		   }
     		   else if (status == LoadStatus.IDLE){
     			  GetNearEdippers(ZXConfig.getDeviceId());
     		   }
     		  else if (status == LoadStatus.LOADED){
     			  return;
     		   }
     	   }
     	  });
    	
    	GridLayout.Spec rowSpec = GridLayout.spec(0);     
    	GridLayout.Spec columnSpec = GridLayout.spec(0);  
    	GridLayout.LayoutParams params = new GridLayout.LayoutParams(rowSpec, columnSpec);  
        params.setGravity(Gravity.LEFT);  
        mRootGrid.addView(mLoadButton, params);  
        
        mUnloadButton = new ImageButton(this);  
        mUnloadButton.setImageResource(R.drawable.unload_btn_selector);
        mUnloadButton.setBackgroundColor(0);
        mUnloadButton.setOnClickListener(new OnClickListener() {   
     	   @Override
     	   public void onClick(View arg0) {
     		  LoadStatus status = ZXConfig.getLoadStatus();
    		   if (status == LoadStatus.IDLE || status == LoadStatus.WAIT) {
    			   return;
    		   }
    		   
    		   if (status == LoadStatus.LOADED) {
    			   soundsUtils.playSound(soundsUtils.UNLOAD);
    	     	   NotifyUnload(ZXConfig.getDeviceId()); 
    		   }
     	   }
     	  });
        
        rowSpec = GridLayout.spec(1);     
        columnSpec = GridLayout.spec(0);  
        params = new GridLayout.LayoutParams(rowSpec, columnSpec);  
        params.setGravity(Gravity.LEFT);  
        mRootGrid.addView(mUnloadButton, params);  
        
        LoadStatus status = ZXConfig.getLoadStatus();
        if (status == LoadStatus.IDLE) {
        	mLoadButton.setEnabled(true);
        	mUnloadButton.setEnabled(false);
        }
        else if (status == LoadStatus.WAIT) {
        	mLoadButton.setEnabled(true);
        	mUnloadButton.setEnabled(false);
        }
        else {
        	mLoadButton.setEnabled(false);
        	mUnloadButton.setEnabled(true);
        }
    }
    
    public void InitEdipperView() {
    	mRootGrid.removeAllViews();
    
    	if (listRequest.size() == 0) {
    		return;
    	}
    	
        int idx = 0;  
        for(int i = 0; i < 2; i ++)  {
        	if (idx >= listRequest.size()) {
    			break;
    		}
        	
        	for(int j = 0; j < 3; j ++) {  
        		if (idx >= listRequest.size()) {
        			break;
        		}
        		LoadRequest request = listRequest.get(idx);
 
                Button btn = new Button(this, null);  
                //btn.setWidth(120);  
                //btn.setHeight(120);
                //btn.setImageResource(R.drawable.mine_car_normal);
                btn.setTag(request); 
                btn.setText(request.getCarNo());
                btn.setTextColor(Color.WHITE);
                btn.setBackgroundColor(0);
                btn.setBackgroundResource(R.drawable.mine_car_normal);
                btn.setTextSize(45);
                
                btn.setOnClickListener(new OnClickListener() {
                	   
                	   @Override
                	   public void onClick(View arg0) {
                		Button button = (Button)arg0;
                		LoadRequest request = (LoadRequest)button.getTag();
                		String carID = request.getDeviceId();
                		String carNo = request.getCarNo();
                		//String carID = GetCarIDFromCarNo(carNo);
                		//NotifyLoadComplete(carID); 
                		String edipperID = ZXConfig.getDeviceId();
                		
                		//getString(R.string.real_quit)
                		showCompleteReject("", edipperID, carID, carNo);
                		
                		
                	   }
                	  });
                
                idx ++;  
                GridLayout.Spec rowSpec = GridLayout.spec(j);     //设置它的行和列  
                GridLayout.Spec columnSpec = GridLayout.spec(i);  
                GridLayout.LayoutParams params=new GridLayout.LayoutParams(rowSpec,columnSpec);  
                params.setGravity(Gravity.LEFT);  
                mRootGrid.addView(btn,params);  
            } 
        }
    }
    
    public void NotifyLoadComplete(String carID) {
    	
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_LOAD_COMPLETE, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message msg = new Message();
    	msg.what = MSG_LOAD_COMPLETE;
    	msg.arg1 = 0;
    	mHandler.sendMessageDelayed(msg, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.LOAD_COMPLETE_REQUEST);
        Msg.LoadCompleteRequest.Builder request = Msg.LoadCompleteRequest.newBuilder();
        request.setEdipperDeviceId(ZXConfig.getDeviceId());
        request.setCarDeviceId(carID);
        builder.setLoadCompleteRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    public void RejectLoad(String carID) {
    	//soundsUtils.playSound(soundsUtils.);
    	
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_LOAD_REJECT, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message message = new Message();
    	message.what = MSG_LOAD_REJECT;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.REJECT_LOAD_REQUEST);
        Msg.RejectLoadRequest.Builder request = Msg.RejectLoadRequest.newBuilder();
        request.setEdipperDeviceId(ZXConfig.getDeviceId());
        request.setCarDeviceId(carID);
        builder.setRejectLoadRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    public void NotifyNormalExit() {
    	soundsUtils.playSound(soundsUtils.YES);
    	
    	ZXConfig.setRunningStatus(RunningStatus.CLOSED);
    	
    	String strError = String.format("NotifyNormalExit(), RunningStatus: %s", ZXConfig.getRunningStatus().toString());
    	//writeLog(strError);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.EXIT_NOTIFICATION);
        Msg.ExitNotification.Builder request = Msg.ExitNotification.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        request.setExitType(1);
        builder.setExitNotification(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    public void NotifyAbnormalExit() {
    	soundsUtils.playSound(soundsUtils.NO);
    	
    	ZXConfig.setRunningStatus(RunningStatus.CLOSED);
    	
    	String strError = String.format("NotifyAbnormalExit(), RunningStatus: %s", ZXConfig.getRunningStatus().toString());
    	//writeLog(strError);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.EXIT_NOTIFICATION);
        Msg.ExitNotification.Builder request = Msg.ExitNotification.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        request.setExitType(0);
        builder.setExitNotification(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    
    public void FuelConfirm(Boolean flag) {
    	//soundsUtils.playSound(soundsUtils.NO);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.FUEL_RESPONSE);
        Msg.FuelResponse.Builder response = Msg.FuelResponse.newBuilder();
        response.setDeviceId(ZXConfig.getFuelCarID());
        response.setTargetDeviceName(ZXConfig.getCarNo());
        response.setVolume(ZXConfig.getFuelVolume());
        if(flag) {
        	response.setResult(1);
        }
        else {
        	response.setResult(0);
        }

        builder.setFuelResponse(response);
        NettyUtils.getInstance().sendMsg(builder.build());
        
        mFuelConfirmDialog.dismiss();
    }
    
    @Override
	public void responseFuelRequest(final Msg.FuelRequest msg){
		// TODO Auto-generated method stub
		
		runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	if (msg != null) {
            		ZXConfig.setFuelCarID(msg.getDeviceId());
            		ZXConfig.setFuelVolume(msg.getVolume());
            
            		
            		mFuelConfirmDialog.setText("是否已加油" + (int)msg.getVolume() + "升?");
            		mFuelConfirmDialog.show();
                }
            }
        });
	}
	
	@Override
    public void responseFuelResponse(final Msg.FuelResponse msg){
		// TODO Auto-generated method stub

	}
	
	@Override
	public void responseTargetNotification(Msg.TargetNotification msg) {
		// TODO Auto-generated method stub
		String result = "[";
		result += String.valueOf(msg.getLongitude());
		result += ",";
		result += String.valueOf(msg.getLatitude());
		result += "]";
		
		mJsDataProvider.SetTarget(result);

	}
    
    private void NotifyUnload(String deviceID) {
    	PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);
    	//mHandler.sendEmptyMessageDelayed(MSG_UNLOAD_REQUEST, REQUEST_TIMEOUT_INTERVAL);
    	
    	Message message = new Message();
    	message.what = MSG_UNLOAD_REQUEST;
    	message.arg1 = 0;
    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
    	
    	Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.UNLOAD_REQUEST);
        Msg.UnloadRequest.Builder request = Msg.UnloadRequest.newBuilder();
        request.setDeviceId(deviceID);
        builder.setUnloadRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());                
    }
    
    private String GetCarIDFromCarNo(String carNo) {
    	String carID = "";
    	for(int i = 0; i < listRequest.size(); i ++) {  
    		LoadRequest request = listRequest.get(i);
    		if (carNo == request.getCarNo()) {
    			carID = request.getDeviceId();
    			break;
    		}
    	}
    	
    	return carID;
    }
    
    private void refreshGPS(Msg.GPSData msg){
    	lat_tv.setText(msg.getLatitude());
    	lon_tv.setText(msg.getLongitude());
    	//mSpeed.setText(String.valueOf(Integer.valueOf(msg.getSpeed())));
    	String speed = msg.getSpeed();
    	mSpeed.setText(speed.substring(0, speed.indexOf(".")));

        //System.out.println("refreshGPS lon:" + msg.getLongitude() + " lat:" + msg.getLatitude());
        mWebView.loadUrl("javascript:MapPanto(" + msg.getLatitude() + "," + msg.getLongitude() + ")");
        mWebView.loadUrl("javascript:setTarget("+ msg.getLatitude() + ","+ msg.getLongitude() + ")");
    }
    private void refreshOil(final double point){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //mOilStatus.setPoint((float) point);
            }
        });
    }
    private void showLimitedSpeed(int limitedSpeed){
    	mLimitedSpeed.setText(String.valueOf(limitedSpeed) + "");
    }
    
    
    private void showSpeed(float speed){
    	mSpeed.setText((int)speed+"");
    }
    
	private void refreshDate(){
		SimpleDateFormat dateFormat = new SimpleDateFormat("EEEE,M月d日");
		SimpleDateFormat timeFormat = new SimpleDateFormat("kk:mm");
		Calendar cl = Calendar.getInstance();
		mDate.setText(dateFormat.format(cl.getTime()));
		mTime.setText(timeFormat.format(cl.getTime()));
    }
	
	private void regeditReceiver(int what){
		IntentFilter filter = new IntentFilter();
		switch(what){
		case TIME_RECEIVER:
		     filter.addAction(Intent.ACTION_TIME_TICK);
		     filter.addAction(Intent.ACTION_TIME_CHANGED);
		     filter.addAction(Intent.ACTION_TIMEZONE_CHANGED);
		     registerReceiver(mTimeReceiver, filter, null, null);
		     break;
		case NET_RECEIVER:
			 filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
			 registerReceiver(mConnetcReceiver, filter, null, null);
		default:
			break;
		}
	}
	private int getNetType(){
		ConnectivityManager cm = (ConnectivityManager)getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo wifi = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        NetworkInfo mobile = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        if (wifi != null && wifi.isConnected()) {
            if (mobile != null && mobile.isConnected()) {
                return NETWORK_BOTH;
            } else {
                return NETWORK_WIFI;
            }
        } else if (mobile != null && mobile.isConnected()) {
            return NETWORK_MOBILE;
        }
        return NETWORK_NONE;
	}
	private void refreshNet(){
	    switch(getNetType()){
        case NETWORK_BOTH:
           gprsIbt.setImageResource(R.drawable.gprson);
     	   //netIbt.setImageResource(R.drawable.serveron);
     	   break;
        case NETWORK_MOBILE:
        	gprsIbt.setImageResource(R.drawable.gprson);
     	   //netIbt.setImageResource(R.drawable.serveron);
     	   break;
        case NETWORK_WIFI:
        	gprsIbt.setImageResource(R.drawable.gprsoff);
     	   //netIbt.setImageResource(R.drawable.serveroff);
     	   break;
        case NETWORK_NONE:
        	gprsIbt.setImageResource(R.drawable.gprsoff);
     	   //netIbt.setImageResource(R.drawable.serveroff);
     	   break;
        } 
	}
	//private void refreshDistance(String what,int distance){
	//	mDistance.setText(String.format("%s:%d m", what,distance));
	//}

	/**
	 * Provides a hook for calling "alert" from javascript. Useful for debugging
	 * your javascript.
	 */
	final class MyWebChromeClient extends WebChromeClient {
		@Override
		public boolean onJsAlert(WebView view, String url, String message,
				JsResult result) {
			result.confirm();
			return true;
		}
	}

    private void queryProduction(int period) {
        Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.PRODUCTION_REQUEST);
        Msg.ProductionRequest.Builder request = Msg.ProductionRequest.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        request.setPeriod(period);
        builder.setProductionRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
    }
    private BroadcastReceiver mTimeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            refreshDate();
        }
    };

    private BroadcastReceiver mConnetcReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            refreshNet();
        }
    };



	@Override
	public void onClick(View v) {
			switch(v.getId()){
			case R.id.output:
				if(ZXConfig.getconnStatus() == 0) {
					return;
				}
				
				PromptUtils.showProgressDialog(this, "请求中，请稍候…", false, false);

		    	Message message = new Message();
		    	message.what = MSG_PRODUCTION_REQUEST;
		    	message.arg1 = 0;
		    	mHandler.sendMessageDelayed(message, REQUEST_TIMEOUT_INTERVAL);
		    	
				if (mProductionInfoDialog != null && mProductionInfoDialog.isShowing()) {
					mProductionInfoDialog.dismiss();
                }

				Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
		        builder.setType(Msg.MessageType.PRODUCTION_INFO_REQUEST);
		        Msg.ProductionInfoRequest.Builder request = Msg.ProductionInfoRequest.newBuilder();
		        request.setDeviceId(ZXConfig.getDeviceId());
		        builder.setProductionInfoRequest(request);
		        NettyUtils.getInstance().sendMsg(builder.build());
                
				break;
				
			case R.id.play:
				if(ZXConfig.getconnStatus() == 0) {
					return;
				}
				
				if(soundEnabled) {
					soundsUtils.setMute(true);
					mPlay.setImageResource(R.drawable.speakeroff);
					soundEnabled = !soundEnabled;
				}
				else {
					soundsUtils.setMute(false);
					mPlay.setImageResource(R.drawable.speakeron);
					soundEnabled = !soundEnabled;
				}
				
				break;
				
			case R.id.recode:
				if(ZXConfig.getconnStatus() == 0) {
					return;
				}
				
                if (!mRecordUtils.isRecording()) {
                    //mRecordUtils.writeLog("Record button on touch down");
                    mRecode.setImageResource(R.drawable.chaton);
                    mLastRecordTime = System.currentTimeMillis();
                    mRecordUtils.playBeep();
                    try {
                        mRecordUtils.beginRecorder();
                    } catch (Throwable throwable) {
                        PromptUtils.showToast("录音失败，请重试！", Toast.LENGTH_SHORT);
                        throwable.printStackTrace();
                    }
                    	
                    mHandler.sendEmptyMessageDelayed(MSG_RECORD_TIMEOUT, 10 * 1000);
                } else {

                	mHandler.sendEmptyMessage(MSG_RECORD_STOP);
                	
                    mRecode.setImageResource(R.drawable.chatoff);

                    long s = System.currentTimeMillis() - mLastRecordTime;
                    if(s < 1000) {

                        PromptUtils.showToast(R.string.try_again, Toast.LENGTH_LONG); //语音指令太短
                        if(TextUtils.isEmpty(mRecordUtils.stopRecorder())) {
                            mRecordUtils.deleteEncodeFile();
                        }
                    }

                    //end
                    mRecordUtils.playBeep();
                    String recordPath = mRecordUtils.stopRecorder();
                    if(!TextUtils.isEmpty(recordPath)) {
                        new UploadRecordTask().execute(recordPath);
                    }
                }
                break;
			}
		}
	
	private class UploadRecordTask extends AsyncTask<String, Integer, Boolean> {

        @SuppressWarnings("resource")
		@Override
        protected Boolean doInBackground(String... params) {
            if (TextUtils.isEmpty(params[0])) {
                return false;
            }
            
            String fileName = params[0];
            int nIdx = fileName.lastIndexOf("/");
            if(nIdx == -1) {
            	return false;
            }
            
            String audio_name = fileName.substring(nIdx + 1);
            
            File recodeFile = new File(fileName);
            if(!recodeFile.exists()||!recodeFile.isFile()) {
                return false;
            }
            
            long file_len = recodeFile.length();
			FileInputStream fis;
            ByteArrayOutputStream bos = null;
            
			try {
				fis = new FileInputStream(recodeFile);
				bos = new ByteArrayOutputStream();
	            byte[] buffer = new byte[(int)file_len];
	            int packageLength;
	            packageLength = fis.read(buffer);
	            bos.write(buffer, 0, packageLength);
			} catch (FileNotFoundException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
            
            String url = SERVER_ADDRESS + "audio/upload";
    		Map<String, Object> pars = new HashMap<String, Object>();
    		pars.put("from", ZXConfig.getDeviceId());
    		pars.put("to", "web");
    		pars.put("audio_name", audio_name);
    		pars.put("audio", bos.toByteArray()); //ByteString.copyFrom(bos.toByteArray())

    		mAqueryUploadAudio.ajax(url, pars, JSONObject.class, new AjaxCallback<JSONObject>() {
    	        public void callback(String url, JSONObject json, AjaxStatus status) {
    	    	    
    	        	if(json == null){ 
    	        		return;
    	        	}
    	        	
    	        	SyncData response = null;
    				try {
    					response = SyncData.parseJsonObject(json.toString());
    					if(response.status == null 
    							|| response.status.length() <= 0 
    							|| response.status.contentEquals("error") == true )
    					{
    						Loger.print("UploadRecordTask::error happen: " + response.message); 
    					}
    					else if(response.status.contentEquals("ok") == true) {
    						Loger.print("UploadRecordTask::success"); 
    					}
    					
    					return;
    					
    				} catch (JsonSyntaxException e) {
    					e.printStackTrace();
    				} catch (UnsupportedEncodingException e) {
    					e.printStackTrace();
    				}
    	        }
    	    }.header("ENCTYPE", "multipart/form-data"));
    	    
            return true;
        }

        @Override
        protected void onPostExecute(Boolean b) {
            PromptUtils.showToast(b ? "语音消息发送成功" : "语音消息发送失败", Toast.LENGTH_LONG);
            super.onPostExecute(b);
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);
        }
    }

    /*private class UploadRecordTask extends AsyncTask<String, Integer, Boolean> {

            @Override
            protected Boolean doInBackground(String... params) {
                if (TextUtils.isEmpty(params[0])) {
                    return false;
                }
                File recodeFile = new File(params[0]);
                if(!recodeFile.exists()||!recodeFile.isFile()) {
                    return false;
                }

                Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                builder.setType(Msg.MessageType.AUDIO_FILE_DATA);
                Msg.AudioFileData.Builder audioBuilder = Msg.AudioFileData.newBuilder();
                audioBuilder.setDeviceId(ZXConfig.getDeviceId());
                audioBuilder.setFileName(ZXConfig.getDeviceId() + "_" + recodeFile.getName());
                int packageCount = (int) Math.ceil((float)recodeFile.length()/(1024*512));
                audioBuilder.setTotalPackets(packageCount);
                FileInputStream fis = null;
                ByteArrayOutputStream bos = null;
                int packageLength = 0;
                byte[] buffer = new byte[512 * 1024];
                try {
                    fis = new FileInputStream(recodeFile);
                    int count = 0;
                    while((packageLength=fis.read(buffer))!=-1){
                        count++;
                        bos = new ByteArrayOutputStream();
                        bos.write(buffer, 0, packageLength);
                        audioBuilder.setData(ByteString.copyFrom(bos.toByteArray()));
                        audioBuilder.setDataLen(packageLength);
                        audioBuilder.setCurrentPacket(count);
                        builder.setAudioFileData(audioBuilder);
                        NettyUtils.getInstance().sendMsg(builder.build());
                    }
                    return true;
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }catch(IOException e){
                    System.out.println("发送消息IO错误" + e.getMessage());
                    e.printStackTrace();
                }finally{
                    try {
                        fis.close();
                        bos.close();
                    } catch (Exception e) {
                        e.printStackTrace();

                    }
                }
                return false;
            }

            @Override
            protected void onPostExecute(Boolean b) {
                PromptUtils.showToast(b ? "语音消息发送成功" : "语音消息发送失败", Toast.LENGTH_LONG);
                super.onPostExecute(b);
            }

            @Override
            protected void onProgressUpdate(Integer... values) {
                super.onProgressUpdate(values);
            }
        }*/
}
