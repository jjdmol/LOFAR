//#  jOTDB_jOTDBconnection.cc: Manages the connection with the OTDB database.
//#
//#  Copyright (C) 2002-2004
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

#
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCommon.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.h>
#include <OTDB/OTDBconnection.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <string>
#include <iostream>


using namespace boost::posix_time;
using namespace LOFAR::OTDB;
using namespace std;

OTDBconnection* theirConn;

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_initOTDBconnection(JNIEnv *env, jobject, jstring username, jstring passwd, jstring database, jstring hostname) {
  const char* user = env->GetStringUTFChars(username, 0);
  const char* pass = env->GetStringUTFChars(passwd, 0);
  const char* db = env->GetStringUTFChars(database, 0);
  const char* hn = env->GetStringUTFChars(hostname, 0);
  const string u (user);
  const string p (pass);
  const string d (db);
  const string h (hn);

  try {
    theirConn = new OTDBconnection(u, p, d, h);
    env->ReleaseStringUTFChars(username, user);
    env->ReleaseStringUTFChars(passwd, pass);
    env->ReleaseStringUTFChars(database, db);
    env->ReleaseStringUTFChars(hostname, hn);
  } catch (exception &ex) {
    cout << "Exception during new OTDBconnection(" << u << "," << p << "," << d << "," << h << ") : "<< ex.what() << endl;
    
    env->ReleaseStringUTFChars(username, user);
    env->ReleaseStringUTFChars(passwd, pass);
    env->ReleaseStringUTFChars(database, db);
    env->ReleaseStringUTFChars(hostname, hn);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_isConnected(JNIEnv *env, jobject) {
  jboolean connected;
  try {
    connected = theirConn->isConnected();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connected "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}


JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_connect(JNIEnv *env, jobject) {
  jboolean connected;
  try {
    connected = (jboolean)theirConn->connect();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connect "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getTreeInfo(JNIEnv *env, jobject, jint treeID, jboolean isMomID) {
  OTDBtree aTree;
  try {
    aTree = theirConn->getTreeInfo((int)treeID, isMomID);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeInfo(" << treeID << "," << isMomID << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBtree(env, aTree);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getStateList(JNIEnv *env, jobject, jint treeID, jboolean isMomID, jstring beginDate, jstring endDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  jobject statesVector;
  try {

    vector<TreeState> states = theirConn->getStateList(treeID, isMomID,time_from_string(bd), time_from_string(ed));
    vector<TreeState>::iterator statesIterator;
    

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    statesVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (statesIterator = states.begin(); statesIterator != states.end(); statesIterator++) {
      env->CallObjectMethod(statesVector, mid_Vector_add,convertTreeState(env, *statesIterator ));
    }

    env->ReleaseStringUTFChars(beginDate, bd);
    env->ReleaseStringUTFChars(endDate, ed);  
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateList(" << treeID << "," << isMomID << "," <<
    time_from_string(bd) << "," << time_from_string(ed) << ") "<< ex.what() << endl;
    
    env->ReleaseStringUTFChars(beginDate, bd);
    env->ReleaseStringUTFChars(endDate, ed);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(statesVector);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getTreeList(JNIEnv *env, jobject, jshort treeType, jshort classifType) {
  
  jobject itemVector;
  try {
    vector<OTDBtree> trees = theirConn->getTreeList(treeType, classifType);
    vector<OTDBtree>::iterator treeIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBtree(env, *treeIterator));
    }
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << "," << classifType << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}


JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_errorMsg(JNIEnv *env, jobject) {
  jstring jstr;
  try {
    jstr = env->NewStringUTF(theirConn->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::errorMsg " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getAuthToken(JNIEnv *env, jobject) {
  jint token;
  try {
    token = (jint)theirConn->getAuthToken();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getAuthToken " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return token;
}

JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getDBName(JNIEnv *env, jobject) {
  jstring jstr;
  try {
    jstr = env->NewStringUTF(theirConn->getDBName().c_str());
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getDBName " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


