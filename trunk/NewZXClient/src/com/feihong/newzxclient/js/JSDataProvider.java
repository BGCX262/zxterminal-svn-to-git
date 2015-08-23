/**
 * 
 */
package com.feihong.newzxclient.js;

import android.util.Log;
import android.webkit.JavascriptInterface;
//import android.webkit.JavascriptInterface;

/**
 * @author Administrator
 *
 */
public class JSDataProvider {
	
	private double m_dLongtitude;	//经度
	private double m_dLatitude;		//纬度
	private double m_dSpeed;		//速度
	private double m_dElevation;	//海拔
	private double m_dDirection;	//方向
	private double m_dRadius;		//半径
	private String m_sWarning;      //防碰撞警告
	private String m_sTarget;       //目标
	
	@JavascriptInterface
	public void SetLongtitude(double dLongtitude)
	{
		m_dLongtitude = dLongtitude;
	}

	@JavascriptInterface
    public void SetLatitude(double dLatitude)
	{
		m_dLatitude = dLatitude;
	}

	@JavascriptInterface
    public void SetSpeed(double dSpeed)
	{
		m_dSpeed = dSpeed;
	}

	@JavascriptInterface
    public void SetElevation(double dElevation)
	{
		m_dElevation = dElevation;
	}

	@JavascriptInterface
    public void SetRotation(double dDirection)
	{
		m_dDirection = dDirection;
	}

	@JavascriptInterface
    public void SetRadius(double dRadius)
	{
		m_dRadius = dRadius;
	}

	@JavascriptInterface
    public double GetLongtitude() {
		return m_dLongtitude;
	}

	@JavascriptInterface
    public double GetLatitude() {
		return m_dLatitude;
	}

	@JavascriptInterface
    public double GetSpeed()
	{
		return m_dSpeed;
	}

	@JavascriptInterface
    public double GetElevation()
	{
		return m_dElevation;
	}

	@JavascriptInterface
    public double GetRotation()
	{
		return m_dDirection;
	}

	@JavascriptInterface
    public double GetRadius()
	{
		return m_dRadius;
	}
	
	@JavascriptInterface
    public String GetWarning()
	{
		return m_sWarning;
	}

	@JavascriptInterface
    public void SetWarning(String sWarning)
	{
		m_sWarning = sWarning;
	}
	
	@JavascriptInterface
    public String GetTarget()
	{
		return m_sTarget;
	}

	@JavascriptInterface
    public void SetTarget(String sTarget)
	{
		m_sTarget = sTarget;
	}
}

