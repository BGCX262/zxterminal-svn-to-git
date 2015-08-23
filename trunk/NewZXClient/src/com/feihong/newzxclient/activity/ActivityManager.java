package com.feihong.newzxclient.activity;

/**
 * @author hao.xiong
 * @version 1.0.0
 */

import android.app.Activity;
import com.feihong.newzxclient.service.MainService;

import java.util.HashSet;
import java.util.Set;

/**
 * Activity的管理
 * @author hu.cao
 * @version 1.0.0
 */
public class ActivityManager {

    /** 保存在栈里的所有Activity */
    private Set<Activity> mActivities = new HashSet<Activity>();
    /** 当前显示的Activity */
    private Activity mCurrentActivity = null;
    /** 栈顶Activity */
    private Activity mLastActivity = null;
    /** 栈顶Activity的上一个activity，解决跳转到其他应用回来以后LastActivity和CurrentActivity是一样的问题 */
    private Activity mSavedLastActivity = null;

    private static ActivityManager sInstance;

    /**
     * 获取ActivityManager实例
     * @return ActivityManager实例
     */
    public static ActivityManager instance() {
        if (sInstance == null) {
            sInstance = new ActivityManager();
        }
        return sInstance;
    }

    /**
     * 当Activity执行onCreate时调用 - 保存启动的Activity
     * @param activity 执行onCreate的Activity
     */
    public void onCreate(Activity activity) {
        mActivities.add(activity);
    }

    /**
     * 当Activity执行onDestroy时调用 - 移除销毁的Activity
     * @param activity 执行onDestroy时的Activity
     */
    public void onDestroy(Activity activity) {
        if (mLastActivity == activity) {
            mLastActivity = null;
        }

        if (mSavedLastActivity == activity) {
            mSavedLastActivity = null;
        }

        mActivities.remove(activity);
    }

    /**
     * 关闭所有activity
     */
    public void finishActivities() {
        for (Activity activity : mActivities) {
            activity.finish();
        }
        mActivities.clear();
    }

    /**
     * 当Activity执行onResume时调用 - 保存当前显示的activity，更新栈顶Activity
     * @param activity 执行onResume的Activity
     */
    public void onResume(Activity activity) {
        mCurrentActivity = activity;

        //如果LastActivity和CurrentActivity相同，则说明是从其他应用返回来，此时LastActivity应该取前一次的Activity，否则和当前Activity是一个
        if (mLastActivity == activity) {
            mLastActivity = mSavedLastActivity;
        }
    }

    /**
     * 当Activity执行onPause时调用 - 清除当前显示的Activity
     * @param activity 执行onPause的Activity
     */
    public void onPause(BaseActivity activity) {
        mSavedLastActivity = mLastActivity;
        mLastActivity = activity;
    }

    /**
     * 获取当前显示的Activity
     * @return 当前显示的Activity，可能为空
     */
    public Activity getCurrentActivity() {
        return mCurrentActivity;
    }

    /**
     * 获取栈顶的Activity
     * @return 栈顶的Activity
     */
    public Activity getLastActivity() {
        return mLastActivity;
    }

    /**
     * 获取所有的Activities
     * @return Activities
     */
    public Set<Activity> getActivities() {
        return mActivities;
    }

    /**
     * 判断某个activity是否是当前activity
     * @param activity  activity
     * @return  是不是当前显示的activity
     */
    public boolean isCurrentActivity(BaseActivity activity) {
        return activity != null && activity == getCurrentActivity();
    }

    /**
     * 获取MainService
     * @return MainService
     */
    public MainService getMainService() {
        if (mCurrentActivity instanceof BaseActivity) {
            return ((BaseActivity)mCurrentActivity).getMainService();
        }
        return null;
    }
}

