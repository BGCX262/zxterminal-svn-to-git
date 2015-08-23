package com.feihong.newzxclient.config;

import android.content.Context;
import android.os.Environment;
import android.text.format.Time;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;

import com.feihong.newzxclient.bean.Constant.LoadStatus;

import zaoxun.Msg.CarType;
import zaoxun.Msg.LoginResponse;
import zaoxun.Msg.Material;
import zaoxun.Msg.RunningStatus;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public class ZXConfig {

    private static String sDeviceId = "";
    private static String mRecordPath = "zaoxunlog/record";
    private static String mLogPath = "zaoxunlog/log";
    
    private static String mCarNo;
    private static CarType mCarType;
    private static RunningStatus mRunningStatus = RunningStatus.RUNNING;
    private static String sEdipperID;
    private static int connStatus = 0;

    private static ArrayList<Material> materialList;
    private static int mMaterialID;
    private static String mMaterialName;
    
    private static int mDriverID;
    private static String mDriverName;
    
    private static LoadStatus mLoadStatus;
    private static double mSpeedLimit;
    
    private static int mTimes = 0;
    private static double mDistance = 0;
    private static double mTon = 0;
    private static double mFill = 0;
    private static double mTemp = 0;
    
    private static String mFuelCarID;
    private static double mFuelVolume;
    
    private static long mLastRecvHBTime = 0;
    //private static Date mFuelVolume;
    
    private static int mLoginStatus = 0;
    
    public static void init(Context context) {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            mRecordPath = Environment.getExternalStorageDirectory() + File.separator + mRecordPath + File.separator;
            mLogPath = Environment.getExternalStorageDirectory() + File.separator + mLogPath + File.separator;
        } else {
            mRecordPath = context.getFilesDir() + File.separator + mRecordPath + File.separator;
            mLogPath = context.getFilesDir() + File.separator + mLogPath + File.separator;
        }
        new File(mRecordPath).mkdirs();
        new File(mLogPath).mkdirs();
    }

    public static void setDeviceId(String deviceId) {
        sDeviceId = deviceId;
    }

    public static String getDeviceId() {
        return sDeviceId;
    }
    
    public static void setEdipperId(String deviceId) {
        sEdipperID = deviceId;
    }

    public static String getEdipperId() {
        return sEdipperID;
    }
    
    public static void setCarNo(String carNo) {
        mCarNo = carNo;
    }

    public static String getCarNo() {
        return mCarNo;
    }
    
    public static void setCarType(CarType carType) {
        mCarType = carType;
    }

    public static CarType getCarType() {
        return mCarType;
    }
    
    public static void setRunningStatus(RunningStatus runningStatus) {
    	mRunningStatus = runningStatus;
    }

    public static RunningStatus getRunningStatus() {
        return mRunningStatus;
    }
    
    public static void setconnStatus(int status) {
    	connStatus = status;
    }

    public static int getconnStatus() {
        return connStatus;
    }
    
    public static void setMaterialList(ArrayList<Material> list) {
    	materialList = list;
    }

    public static ArrayList<Material> getMaterialList() {
        return materialList;
    }
    
    public static void setMaterialID(int id) {
    	mMaterialID = id;
    }

    public static int getMaterialID() {
        return mMaterialID;
    }
    
    public static void setMaterialName(String name) {
    	mMaterialName = name;
    }

    public static String getMaterialName() {
        return mMaterialName;
    }

    public static void setDriverID(int id) {
    	mDriverID = id;
    }

    public static int getDriverID() {
        return mDriverID;
    }
    
    public static void setDriverName(String name) {
    	mDriverName = name;
    }

    public static String getDriverName() {
        return mDriverName;
    }
    
    public static void setLoadStatus(LoadStatus status) {
    	mLoadStatus = status;
    }

    public static LoadStatus getLoadStatus() {
        return mLoadStatus;
    }
    
    public static void setSpeedlimit(double speed) {
    	mSpeedLimit = speed;
    }

    public static double getSpeedlimit() {
        return mSpeedLimit;
    }
    
    
    public static void setTimes(int times) {
    	mTimes = times;
    }

    public static int getTimes() {
        return mTimes;
    }
    
    public static void setDistance(double distance) {
    	mDistance = distance;
    }

    public static double getDistance() {
        return mDistance;
    }
    
    public static void setTon(double ton) {
    	mTon = ton;
    }

    public static double getTon() {
        return mTon;
    }
    
    public static void setFill(double fill) {
    	mFill = fill;
    }

    public static double getFill() {
        return mFill;
    }
    
    public static void setTemp(double temp) {
    	mTemp = temp;
    }

    public static double getTemp() {
        return mTemp;
    }
    
    public static void setFuelCarID(String id) {
    	mFuelCarID = id;
    }

    public static String getFuelCarID() {
        return mFuelCarID;
    }
    
    public static void setFuelVolume(double volume) {
    	mFuelVolume = volume;
    }

    public static double getFuelVolume() {
        return mFuelVolume;
    }
    
    public static void setLastRecvHBTime(long time) {
    	mLastRecvHBTime = time;
    }

    public static long getLastRecvHBTime() {
        return mLastRecvHBTime;
    }
    
    public static String getRecordPath(final String suffix) {
        Time time = new Time();
        time.setToNow();
        return mRecordPath + time.format("%Y%m%d%H%M%S") + "." + suffix;
    }

    public static String getLogPath(final String suffix) {
        Time time = new Time();
        time.setToNow();
        return mLogPath + time.format("%Y%m%d%H%M%S") + "." + suffix;
    }
    
    public static int getLoginStatus() {
    	return mLoginStatus;
    }
    
    public static void setLoginStatus(int status) {
    	mLoginStatus = status;
    }
}
