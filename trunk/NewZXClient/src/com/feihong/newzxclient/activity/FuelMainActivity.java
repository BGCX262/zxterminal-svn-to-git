package com.feihong.newzxclient.activity;

import java.io.IOException;

import zaoxun.Msg;
import zaoxun.Msg.AntiCollisionNotification;
import zaoxun.Msg.AudioFileData;
import zaoxun.Msg.CancelLoadRequest;
import zaoxun.Msg.CancelLoadResponse;
import zaoxun.Msg.CarType;
import zaoxun.Msg.CommandRequest;
import zaoxun.Msg.DeviceStatusRequest;
import zaoxun.Msg.GPSData;
import zaoxun.Msg.GetEDippersResponse;
import zaoxun.Msg.LoadCompleteRequest;
import zaoxun.Msg.LoadCompleteResponse;
import zaoxun.Msg.LoadRequest;
import zaoxun.Msg.LoadResponse;
import zaoxun.Msg.MaterialsNotification;
import zaoxun.Msg.MaterialsResponse;
import zaoxun.Msg.OilQuantityResponse;
import zaoxun.Msg.ProductionInfoResponse;
import zaoxun.Msg.ProductionResponse;
import zaoxun.Msg.RejectLoadRequest;
import zaoxun.Msg.RejectLoadResponse;
import zaoxun.Msg.ShortMessage;
import zaoxun.Msg.SpeedLimitMessage;
import zaoxun.Msg.UnloadResponse;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;

import android.widget.EditText;
import android.widget.ImageView;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.callback.OnResponseMessageListener;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.js.JSDataProvider;
import com.feihong.newzxclient.tcp.NettyUtils;
import com.feihong.newzxclient.util.PromptUtils;
import com.feihong.newzxclient.util.RecordUtils;
import com.feihong.newzxclient.widget.MessageDialog;

public class FuelMainActivity extends BaseActivity implements OnClickListener, OnResponseMessageListener {

	private MessageDialog mMessageDialog;
	
	private ImageView mFuelOK;
	private EditText mTargetName;
	private EditText mFuelVolume;
	private  ImageView mClearNoIv;
	private  ImageView mClearVolumeIv;
	
