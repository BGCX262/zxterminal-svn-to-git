package com.feihong.newzxclient.widget;

import android.content.Context;
import android.graphics.*;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.webkit.WebView;

/**
 * Created with IntelliJ IDEA.
 * User: Administrator
 * Date: 13-11-16
 * Time: ????4:57
 * To change this template use File | Settings | File Templates.
 */
public class DrawLineWebView extends WebView {

    float preX;
    float preY;
    private Path path;
    public Paint paint = null;
    Bitmap cacheBitmap = null;
    Canvas cacheCanvas = null;
    private int mScreenWidth;
    private int mScreenHeight;

    private boolean mIsDrawing = false;

    public DrawLineWebView(Context context) {
        super(context);
    }

    public DrawLineWebView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mScreenWidth = context.getResources().getDisplayMetrics().widthPixels;
        mScreenHeight = context.getResources().getDisplayMetrics().heightPixels;
        cacheBitmap = Bitmap.createBitmap(mScreenWidth, mScreenHeight,
                Bitmap.Config.ARGB_4444);
        cacheCanvas = new Canvas();

        path = new Path();
        cacheCanvas.setBitmap(cacheBitmap);

        paint = new Paint(Paint.DITHER_FLAG);
        paint.setColor(Color.RED);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(3);
        paint.setAntiAlias(true);
        paint.setDither(true);
    }

    public void clearCache() {
        if (!mIsDrawing) {
            if (!cacheBitmap.isRecycled()) {
                cacheBitmap.recycle();
                cacheBitmap = null;
            }
            cacheBitmap = Bitmap.createBitmap(mScreenWidth, mScreenHeight,
                    Bitmap.Config.ARGB_4444);
            cacheCanvas.setBitmap(cacheBitmap);
            invalidate();
        }
    }

    public void triggerDrawing() {
        mIsDrawing = !mIsDrawing;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

            float x = event.getX() + getScrollX();
            float y = event.getY() + getScrollY();

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    if (!mIsDrawing) {
                         clearCache();
                    }
                    path.moveTo(x, y);
                    preX = x;
                    preY = y;
                    break;
                case MotionEvent.ACTION_MOVE:
                    if (mIsDrawing) {
                        path.quadTo(preX, preY, x, y);
                        preX = x;
                        preY = y;
                    }
                    break;
                case MotionEvent.ACTION_UP:
                    if (mIsDrawing) {
                        cacheCanvas.drawPath(path, paint);
                        path.reset();
                    }
                    mIsDrawing = false;
                    break;
                default:
                    break;
            }
            invalidate();
        return mIsDrawing || super.onTouchEvent(event);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Paint bmpPaint = new Paint();
        canvas.drawBitmap(cacheBitmap, 0, 0, bmpPaint);
        canvas.drawPath(path, paint);
    }

}
