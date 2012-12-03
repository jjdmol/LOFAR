#ifndef __nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance__
#define __nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_initTreeMaintenance (JNIEnv *env, jobject);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadMasterFile (JNIEnv *env, jobject, jstring);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadComponentFile__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jstring, jstring, jstring);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadComponentFile__Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject, jstring, jstring);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadComponentFile__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentList__Ljava_lang_String_2Z (JNIEnv *env, jobject, jstring, jboolean);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentList__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentList__ (JNIEnv *env, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentNode (JNIEnv *env, jobject, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentParams (JNIEnv *env, jobject, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveComponentNode (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_isTopComponent (JNIEnv *env, jobject, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteComponentNode (JNIEnv *env, jobject, jint);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getFullComponentName (JNIEnv *env, jobject, jobject);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_buildTemplateTree (JNIEnv *env, jobject, jint, jshort);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_newTemplateTree (JNIEnv *env, jobject);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_copyTemplateTree (JNIEnv *env, jobject, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_assignTemplateName (JNIEnv *env, jobject, jint, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_assignProcessType (JNIEnv *env, jobject, jint, jstring, jstring, jstring);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getNode (JNIEnv *env, jobject, jint, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getParam__II (JNIEnv *env, jobject, jint, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getParam__Lnl_astron_lofar_sas_otb_jotdb3_jOTDBnode_2 (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveParam (JNIEnv *env, jobject, jobject);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getItemList__III (JNIEnv *env, jobject, jint, jint, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getItemList__ILjava_lang_String_2 (JNIEnv *env, jobject, jint, jstring);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_dupNode (JNIEnv *env, jobject, jint, jint, jshort);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_addComponent__IIILjava_lang_String_2 (JNIEnv *env, jobject, jint, jint, jint, jstring);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_addComponent__III (JNIEnv *env, jobject, jint, jint, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveNode (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveNodeList (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteNode (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteNodeList (JNIEnv *env, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_checkTreeConstraints__II (JNIEnv *env, jobject, jint, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_checkTreeConstraints__I (JNIEnv *env, jobject, jint);
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_instanciateTree (JNIEnv *env, jobject, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_pruneTree (JNIEnv *env, jobject, jint, jshort);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_exportTree (JNIEnv *env, jobject, jint, jint, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_exportResultTree (JNIEnv *env, jobject, jint, jint, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteTree (JNIEnv *env, jobject, jint);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getTopNode (JNIEnv *env, jobject, jint);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setMomInfo (JNIEnv *env, jobject, jint, jint, jint, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setClassification (JNIEnv *env, jobject, jint, jshort);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setTreeState (JNIEnv *env, jobject, jint, jshort);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setDescription (JNIEnv *env, jobject, jint, jstring);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setSchedule (JNIEnv *env, jobject, jint, jstring, jstring);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_errorMsg (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance__ */
