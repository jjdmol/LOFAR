#ifndef __nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection__
#define __nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_initOTDBconnection (JNIEnv *env, jobject, jstring, jstring, jstring, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_isConnected (JNIEnv *env, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_connect (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_disconnect (JNIEnv *env, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeInfo__IZ (JNIEnv *env, jobject, jint, jboolean);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeInfo__I (JNIEnv *env, jobject, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jshort, jshort, jint, jstring, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jshort, jshort, jint, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2 (JNIEnv *env, jobject, jshort, jshort, jint, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSI (JNIEnv *env, jobject, jshort, jshort, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SS (JNIEnv *env, jobject, jshort, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZLjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jint, jboolean, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZLjava_lang_String_2 (JNIEnv *env, jobject, jint, jboolean, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZ (JNIEnv *env, jobject, jint, jboolean);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__I (JNIEnv *env, jobject, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getDefaultTemplates (JNIEnv *env, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getExecutableTrees__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getExecutableTrees__ (JNIEnv *env, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeGroup (JNIEnv *env, jobject, jshort, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod__SLjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jshort, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod__SLjava_lang_String_2 (JNIEnv *env, jobject, jshort, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_newGroupID (JNIEnv *env, jobject);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_errorMsg (JNIEnv *env, jobject);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getAuthToken (JNIEnv *env, jobject);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getDBName (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection__ */
