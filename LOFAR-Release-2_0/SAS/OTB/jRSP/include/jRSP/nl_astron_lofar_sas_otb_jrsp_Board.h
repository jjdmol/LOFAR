/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class nl_astron_lofar_sas_otb_jrsp_Board */

#ifndef _Included_nl_astron_lofar_sas_otb_jrsp_Board
#define _Included_nl_astron_lofar_sas_otb_jrsp_Board
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    init
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_init
  (JNIEnv *, jobject, jstring);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    delete
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_delete
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    retrieveStatus
 * Signature: (II)[Lnl/astron/lofar/sas/otb/jrsp/BoardStatus;
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_retrieveStatus
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    setWaveformSettings
 * Signature: (IIDSII)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_setWaveformSettings
  (JNIEnv *, jobject, jint, jint, jdouble, jshort, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getSubbandStats
 * Signature: (II)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getSubbandStats
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getWaveformSettings
 * Signature: (II)[Lnl/astron/lofar/sas/otb/jrsp/WGRegisterType;
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getWaveformSettings
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getNrRCUs
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getNrRCUs
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getNrRSPBoards
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getNrRSPBoards
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getMaxRSPBoards
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getMaxRSPBoards
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    setFilter
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_setFilter
  (JNIEnv *, jobject, jint, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    sendClear
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_sendClear
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    sendReset
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_sendReset
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    sendSync
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_sendSync
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jrsp_Board
 * Method:    getBeamletStats
 * Signature: (II)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_nl_astron_lofar_sas_otb_jrsp_Board_getBeamletStats
  (JNIEnv *, jobject, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
