package com.feihong.newzxclient.service;

import android.app.Activity;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.*;
import android.provider.Settings;
import android.text.TextUtils;
import android.text.format.Time;
import android.util.Log;
import android.widget.Toast;
import com.baidu.location.BDLocation;
import com.baidu.location.BDLocationListener;
import com.baidu.location.LocationClient;
import com.feihong.newzxclient.activity.ActivityManager;
import com.feihong.newzxclient.callback.OnResponseCheckListener;
import com.feihong.newzxclient.callback.OnResponseMessageListener;
import com.feihong.newzxclient.callback.OnResponseRegisterListener;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.tcp.*;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;
import com.google.protobuf.InvalidProtocolBufferException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;

import io.netty.buffer.ByteBuf;
import zaoxun.Msg;
import zaoxun.Msg.LoginResponse;
import zaoxun.Msg.Material;

/**
 * MainService
 *
 * @author xiong
 * @version 1.0.0
 */
public class MainService extends Service  implements SensorEventListener{

    private static final String TAG = "MainService";

    private static final int MSG_ID = 0x01;
    private static final long REQUEST_INTERVAL = 100;
    
    private static final long SEND_GPS_INTERVAL = 5 * 1000;
    private static final long CHECK_HB_INTERVAL = 20 * 1000;
    private static final long SEND_HB_INTERVAL = 10 * 1000;
    private static final long HB_TIMEOUT_VALUE = 20;

    private long mLastSendTime = 0;
    private final LocalBinder mBinder = new LocalBinder();

    //private LocationClient mLocationClient = null;
    private double mLatitude;
    private double mLongitude;

    private Location mLocation = null;
    private Msg.GPSData.Builder mGPSBuilder = null;
    private Timer mSendGPSTimer = null;
    private Timer mHBTimer = null;
    private Timer mSendHBTimer = null;
    
    private SensorManager mSensorManager;
    private Sensor mOrientation;
    
    private double mRotation = 0;
    
    public LoginResponse mLoginResponse;
    
    private boolean reconnect_flag = true;
    private LocationManager locationManager;
    private AndroidLocationListener listener;
    
    /**
     * 绑定服务时的返回对象
     */
    public class LocalBinder extends Binder {
        /**
         * 获取服务本身
         *
         * @return 当前服务本身
         */
        public MainService getService() {
            return MainService.this;
        }
    }

