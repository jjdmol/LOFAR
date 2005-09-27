//#  jOTDB_jTreeMaintenance.cc: Maintenance on complete trees.
//#
//#  Copyright (C) 2002-2005
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
#include <jOTDB/jOTDB_jOTDBcommon.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/VICnodeDef.h>
#include <string>
#include <iostream>

using namespace LOFAR::OTDB;

TreeMaintenance *treemain;

namespace LOFAR 
{
   namespace jOTDB
     {
       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_initTreeMaintenance
         (JNIEnv *, jobject)
       {
	 treemain = new TreeMaintenance (OTDBconn);
       }

       JNIEXPORT jobject JNICALL Java_jOTDB_jTreeMaintenance_getNode__I
         (JNIEnv *env, jobject, jint aNodeID)
       {
	 VICnodeDef aNodeDef = treemain->getNode (aNodeID);
	 
	 jobject jNodeDef;
	 jclass class_jVICnodeDef = env->FindClass ("jOTDB/jVICnodeDef");
	 jmethodOD mid_jVICnodeDef_cons = env->GetMethodID (class_jVICnodeDef, "<init>", "()V");
	 jNodeDef = env->NewObject (class_jVICnodeDef, mid_jVICnodeDef_cons);

	 jfieldID fid_jVICnodeDef_name = env->GetFieldID (class_jVICnodeDef, "name", "Ljava/lang/String;");
	 jfieldID fid_jVICnodeDef_version = env->GetFieldID (class_jVICnodeDef, "version", "I");
	 jfieldID fid_jVICnodeDef_classif = env->GetFieldID (class_jVICnodeDef, "classif", "S");
	 jfieldID fid_jVICnodeDef_constraints = env->GetFieldID (class_jVICnodeDef, "constraints", "Ljava/lang/String;");
	 jfieldID fid_jVICnodeDef_description = env->GetFieldID (class_jVICnodeDef, "description", "Ljava/lang/String;");
	 jfieldID fid_jVICnodeDef_itsNodeID = env->GetFieldID (class_jVICnodeDef, "itsNodeID", "I");

	 env->SetObjectField (jNodeDef, fid_jVICnodeDef_name, env->NewStringUTF (aNodeDef.name.c_str()));
	 env->SetIntField (jNodeDef, fid_jVICnodeDef_version, aNodeDef.version);
	 env->SetShortField (jNodeDef, fid_jVICnodeDef_classif, aNodeDef.classif);
	 env->SetObjectField (jNodeDef, fid_jVICnodeDef_constraints, env->NewStringUTF (aNodeDef.constraints.c_str()));
	 env->SetObjectField (jNodeDef, fid_jVICnodeDef_description, env->NewStringUTF (aNodeDef.description.c_str()));
	 env->SetIntField (jNodeDef, fid_jVICnodeDef_itsNodeID, aNodeDef.nodeID());

	 return jNodeDef;
       }
       
       JNIEXPORT jint JNICALL Java_jOTDB_jTreeMaintenance_buildTemplateTree
         (JNIEnv *, jobject, jint topNodeID, jshort aClassif)
       {
	 jint treeID = treemain->buildTemplateTree (topNodeID, aClassif);
	 return treeID;
       }

       JNIEXPORT jint JNICALL Java_jOTDB_jTreeMaintenance_copyTemplateTree
         (JNIEnv *, jobject, jint aTreeID)
       {
	 jint treeID = treemain->copyTemplateTree (aTreeID);
	 return treeID;
       }

       JNIEXPORT jobject JNICALL Java_jOTDB_jTreeMaintenance_getNode__II
         (JNIEnv *, jobject, jint aTreeID, jint aNodeID)
       {
	 OTDBnode aNode = treemain->getNode (aTreeID, aNodeID);
	 return convertOTDBnode (aNode);
       }

