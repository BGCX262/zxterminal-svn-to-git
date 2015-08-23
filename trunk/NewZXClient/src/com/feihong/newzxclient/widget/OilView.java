package com.feihong.newzxclient.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.widget.ImageView;

public class OilView extends ImageView{
	public float mPoint =1;
	public Paint mPaint;

	public OilView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mPaint = new Paint();
		mPaint.setColor(Color.RED);
		// TODO Auto-generated constructor stub
	}
	public OilView(Context context, AttributeSet attrs) {
		this(context, attrs,1);
		// TODO Auto-generated constructor stub
	}
	public OilView(Context context) {
		this(context,null);
		// TODO Auto-generated constructor stub
	}
	public void setPoint(float point){
		mPoint = 1 - point;
		invalidate();
	}
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		canvas.drawRect(0, getHeight()*mPoint, getWidth(), getHeight(), mPaint);
	}
}
