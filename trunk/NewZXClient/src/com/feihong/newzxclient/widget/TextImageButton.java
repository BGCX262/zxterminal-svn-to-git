package com.feihong.newzxclient.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.widget.ImageButton;

public class TextImageButton extends ImageButton {
	private String text = null;  //Ҫ��ʾ������
    private int color;           //���ֵ���ɫ

    public TextImageButton(Context context, AttributeSet attrs) {
        super(context,attrs);
        
    }

    public void setText(String text) {
        this.text = text;       //��������
    }
    
    public String getText() {
        return this.text;       //��ȡ����
    }

    public void setColor(int color) {
        this.color = color;    //����������ɫ
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //Paint paint = new Paint();
        //paint.setTextAlign(Paint.Align.CENTER);
        //paint.setColor(color);
        //canvas.drawText(text, 15, 20, paint);  //��������
        
        Paint textPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.DEV_KERN_TEXT_FLAG);// ���û���
		textPaint.setTextSize(60.0f);// �����С
		textPaint.setTypeface(Typeface.DEFAULT_BOLD);// ����Ĭ�ϵĿ��
		textPaint.setTextAlign(Paint.Align.CENTER);
		textPaint.setColor(color);// ���õ���ɫ
		canvas.drawText(text, 15, 20, textPaint);// ������ȥ�֣���ʼδ֪x,y������ֻ�ʻ���
    }

}