       JNIEXPORT jobject JNICALL Java_jOTDB_jTreeMaintenance_getItemList__III
         (JNIEnv *env, jobject, jint aTreeID, jint topNode, jint depth)
       {
	 vector<OTDBnode> itemList = treemain->getItemList (aTreeID, topNode, depth);
	 vector<OTDBnode>::iterator itemIterator;
	 
	 // Construct java Vector
	 jobject itemVector;
	 jclass class_Vector = env->FindClass("java/util/Vector");
	 jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
	 itemVector = env->NewObject(class_Vector, mid_Vector_cons);
	 jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
	 
	 for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
	   env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (itemIterator);
	   
	 return itemVector;
       }
       
       JNIEXPORT jobject JNICALL Java_jOTDB_jTreeMaintenance_getItemList__ILjava_lang_String_2
         (JNIEnv *env, jobject, jint aTreeID, jstring aNameFragment)
       {
	 const char* nf = env->GetStringUTFChars (aNameFragment);
	 const string nameFragment (nf);
	 
	 vector<OTDBnode> itemList = treemain->getItemList (aTreeID, nameFragment);
	 vector<OTDBnode>::iterator itemIterator;
	 
	 // Construct java Vector
	 jobject itemVector;
	 jclass class_Vector = env->FindClass("java/util/Vector");
	 jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
	 itemVector = env->NewObject(class_Vector, mid_Vector_cons);
	 jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
	 
	 for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
	   env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (itemIterator));
			 
	 env->ReleaseStringUTFChars (aNameFragment, nf);
	
	 return itemVector;
       }

