package com.feihong.newzxclient.widget;

import android.app.Dialog;
import android.content.Context;
import android.view.*;
import android.widget.LinearLayout;
import android.widget.TextView;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.util.DisplayUtils;

/**
 * baseDialog
 * @author hu.cao
 * @version 1.0.0
 */
public class BaseDialog extends Dialog {

    private LinearLayout mLayoutWrapper;

    protected BaseDialog(Context context) {
        super(context, R.style.Theme_NoTitle_Dialog);
        setCanceledOnTouchOutside(true);
        mLayoutWrapper = (LinearLayout) View.inflate(context, R.layout.base_dailog, null);
    }

    private void setLayoutParams() {
        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.width = (int) (DisplayUtils.getWidthPixels() * 0.75f);
        params.height = WindowManager.LayoutParams.WRAP_CONTENT;
        params.gravity = Gravity.CENTER;
        window.setAttributes(params);
    }

    public void setWidth(int width) {
        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.width = width;
        window.setAttributes(params);
    }

    @Override
    public void setContentView(int layoutResID) {
        View view = View.inflate(getContext(), layoutResID, null);
        mLayoutWrapper.addView(view);
        super.setContentView(mLayoutWrapper);
        setLayoutParams();
    }

    @Override
    public void setContentView(View view) {
        mLayoutWrapper.addView(view);
        super.setContentView(mLayoutWrapper);

        setLayoutParams();
    }

    @Override
    public void setContentView(View view, ViewGroup.LayoutParams params) {
        mLayoutWrapper.addView(view, params);
        super.setContentView(mLayoutWrapper);

        setLayoutParams();
    }

    @Override
    public void setTitle(CharSequence title) {
        ((TextView)mLayoutWrapper.findViewById(R.id.id_dialog_title_text)).setText(title);
    }

    @Override
    public void setTitle(int titleId) {
        setTitle(getContext().getString(titleId));
    }
}
