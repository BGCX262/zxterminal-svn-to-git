package com.feihong.newzxclient.widget;

import java.util.ArrayList;

import zaoxun.Msg.EDipperInfo;
import android.content.Context;
import android.view.*;
import android.widget.*;
import android.widget.ImageView.ScaleType;

import com.feihong.newzxclient.R;
import com.feihong.newzxclient.util.DisplayUtils;

public class LoadRequestDialog extends BaseDialog {

    private GridView mGridView;
    private EDipperInfo[] mEdippers;
    private ArrayList<EDipperInfo> mEdippersList = new ArrayList<EDipperInfo>();
    private EDipperInfo mSelectedEdipper;
    private OnEdipperItemSelectedListener mListener;

    public interface OnEdipperItemSelectedListener {
        public void onEdipperItemSelected(EDipperInfo edipper);
    }

    /**
     * @param context 上下文对象
     */
    public LoadRequestDialog(Context context, OnEdipperItemSelectedListener listener) {
        super(context);
        mListener = listener;
        mGridView = (GridView) View.inflate(context, R.layout.load_request_dialog, null);
        setContentView(mGridView);
        setTitle(R.string.edipper_select_dialog_title);
        //mMaterials = context.getResources().getStringArray(R.array.material);
        mEdippers = new EDipperInfo[2];
        if (mEdippersList.size() > 0) {
        	mSelectedEdipper = mEdippersList.get(0);
        }
        //mSelectedEdipper = mEdippers[0];
        mGridView.setAdapter(mAdapter);
        mGridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mListener.onEdipperItemSelected(mEdippersList.get(position));
                dismiss();
            }
        });

        setWidth((int) (DisplayUtils.getWidthPixels() * 0.5f));
    }

    public void setSelectItem(EDipperInfo material) {
    	mSelectedEdipper = material;
       mAdapter.notifyDataSetChanged();
    }
    
    public void setDataSet(ArrayList<EDipperInfo> materials) {
    	mEdippersList = materials;
        //mAdapter.notifyDataSetChanged();
     }

    private BaseAdapter mAdapter = new BaseAdapter() {
        @Override
        public int getCount() {
            return mEdippersList != null ? mEdippersList.size() : 0;
        }

        @Override
        public Object getItem(int position) {
            return mEdippersList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = View.inflate(getContext(), R.layout.edipper_item, null);
            }
            
            Button btn = (Button) convertView;
            
            if (position >= mEdippersList.size() || mEdippersList.get(position) == null) {
            	return null;
            }
            
            btn.setText(mEdippersList.get(position).getDeviceNo());

/*            textView.setSelected(mSelectedEdipper.equals(mEdippers[position]));
            textView.setBackgroundResource(mSelectedEdipper.equals(mEdippers[position])
                    ? R.drawable.shape_material_item_selected_bg : R.drawable.shape_query_item_bg);*/
            return convertView;
        }
    };
}
