package com.feihong.newzxclient.util;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.SoundPool;

import com.feihong.newzxclient.R;

public class SoundsUtils {

	public static SoundPool soundPool=null;
	public static SoundsUtils soundUtils=null;
	private static final int MAX_PLAY_COUNT=100;
	private float volumnRatio=1;
	
	public static Map<Integer,Integer> soundPoolMap = new HashMap<Integer, Integer>();
	
	public static final int ACCEPT=100;
	public static final int ACCEPT_ONE=101;
	public static final int ACCEPT_TWO=102;
	public static final int ALARM_ONE=103;
	public static final int ALARM_TWO=104;
	public static final int ARRIVED=105;
	public static final int A=106;
	public static final int B=107;
	public static final int C=108;
	public static final int CANCEL=109;
	public static final int CANCELLOAD=110;
	public static final int CHECKEND=111;
	public static final int CHECKING=112;
	public static final int CHOOSELOADER=113;
	public static final int D=114;
	public static final int E=115;
	public static final int F=116;
	public static final int G=117;
	public static final int H=118;
	public static final int I=119;
	public static final int J=120;
	public static final int K=121;
	public static final int L=122;
	public static final int M=123;
	public static final int MATERIAL=124;
	public static final int MATERIALUPDATE=125;
	public static final int N=126;
	public static final int NO=127;
	
	public static final int NOTIFY=128;
	public static final int O=129;
	public static final int OUTSOURCING=130;
	public static final int OVERSPEED=131;
	public static final int P=132;
	public static final int Q=133;
	public static final int QUIT=134;
	public static final int QUITSTATUS=135;
	public static final int R1=136;
	public static final int S=137;
	public static final int SUBMIT=138;
	public static final int T=139;
	public static final int U=140;
	public static final int UNLOAD=141;
	public static final int UPDATE=142;
	public static final int V=143;
	public static final int VOICEFAIL=144;
	public static final int VOICEOK=145;
	public static final int W=146;
	public static final int WELCOME=147;
	public static final int X=148;
	public static final int Y=149;
	public static final int YES=150;
	public static final int Z=151;
	
	public static final int LOADDONE=152;
	public static final int REBOOT=153;
	
	private  Context context;
	private static MediaPlayer mediaPlayer;
	public SoundsUtils(Context context){
		this.context=context;
		initSoundPool();
		getVolumnRatio(context);
	}
	public static void initMediaPlayer(){
		if(mediaPlayer==null){
			mediaPlayer=new MediaPlayer();
		}
	}
	public static SoundsUtils shareInstance(Context context){
		if(soundUtils==null){
			soundUtils=new SoundsUtils(context);
		}
		return soundUtils;
	}
	
	public void getVolumnRatio(Context context){
		   //实例化AudioManager对象，控制声音
	    AudioManager am = (AudioManager)context.getSystemService(context.AUDIO_SERVICE);
	    //最大音量
	    float audioMaxVolumn = am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
	    //当前音量
	    float audioCurrentVolumn = am.getStreamVolume(AudioManager.STREAM_MUSIC);
	    volumnRatio = audioCurrentVolumn/audioMaxVolumn;
	    
	}
	
	public void setMute(boolean flag){
	    AudioManager am = (AudioManager)context.getSystemService(context.AUDIO_SERVICE);
	    am.setStreamMute(AudioManager.STREAM_MUSIC, flag);
	    
	    getVolumnRatio(this.context);
	}
	
	public static enum SoundsNumber{
		ZERO_SOUND(0,R.raw.zero),
		ONE_SOUND(1,R.raw.one),
		TWO_SOUND(2,R.raw.two),
		THREE_SOUND(3,R.raw.three),
		FOUR_SOUND(4,R.raw.four),
		FIVE_SOUND(5,R.raw.five),
		SIX_SOUND(6,R.raw.six),
		SEVEN_SOUND(7,R.raw.seven),
		EIGHT_SOUND(8,R.raw.eight),
		NINE_SOUND(9,R.raw.nine);
		public int tag;
		public int raw;
		
		public int getTag() {
			return tag;
		}

		public void setTag(int tag) {
			this.tag = tag;
		}

		public int getRaw() {
			return raw;
		}

		public void setRaw(int raw) {
			this.raw = raw;
		}

		
	
		private SoundsNumber(int tag,int raw){
			this.tag=tag;
			this.raw=raw;
		}
		public static Map<Integer,Integer> raws = new HashMap<Integer, Integer>();

		
		public  static void initNumberRaw(Context context){
			for(SoundsNumber s:values()){
				raws.put(s.tag, soundPool.load(context, s.getRaw(), 1));
			}
		}
	}
	

	
	public static enum Sounds{
	
		
		ZERO_SOUND(0,R.raw.zero),
		ONE_SOUND(1,R.raw.one),
		TWO_SOUND(2,R.raw.two),
		THREE_SOUND(3,R.raw.three),
		FOUR_SOUND(4,R.raw.four),
		FIVE_SOUND(5,R.raw.five),
		SIX_SOUND(6,R.raw.six),
		SEVEN_SOUND(7,R.raw.seven),
		EIGHT_SOUND(8,R.raw.eight),
		NINE_SOUND(9,R.raw.nine),
		
