    //#  jUnitConv.cc:
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jUnitConv.h>
#include <OTDB/OTDBtypes.h>
#include <string>
#include <iostream>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <OTDB/UnitConv.h>

using namespace LOFAR::OTDB;
using namespace std;

 
/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    initUnitConv
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_initUnitConv(JNIEnv *env, jobject jUnitConv) {
  string name = getOwnerExt(env,jUnitConv);
  try {
    OTDBconnection* aConn=getConnection(name);
    UnitConv* aUConv = new UnitConv(aConn);
    theirC_ObjectMap[name+"_UnitConv"]=(void*)aUConv;
  } catch (exception &ex) {
    cout << "Exception during new UnitConv" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    get
 * Signature: (Ljava/lang/String;)S
 */
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_get__Ljava_lang_String_2(JNIEnv *env, jobject jUnitConv, jstring aConv) {
  
  const char* chars = env->GetStringUTFChars (aConv, 0);
  const string str (chars);
  
  short ret;
  try {
    ret = ((UnitConv*)getCObjectPtr(env,jUnitConv,"_UnitConv"))->get (str);
  
    env->ReleaseStringUTFChars (aConv, chars);	     
  } catch (exception &ex) {
    cout << "Exception during UnitConv::get(" << str << ") " << ex.what() << endl; 

    env->ReleaseStringUTFChars (aConv, chars);	     
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return ret;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    get
 * Signature: (S)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_get__S(JNIEnv *env, jobject jUnitConv, jshort aConv) {
  jstring jstr;
  try {
    jstr= env->NewStringUTF (((UnitConv*)getCObjectPtr(env,jUnitConv,"_UnitConv"))->get(aConv).c_str());
  } catch (exception &ex) {
    cout << "Exception during UnitConv::get(" << aConv << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return jstr;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    getTypes
 * Signature: ()Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_getTypes(JNIEnv *env, jobject jUnitConv) {

  
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

  UnitConv* unitConv=((UnitConv*)getCObjectPtr(env,jUnitConv,"_UnitConv"));
  put= env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  if ( env->ExceptionOccurred() )
    return 0;
 
  try {
    unitConv->top();
    do {
      unitType key;
      string value;

      if (unitConv->get(key, value)) {
        env->CallObjectMethod(result, put, env->NewObject(shortClass,
							  shortInit,
							  (jshort)key), 
							  env->NewStringUTF(value.c_str()));

        if ( env->ExceptionOccurred() )
	  return 0;
     }
    } while (unitConv->next());
  } catch (exception &ex) {
    cout << "Exception during UnitConv::getTypes"  << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }


  return result;

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    top
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_top(JNIEnv *env, jobject jUnitConv) {
    
  try {
    ((UnitConv*)getCObjectPtr(env,jUnitConv,"_UnitConv"))->top();
  } catch (exception &ex) {
    cout << "Exception during UnitConv::top" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jUnitConv
 * Method:    next
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jUnitConv_next(JNIEnv *env, jobject jUnitConv) {
  jboolean aBool;
  try {
    aBool=((UnitConv*)getCObjectPtr(env,jUnitConv,"_UnitConv"))->next();
  } catch (exception &ex) {
    cout << "Exception during UnitConv::next" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aBool;
}
