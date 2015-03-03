//#  jClassifConv.cc: 
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jClassifConv.h>
#include <OTDB/OTDBtypes.h>
#include <string>
#include <iostream>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <OTDB/ClassifConv.h>

using namespace LOFAR::OTDB;
using namespace std;


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    initClassifConv
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_initClassifConv(JNIEnv *env, jobject jClassifConv) {
  string name = getOwnerExt(env,jClassifConv);
  try {
    OTDBconnection* aConn=getConnection(name);
    ClassifConv* aCConv = new ClassifConv(aConn);
    theirC_ObjectMap.erase(name+"_ClassifConv");
    theirC_ObjectMap[name+"_ClassifConv"]=(void*)aCConv;

  } catch (exception &ex) {
    cout << "Exception during new ClassifConv " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    get
 * Signature: (Ljava/lang/String;)S
 */
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_get__Ljava_lang_String_2(JNIEnv *env, jobject jClassifConv, jstring aConv) {
  
  const char* chars = env->GetStringUTFChars (aConv, 0);
  const string str (chars);
  short ret(0);
  try {
    ret = ((ClassifConv*)getCObjectPtr(env,jClassifConv,"_ClassifConv"))->get (str);
    env->ReleaseStringUTFChars (aConv, chars);	     
  } catch (exception &ex) {
    cout << "Exception during ClassifConv::get("<< str << ") " << ex.what() << endl;
    
    env->ReleaseStringUTFChars (aConv, chars);	     
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
 
  return ret;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    get
 * Signature: (S)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_get__S(JNIEnv *env, jobject jClassifConv, jshort aConv) {

  jstring jstr(0);
  try {
    jstr = env->NewStringUTF (((ClassifConv*)getCObjectPtr(env,jClassifConv,"_ClassifConv"))->get(aConv).c_str());
  } catch (exception &ex) {
    cout << "Exception during ClassifConv::get("<< aConv << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    getTypes
 * Signature: ()Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_getTypes(JNIEnv *env, jobject jClassifConv) {

  
  // Construct java Map
  jobject result;
  jclass mapClass, shortClass;
  jmethodID mapInit, put, shortInit;
    
  mapClass = env->FindClass("java/util/HashMap");
  mapInit = env->GetMethodID(mapClass, "<init>", "()V");
  result = env->NewObject(mapClass, mapInit);

  shortClass = env->FindClass("java/lang/Short");
  shortInit = env->GetMethodID(shortClass, "<init>", "(S)V");
  


  if ( env->ExceptionOccurred() )
    return 0;
    
  put= env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  if ( env->ExceptionOccurred() )
    return 0;
  ClassifConv* classifConv=((ClassifConv*)getCObjectPtr(env,jClassifConv,"_ClassifConv"));
  try {
    classifConv->top();
    do {
      classifType key;
      string value;

      if (classifConv->get(key, value)) {
        env->CallObjectMethod(result, put, env->NewObject(shortClass,
	  						  shortInit,
							  (jshort)key), 
							  env->NewStringUTF(value.c_str()));

        if ( env->ExceptionOccurred() )
  	  return 0;
      }
    } while (classifConv->next());
  } catch (exception &ex) {
    cout << "Exception during ClassifConv::getTypes " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return result;

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    top
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_top(JNIEnv *env, jobject jClassifConv) {
  try {
    ((ClassifConv*)getCObjectPtr(env,jClassifConv,"_ClassifConv"))->top();
  } catch (exception &ex) {
    cout << "Exception during ClassifConv::top" << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jClassifConv
 * Method:    next
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_next(JNIEnv *env, jobject jClassifConv) {
  jboolean aBool(0);
  try {
    aBool=((ClassifConv*)getCObjectPtr(env,jClassifConv,"_ClassifConv"))->next();
  } catch (exception &ex) {
    cout << "Exception during ClassifConv::next" << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return aBool;
}
