#ifndef __nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv__
#define __nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_initTreeTypeConv (JNIEnv *env, jobject);
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_get__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_get__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_getTypes (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_top (JNIEnv *env, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_next (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv__ */
