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

//# Includes
#include <Common/LofarLogger.h>
#include <jni.h>
#include <jOTDB/jOTDB_jOTDBconnection.h>
#include <OTDB/OTDBconnection.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <string>
#include <iostream>
#include <jOTDB/jOTDB_jOTDBcommon.h>

using namespace boost::posix_time;
using namespace LOFAR::OTDB;

namespace LOFAR 
{
   namespace jOTDB
     {
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    initOTDBconnection
	 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
	 */
	JNIEXPORT void JNICALL jOTDB::Java_jOTDB_jOTDBconnection_initOTDBconnection
          (JNIEnv *env, jobject, jstring username, jstring passwd, jstring database)	  
	{
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
	
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    isConnected
	 * Signature: ()Ljava/lang/Boolean;
	 */
	JNIEXPORT jboolean JNICALL jOTDB::Java_jOTDB_jOTDBconnection_isConnected
	  (JNIEnv *, jobject)
	  {
	     jboolean connected;
	     connected = OTDBconn->isConnected();
	     return connected;
	  }
	
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    connect
	 * Signature: ()Ljava/lang/Boolean;
	 */
	JNIEXPORT jboolean JNICALL jOTDB::Java_jOTDB_jOTDBconnection_connect
	  (JNIEnv *, jobject)
	  {
	     jboolean connected;
	     connected = (jboolean)OTDBconn->connect();
	     return connected;
	  }
	
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    getTreeInfo
	 * Signature: (Ljava/lang/String;)LjOTDB/jOTDBtree;
	 */
	JNIEXPORT jobject JNICALL jOTDB::Java_jOTDB_jOTDBconnection_getTreeInfo
	  (JNIEnv *env, jobject, jint treeID)
	  {
	     OTDBtree aTree = OTDBconn->getTreeInfo((int)treeID);
	     
	     // Create a jOTDBtree object
	     jobject myTree;
	     jclass class_jOTDBtree = env->FindClass("jOTDB/jOTDBtree");
	     jmethodID mid_jOTDBtree_cons = env->GetMethodID(class_jOTDBtree, "<init>", "(I)V");
	     myTree = env->NewObject(class_jOTDBtree, mid_jOTDBtree_cons, treeID);
	     
	     // Get members
	     jfieldID fid_jOTDBtree_classification = env->GetFieldID (class_jOTDBtree, "classification", "S");
	     jfieldID fid_jOTDBtree_creator = env->GetFieldID (class_jOTDBtree, "creator", "Ljava/lang/String;");
	     jfieldID fid_jOTDBtree_creationDate = env->GetFieldID (class_jOTDBtree, "creationDate", "Ljava/lang/String;");
	     jfieldID fid_jOTDBtree_type = env->GetFieldID (class_jOTDBtree, "type", "S");
	     jfieldID fid_jOTDBtree_state = env->GetFieldID (class_jOTDBtree, "state", "S");
	     jfieldID fid_jOTDBtree_originalTree = env->GetFieldID (class_jOTDBtree, "originalTree", "I");
	     jfieldID fid_jOTDBtree_campaign = env->GetFieldID (class_jOTDBtree, "campaign", "Ljava/lang/String;");
	     jfieldID fid_jOTDBtree_starttime = env->GetFieldID (class_jOTDBtree, "starttime", "Ljava/lang/String;");
	     jfieldID fid_jOTDBtree_stoptime = env->GetFieldID (class_jOTDBtree, "stoptime", "Ljava/lang/String;");   

	     // Fill members
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

	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    getTreeList
	 * Signature: ()Ljava/util/Vector;
	 */
	JNIEXPORT jobject JNICALL jOTDB::Java_jOTDB_jOTDBconnection_getTreeList
	  (JNIEnv *env, jobject, jshort treeType, jshort classifiType)
	  {
	     vector<OTDBtree> trees = OTDBconn->getTreeList(treeType, classifiType);     
	     vector<OTDBtree>::iterator treesIterator;
	     
	     // Construct java Integer
	     jobject treeID;
	     jclass class_Integer = env->FindClass("java/lang/Integer");
	     jmethodID mid_Integer_cons = env->GetMethodID(class_Integer, "<init>", "(I)V");
	     
	     // Construct java Vector
	     jobject vecTreeIDs;
	     jclass class_Vector = env->FindClass("java/util/Vector");
	     jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
	     vecTreeIDs = env->NewObject(class_Vector, mid_Vector_cons);
	     jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
	     	     
	     for (treesIterator = trees.begin(); treesIterator != trees.end(); treesIterator++)
	       {
		  treeID = env->NewObject(class_Integer, mid_Integer_cons, (jint)treesIterator->treeID());
		  env->CallObjectMethod(vecTreeIDs, mid_Vector_add, treeID);
	       }
	     
	     return(vecTreeIDs);
	  }
	
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    errorMsg
	 * Signature: ()Ljava/lang/String;
	 */
	JNIEXPORT jstring JNICALL jOTDB::Java_jOTDB_jOTDBconnection_errorMsg
	  (JNIEnv *env, jobject)
	  {
	     jstring jstr = env->NewStringUTF(OTDBconn->errorMsg().c_str());
	     return jstr;
	  }
	
	/*
	 * Class:     jOTDB_jOTDBconnection
	 * Method:    getAuthToken
	 * Signature: ()I
	 */
	JNIEXPORT jint JNICALL jOTDB::Java_jOTDB_jOTDBconnection_getAuthToken
	  (JNIEnv *, jobject)
	  {
	     jint token = (jint)OTDBconn->getAuthToken();
	     return token;
	  }
	
	OTDBconnection* getConnection ()
	  {
	     return OTDBconn;
	  }
     } // namespace jOTDB
} // namespace LOFAR