		ACCEPT_SOUND(ACCEPT,R.raw.accept),
		ACCEPT_ONE_SOUND(ACCEPT_ONE,R.raw.accept_one),
		ACCEPT_TWO_SOUND(ACCEPT_TWO,R.raw.accept_two),
		ALARM_ONE_SOUND(ALARM_ONE,R.raw.alarm_one),
		ALARM_TWO_SOUND(ALARM_TWO,R.raw.alarm_two),
		ARRIVED_SOUND(ARRIVED,R.raw.arrived),
		A_SOUND(A,R.raw.a),
		B_SOUND(B,R.raw.b),
		C_SOUND(C,R.raw.c),
		CANCEL_SOUND(CANCEL,R.raw.cancel),
		CANCELLOAD_SOUND(CANCELLOAD,R.raw.cancelload),
		CHECKEND_SOUND(CHECKEND,R.raw.checkend),
		CHECKING_SOUND(CHECKING,R.raw.checking),
		CHOOSELOADER_SOUND(CHOOSELOADER,R.raw.chooseloader),
		D_SOUND(D,R.raw.d),
		E_SOUND(E,R.raw.e),
		F_SOUND(F,R.raw.f),
		G_SOUND(G,R.raw.g),

		H_SOUND(H,R.raw.h),
		I_SOUND(I,R.raw.i),
		J_SOUND(J,R.raw.j),
		K_SOUND(K,R.raw.k),
		L_SOUND(L,R.raw.l),
		M_SOUND(M,R.raw.m),
		MATERIAL_SOUND(MATERIAL,R.raw.material),
		MATERIALUPDATE_SOUND(MATERIALUPDATE,R.raw.materialupdate),
		N_SOUND(N,R.raw.n),
		NO_SOUND(NO,R.raw.no),
		NOTIFY_SOUND(NOTIFY,R.raw.notify),
		O_SOUND(O,R.raw.o),
		OUTSOURCING_SOUND(OUTSOURCING,R.raw.outsourcing),
		OVERSPEED_SOUND(OVERSPEED,R.raw.overspeed),

		P_SOUND(P,R.raw.p),
		Q_SOUND(Q,R.raw.q),
		QUIT_SOUND(QUIT,R.raw.quit),
		QUITSTATUS_SOUND(QUITSTATUS,R.raw.quitstatus),
		REBOOT_SOUND(REBOOT,R.raw.reboot),
		R_SOUND(R1,R.raw.r),
		S_SOUND(S,R.raw.s),
		SUBMIT_SOUND(SUBMIT,R.raw.submit),

		T_SOUND(T,R.raw.t),
		U_SOUND(U,R.raw.u),
		UNLOAD_SOUND(UNLOAD,R.raw.unload),
		UPDATE_SOUND(UPDATE,R.raw.update),
		V_SOUND(V,R.raw.v),

		VOICEFAIL_SOUND(VOICEFAIL,R.raw.voicefail),
		VOICEOK_SOUND(VOICEOK,R.raw.voiceok),
		W_SOUND(W,R.raw.w),
		WELCOME_SOUND(WELCOME,R.raw.welcome),
		X_SOUND(X,R.raw.x),
		Y_SOUND(Y,R.raw.y),
		YES_SOUND(YES,R.raw.yes),
		Z_SOUND(Z,R.raw.z),

		LOADDONE_SOUND(LOADDONE,R.raw.loaddone),
		
		;
		
		public int tag;
		public int raw;
		
		public int getTag() {
			return tag;
		}

		public void setTag(int tag) {
			this.tag = tag;
		}

		public int getRaw() {
			return raw;
		}

		public void setRaw(int raw) {
			this.raw = raw;
		}

		public static Map<Integer, Integer> getTagToRaw() {
			return tagToRaw;
		}

		public static void setTagToRaw(Map<Integer, Integer> tagToRaw) {
			Sounds.tagToRaw = tagToRaw;
		}

		private Sounds(int tag,int raw){
			this.tag=tag;
			this.raw=raw;
		}
		public static Map<Integer,Integer> tagToRaw=new HashMap<Integer, Integer>();
		
		static {
			for(Sounds s:values()){
				tagToRaw.put(s.tag, s.raw);
			}
		}
	}
	
	
	
	/**
	 * init the sound pool..
	 * @return
	 */
	
