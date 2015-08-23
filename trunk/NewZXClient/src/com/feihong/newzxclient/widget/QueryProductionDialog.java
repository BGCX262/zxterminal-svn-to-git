package com.feihong.newzxclient.widget;

import android.content.Context;
import android.view.View;
import com.feihong.newzxclient.R;

public class QueryProductionDialog extends BaseDialog{

    /**
     * ���캯�� - ���͵�һ��������ʾ
     * @param context �����Ķ���
     */
    public QueryProductionDialog(Context context, View.OnClickListener dutyListener, View.OnClickListener dateListener
            , View.OnClickListener monthListener) {
        super(context);
        setContentView(R.layout.querydialog);
        setTitle(R.string.dialog_title);
        findViewById(R.id.btn_production).setOnClickListener(dutyListener);
        findViewById(R.id.btn_distance).setOnClickListener(dateListener);
        findViewById(R.id.btn_times).setOnClickListener(monthListener);
    }

}
