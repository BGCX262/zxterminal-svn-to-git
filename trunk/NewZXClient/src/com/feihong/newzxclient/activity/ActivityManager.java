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
 * Activity�Ĺ���
 * @author hu.cao
 * @version 1.0.0
 */
public class ActivityManager {

    /** ������ջ�������Activity */
    private Set<Activity> mActivities = new HashSet<Activity>();
    /** ��ǰ��ʾ��Activity */
    private Activity mCurrentActivity = null;
    /** ջ��Activity */
    private Activity mLastActivity = null;
    /** ջ��Activity����һ��activity�������ת������Ӧ�û����Ժ�LastActivity��CurrentActivity��һ�������� */
    private Activity mSavedLastActivity = null;

    private static ActivityManager sInstance;

    /**
     * ��ȡActivityManagerʵ��
     * @return ActivityManagerʵ��
     */
    public static ActivityManager instance() {
        if (sInstance == null) {
            sInstance = new ActivityManager();
        }
        return sInstance;
    }

    /**
     * ��Activityִ��onCreateʱ���� - ����������Activity
     * @param activity ִ��onCreate��Activity
     */
    public void onCreate(Activity activity) {
        mActivities.add(activity);
    }

    /**
     * ��Activityִ��onDestroyʱ���� - �Ƴ����ٵ�Activity
     * @param activity ִ��onDestroyʱ��Activity
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
     * �ر�����activity
     */
    public void finishActivities() {
        for (Activity activity : mActivities) {
            activity.finish();
        }
        mActivities.clear();
    }

    /**
     * ��Activityִ��onResumeʱ���� - ���浱ǰ��ʾ��activity������ջ��Activity
     * @param activity ִ��onResume��Activity
     */
    public void onResume(Activity activity) {
        mCurrentActivity = activity;

        //���LastActivity��CurrentActivity��ͬ����˵���Ǵ�����Ӧ�÷���������ʱLastActivityӦ��ȡǰһ�ε�Activity������͵�ǰActivity��һ��
        if (mLastActivity == activity) {
            mLastActivity = mSavedLastActivity;
        }
    }

    /**
     * ��Activityִ��onPauseʱ���� - �����ǰ��ʾ��Activity
     * @param activity ִ��onPause��Activity
     */
    public void onPause(BaseActivity activity) {
        mSavedLastActivity = mLastActivity;
        mLastActivity = activity;
    }

    /**
     * ��ȡ��ǰ��ʾ��Activity
     * @return ��ǰ��ʾ��Activity������Ϊ��
     */
    public Activity getCurrentActivity() {
        return mCurrentActivity;
    }

    /**
     * ��ȡջ����Activity
     * @return ջ����Activity
     */
    public Activity getLastActivity() {
        return mLastActivity;
    }

    /**
     * ��ȡ���е�Activities
     * @return Activities
     */
    public Set<Activity> getActivities() {
        return mActivities;
    }

    /**
     * �ж�ĳ��activity�Ƿ��ǵ�ǰactivity
     * @param activity  activity
     * @return  �ǲ��ǵ�ǰ��ʾ��activity
     */
    public boolean isCurrentActivity(BaseActivity activity) {
        return activity != null && activity == getCurrentActivity();
    }

    /**
     * ��ȡMainService
     * @return MainService
     */
    public MainService getMainService() {
        if (mCurrentActivity instanceof BaseActivity) {
            return ((BaseActivity)mCurrentActivity).getMainService();
        }
        return null;
    }
}

