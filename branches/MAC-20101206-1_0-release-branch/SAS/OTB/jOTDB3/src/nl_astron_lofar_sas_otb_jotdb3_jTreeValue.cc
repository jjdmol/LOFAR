//#  jTreeValue.cc: Get KVT valuess from database  via JNI
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
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jTreeValue.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <string>
#include <iostream>

using namespace boost::posix_time;
using namespace LOFAR::OTDB;
using namespace std;


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeValue
 * Method:    addKVT
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVT__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject jTreeValue, jstring key, jstring val, jstring time) {

  // create the connection with the c++ TreeVal
  setTreeValConnection(env,jTreeValue);

  const char* ak = env->GetStringUTFChars (key, 0);
  const char* av = env->GetStringUTFChars (val, 0);
  const char* at = env->GetStringUTFChars (time, 0);

  const string aKey(ak);
  const string aValue(av);
  const string aT(at);
  
  const ptime aTime (time_from_string (aT));

  jboolean succes;
  try {
    succes=((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"))->addKVT(aKey,aValue,aTime);
    
    env->ReleaseStringUTFChars (key ,ak);
    env->ReleaseStringUTFChars (val ,av);
    env->ReleaseStringUTFChars (time ,at);
    setErrorMsg(env,jTreeValue);
  } catch (exception &ex) {
    cout << "Exception during treeValue::addKVT(" << aKey << "," << aValue << "," << aTime << ") " << ex.what() << endl; 

    env->ReleaseStringUTFChars (key ,ak);
    env->ReleaseStringUTFChars (val ,av);
    env->ReleaseStringUTFChars (time ,at);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeValue
 * Method:    addKVT
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jOTDBvalue;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVT__Lnl_astron_lofar_sas_otb_jotdb3_jOTDBvalue_2(JNIEnv *env, jobject jTreeValue, jobject jOTDBval) {

  // create the connection with the c++ TreeVal
  setTreeValConnection(env,jTreeValue);

  OTDBvalue anOTDBvalue = convertjOTDBvalue (env, jOTDBval);
  jboolean succes;
  try {
    succes = ((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"))->addKVT(anOTDBvalue);
    setErrorMsg(env,jTreeValue);
  } catch (exception &ex) {
    cout << "Exception during treeValue::addKVT" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }


  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeValue
 * Method:    addKVTlist
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_addKVTlist(JNIEnv *env, jobject jTreeValue, jobject aValList ) {

  // create the connection with the c++ TreeVal
  setTreeValConnection(env,jTreeValue);

  OTDBvalue aValue;
			      
  // Construct java Vector
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_elementAt = env->GetMethodID(class_Vector, "elementAt",
						    "(I)Ljava/lang/Object;");
  jmethodID mid_Vector_size = env->GetMethodID(class_Vector, "size", "()I");
  int size=env->CallIntMethod (aValList, mid_Vector_size);
  
  vector<OTDBvalue> aCValList(size);
  
  for (int i = 0; i < size; i++) { 
    aValue = convertjOTDBvalue (env,
				env->CallObjectMethod (aValList, 
						       mid_Vector_elementAt,i));
    aCValList[i]=aValue;
  }

  setErrorMsg(env,jTreeValue);

  jboolean succes;
  try {
    succes=((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"))->addKVTlist(aCValList);
  } catch (exception &ex) {
    cout << "Exception during treeValue::addKVTlist" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeValue
 * Method:    searchInPeriod
 * Signature: (IILjava/lang/String;Ljava/lang/String;Z)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_searchInPeriod(JNIEnv *env, jobject jTreeValue, jint topNode, jint depth, jstring beginDate, jstring endDate, jboolean mostRecentOnly) {
  
  // create the connection with the c++ TreeVal
  setTreeValConnection(env,jTreeValue);

  
  
  
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);
  const string beginTime (bd);
  const string endTime (ed);
  
  
  const ptime ts (time_from_string (beginTime));  
  const ptime te (time_from_string (endTime));  
  jobject valueVector;
  
  try {
    vector<OTDBvalue> valueList = ((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"))->searchInPeriod (topNode, depth, ts, te, mostRecentOnly);
    vector<OTDBvalue>::iterator valueIterator;
  
  
    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    valueVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
  
    for (valueIterator = valueList.begin(); valueIterator != valueList.end(); valueIterator++)
      env->CallObjectMethod(valueVector, mid_Vector_add, convertOTDBvalue (env, *valueIterator));
    
    env->ReleaseStringUTFChars (beginDate, bd);
    env->ReleaseStringUTFChars (endDate, ed);	     
    
    setErrorMsg(env,jTreeValue);
  } catch (exception &ex) {
    cout << "Exception during treeValue::searchInPeriod(" << topNode << "," 
	 << depth << "," << ts << "," << mostRecentOnly << ") "<< ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }
  return valueVector;	     
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeValue
 * Method:    getSchedulableItems
 * Signature: (I)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeValue_getSchedulableItems(JNIEnv *env, jobject jTreeValue, jint aNodeID) {

  // create the connection with the c++ TreeVal
  setTreeValConnection(env,jTreeValue);
  jobject itemVector;

  try {
    vector<OTDBvalue> itemList = ((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"))->getSchedulableItems(aNodeID);
    vector<OTDBvalue>::iterator itemIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, 
			    mid_Vector_add, 
			    convertOTDBvalue (env, *itemIterator));

    setErrorMsg(env,jTreeValue);
  } catch (exception &ex) {
    cout << "Exception during treeValue::getSchedulableItems(" << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return itemVector;
}



void  setTreeValConnection(JNIEnv *env, jobject jTreeValue) {
  // get the  callerclass
  jclass jTreeValue_class=env->GetObjectClass(jTreeValue);

  // get the methodID
  jfieldID id_treeID = env->GetFieldID (jTreeValue_class, "itsTreeID","I");

  // get the value
  jint aTreeID = env->GetIntField(jTreeValue,id_treeID);

  string name = getOwnerExt(env,jTreeValue);
  try {
    OTDBconnection* aConn=getConnection(name);
    TreeValue* treeVal(0);
    treeVal=((TreeValue*)getCObjectPtr(env,jTreeValue,"_TreeValue"));
    if (treeVal == 0) {
        treeVal = new TreeValue(aConn,aTreeID);
        theirC_ObjectMap[name+"_TreeValue"]=(void*)treeVal;
    }

  } catch (exception &ex) {
    cout << "Exception during new treeValue" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

void setErrorMsg(JNIEnv *env, jobject jTreeValue) {
  // get the  callerclass
  jclass jTreeValue_class=env->GetObjectClass(jTreeValue);

  // get the methodID
  jfieldID id_errorMsg = env->GetFieldID (jTreeValue_class, "itsErrorMsg","Ljava/lang/String;");

  // get the ErrorMsg
  jstring errorMsg;
  try {
    errorMsg=env->NewStringUTF(((OTDBconnection*)getCObjectPtr(env,jTreeValue,"_OTDBconnection"))->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during errorMsg" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  // set the ErrorMsg in the jTreeValue
  env->SetObjectField(jTreeValue,id_errorMsg,errorMsg);
}
