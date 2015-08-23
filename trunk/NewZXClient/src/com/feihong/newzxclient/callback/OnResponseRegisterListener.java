package com.feihong.newzxclient.callback;

import zaoxun.Msg;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public interface OnResponseRegisterListener {
    /**
     * @param msg msg
     */
    public void responseRegister(Msg.LoginResponse msg);
}
