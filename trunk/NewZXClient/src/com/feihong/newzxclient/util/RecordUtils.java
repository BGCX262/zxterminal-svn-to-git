package com.feihong.newzxclient.util;

import android.content.Context;
import android.media.*;
import android.os.Environment;
import android.text.format.Time;
import android.util.Log;
import android.widget.Toast;
import com.feihong.newzxclient.R;
import com.feihong.newzxclient.config.ZXConfig;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

/**
 * Created with IntelliJ IDEA.
 * User: Administrator
 * Date: 13-11-18
 * Time: 下午10:17
 * To change this template use File | Settings | File Templates.
 */
public class RecordUtils {

    static {
        System.loadLibrary("mp3lame");
    }
    private native void initEncoder(int numChannels, int sampleRate, int bitRate, int mode, int quality);
    private native void destroyEncoder();
    private native int encodeFile(String sourcePath, String targetPath);

    public  static final String TAG = "ZAO_LOG";
    private static final String SERVER_ADDRESS = "http://42.121.132.124/";
    public static final int NUM_CHANNELS = 1;		/* 默认也是2 */
    public static final int SAMPLE_RATE = 16000;	/* 采样率 */
    public static final int BITRATE = 32;			/* 比特率 */
    public static final int MODE = 1;				/* mode = 0,1,2,3 = stereo,jstereo,dualchannel(not supported),mono default */
    public static final int QUALITY = 5;			/* 2=high  5 = medium  7=low */

    private MediaPlayer mMediaPlayer;
    private MediaRecorder mMediaRecorder;
    private String mMediaFileName;
    private SoundPool mSoundPool;
    private int mSoundId;
    private AudioRecord mRecorder;
    private short[] mBuffer;
    private File mRawFile;
    private File mEncodedFile;
    private String mLogFileName;
    boolean isRec;  //是否正在录音
    private boolean mIsMedia = false;

    public RecordUtils(Context context, boolean isMedia) {
        mSoundPool = new SoundPool(10, AudioManager.STREAM_MUSIC, 0);
        mSoundId   = mSoundPool.load(context, R.raw.beep, mSoundId);
        mLogFileName = ZXConfig.getLogPath("log");
        mIsMedia = isMedia;
        initRecorder();
        mMediaRecorder = new MediaRecorder();
/*        File file = new File(mLogFileName);
        if(!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }*/
    }

    //=============================================
    // MP3 录音、编码
    //=============================================
    private void initRecorder() {

        if(!mIsMedia) {
            int bufferSize = AudioRecord.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
            mBuffer   = new short[bufferSize];
            mRecorder = new AudioRecord(MediaRecorder.AudioSource.MIC, SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize);
            writeLog("Init recoder with wav format, use for mp3 encoding");

            //init mp3
            initEncoder(NUM_CHANNELS, SAMPLE_RATE, BITRATE, MODE, QUALITY);
        }
        else {
            writeLog("Init recoder with mp4 format, use for aac encoding");
        }
    }

    public void playBeep() {
        if (mSoundPool != null) {
            mSoundPool.play(mSoundId, 1, 1, 0, 0, 1);
        }
    }

    /**
     *  录音 + 拍照
     */
    public void beginRecorder() throws Throwable {

        isRec = true;

        if(!mIsMedia)
        {
            mRecorder.startRecording();
            mRawFile = new File(ZXConfig.getRecordPath("raw"));
            startBufferedWrite(mRawFile);
            writeLog("Prepare raw recorder");
        }
        else {

            mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);  //设置麦克风
            mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);//设置输出格式: THREE_GPP
            mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);  //设置音频编码Encoder
            mMediaFileName = ZXConfig.getRecordPath("mp4");
            mMediaRecorder.setOutputFile(mMediaFileName);  //设置音频文件保存路径
            writeLog("Prepare mp4 recorder");
            try {
                mMediaRecorder.prepare();
            } catch (IllegalStateException e) {
                isRec = false;
                writeLog("Begin mp4 error: IllegalStateException");
                e.printStackTrace();
            } catch (IOException e) {
                isRec = false;
                e.printStackTrace();
                writeLog("Begin mp4 error: IOException");
            }
            mMediaRecorder.start();  //开始录制
            writeLog("Start mp4 recorder...");
        }
    }

    /**
     * stopRecorder
     * @return
     */
    public String stopRecorder() {

        isRec = false;

        if(!mIsMedia) {

            mRecorder.stop();
            mEncodedFile = new File(ZXConfig.getRecordPath("mp3"));
            int result = encodeFile(mRawFile.getAbsolutePath(), mEncodedFile.getAbsolutePath());
            if (result == 0) {
                writeLog("Encoded to " + mEncodedFile.getName() + " success");
                writeLog("Stop mp3 recorder success");
                return mEncodedFile != null ? mEncodedFile.getAbsolutePath() : null;
            }
            else {
                writeLog("Encoded to " + mEncodedFile.getName() + " failed");
                return null;
            }
        }
        else {
            writeLog("Start to stop mp4 recorder...");
            try {
                mEncodedFile = new File(mMediaFileName);
                mMediaRecorder.stop();
                writeLog("Stop mp4 recorder success, file name = " + mEncodedFile.getName());
            } catch (RuntimeException  e) {
                mMediaRecorder.reset();
//				mEncodedFile.deleteOnExit();
                if(mEncodedFile != null)
                    mEncodedFile.delete();
                writeLog("Stop mp4 recorder failed, catch RuntimeException,delete the file");
                return null;
            }
            return mEncodedFile != null ? mEncodedFile.getAbsolutePath() : null;
        }
    }

    public void deleteEncodeFile() {
        if(mEncodedFile != null) {
            mEncodedFile.delete();
        }
    }

    private void startBufferedWrite(final File file) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                DataOutputStream output = null;
                try {
                    output = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(file)));
                    while (isRec) {
                        int readSize = mRecorder.read(mBuffer, 0, mBuffer.length);
                        for (int i = 0; i < readSize; i++) {
                            output.writeShort(mBuffer[i]);
                        }
                    }
                } catch (IOException e) {
                    PromptUtils.showToast(R.string.try_again, Toast.LENGTH_LONG);
                } finally {
                    if (output != null) {
                        try {
                            output.flush();
                        } catch (IOException e) {
                            PromptUtils.showToast(R.string.try_again, Toast.LENGTH_LONG);
                        } finally {
                            try {
                                output.close();
                            } catch (IOException e) {
                                PromptUtils.showToast(R.string.try_again, Toast.LENGTH_LONG);
                            }
                        }
                    }
                }
            }
        }).start();
    }

    public void release() {
        if(mMediaPlayer != null)
            mMediaPlayer.release();
        if(mMediaRecorder != null)
            mMediaRecorder.release();
        if(mSoundPool != null)
            mSoundPool.release();
        if(mRecorder != null)
            mRecorder.release();
        //destroyEncoder();
    }

    public boolean isRecording() {
        return isRec;
    }

    public void writeLog(String text){
/*        Log.d(TAG, text);
        //System.out.println("======" + TAG + "======" + text);
        try {
            FileOutputStream stream = new FileOutputStream(mLogFileName, true);
            Time time = new Time();
            time.setToNow();
            String log = time.format("%Y-%m-%d %H:%M:%S  ") + text +" \r\n";
            byte[] buffer = log.getBytes();
            stream.write(buffer);
            stream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }*/
    }
}
