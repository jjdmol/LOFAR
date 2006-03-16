//#  jTreeMaintenance.cc: Maintenance on complete trees.
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
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCommon.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBparam.h>
#include <iostream>
#include <string>

using namespace LOFAR::OTDB;
using namespace std;
//using std::string;
//using std::vector;

extern OTDBconnection*  theirConn;
TreeMaintenance* theirTM;


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    initTreeMaintenance
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_initTreeMaintenance  (JNIEnv *, jobject) {
  theirTM = new TreeMaintenance (theirConn);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    loadMasterFile
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_loadMasterFile(JNIEnv *env, jobject, jstring aName) {
  jboolean isCopy;
  const char* name = env->GetStringUTFChars (aName, &isCopy);
  jint retVal = theirTM->loadMasterFile(name);
  env->ReleaseStringUTFChars (aName, name);

  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    loadComponentFile
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_loadComponentFile(JNIEnv *env, jobject, jstring aName) {
  jboolean isCopy;
  const char* name = env->GetStringUTFChars (aName, &isCopy);
  jint retVal = theirTM->loadComponentFile(name);
  env->ReleaseStringUTFChars (aName, name);

  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getComponentList
 * Signature: (Ljava/lang/String;Z)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getComponentList(JNIEnv *env, jobject, jstring aName , jboolean topOnly) {

  jboolean isCopy;
  const char* name = env->GetStringUTFChars (aName, &isCopy);
  vector<VICnodeDef> itemList = theirTM->getComponentList(name, topOnly);
  env->ReleaseStringUTFChars (aName, name);

  vector<VICnodeDef>::iterator itemIterator;

  // Construct java Vector
  jobject itemVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  itemVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

  for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
    env->CallObjectMethod(itemVector, mid_Vector_add, convertVICnodeDef (env, *itemIterator));
  
  return itemVector;
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getComponentNode
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb2/jVICnodeDef;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getComponentNode(JNIEnv *env, jobject, jint aNodeID) {
  VICnodeDef aNodeDef = theirTM->getComponentNode (aNodeID);
  return convertVICnodeDef (env, aNodeDef);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getComponentParams
 * Signature: (I)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getComponentParams(JNIEnv *env, jobject, jint aNodeID) {
  
  vector<OTDBparam> itemList = theirTM->getComponentParams(aNodeID);
  vector<OTDBparam>::iterator itemIterator;

  // Construct java Vector
  jobject itemVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  itemVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

  for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
    env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBparam(env, *itemIterator));
  
  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveComponentNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jVICnodeDef;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveComponentNode(JNIEnv *env, jobject, jobject jVICnodeDef) {

  VICnodeDef aVICnodeDef = convertjVICnodeDef (env, jVICnodeDef);
  jboolean succes = theirTM->saveComponentNode(aVICnodeDef);
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    buildTemplateTree
 * Signature: (IS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_buildTemplateTree(JNIEnv *, jobject, jint topNodeID, jshort aClassif) {
  jint treeID = theirTM->buildTemplateTree (topNodeID, aClassif);
  return treeID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    newTemplateTree
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_newTemplateTree(JNIEnv *, jobject) {
  jint treeID = theirTM->newTemplateTree();
  return treeID;
  
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    copyTemplateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_copyTemplateTree(JNIEnv *, jobject, jint aTreeID) {
  jint treeID = theirTM->copyTemplateTree (aTreeID);
  return treeID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getNode
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getNode(JNIEnv *env, jobject, jint aTreeID, jint aNodeID) {
  OTDBnode aNode = theirTM->getNode (aTreeID, aNodeID);
  return convertOTDBnode (env, aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getParam
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBparam;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getParam(JNIEnv *env, jobject, jint treeID, jint paramID ) {
  OTDBparam aNode = theirTM->getParam (treeID, paramID);
  return convertOTDBparam (env, aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveParam
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBparam;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveParam(JNIEnv *env, jobject, jobject jParam) {
  OTDBparam aParam = convertjOTDBparam(env,jParam);
  jboolean succes = theirTM->saveParam (aParam);
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getItemList
 * Signature: (III)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getItemList__III(JNIEnv *env, jobject, jint aTreeID, jint topNode, jint depth) {
  vector<OTDBnode> itemList = theirTM->getItemList (aTreeID, topNode, depth);
  vector<OTDBnode>::iterator itemIterator;
  
  // Construct java Vector
  jobject itemVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  itemVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
  
  for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
    env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (env, *itemIterator));
  
  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getItemList
 * Signature: (ILjava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getItemList__ILjava_lang_String_2(JNIEnv *env, jobject, jint aTreeID, jstring aNameFragment) {
  const char* nf = env->GetStringUTFChars (aNameFragment, 0);
  const string nameFragment (nf);
  
  vector<OTDBnode> itemList = theirTM->getItemList (aTreeID, nameFragment);
  vector<OTDBnode>::iterator itemIterator;
  
  // Construct java Vector
  jobject itemVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  itemVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
  
  for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
    env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (env, *itemIterator));
  
  env->ReleaseStringUTFChars (aNameFragment, nf);
  
  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    dupNode
 * Signature: (IIS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_dupNode(JNIEnv *, jobject, jint aTreeID, jint orgNodeID, jshort newIndex) {
  return theirTM->dupNode (aTreeID, orgNodeID, newIndex);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    addComponent
 * Signature: (III)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_addComponent(JNIEnv *, jobject, jint compID, jint treeID, jint nodeID) {
  return theirTM->addComponent(compID,treeID,nodeID);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveNode(JNIEnv *env, jobject, jobject jNode) {
  OTDBnode aNode = convertjOTDBnode (env, jNode);
  return theirTM->saveNode (aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    saveNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_saveNodeList(JNIEnv *env, jobject, jobject aNodeList) {
  jboolean succes;
  OTDBnode aNode;
  // Construct java Vector
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_elementAt = env->GetMethodID(class_Vector, "elementAt", "(I)Ljava/lang/Object;");
  jmethodID mid_Vector_size = env->GetMethodID(class_Vector, "size", "()I");	  
  
  for (int i = 0; i < env->CallIntMethod (aNodeList, mid_Vector_size); i++)
    {	       
      aNode = convertjOTDBnode (env, env->CallObjectMethod (aNodeList, mid_Vector_elementAt, i));
      succes = theirTM->saveNode (aNode);
      if (!succes)
	return succes;
    }
  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteNode(JNIEnv *env, jobject, jobject jNode) {
  jclass class_jOTDBnode = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jOTDBnode");
  jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
  jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
  
  // Get original OTDB node
  OTDBnode aNode = theirTM->getNode (env->GetIntField (jNode, fid_jOTDBnode_itsTreeID), env->GetIntField (jNode, fid_jOTDBnode_itsNodeID));
  
  return theirTM->deleteNode (aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteNodeList(JNIEnv *env, jobject, jobject jNodeList) {
  jboolean succes;
  OTDBnode aNode;
  // Construct java Vector
  jobject jNode;
  jclass class_Vector = env->FindClass ("java/util/Vector");
  jmethodID mid_Vector_elementAt = env->GetMethodID (class_Vector, "elementAt", "(I)Ljava/lang/Object;");
  jmethodID mid_Vector_size = env->GetMethodID (class_Vector, "size", "()I");	  
  
  jclass class_jOTDBnode = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jOTDBnode");
  jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
  jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
  
  for (int i = 0; i < env->CallIntMethod (jNodeList, mid_Vector_size); i++)
    {	  
      jNode = env->CallObjectMethod (jNodeList, mid_Vector_elementAt, i);
      // Get original OTDB node
      aNode = theirTM->getNode (env->GetIntField (jNode, fid_jOTDBnode_itsTreeID), env->GetIntField (jNode, fid_jOTDBnode_itsNodeID));
      succes = theirTM->deleteNode (aNode);
      if (!succes)
	return succes;
    }	   
  return succes;	  
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    checkTreeConstraints
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeaintenance_checkTreeConstraints(JNIEnv *, jobject, jint aTreeID, jint topNode) {
  //  return theirTM->checkTreeConstraints (aTreeID, topNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    instanciateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_instanciateTree(JNIEnv *, jobject, jint baseTree) {
  return theirTM->instanciateTree (baseTree);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    pruneTree
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_pruneTree(JNIEnv *, jobject, jint aTreeID, jshort pruningLevel) {
  return theirTM->pruneTree (aTreeID, pruningLevel);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    exportTree
 * Signature: (IILjava/lang/String;IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_exportTree(JNIEnv *env, jobject, jint treeID, jint topItem, jstring aName, jint outputFormat, jboolean folded) {

  jboolean isCopy;
  const char* name = env->GetStringUTFChars (aName, &isCopy);
  jboolean succes = theirTM->exportTree (treeID, topItem, name, (TreeMaintenance::formatType)outputFormat , folded);
  env->ReleaseStringUTFChars (aName, name);
  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    deleteTree
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_deleteTree(JNIEnv *env, jobject, jint aTreeID) {
  jboolean succes;
  try {
    succes=theirTM->deleteTree (aTreeID);
  } catch (exception ex) {
    cout << "Exception found: "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
    //    env->ExceptionClear();
  }
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    getTopNode
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb2/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_getTopNode(JNIEnv *env, jobject, jint aTreeID) {
  OTDBnode aNode = theirTM->getTopNode (aTreeID);
  return convertOTDBnode (env, aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setMomInfo
 * Signature: (IILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setMomInfo(JNIEnv *env, jobject, jint aTreeID, jint aMomID, jstring aCampaign) {
  jboolean isCopy;
  const char* name = env->GetStringUTFChars (aCampaign, &isCopy);
  jboolean succes = theirTM->setMomInfo (aTreeID, aMomID, name);
  env->ReleaseStringUTFChars (aCampaign, name);
  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setClassification
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setClassification(JNIEnv *, jobject, jint aTreeID, jshort aClassification) {
  return theirTM->setClassification (aTreeID, aClassification);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setTreeState
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setTreeState(JNIEnv *, jobject, jint aTreeID, jshort aState) {
  return theirTM->setTreeState (aTreeID, aState);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setDescription
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setDescription(JNIEnv *env, jobject, jint aTreeID, jstring aDesc) {
  jboolean isCopy;
  const char* desc = env->GetStringUTFChars (aDesc, &isCopy);
  jboolean succes = theirTM->setDescription (aTreeID, desc);
  env->ReleaseStringUTFChars (aDesc, desc);
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    setSchedule
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_setSchedule(JNIEnv *env, jobject, jint treeID, jstring aStartTime, jstring anEndTime) {
  const char* bd = env->GetStringUTFChars (aStartTime, 0);
  const char* ed = env->GetStringUTFChars (anEndTime, 0);
  const string startTime (bd);
  const string endTime (ed);
  const ptime ts (time_from_string (startTime));
  const ptime te (time_from_string (endTime));
  return theirTM->setSchedule(treeID,ts,te);
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance_errorMsg(JNIEnv *env, jobject) {
  return env->NewStringUTF(theirConn->errorMsg().c_str());
}


