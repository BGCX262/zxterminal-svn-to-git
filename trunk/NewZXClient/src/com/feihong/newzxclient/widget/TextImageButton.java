package com.feihong.newzxclient.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.widget.ImageButton;

public class TextImageButton extends ImageButton {
	private String text = null;  //要显示的文字
    private int color;           //文字的颜色

    public TextImageButton(Context context, AttributeSet attrs) {
        super(context,attrs);
        
    }

    public void setText(String text) {
        this.text = text;       //设置文字
    }
    
    public String getText() {
        return this.text;       //获取文字
    }

    public void setColor(int color) {
        this.color = color;    //设置文字颜色
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //Paint paint = new Paint();
        //paint.setTextAlign(Paint.Align.CENTER);
        //paint.setColor(color);
        //canvas.drawText(text, 15, 20, paint);  //绘制文字
        
        Paint textPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.DEV_KERN_TEXT_FLAG);// 设置画笔
		textPaint.setTextSize(60.0f);// 字体大小
		textPaint.setTypeface(Typeface.DEFAULT_BOLD);// 采用默认的宽度
		textPaint.setTextAlign(Paint.Align.CENTER);
		textPaint.setColor(color);// 采用的颜色
		canvas.drawText(text, 15, 20, textPaint);// 绘制上去字，开始未知x,y采用那只笔绘制
    }

}
