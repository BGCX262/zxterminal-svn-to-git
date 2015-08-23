package com.feihong.newzxclient.activity;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import com.feihong.newzxclient.service.MainService;
import com.feihong.newzxclient.util.SoundsUtils;

/**
 * @author xiong
 * @version 1.0.0
 */
public class BaseActivity extends Activity {


    private MainService mMainService;
    public SoundsUtils soundsUtils;
    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mMainService = null;
            BaseActivity.this.onMainServiceDisconnected();
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mMainService = ((MainService.LocalBinder)service).getService();
            BaseActivity.this.onMainServiceConnected();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = new Intent(this, MainService.class);
        bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
        ActivityManager.instance().onCreate(this);
        soundsUtils=SoundsUtils.shareInstance(getApplicationContext());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(mServiceConnection);
        ActivityManager.instance().onDestroy(this);
        if(soundsUtils!=null){
        	soundsUtils.release();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (isConnected()) {
            onMainserviceResumed();
        }
        ActivityManager.instance().onResume(this);
    }

    protected void onMainserviceResumed() {
    }

    public final MainService getMainService() {
        return mMainService;
    }

    public final boolean isConnected() {
        return mMainService != null;
    }

    protected void onMainServiceConnected() {
        onMainserviceResumed();
    }

    protected void onMainServiceDisconnected() {
    }
}
