#ifndef __nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv__
#define __nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_initParamTypeConv (JNIEnv *env, jobject);
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_get__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_get__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_getTypes (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_top (JNIEnv *env, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv_next (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv__ */
