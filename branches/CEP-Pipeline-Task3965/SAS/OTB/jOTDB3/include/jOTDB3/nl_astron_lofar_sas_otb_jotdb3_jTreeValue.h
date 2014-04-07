#ifndef __nl_astron_lofar_sas_otb_jotdb3_jTreeValue__
#define __nl_astron_lofar_sas_otb_jotdb3_jTreeValue__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVT__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jstring, jstring, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVT__Lnl_astron_lofar_sas_otb_jotdb3_jOTDBvalue_2 (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVTlist (JNIEnv *env, jobject, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getBrokenHardware__Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getBrokenHardware__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getBrokenHardware__ (JNIEnv *env, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_searchInPeriod__IILjava_lang_String_2Ljava_lang_String_2Z (JNIEnv *env, jobject, jint, jint, jstring, jstring, jboolean);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_searchInPeriod__IILjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jint, jint, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_searchInPeriod__IILjava_lang_String_2 (JNIEnv *env, jobject, jint, jint, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_searchInPeriod__II (JNIEnv *env, jobject, jint, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getSchedulableItems__I (JNIEnv *env, jobject, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getSchedulableItems__ (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jTreeValue__ */
