package com.feihong.newzxclient.util;

import com.baidu.location.BDLocation;
import com.baidu.location.BDLocationListener;
import com.baidu.location.LocationClient;
import com.baidu.location.LocationClientOption;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.Message;
import android.widget.Toast;

/**
 * 
 * @author Administrator
 * 
 */
public class NetConnectUtils {

	public  static final int LONGITUDE_LATITUDE = 100;
	public static final int ERROR = 101;

	public static int getNetType(Context context) {
		ConnectivityManager manager = (ConnectivityManager) context
				.getSystemService(context.CONNECTIVITY_SERVICE);
		NetworkInfo activeNetInfo = manager.getActiveNetworkInfo();// 获取网络的连接情

        if (activeNetInfo != null) {
            if (activeNetInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                // 判断WIFI网
                return ConnectivityManager.TYPE_WIFI;
            } else if (activeNetInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
                // 判断gps网
                return ConnectivityManager.TYPE_MOBILE;
            }
        }

		return -1;
	}

	public static void onRequestLocation(final Context context, final Handler handler) {
		final LocationClient mLocationClient = new LocationClient(
				context.getApplicationContext());
		mLocationClient.registerLocationListener(new BDLocationListener() {
			@Override
			public void onReceivePoi(BDLocation db) {
			}

			@Override
			public void onReceiveLocation(BDLocation db) {
				Message msg = handler.obtainMessage();
				if (db != null) {
					// mCustomerInfoEntity.setLatitude(String.valueOf(db
					// .getLatitude()));
					// mCustomerInfoEntity.setLongitude(String.valueOf(db
					// .getLongitude()));
					// FriendHttpUtil.getFriends(mCustomerInfoEntity, 3,
					// FriendMainActivity.this);
					msg.obj = db.getLongitude() + " " + db.getLatitude();
					msg.what = LONGITUDE_LATITUDE;
					handler.sendMessage(msg);
				} else {
					msg.what = ERROR;
					msg.obj = "没有获取到经纬度";
					handler.sendMessage(msg);
				}
				if (mLocationClient != null &&  mLocationClient.isStarted()) {
					mLocationClient.unRegisterLocationListener(this);
					mLocationClient.stop();
				}
			}
		});
		LocationClientOption clientOption = new LocationClientOption();
		clientOption.setOpenGps(true);
		clientOption.setCoorType("gps");
		clientOption.setPoiNumber(3);
		clientOption.setAddrType("detail");
		clientOption.setServiceName("com.baidu.location.service_v3.3");
		clientOption.setPoiExtraInfo(true);
		clientOption.setPriority(LocationClientOption.NetWorkFirst);
		clientOption.disableCache(true);
		clientOption.setPoiDistance(1000);
		clientOption.setScanSpan(6000);
		mLocationClient.setLocOption(clientOption);
		mLocationClient.start();
		mLocationClient.requestLocation();
	}

}