	private View currentView = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.fuel_main);
		
		mTargetName = (EditText)this.findViewById(R.id.carno_edittext);
		mFuelVolume = (EditText)this.findViewById(R.id.volume_edittext);
		mClearNoIv = (ImageView)this.findViewById(R.id.clear_car_no_imageview);
		mClearVolumeIv = (ImageView)this.findViewById(R.id.clear_volume_imageview);
		
		mFuelOK = (ImageView) this.findViewById(R.id.ok_imageview);
		
		mMessageDialog = new MessageDialog(this);
		
		mFuelOK.setOnClickListener(new android.view.View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				
				requestFuel();

			}
		});
		
		setListener();
		
		mTargetName.setOnFocusChangeListener(mFocusChangeListener);
		mFuelVolume.setOnFocusChangeListener(mFocusChangeListener);
		
		mTargetName.setInputType(InputType.TYPE_NULL);
		mFuelVolume.setInputType(InputType.TYPE_NULL);
		
		currentView = mTargetName;
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

        mFuelOK.setOnClickListener(this);
        mClearNoIv.setOnClickListener(this);
        mClearVolumeIv.setOnClickListener(this);

    }

    private View.OnClickListener mNumClickListener =  new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	if(currentView == mTargetName ) {
        		mTargetName.setText(mTargetName.getText().toString() + v.getTag());
        	}
        	else {
        		mFuelVolume.setText(mFuelVolume.getText().toString() + v.getTag());
        	}
        }
    };
    
    private View.OnFocusChangeListener mFocusChangeListener = new View.OnFocusChangeListener() {
    	@Override
    	public void onFocusChange(View v, boolean hasFocus) {
            // TODO Auto-generated method stub
		    if(hasFocus) {
		    	switch (v.getId()) {
	            case R.id.carno_edittext:
	            	currentView = mTargetName;
	                break;
	                
	            case R.id.volume_edittext:
	            	currentView = mFuelVolume;
	                break;
	                
	            default:
	                break;
		    	}
	        }      
		}
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.clear_car_no_imageview:
                clearCarNoEvent();
                break;
                
            case R.id.clear_volume_imageview:
                clearVolumeEvent();
                break;
                
            case R.id.ok_imageview:
            	requestFuel();
                break;
            default:

                break;
        }

    }

    private void clearCarNoEvent(){
        String content = mTargetName.getText().toString();
        if(content.length() > 0){
        	mTargetName.setText(content.substring(0, content.length()-1));
        }
    }
    
    private void clearVolumeEvent(){
        String content = mFuelVolume.getText().toString();
        if(content.length() > 0){
        	mFuelVolume.setText(content.substring(0, content.length()-1));
        }
    }
	
	private void requestFuel() {
        Msg.CommonMessage.Builder builder = Msg.CommonMessage.newBuilder();
        builder.setType(Msg.MessageType.FUEL_REQUEST);
        Msg.FuelRequest.Builder request = Msg.FuelRequest.newBuilder();
        request.setDeviceId(ZXConfig.getDeviceId());
        request.setTargetDeviceName(mTargetName.getText().toString().trim());
        request.setVolume(Double.valueOf(mFuelVolume.getText().toString().trim()));
        request.setMode(2);
        request.setType(0);
        builder.setFuelRequest(request);
        NettyUtils.getInstance().sendMsg(builder.build());
        //PromptUtils.showProgressDialog(this, "«Î«Û÷–£¨«Î…‘∫Ú°≠", true);
        
        mMessageDialog.setMessage(getString(R.string.fuel_submitted));
        mMessageDialog.show();
    }

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
    }
	
	@Override
	public void responseOilQuantity(OilQuantityResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseProduction(ProductionResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseSpeedLimit(SpeedLimitMessage msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseGPS(GPSData msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseAudioFile(AudioFileData msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseShortMessage(ShortMessage msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseDirection(double direction) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseLoadRequest(LoadRequest msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseLoadResponse(LoadResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseGetEdippersResponse(GetEDippersResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseLoadCompleteRequest(LoadCompleteRequest msg) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void responseLoadCompleteResponse(LoadCompleteResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseUnloadResponse(UnloadResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseMaterialsNotification(MaterialsNotification msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseCancelLoadRequest(CancelLoadRequest msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseCancelLoadResponse(CancelLoadResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseRejectCancelRequest(RejectLoadRequest msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseRejectCancelResponse(RejectLoadResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseDeviceStatusRequest(
			DeviceStatusRequest msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseAntiCollisionNotification(AntiCollisionNotification msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseMaterialsResponse(MaterialsResponse msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseCommandRequest(CommandRequest msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void responseProductionInfoResponse(ProductionInfoResponse msg) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void responseFuelRequest(final Msg.FuelRequest msg){
		// TODO Auto-generated method stub
		
	}
	
	@Override
    public void responseFuelResponse(final Msg.FuelResponse msg){
		// TODO Auto-generated method stub
		
		runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	PromptUtils.dismissProgressDialog();
            	
            	if (msg != null) {
            		if(msg.getResult() == 1) {
            			mMessageDialog.setMessage(getString(R.string.fuel_ok));
            		}
            		else {
            			mMessageDialog.setMessage(getString(R.string.fuel_reject));
            		}
                }
            	
            	mMessageDialog.show();
            }
        });
	}
	
	@Override
	public void responseTargetNotification(Msg.TargetNotification msg) {
		// TODO Auto-generated method stub

	}
	
	@Override
	public void responseManualLoadCompleteRequest(Msg.ManualLoadCompleteRequest msg) {
		// TODO Auto-generated method stub

	}

	@Override
    public void responseManualUnloadRequest(Msg.ManualUnloadRequest msg) {
		// TODO Auto-generated method stub

	}


}
