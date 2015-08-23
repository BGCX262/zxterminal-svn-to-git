package com.feihong.newzxclient.widget;

import android.content.Context;
import android.view.View;
import android.widget.TextView;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.config.ZXConfig;
import com.feihong.newzxclient.util.DisplayUtils;

public class ProductionInfoDialog extends BaseDialog{

	private TextView mTimes;
	private TextView mDistance;
	private TextView mMine;
	private TextView mNonMine;
	private TextView mTemp;
	
    /**
     * 构造函数 - 赠送第一份礼物提示
     * @param context 上下文对象
     */
    public ProductionInfoDialog(Context context) {
        super(context);
        setContentView(R.layout.production_info_dialog);
        setTitle(R.string.production_info);
        setWidth((int)(DisplayUtils.getWidthPixels() * 0.5f));
        
        mTimes = (TextView)this.findViewById(R.id.run_times);  
        mDistance = (TextView)this.findViewById(R.id.run_distance);
        mMine = (TextView)this.findViewById(R.id.mine_production);
        mNonMine = (TextView)this.findViewById(R.id.non_mine_production);
        mTemp = (TextView)this.findViewById(R.id.temp_production);
        
        UpdateData();
    }
    
    public void UpdateData() {
    	mTimes.setText(String.valueOf(ZXConfig.getTimes()));
    	mDistance.setText(String.valueOf((int)ZXConfig.getDistance()));
    	mMine.setText(String.valueOf(ZXConfig.getFill()));
    	mNonMine.setText(String.valueOf(ZXConfig.getTon()));
    	mTemp.setText(String.valueOf(ZXConfig.getTemp()));
    }

}
