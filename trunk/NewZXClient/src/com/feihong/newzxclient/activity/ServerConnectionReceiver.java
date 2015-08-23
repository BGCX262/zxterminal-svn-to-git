package com.feihong.newzxclient.activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.ImageView;
import android.widget.Toast;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.util.PromptUtils;

/**
 * Created by hui.yin on 14-1-22.
 */
public class ServerConnectionReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MainActivity mainActivity = (MainActivity) context;
        ImageView netIbt = (ImageView) mainActivity.findViewById(R.id.net);

        String msg = intent.getStringExtra("status");
        if (msg.equals("1")){
            netIbt.setImageResource(R.drawable.serveron);
            mainActivity.EnableUI(true);
            PromptUtils.showToast("connected", Toast.LENGTH_SHORT);
        }
        else{
            netIbt.setImageResource(R.drawable.serveroff);
            mainActivity.EnableUI(false);
            PromptUtils.showToast("disconnect", Toast.LENGTH_SHORT);
        }
    }
}