	private  SoundPool initSoundPool(){
		if(soundPool==null){
			soundPool=new SoundPool(MAX_PLAY_COUNT, AudioManager.STREAM_MUSIC, 0);
		}
		
	    soundPoolMap.put(CHECKING, soundPool.load(context, R.raw.checking, 1)); 
	    soundPoolMap.put(WELCOME, soundPool.load(context, R.raw.welcome, 1)); 
	    soundPoolMap.put(OVERSPEED, soundPool.load(context, R.raw.overspeed, 1)); 
	    soundPoolMap.put(VOICEFAIL, soundPool.load(context, R.raw.voicefail, 1)); 
	    soundPoolMap.put(VOICEOK, soundPool.load(context, R.raw.voiceok, 1)); 
	    soundPoolMap.put(ALARM_ONE, soundPool.load(context, R.raw.alarm_one, 1)); 
	    soundPoolMap.put(ALARM_TWO, soundPool.load(context, R.raw.alarm_two, 1)); 
	    soundPoolMap.put(QUITSTATUS, soundPool.load(context, R.raw.quitstatus, 1)); 
	    soundPoolMap.put(REBOOT, soundPool.load(context, R.raw.reboot, 1)); 
	    soundPoolMap.put(NO, soundPool.load(context, R.raw.no, 1)); 
	    
	    soundPoolMap.put(YES, soundPool.load(context, R.raw.yes, 1)); 
	    soundPoolMap.put(UPDATE, soundPool.load(context, R.raw.update, 1)); 
	    soundPoolMap.put(ARRIVED, soundPool.load(context, R.raw.arrived, 1)); 
	    soundPoolMap.put(NOTIFY, soundPool.load(context, R.raw.notify, 1)); 
	    soundPoolMap.put(CHOOSELOADER, soundPool.load(context, R.raw.chooseloader, 1)); 
	    soundPoolMap.put(SUBMIT, soundPool.load(context, R.raw.submit, 1)); 
	    
	    soundPoolMap.put(LOADDONE, soundPool.load(context, R.raw.loaddone, 1)); 
	    soundPoolMap.put(UNLOAD, soundPool.load(context, R.raw.unload, 1)); 
	    soundPoolMap.put(CANCELLOAD, soundPool.load(context, R.raw.cancelload, 1)); 
	    soundPoolMap.put(CANCEL, soundPool.load(context, R.raw.cancel, 1)); 
	    soundPoolMap.put(ACCEPT, soundPool.load(context, R.raw.accept, 1)); 
	    soundPoolMap.put(ACCEPT_ONE, soundPool.load(context, R.raw.accept_one, 1)); 
	    soundPoolMap.put(ACCEPT_TWO, soundPool.load(context, R.raw.accept_two, 1)); 
	    soundPoolMap.put(OUTSOURCING, soundPool.load(context, R.raw.outsourcing, 1)); 
	    soundPoolMap.put(MATERIAL, soundPool.load(context, R.raw.material, 1)); 
	    soundPoolMap.put(MATERIALUPDATE, soundPool.load(context, R.raw.materialupdate, 1)); 

		return soundPool;
	}
	
	
	/**
	 * play the sound..
	 * @param tag
	 */
	public synchronized  void playSound(int sampleId){
		if(soundPool!=null){
			
			//pauseSound(tag);
			//final int soundID=getSoundId(tag);
			int id = soundPoolMap.get(sampleId);
			soundPool.play(id, volumnRatio, volumnRatio, 1, 0, 1);
			
//			soundPool.setOnLoadCompleteListener(new OnLoadCompleteListener() {
//	              @Override
//	              public void onLoadComplete(SoundPool soundPool, int sampleId,
//	                  int status) {
//	            	  soundPool.play(sampleId, volumnRatio, volumnRatio, 1, 0, 1);
//	              } 
//	        });
		}
	}
	
	/**
	 * pause the sound..
	 * @param tag
	 */
	public synchronized void pauseSound(int sampleId){
		if(soundPool!=null){
			
			soundPool.pause(sampleId);
		}
	}
	
	/**
	 * stop the sound...
	 * @param tag
	 */
	public synchronized void stopSound(int sampleId){
		if(soundPool!=null){
			
			soundPool.stop(sampleId);
		}
	}
	
	/**
	 * release the sound pool...
	 */
	
	public synchronized void release(){
		if(soundPool!=null){
			soundPool.release();
			soundPool=null;
			soundUtils=null;
		}
	}
	
	/**
	 * use media play play music. complete so finish..
	 * @param tag
	 */
	public synchronized void playMedia(int tag){
		try{
		
			MediaPlayer	mediaPlayer=MediaPlayer.create(context, tag);
			mediaPlayer.setOnCompletionListener(new OnCompletionListener() {
				
				@Override
				public void onCompletion(MediaPlayer mp) {
					// TODO Auto-generated method stub
					mp.release();
					mp=null;
				}
			});
			
			mediaPlayer.start();
		
		}catch(Exception e){
			e.printStackTrace();
		}
	}
	
	
}
