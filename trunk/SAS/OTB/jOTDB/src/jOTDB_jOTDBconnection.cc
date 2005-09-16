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

#include <jni.h>
#include "jOTDB_jOTDBconnection.h"
#include <OTDB/OTDBconnection.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <string>

using namespace boost::posix_time;

// very ugly global parameter
OTDBconnection OTDBconn;

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    initOTDBconnection
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jOTDB_jOTDBconnection_initOTDBconnection
  (JNIEnv *env, jobject obj, jstring username, jstring passwd, jstring database)
{
  const char* user = env->GetStringUTFChars(username, 0);
  const char* pass = env->GetStringUTFChars(passwd, 0);
  const char* db = env->GetStringUTFChars(database, 0);

  OTDBconn = new OTDBconnection(user, pass, db);
  OTDBconn.connect();

  env->ReleaseStringUTFChars(username, user);
  env->ReleaseStringUTFChars(passwd, pass);
  env->ReleaseStringUTFChars(database, db);
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    isConnected
 * Signature: ()Ljava/lang/Boolean;
 */
JNIEXPORT jboolean JNICALL Java_jOTDB_jOTDBconnection_isConnected
  (JNIEnv *, jobject)
{
    jboolean connected;
    connected = OTDBconn.isConnected();
    return connected;
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    connect
 * Signature: ()Ljava/lang/Boolean;
 */
JNIEXPORT jboolean JNICALL Java_jOTDB_jOTDBconnection_connect
  (JNIEnv *, jobject)
{
    jboolean connected;
    connected = (jboolean)OTDBconn.connect();
    return connected;
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getTreeInfo
 * Signature: (Ljava/lang/String;)LjOTDB/jOTDBtree;
 */
JNIEXPORT jobject JNICALL Java_jOTDB_jOTDBconnection_getTreeInfo
  (JNIEnv *env, jobject _this, jint aTreeID)
{
  OTDBtree aTree;
  aTree = OTDBconn.getTreeInfo((int)treeID);

  // Create a jOTDBtree object
  jobject myTree;
  jclass class_jOTDBtree = env->FindClass("jOTDB/jOTDBtree");
  jmethodID mid_jOTDBtree_cons = env->GetMethodID(class_jOTDBtree, "<init>", "()V");
  myTree = env->NewObject(class_jOTDBtree, mid_jOTDBtree_cons);

  // Get members
  jfieldID fid_jOTDBtree_classification = env->GetFieldID(class_jOTDBtree, "classification", "I");
  jfieldID fid_jOTDBtree_creator = env->GetFieldID(class_jOTDBtree, "creator", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_creationDate = env->GetFieldID(class_jOTDBtree, "creationDate", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_type = env->GetFieldID(class_jOTDBtree, "type", "I");
  jfieldID fid_jOTDBtree_originalTree = env->GetFieldID(class_jOTDBtree, "originalTree", "I");
  jfieldID fid_jOTDBtree_campaign = env->GetFieldID(class_jOTDBtree, "campaign", "Ljava/lang/string;");
  jfieldID fid_jOTDBtree_starttime = env->GetFieldID(class_jOTDBtree, "starttime", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_stoptime = env->GetFieldID(class_jOTDBtree, "stoptime", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_itsTreeID = env->GetFieldID(class_jOTDBtree, "itsTreeID", "I");

  // Fill members
  env->SetIntField(myTree, fid_jOTDBtree_classification, aTree.classification);
  env->SetObjectField(myTree, fid_jOTDBtree_creator, (jstring)aTree.creator);
  env->SetObjectField(myTree, fid_jOTDBtree_creationDate, (jstring)to_simple_string(aTree.creationDate));
  env->SetIntField(myTree, fid_jOTDBtree_type, aTree.type);
  env->SetIntField(myTree, fid_jOTDBtree_state, aTree.state);
  env->SetIntField(myTree, fid_jOTDBtree_originalTree, aTree.originalTree);
  env->SetIntField(myTree, fid_jOTDBtree_starttime, (jstring)to_simple_string(aTree.starttime));
  env->SetIntField(myTree, fid_jOTDBtree_stoptime, (jstring)to_simple_string(aTree.stoptime));
  env->SetIntField(myTree, fid_jOTDBtree_itsTreeID, aTree.treeID());
  
  return aTree;
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getTreeList
 * Signature: ()Ljava/util/Vector;
 */
JNIEXPORT jintArray JNICALL Java_jOTDB_jOTDBconnection_getTreeList
  (JNIEnv *env, jobject _this, jshort treeType, jshort classifiType)
{
  vector<OTDBtree> trees;
  trees = OTDBconn.getTreeList(treeType, classifiType);
  jintArray treeIDs; 
  jint *treeID;

  treeIDs = env->NewIntArray(trees->size());

  env->SetIntArrayRegion((jintArray)treeIDs, (jsize)0, (jsize)trees->size(), trees-toArray());

  return(treeIDs);
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jOTDB_jOTDBconnection_errorMsg
  (JNIEnv *env, jobject _this)
{
  jstring jstr;
  jstr = env->NewStringUTF(OTDBconn.errorMsg());
  return jstr;
}

/*
 * Class:     jOTDB_jOTDBconnection
 * Method:    getAuthToken
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_jOTDB_jOTDBconnection_getAuthToken
  (JNIEnv * env, jobject _this)
{
    jint token;
    token = (jint)OTDBconn.getAuthToken();
    return token;
}
