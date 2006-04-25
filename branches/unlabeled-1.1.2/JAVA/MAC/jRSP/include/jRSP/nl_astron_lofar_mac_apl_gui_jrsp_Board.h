/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class nl_astron_lofar_mac_apl_gui_jrsp_Board */

#ifndef _Included_nl_astron_lofar_mac_apl_gui_jrsp_Board
#define _Included_nl_astron_lofar_mac_apl_gui_jrsp_Board
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    init
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_init
  (JNIEnv *, jobject, jstring);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    delete
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_delete
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    retrieveStatus
 * Signature: (II)[Lnl/astron/lofar/mac/apl/gui/jrsp/BoardStatus;
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_retrieveStatus
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    retrieveNofBoards
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_retrieveNofBoards
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    setWaveformSettings
 * Signature: (IIIII)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_setWaveformSettings
  (JNIEnv *, jobject, jint, jint, jint, jint, jint);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    getSubbandStats
 * Signature: (II)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_getSubbandStats
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_mac_apl_gui_jrsp_Board
 * Method:    test
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_test
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
