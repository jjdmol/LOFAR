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

jobject convertOTDBtree(JNIEnv *env, OTDBtree aTree) {
  jobject jTree;
  jclass class_jOTDBtree = env->FindClass("nl/astron/lofar/sas/otb/jotdb2/jOTDBtree");
  
  jmethodID mid_jOTDBtree_cons = env->GetMethodID (class_jOTDBtree, "<init>", "(I)V");
  jTree = env->NewObject (class_jOTDBtree, mid_jOTDBtree_cons,aTree.treeID ());
  
  jfieldID fid_jOTDBtree_momID = env->GetFieldID (class_jOTDBtree, "momID", "I");
  jfieldID fid_jOTDBtree_classification = env->GetFieldID (class_jOTDBtree, "classification", "S");
  jfieldID fid_jOTDBtree_creator = env->GetFieldID (class_jOTDBtree, "creator", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_creationDate = env->GetFieldID (class_jOTDBtree, "creationDate", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_type = env->GetFieldID (class_jOTDBtree, "type", "S");
  jfieldID fid_jOTDBtree_state = env->GetFieldID (class_jOTDBtree, "state", "S");
  jfieldID fid_jOTDBtree_originalTree = env->GetFieldID (class_jOTDBtree, "originalTree", "I");
  jfieldID fid_jOTDBtree_campaign = env->GetFieldID (class_jOTDBtree, "campaign", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_starttime = env->GetFieldID (class_jOTDBtree, "starttime","Ljava/lang/String;");
  jfieldID fid_jOTDBtree_stoptime = env->GetFieldID (class_jOTDBtree, "stoptime", "Ljava/lang/String;");
  
  env->SetIntField (jTree, fid_jOTDBtree_momID, aTree.momID);
  env->SetShortField (jTree, fid_jOTDBtree_classification, aTree.classification);
  env->SetObjectField (jTree, fid_jOTDBtree_creator, env->NewStringUTF (aTree.creator.c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_creationDate, env->NewStringUTF (to_simple_string(aTree.creationDate).c_str ()));
  env->SetShortField (jTree, fid_jOTDBtree_type, aTree.type);
  env->SetShortField (jTree, fid_jOTDBtree_state, aTree.state);
  env->SetIntField (jTree, fid_jOTDBtree_originalTree, aTree.originalTree);
  env->SetObjectField (jTree, fid_jOTDBtree_campaign, env->NewStringUTF (aTree.campaign.c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_starttime, env->NewStringUTF(to_simple_string(aTree.starttime).c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_stoptime, env->NewStringUTF (to_simple_string(aTree.stoptime).c_str ()));
  
  return jTree;
}


jobject convertTreeState (JNIEnv *env, TreeState aTreeState)
{
  jobject jTreeState;
  jclass class_jTreeState = env->FindClass ("nl/astron/lofar/otb/jotdb2/jTreeState");
  jmethodID mid_jTreeState_cons = env->GetMethodID (class_jTreeState, "<init>", "()V");
  jTreeState = env->NewObject (class_jTreeState, mid_jTreeState_cons);


  jfieldID fid_jTreeState_treeID = env->GetFieldID (class_jTreeState, "treeID", "I");
  jfieldID fid_jTreeState_momID = env->GetFieldID (class_jTreeState, "momID", "I");
  jfieldID fid_jTreeState_newState = env->GetFieldID (class_jTreeState,"newState", "S");
  jfieldID fid_jTreeState_username = env->GetFieldID (class_jTreeState, "username", "Ljava/lang/String;");
  jfieldID fid_jTreeState_timestamp = env->GetFieldID (class_jTreeState, "timestamp", "Ljava/lang/String;");

  env->SetIntField (jTreeState, fid_jTreeState_treeID, aTreeState.treeID);
  env->SetIntField (jTreeState, fid_jTreeState_momID, aTreeState.momID);
  env->SetShortField (jTreeState, fid_jTreeState_newState, aTreeState.newState);
  env->SetObjectField (jTreeState, fid_jTreeState_username, env->NewStringUTF(aTreeState.username.c_str()));
  env->SetObjectField (jTreeState, fid_jTreeState_timestamp, env->NewStringUTF(to_simple_string(aTreeState.timestamp).c_str()));

  return jTreeState;
}

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_initOTDBconnection(JNIEnv *env, jobject, jstring username, jstring passwd, jstring database) {
  const char* user = env->GetStringUTFChars(username, 0);
  const char* pass = env->GetStringUTFChars(passwd, 0);
  const char* db = env->GetStringUTFChars(database, 0);
  const string u (user);
  const string p (pass);
  const string d (db);

  OTDBconn = new OTDBconnection(u, p, d);

  env->ReleaseStringUTFChars(username, user);
  env->ReleaseStringUTFChars(passwd, pass);
  env->ReleaseStringUTFChars(database, db);
}

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_isConnected(JNIEnv *, jobject) {
  jboolean connected;
  connected = OTDBconn->isConnected();
  return connected;
}


JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_connect(JNIEnv *, jobject) {
  jboolean connected;
  connected = (jboolean)OTDBconn->connect();
  return connected;
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getTreeInfo(JNIEnv *env, jobject, jint treeID, jboolean isMomID) {
  OTDBtree aTree = OTDBconn->getTreeInfo((int)treeID, isMomID);

  // Create a jOTDBtree object
  jobject myTree;
  jclass class_jOTDBtree = env->FindClass("nl/astron/lofar/sas/otb/jotdb2/jOTDBtree");
  jmethodID mid_jOTDBtree_cons = env->GetMethodID(class_jOTDBtree, "<init>", "(I)V");
  myTree = env->NewObject(class_jOTDBtree, mid_jOTDBtree_cons, treeID
			  );
  
  // Get members
  jfieldID fid_jOTDBtree_momID = env->GetFieldID (class_jOTDBtree, "momID", "I");
  jfieldID fid_jOTDBtree_classification = env->GetFieldID (class_jOTDBtree, "classification", "S");
  jfieldID fid_jOTDBtree_creator = env->GetFieldID (class_jOTDBtree, "creator", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_creationDate = env->GetFieldID (class_jOTDBtree, "creationDate", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_type = env->GetFieldID (class_jOTDBtree, "type", "S");
  jfieldID fid_jOTDBtree_state = env->GetFieldID (class_jOTDBtree, "state", "S");
  jfieldID fid_jOTDBtree_originalTree = env->GetFieldID (class_jOTDBtree, "originalTree", "I");
  jfieldID fid_jOTDBtree_campaign = env->GetFieldID (class_jOTDBtree, "campaign", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_starttime = env->GetFieldID (class_jOTDBtree, "starttime", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_stoptime = env->GetFieldID (class_jOTDBtree,"stoptime", "Ljava/lang/String;");
  
  // Fill members
  env->SetIntField(myTree, fid_jOTDBtree_momID, (jint)aTree.momID);
  env->SetShortField(myTree, fid_jOTDBtree_classification, (jint)aTree.classification);
  env->SetObjectField(myTree, fid_jOTDBtree_creator, env->NewStringUTF(aTree.creator.c_str()));
  env->SetObjectField(myTree, fid_jOTDBtree_creationDate, env->NewStringUTF(to_simple_string(aTree.creationDate).c_str()));
  env->SetShortField(myTree, fid_jOTDBtree_type, aTree.type);
  env->SetShortField(myTree, fid_jOTDBtree_state, aTree.state);
  env->SetIntField(myTree, fid_jOTDBtree_originalTree, aTree.originalTree);
  env->SetObjectField(myTree, fid_jOTDBtree_campaign, env->NewStringUTF(aTree.campaign.c_str()));
  env->SetObjectField(myTree, fid_jOTDBtree_starttime, env->NewStringUTF(to_simple_string(aTree.starttime).c_str()));
  env->SetObjectField(myTree, fid_jOTDBtree_stoptime, env->NewStringUTF(to_simple_string(aTree.stoptime).c_str()));
  
  return myTree;
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getStateList(JNIEnv *env, jobject, jint treeID, jboolean isMomID, jstring beginDate, jstring endDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  vector<TreeState> states = OTDBconn->getStateList(treeID, isMomID,time_from_string(bd), time_from_string(ed));

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
  vector<OTDBtree> trees = OTDBconn->getTreeList(treeType, classifType);
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
  jstring jstr = env->NewStringUTF(OTDBconn->errorMsg().c_str());
  return jstr;
}


JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection_getAuthToken(JNIEnv *, jobject) {
  jint token = (jint)OTDBconn->getAuthToken();
  return token;
}

OTDBconnection* getConnection ()
{
  return OTDBconn;
}



