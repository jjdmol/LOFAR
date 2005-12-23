/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class jOTDB_jOTDBconnection */

#ifndef _Included_jOTDB_jOTDBconnection
#define _Included_jOTDB_jOTDBconnection
#ifdef __cplusplus
extern "C" {
#endif

   namespace LOFAR
     {

        namespace jOTDB
          {

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    initOTDBconnection
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jOTDB_jOTDBconnection_initOTDBconnection
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    isConnected
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_jOTDB_jOTDBconnection_isConnected
  (JNIEnv *, jobject);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    connect
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_jOTDB_jOTDBconnection_connect
  (JNIEnv *, jobject);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getTreeInfo
 * Signature: (IZ)LjOTDB/jOTDBtree;
 */
JNIEXPORT jobject JNICALL Java_jOTDB_jOTDBconnection_getTreeInfo
  (JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getStateList
 * Signature: (IZLjava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_jOTDB_jOTDBconnection_getStateList
  (JNIEnv *, jobject, jint, jboolean, jstring, jstring);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getTreeList
 * Signature: (SS)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_jOTDB_jOTDBconnection_getTreeList
  (JNIEnv *, jobject, jshort, jshort);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jOTDB_jOTDBconnection_errorMsg
  (JNIEnv *, jobject);

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getAuthToken
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_jOTDB_jOTDBconnection_getAuthToken
  (JNIEnv *, jobject);

	  }
     }

#ifdef __cplusplus
}
#endif
#endif
