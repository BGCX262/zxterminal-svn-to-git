/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_feihong_newzxclient_util_RecordUtils */

#ifndef _Included_com_feihong_newzxclient_util_RecordUtils
#define _Included_com_feihong_newzxclient_util_RecordUtils
#ifdef __cplusplus
extern "C" {
#endif
#undef com_feihong_newzxclient_util_RecordUtils_NUM_CHANNELS
#define com_feihong_newzxclient_util_RecordUtils_NUM_CHANNELS 1L
#undef com_feihong_newzxclient_util_RecordUtils_SAMPLE_RATE
#define com_feihong_newzxclient_util_RecordUtils_SAMPLE_RATE 16000L
#undef com_feihong_newzxclient_util_RecordUtils_BITRATE
#define com_feihong_newzxclient_util_RecordUtils_BITRATE 32L
#undef com_feihong_newzxclient_util_RecordUtils_MODE
#define com_feihong_newzxclient_util_RecordUtils_MODE 1L
#undef com_feihong_newzxclient_util_RecordUtils_QUALITY
#define com_feihong_newzxclient_util_RecordUtils_QUALITY 5L
/*
 * Class:     com_feihong_newzxclient_util_RecordUtils
 * Method:    initEncoder
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL Java_com_feihong_newzxclient_util_RecordUtils_initEncoder
  (JNIEnv *, jobject, jint, jint, jint, jint, jint);

/*
 * Class:     com_feihong_newzxclient_util_RecordUtils
 * Method:    destroyEncoder
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_feihong_newzxclient_util_RecordUtils_destroyEncoder
  (JNIEnv *, jobject);

/*
 * Class:     com_feihong_newzxclient_util_RecordUtils
 * Method:    encodeFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_feihong_newzxclient_util_RecordUtils_encodeFile
  (JNIEnv *, jobject, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
