package com.feihong.newzxclient.callback;

import zaoxun.Msg;

public interface OnResponseCheckListener {
	public void responseCheck(Msg.CheckResponse msg);
	public void responseDeviceStatusRequest(Msg.DeviceStatusRequest msg);
}
