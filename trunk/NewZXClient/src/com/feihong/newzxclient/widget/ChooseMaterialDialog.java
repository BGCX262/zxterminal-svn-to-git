package com.feihong.newzxclient.widget;

import java.util.ArrayList;

import zaoxun.Msg.Material;

import android.content.Context;
import android.view.*;
import android.widget.*;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.util.DisplayUtils;

public class ChooseMaterialDialog extends BaseDialog {

    private GridView mGridView;
    private ArrayList<Material> mMaterials;
    private String mSelectedMaterial;
    private OnMaterialItemSelectedListener mListener;

    public interface OnMaterialItemSelectedListener {
        public void onMaterialItemSelected(Material material);
    }

    /**
     * @param context 上下文对象
     */
    public ChooseMaterialDialog(Context context, OnMaterialItemSelectedListener listener) {
        super(context);
        mListener = listener;
        mGridView = (GridView) View.inflate(context, R.layout.material_dialog, null);
        setContentView(mGridView);
        setTitle(R.string.material_dialog_title);
        //mMaterials = context.getResources().getStringArray(R.array.material);
        mMaterials = new ArrayList<Material>();
        //mSelectedMaterial = mMaterials.get(0).getChineseName();
        mGridView.setAdapter(mAdapter);
        mGridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mListener.onMaterialItemSelected(mMaterials.get(position));
                dismiss();
            }
        });

        setWidth((int) (DisplayUtils.getWidthPixels() * 0.5f));
    }

    public void setSelectItem(String material) {
       mSelectedMaterial = material;
       mAdapter.notifyDataSetChanged();
    }
    
    public void setDataSet(ArrayList<Material> materials) {
    	mMaterials = materials;
        mAdapter.notifyDataSetChanged();
     }

    private BaseAdapter mAdapter = new BaseAdapter() {
        @Override
        public int getCount() {
            return mMaterials != null ? mMaterials.size() : 0;
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = View.inflate(getContext(), R.layout.material_item, null);
            }
            TextView textView = (TextView) convertView;
            textView.setText(mMaterials.get(position).getChineseName());
            textView.setTag(mMaterials.get(position).getMaterialId());
            textView.setSelected(mSelectedMaterial.equals(mMaterials.get(position).getChineseName()));
            textView.setBackgroundResource(mSelectedMaterial.equals(mMaterials.get(position).getChineseName())
                    ? R.drawable.shape_material_item_selected_bg : R.drawable.shape_query_item_bg);
            return convertView;
        }
    };
}