   /* private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_ID) {
                mLocationClient.requestLocation();
                mHandler.removeMessages(MSG_ID);
                mHandler.sendEmptyMessageDelayed(MSG_ID, REQUEST_INTERVAL);
            }
        }
    };*/

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    
    class ReconnectThead extends Thread
    {
        public void run()
        {
        	try {
				Thread.sleep(5000);
			} catch (InterruptedException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
        	
    		while(reconnect_flag) {
    			try {
    				
    				try {
						Thread.sleep(10000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
    				
					if (!NettyUtils.getInstance().IsConnected()) {	
						NettyUtils.getInstance().close();
						
						try {
							Thread.sleep(5000);
						} catch (InterruptedException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						
						PromptUtils.showToast("reconnect", Toast.LENGTH_SHORT);
						
						Loger.print("ReconnectThead,  run() ---- reconnect");

						NettyUtils.getInstance().open(new NettyClientHandler.OnReadListener() {
			                @Override
			                public void read(Object objMsg) throws InvalidProtocolBufferException {
			                	Msg.CommonMessage commonMessage = (Msg.CommonMessage)objMsg;
			                    handleMessage(commonMessage);
			                }
			            });	
					}
	    		} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					String strError = String.format("Recv data error(InterruptedException), Error message: %s", e.getMessage());
					Loger.print(strError);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					String strError = String.format("Recv data error(Exception), Error message: %s", e.getMessage());
					Loger.print(strError);
				}
						
				try {
					Thread.sleep(5000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
    		}
        }
    }
    
    class SendHBThead extends Thread
    {
        public void run()
        {
        	try {
        		SendHBMsg();
        		
			} catch (Exception e) {
				// TODO Auto-generated catch block
				String strError = String.format("SendHBMsg error, Error message: %s", e.getMessage());
				Loger.print(strError);
			}

        	try {
				Thread.sleep(10 * 1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        //open connection
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_FOREGROUND);
        try {
            NettyUtils.getInstance().open(new NettyClientHandler.OnReadListener() {
                @Override
                public void read(Object objMsg) throws InvalidProtocolBufferException {
                	Msg.CommonMessage commonMessage = (Msg.CommonMessage)objMsg;
                    handleMessage(commonMessage);
                }
            });
            
            ReconnectThead reconnectThread = new ReconnectThead();
            reconnectThread.start();
            
        } catch (Exception e) {
            e.printStackTrace();
            
            String strError = String.format("Recv data error, Error message: %s", e.getMessage());
			Loger.print(strError);
        }

        boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled(getContentResolver(), LocationManager.GPS_PROVIDER);
        if (!gpsEnabled) {
            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            PromptUtils.showToast("please open GPS", Toast.LENGTH_SHORT);
        }

        locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        Criteria criteria = new Criteria();
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(true);
        criteria.setBearingRequired(true);
        criteria.setCostAllowed(true);
        criteria.setPowerRequirement(Criteria.POWER_LOW);

        String provider = LocationManager.GPS_PROVIDER;//locationManager.getBestProvider(criteria, true);
        if (TextUtils.isEmpty(provider)) {
            provider = LocationManager.GPS_PROVIDER;
        }
        
        listener = new AndroidLocationListener();
        locationManager.requestLocationUpdates(provider,
        		REQUEST_INTERVAL, 0, listener);

        //create send gps timer
        mSendGPSTimer = new Timer(true);
        mSendGPSTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            SendGPS();
            }
        }, 0,  SEND_GPS_INTERVAL);
        
        mHBTimer = new Timer(true);
        mHBTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            	CheckHeartBeat();
            }
        }, 0,  CHECK_HB_INTERVAL);
        
        mSendHBTimer = new Timer(true);
        mSendHBTimer.scheduleAtFixedRate(new java.util.TimerTask() {
            public void run() {
            	SendHBMsg();
            }
        }, 5000,  SEND_HB_INTERVAL);
        
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mOrientation = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        mSensorManager.registerListener(this, mOrientation, SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void SendGPS() {
        if (mLocation != null) {
            if (!TextUtils.isEmpty(ZXConfig.getDeviceId())) {
                if (mGPSBuilder != null){
                    Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
                    builder.setType(Msg.MessageType.GPS_DATA);

                    SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");     
                    String date = dateFormat.format(new java.util.Date()); 
                    
                    mGPSBuilder.setTime(date);
                    builder.setGpsData(mGPSBuilder);
                    
                    NettyUtils.getInstance().sendMsg(builder.build());
                    mLastSendTime = System.currentTimeMillis();
                    
                    Log.i("Send GPS : ", "at " + mLastSendTime);
                }
            }
        }
    }
    
    private void CheckHeartBeat() {
    	Loger.print("CheckHeartBeat, Begin....");
    	
    	long currentTime = System.currentTimeMillis() / 1000;
    	
    	if(NettyUtils.getInstance().IsConnected()) {    		
        	long lastRecvHBTime = ZXConfig.getLastRecvHBTime();
			Loger.print("CheckHeartBeat, current time: " + String.valueOf(currentTime));
			Loger.print("CheckHeartBeat, last recv time: " + String.valueOf(lastRecvHBTime));
			
        	if(lastRecvHBTime != 0) {
        		if(currentTime - lastRecvHBTime > HB_TIMEOUT_VALUE) {
        			NettyUtils.getInstance().close();
        			
        			String strError = String.format("CheckHeartBeat, close current connection and reconnect. current time: %s, last recv time: %s", 
        					String.valueOf(currentTime), String.valueOf(lastRecvHBTime));
        			Loger.print(strError);
            	}
        	}
    	}
    }
    
    private void SendHBMsg() {
    	if(NettyUtils.getInstance().IsConnected()) {
    		Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
            builder.setType(Msg.MessageType.HEART_BEAT);

            Msg.HeartBeat.Builder hb = Msg.HeartBeat.newBuilder();
            hb.setValue(1);
            builder.setHeartBeat(hb);
            
            NettyUtils.getInstance().sendMsg(builder.build());

            String strError = String.format("Send HB message, current time: %s", 
            		String.valueOf(System.currentTimeMillis() / 1000));
			Loger.print(strError);
    	}
    }
    
    
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
      // Do something here if sensor accuracy changes.
      // You must implement this callback in your code.
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
      float azimuth_angle = event.values[0];
      mRotation = azimuth_angle;
      float pitch_angle = event.values[1];
      float roll_angle = event.values[2];
      // Do something with these orientation angles.

      /*if(mRotation >= 180) {
    	  mRotation += 180;
    	  mRotation = mRotation % 360;
      }*/

      Activity currentActivity = ActivityManager.instance().getCurrentActivity();
      if (currentActivity instanceof OnResponseMessageListener) {
          ((OnResponseMessageListener) currentActivity).responseDirection(mRotation);
      }
    }

    /**
     * android定位监听
     */
    private class AndroidLocationListener implements LocationListener {

        @Override
        public void onLocationChanged(Location location) {
        	mLatitude = location.getLatitude();
        	mLongitude = location.getLongitude();
            mLocation = location;

            String latitude = Double.toString(mLatitude);
            String longitude = Double.toString(mLongitude);
            Activity currentActivity = ActivityManager.instance().getCurrentActivity();

            Msg.GPSData.Builder gps = Msg.GPSData.newBuilder();
            gps.setLatitude(latitude);
            gps.setLongitude(longitude);

            String strTime = String.valueOf(mLocation.getTime());
            Date date = new Date(Long.parseLong(strTime.trim()));
            SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            String dateString = formatter.format(date);
            gps.setTime(String.valueOf(dateString));

            if (!TextUtils.isEmpty(ZXConfig.getDeviceId())) {
                gps.setDeviceId(ZXConfig.getDeviceId());
            }

            gps.setElevation(Double.toString(mLocation.getAltitude()));
            gps.setRadius(Float.toString(mLocation.getAccuracy()));
            gps.setSpeed(Float.toString(mLocation.getSpeed() * 3600 / 1000 ));
            
            /*if(mLocation.hasBearing()) {
            	gps.setDirection(Float.toString(mLocation.getBearing()));
            }*/

            gps.setDirection(Double.toString(mRotation));
            
            mGPSBuilder = gps;

            if (currentActivity instanceof OnResponseMessageListener) {
                ((OnResponseMessageListener) currentActivity).responseGPS(gps.build());
            }

        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {

        }

        @Override
        public void onProviderEnabled(String provider) {

        }

        @Override
        public void onProviderDisabled(String provider) {

        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        //mLocationClient.stop();
        if(mSendGPSTimer != null) {
        	mSendGPSTimer.cancel();
        	mSendGPSTimer = null;
        }
        
        if(mHBTimer != null) {
        	mHBTimer.cancel();
        	mHBTimer = null;
        }
        
        if(mSendHBTimer != null) {
        	mSendHBTimer.cancel();
        	mSendHBTimer = null;
        }

        reconnect_flag = false;
        NettyUtils.getInstance().close();
        
        locationManager.removeUpdates(listener);
        mSensorManager.unregisterListener(this);
    }
    
    public void Stop() {
    	stopSelf();
    }

    public double getLatitude() {
        return (int) (mLatitude * 1000000) / 1000000.0;
    }

    public double getLogitude() {
        return (int) (mLongitude * 1000000) / 1000000.0;
    }

    private void handleMessage(Msg.CommonMessage msg) {
        Activity currentActivity = ActivityManager.instance().getCurrentActivity();
    
        switch (msg.getType()) {
            case LOGIN_RESPONSE:
            	mLoginResponse = msg.getLoginResponse();
                if (currentActivity instanceof OnResponseRegisterListener) {
                    ((OnResponseRegisterListener) currentActivity).responseRegister(msg.getLoginResponse());
                }
                Loger.print("Recv msg[LOGIN_RESPONSE]: " + msg.getLoginResponse().getDeviceId() + " " +  
                		String.valueOf(msg.getLoginResponse().getResult()));
                break;
                
            case CHECK_RESPONSE:
                if (currentActivity instanceof OnResponseCheckListener) {
                    ((OnResponseCheckListener) currentActivity).responseCheck(msg.getCheckResponse());
                }
                Loger.print("Recv msg[CHECK_RESPONSE]: " + msg.getCheckResponse().getDeviceId() + " " +  
                		String.valueOf(msg.getCheckResponse().getStatus()));
                break;
                
            case OIL_QUANTITY_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseOilQuantity(msg.getOilQuantityResponse());
                }
                Loger.print("Recv msg[OIL_QUANTITY_RESPONSE]: " + msg.getOilQuantityResponse().getDeviceId() +  " " + 
                		String.valueOf(msg.getOilQuantityResponse().getQuantity()));
                break;
                
            case SPEED_LIMIT:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseSpeedLimit(msg.getSpeedLimit());
                }
                Loger.print("Recv msg[SPEED_LIMIT]: " + msg.getSpeedLimit().getDeviceId() +  " " + 
                		String.valueOf(msg.getSpeedLimit().getSpeed()));
                break;
                
            case SHORT_MESSAGE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseShortMessage(msg.getShortMessage());
                }
                Loger.print("Recv msg[SHORT_MESSAGE]: " + msg.getShortMessage().getDeviceId() +  " " + 
                		msg.getShortMessage().getContent());
                break;
                
            case COMMAND_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseCommandRequest(msg.getCommandRequest());
                }
                Loger.print("Recv msg[COMMAND_REQUEST]: " + msg.getCommandRequest().getDeviceId() +  " " + 
                		msg.getCommandRequest().getContent());
                break;    
            
            case GPS_DATA:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseGPS(msg.getGpsData());
                }
                Loger.print("Recv msg[GPS_DATA]: " + msg.getGpsData().getDeviceId());
                break;
            case AUDIO_FILE_DATA:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseAudioFile(msg.getAudioFileData());
                }
                Loger.print("Recv msg[AUDIO_FILE_DATA]: " + msg.getAudioFileData().getDeviceId());
                break;
            case PRODUCTION_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseProduction(msg.getProductionResponse());
                }
                Loger.print("Recv msg[PRODUCTION_RESPONSE]: " + msg.getProductionResponse().getDeviceId() +   " " +  
                		String.valueOf(msg.getProductionResponse().getQuantity()));
                break;
                
            ///////////////////////////
            case GET_EDIPPERS_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseGetEdippersResponse(msg.getGetEdippersResponse());
                }
                Loger.print("Recv msg[GET_EDIPPERS_RESPONSE]: " + msg.getGetEdippersResponse().getDeviceId() +    " " + 
                		String.valueOf(msg.getGetEdippersResponse().getEdippersCount()));
                break;
            
            case LOAD_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseLoadRequest(msg.getLoadRequest());
                }
                Loger.print("Recv msg[LOAD_REQUEST]: " + msg.getLoadRequest().getDeviceId() + " " + 
                msg.getLoadRequest().getEdipperDeviceId());
                break;
                
            case LOAD_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseLoadResponse(msg.getLoadResponse());
                }
                Loger.print("Recv msg[LOAD_RESPONSE]: " + msg.getLoadResponse().getDeviceId() +    " " + 
                		String.valueOf(msg.getLoadResponse().getResult()));
                break;
                
            case LOAD_COMPLETE_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseLoadCompleteRequest(msg.getLoadCompleteRequest());
                }
                Loger.print("Recv msg[LOAD_COMPLETE_REQUEST]: " + msg.getLoadCompleteRequest().getEdipperDeviceId());
                break;
                
            case LOAD_COMPLETE_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseLoadCompleteResponse(msg.getLoadCompleteResponse());
                }
                Loger.print("Recv msg[LOAD_COMPLETE_RESPONSE]: " + msg.getLoadCompleteResponse().getEdipperDeviceId() +    " " + 
                		String.valueOf(msg.getLoadCompleteResponse().getResult()));
                break;
                
            case UNLOAD_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseUnloadResponse(msg.getUnloadResponse());
                }
                Loger.print("Recv msg[UNLOAD_RESPONSE]: " + msg.getUnloadResponse().getDeviceId() + " " +
                		String.valueOf(msg.getUnloadResponse().getResult()));
                break;
                
            case MATERIALS_NOTIFICATION:
                int num = msg.getMaterialsNotification().getMaterialCount();
            	ArrayList<Material> materialList = new ArrayList<Material>();
            	for (int i = 0; i < num; i ++) {
            		materialList.add(msg.getMaterialsNotification().getMaterial(i));
            	}
            	
            	ZXConfig.setMaterialList(materialList);
            	
            	if (currentActivity instanceof OnResponseMessageListener) {
            		((OnResponseMessageListener) currentActivity).responseMaterialsNotification(msg.getMaterialsNotification());
                }
            	
            	Loger.print("Recv msg[MATERIALS_NOTIFICATION]: " + String.valueOf(msg.getMaterialsNotification().getMaterialCount()));
                break;
                
            case MATERIALS_RESPONSE:
                int num1 = msg.getMaterialsResponse().getMaterialCount();
            	ArrayList<Material> materialList1 = new ArrayList<Material>();
            	for (int i = 0; i < num1; i ++) {
            		materialList1.add(msg.getMaterialsResponse().getMaterial(i));
            	}
            	
            	ZXConfig.setMaterialList(materialList1);
            	
            	if (currentActivity instanceof OnResponseMessageListener) {
            		((OnResponseMessageListener) currentActivity).responseMaterialsResponse(msg.getMaterialsResponse());
                }
            	
            	Loger.print("Recv msg[MATERIALS_RESPONSE]: " + String.valueOf(msg.getMaterialsResponse().getMaterialCount()));
            	break;
                
            case CANCEL_LOAD_REQUEST:
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseCancelLoadRequest(msg.getCancelLoadRequest());
                }
            	
            	Loger.print("Recv msg[CANCEL_LOAD_REQUEST]: " + msg.getCancelLoadRequest().getEdipperDeviceId());
                break;
                
            case CANCEL_LOAD_RESPONSE:
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseCancelLoadResponse(msg.getCancelLoadResponse());
                }
            	
            	Loger.print("Recv msg[CANCEL_LOAD_RESPONSE]: " + msg.getCancelLoadResponse().getCarDeviceId() +    " " + 
                		String.valueOf(msg.getCancelLoadResponse().getResult()));
                break;
                
            case REJECT_LOAD_REQUEST:
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseRejectCancelRequest(msg.getRejectLoadRequest());
                }
            	Loger.print("Recv msg[REJECT_LOAD_REQUEST]: " + msg.getRejectLoadRequest().getEdipperDeviceId());
                break;
                
            
            case REJECT_LOAD_RESPONSE:
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseRejectCancelResponse(msg.getRejectLoadResponse());
                }
            	Loger.print("Recv msg[REJECT_LOAD_RESPONSE]: " + msg.getRejectLoadResponse().getEdipperDeviceId() +   " " +  
                		String.valueOf(msg.getRejectLoadResponse().getResult()));
                break;    
                
            case DEVICE_STATUS_REQUEST:
            	ZXConfig.setRunningStatus(msg.getDeviceStatusRequest().getStatus());
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseDeviceStatusRequest(msg.getDeviceStatusRequest());
            	}
                    
                if (currentActivity instanceof OnResponseCheckListener) {
                    ((OnResponseCheckListener) currentActivity).responseDeviceStatusRequest(msg.getDeviceStatusRequest());
                }
                
                Loger.print("Recv msg[DEVICE_STATUS_REQUEST]: " + msg.getDeviceStatusRequest().getDeviceId() +  " " +
                		String.valueOf(msg.getDeviceStatusRequest().getStatus()));
                break;
            
            case ANTI_COLLISION_NOTIFICATION:
            	if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseAntiCollisionNotification(msg.getAnticollisionNotification());
                }
   			
    			Loger.print("Recv msg[ANTI_COLLISION_NOTIFICATION]: " + String.valueOf(msg.getAnticollisionNotification().getAngleCount()));
                break;
                
            case PRODUCTION_INFO_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseProductionInfoResponse(msg.getProductionInfoResponse());
                }
                Loger.print("Recv msg[PRODUCTION_INFO_RESPONSE]: " + msg.getProductionInfoResponse().getDeviceId() +   " " +  
                		String.valueOf(msg.getProductionInfoResponse().getFill()));
                break; 
                
            case FUEL_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseFuelRequest(msg.getFuelRequest());
                }
                
                Loger.print("Recv msg[FUEL_REQUEST]: " + msg.getFuelRequest().getDeviceId() +    " " + 
                		String.valueOf(msg.getFuelRequest().getVolume()));
                break; 
                
            case FUEL_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseFuelResponse(msg.getFuelResponse());
                }
                
                Loger.print("Recv msg[FUEL_RESPONSE]: " + msg.getFuelResponse().getDeviceId() + " " + 
                String.valueOf(msg.getFuelResponse().getResult()));
                break; 
                
            case HEART_BEAT:
            	long lastRecvTime = System.currentTimeMillis() / 1000;
                ZXConfig.setLastRecvHBTime(lastRecvTime);
    			
    			Loger.print("Recv msg[HEART_BEAT]: " + String.valueOf(msg.getHeartBeat().getValue()));
                break; 
                
            case TARGET_NOTIFICATION:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseTargetNotification(msg.getTargetNotification());
                }
    			
    			Loger.print("Recv msg[TARGET_NOTIFICATION]: " + msg.getTargetNotification().getDeviceId());
                break; 
                
            case MANUAL_LOAD_COMPLETE_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseManualLoadCompleteRequest(msg.getManualLoadCompleteRequest());
                }
    			
    			Loger.print("Recv msg[MANUAL_LOAD_COMPLETE_REQUEST]: " + msg.getManualLoadCompleteRequest().getCarDeviceId());
                break; 
                
/*            case MANUAL_LOAD_COMPLETE_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseManualLoadCompleteResponse(msg.getManualLoadCompleteResponse());
                }
    			
    			Loger.print("Recv msg[MANUAL_LOAD_COMPLETE_RESPONSE]: " + msg.getManualLoadCompleteResponse().getCarDeviceId());
                break; */
            
            case MANUAL_UNLOAD_REQUEST:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseManualUnloadRequest(msg.getManualUnloadRequest());
                }
    			
    			Loger.print("Recv msg[MANUAL_UNLOAD_REQUEST]: " + msg.getManualUnloadRequest().getDeviceId());
                break; 
                
/*            case MANUAL_UNLOAD_RESPONSE:
                if (currentActivity instanceof OnResponseMessageListener) {
                    ((OnResponseMessageListener) currentActivity).responseManualUnloadResponse(msg.getManualUnloadResponse());
                }
    			
    			Loger.print("Recv msg[MANUAL_UNLOAD_RESPONSE]: " + msg.getManualUnloadResponse().getDeviceId());
                break; */
                
            default:
                break;
        }
    }
}