       JNIEXPORT jint JNICALL Java_jOTDB_jTreeMaintenance_dupNode
         (JNIEnv *, jobject, jint aTreeID, jint orgNodeID, jshort newIndex)
       {
	 jint nodeID = treemain->dupNode (aTreeID, orgNodeID, newIndex);
	 return nodeID;
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_saveNode
         (JNIEnv *, jobject, jobject aNode)
       {
	 
	 jboolean succes = treemain->saveNode (
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_saveNodeList
         (JNIEnv *, jobject, jobject)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_deleteNode
         (JNIEnv *, jobject, jobject)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_deleteNodeList
         (JNIEnv *, jobject, jobject)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_checkTreeConstraints
         (JNIEnv *, jobject, jint, jint)
       {
       }

       JNIEXPORT jint JNICALL Java_jOTDB_jTreeMaintenance_instanciateTree
         (JNIEnv *, jobject, jint)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_pruneTree
         (JNIEnv *, jobject, jint, jshort)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_deleteTree
         (JNIEnv *, jobject, jint)
       {
       }

       JNIEXPORT jobject JNICALL Java_jOTDB_jTreeMaintenance_getTopNode
         (JNIEnv *, jobject, jint)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_setClassification
         (JNIEnv *, jobject, jint, jshort)
       {
       }

       JNIEXPORT jboolean JNICALL Java_jOTDB_jTreeMaintenance_setTreeState
         (JNIEnv *, jobject, jint, jshort)
       {
       }

       JNIEXPORT jstring JNICALL Java_jOTDB_jTreeMaintenance_errorMsg
         (JNIEnv *, jobject)
       {
       }

       jobject convertOTDBnode (OTDBnode aNode)
	 {
	   jobject jNode;
	   jclass class_jOTDBnode = env->FindClass ("jOTDB/jOTDBnode");
	   jmethodOD mid_jOTDB_cons = env->GetMethodID (class_jOTDBnode, "<init>", "()V");
	   jNode = env->NewObject (class_jOTDBnode, mid_jOTDBnode_cons);
	   
	   jfieldID fid_jOTDBnode_name = env->GetFieldID (class_jOTDBnode, "name", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_index = env->GetFieldID (class_jOTDBnode, "index", "S");
	   jfieldID fid_jOTDBnode_leaf = env->GetFieldID (class_jOTDBnode, "leaf", "Z");
	   jfieldID fid_jOTDBnode_instances = env->GetFieldID (class_jOTDBnode, "instances", "S;");
	   jfieldID fid_jOTDBnode_limits = env->GetFieldID (class_jOTDBnode, "limits", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_description = env->GetFieldID (class_jOTDBnode, "description", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
	   jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
	   jfieldID fid_jOTDBnode_itsParentID = env->GetFieldID (class_jOTDBnode, "itsParentID", "I");
	   jfieldID fid_jOTDBnode_itsParamDefID = env->GetFieldID (class_jOTDBnode, "itsParamDefID", "I");
	   
	   env->SetObjectField (jNode, fid_jOTDBnode_name, env->NewStringUTF (aNode.name.c_str()));
	   env->SetShortField (jNode, fid_jOTDBnode_index, aNode.index);
	   env->SetBooleanField (jNode, fid_jOTDBnode_leaf, aNode.leaf);
	   env->SetShortField (jNode, fid_jOTDBnode_instances, aNode.instances);
	   env->SetObjectField (jNode, fid_jOTDBnode_limits, env->NewStringUTF (aNode.limits.c_str()));
	   env->SetObjectField (jNode, fid_jOTDBnode_description, env->NewStringUTF (aNode.description.c_str()));
	   env->SetIntField (jNode, fid_jOTDBnode_itsTreeID, aNode.treeID());
	   env->SetIntField (jNode, fid_jOTDBnode_itsNodeID, aNode.nodeID());
	   env->SetIntField (jNode, fid_jOTDBnode_itsParentID, aNode.parentID());
	   env->SetIntField (jNode, fid_jOTDBnode_itsParamDefID, aNode.paramDefID());
	   
	   return jNode;	 
	 }
       
       OTDBnode convertjOTDBnode (jobject jNode)
	 {
	   OTDBnode aNode();

	   jclass class_jOTDBnode = env->FindClass ("jOTDB/jOTDBnode");
	   jfieldID fid_jOTDBnode_name = env->GetFieldID (class_jOTDBnode, "name", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_index = env->GetFieldID (class_jOTDBnode, "index", "S");
	   jfieldID fid_jOTDBnode_leaf = env->GetFieldID (class_jOTDBnode, "leaf", "Z");
	   jfieldID fid_jOTDBnode_instances = env->GetFieldID (class_jOTDBnode, "instances", "S;");
	   jfieldID fid_jOTDBnode_limits = env->GetFieldID (class_jOTDBnode, "limits", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_description = env->GetFieldID (class_jOTDBnode, "description", "Ljava/lang/String;");
	   jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
	   jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
	   jfieldID fid_jOTDBnode_itsParentID = env->GetFieldID (class_jOTDBnode, "itsParentID", "I");
	   jfieldID fid_jOTDBnode_itsParamDefID = env->GetFieldID (class_jOTDBnode, "itsParamDefID", "I");
	   
	   const char* n = env->GetStringUTFChars ((jstring)env->GetObjectField (jNode, fid_jOTDBnode_name));
	   const string name (n);
	   aNode.name = name;
	   env->ReleaseStringUTFChars ((jstring)env->GetObjectField (jNode, fid_jOTDBnode_name), n);

	   env->GetShortField (jNode, fid_jOTDBnode_index, aNode.index);
	   env->GetBooleanField (jNode, fid_jOTDBnode_leaf, aNode.leaf);
	   env->GetShortField (jNode, fid_jOTDBnode_instances, aNode.instances);
	   env->GetObjectField (jNode, fid_jOTDBnode_limits, env->NewStringUTF (aNode.limits.c_str()));
	   env->GetObjectField (jNode, fid_jOTDBnode_description, env->NewStringUTF (aNode.description.c_str()));
	   env->GetIntField (jNode, fid_jOTDBnode_itsTreeID, aNode.treeID());
	   env->GetIntField (jNode, fid_jOTDBnode_itsNodeID, aNode.nodeID());
	   env->GetIntField (jNode, fid_jOTDBnode_itsParentID, aNode.parentID());
	   env->GetIntField (jNode, fid_jOTDBnode_itsParamDefID, aNode.paramDefID());
	   
	   return jNode;	 

	 }

     } // namespace jOTDB
} // namespace LOFAR
