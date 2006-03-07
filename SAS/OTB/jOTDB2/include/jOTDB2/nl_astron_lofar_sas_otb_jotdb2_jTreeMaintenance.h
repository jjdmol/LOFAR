/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance */

#ifndef _Included_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
#define _Included_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    initTreeMaintenance
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_initTreeMaintenance
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getNodeDef
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb2/jVICnodeDef;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getNodeDef
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    buildTemplateTree
 * Signature: (IS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_buildTemplateTree
  (JNIEnv *, jobject, jint, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    copyTemplateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_copyTemplateTree
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getNode
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getNode
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setMomInfo
 * Signature: (IILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setMomInfo
  (JNIEnv *, jobject, jint, jint, jstring);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getItemList
 * Signature: (III)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getItemList__III
  (JNIEnv *, jobject, jint, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getItemList
 * Signature: (ILjava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getItemList__ILjava_lang_String_2
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    dupNode
 * Signature: (IIS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_dupNode
  (JNIEnv *, jobject, jint, jint, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveNode
  (JNIEnv *, jobject, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveNodeList
  (JNIEnv *, jobject, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteNode
  (JNIEnv *, jobject, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteNodeList
  (JNIEnv *, jobject, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    checkTreeConstraints
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_checkTreeConstraints
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    instanciateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_instanciateTree
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    pruneTree
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_pruneTree
  (JNIEnv *, jobject, jint, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteTree
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteTree
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getTopNode
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getTopNode
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setClassification
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setClassification
  (JNIEnv *, jobject, jint, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setTreeState
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setTreeState
  (JNIEnv *, jobject, jint, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_errorMsg
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getParam
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBparam;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getParam
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveParam
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBparam;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveParam
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
