package com.feihong.newzxclient;

import android.app.Application;
import android.os.Environment;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.util.DisplayUtils;
import com.feihong.newzxclient.util.Loger;
import com.feihong.newzxclient.util.PromptUtils;

import java.io.File;

/**
 * @author song
 * @time 2013-09-26
 */
public class ZXApplication extends Application {
	@Override
	public void onCreate() {
        super.onCreate();
        DisplayUtils.init(this);
        PromptUtils.init(this);
        ZXConfig.init(this);
        
        Loger.openPrint();
    }
}