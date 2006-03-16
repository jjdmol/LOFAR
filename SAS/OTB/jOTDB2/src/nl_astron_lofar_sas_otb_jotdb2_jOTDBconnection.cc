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

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_initOTDBconnection(JNIEnv *env, jobject, jstring username, jstring passwd, jstring database) {
  const char* user = env->GetStringUTFChars(username, 0);
  const char* pass = env->GetStringUTFChars(passwd, 0);
  const char* db = env->GetStringUTFChars(database, 0);
  const string u (user);
  const string p (pass);
  const string d (db);

  theirConn = new OTDBconnection(u, p, d);

  env->ReleaseStringUTFChars(username, user);
  env->ReleaseStringUTFChars(passwd, pass);
  env->ReleaseStringUTFChars(database, db);
}

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_isConnected(JNIEnv *, jobject) {
  jboolean connected;
  connected = theirConn->isConnected();
  return connected;
}


JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_connect(JNIEnv *, jobject) {
  jboolean connected;
  connected = (jboolean)theirConn->connect();
  return connected;
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getTreeInfo(JNIEnv *env, jobject, jint treeID, jboolean isMomID) {
  OTDBtree aTree = theirConn->getTreeInfo((int)treeID, isMomID);

  return convertOTDBtree(env, aTree);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getStateList(JNIEnv *env, jobject, jint treeID, jboolean isMomID, jstring beginDate, jstring endDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  vector<TreeState> states = theirConn->getStateList(treeID, isMomID,time_from_string(bd), time_from_string(ed));

  vector<TreeState>::iterator statesIterator;

  // Construct java Vector
  jobject statesVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  statesVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

  for (statesIterator = states.begin(); statesIterator != states.end(); statesIterator++)
    {
      env->CallObjectMethod(statesVector, mid_Vector_add,convertTreeState(env, *statesIterator ));
    }

  return(statesVector);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getTreeList(JNIEnv *env, jobject, jshort treeType, jshort classifType) {
  vector<OTDBtree> trees = theirConn->getTreeList(treeType, classifType);
  vector<OTDBtree>::iterator treeIterator;

  // Construct java Vector
  jobject itemVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

  itemVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


  for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++)
    {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBtree(env, *treeIterator));
    }

  return(itemVector);
}


JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_errorMsg(JNIEnv *env, jobject) {
  jstring jstr = env->NewStringUTF(theirConn->errorMsg().c_str());
  return jstr;
}


JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getAuthToken(JNIEnv *, jobject) {
  jint token = (jint)theirConn->getAuthToken();
  return token;
}



