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
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBparam.h>
#include <iostream>
#include <string>

using namespace LOFAR::OTDB;
using namespace std;

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    initTreeMaintenance
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_initTreeMaintenance (JNIEnv *env, jobject jTreeMaintenance) {

  string name = getOwnerExt(env,jTreeMaintenance);
  try {
    OTDBconnection* aConn=getConnection(name);
    TreeMaintenance* aTM = new TreeMaintenance(aConn);
    theirC_ObjectMap[name+"_TreeMaintenance"]=(void*)aTM;

  } catch (exception &ex) {
    cout << "Exception during new TreeMaintenance "<< ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    loadMasterFile
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadMasterFile(JNIEnv *env, jobject jTreeMaintenance, jstring aName) {

  jint retVal;
  const char* name;
  try {

    jboolean isCopy;  
    name = env->GetStringUTFChars (aName, &isCopy);
    retVal =((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance")) ->loadMasterFile(name);
    env->ReleaseStringUTFChars (aName, name);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::loadMasterFile(" << name << ") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aName, name);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  

  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    loadComponentFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_loadComponentFile(JNIEnv *env, jobject jTreeMaintenance, jstring aName, jstring aForcedVersionNr, jstring aForcedQualifier) {

  jint retVal;
  const char* name;
  const char* forcedVersionNr;
  const char* forcedQualifier;
  try {
    jboolean isCopy;
    name = env->GetStringUTFChars (aName, &isCopy);
    forcedVersionNr = env->GetStringUTFChars (aForcedVersionNr, &isCopy);
    forcedQualifier = env->GetStringUTFChars (aForcedQualifier, &isCopy);
    retVal = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->loadComponentFile(name,forcedVersionNr,forcedQualifier);
    env->ReleaseStringUTFChars (aName, name);
    env->ReleaseStringUTFChars (aForcedVersionNr, forcedVersionNr);
    env->ReleaseStringUTFChars (aForcedQualifier, forcedQualifier);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::loadComponentFile(" << name << "," << forcedVersionNr << "," << forcedQualifier <<") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aName, name);
    env->ReleaseStringUTFChars (aForcedVersionNr, forcedVersionNr);
    env->ReleaseStringUTFChars (aForcedQualifier, forcedQualifier);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getComponentList
 * Signature: (Ljava/lang/String;Z)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentList(JNIEnv *env, jobject jTreeMaintenance, jstring aName , jboolean topOnly) {

  jobject itemVector;
  const char* name;
  try {
    jboolean isCopy;
    name = env->GetStringUTFChars (aName, &isCopy);
    vector<VICnodeDef> itemList = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getComponentList(name, topOnly);
    env->ReleaseStringUTFChars (aName, name);

    vector<VICnodeDef>::iterator itemIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, mid_Vector_add, convertVICnodeDef (env, *itemIterator));
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getComponentList(" << name << "," << topOnly << ") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aName, name);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
   
  return itemVector;
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getComponentNode
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb3/jVICnodeDef;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentNode(JNIEnv *env, jobject jTreeMaintenance, jint aNodeID) {

  VICnodeDef aNodeDef;
  try {
    aNodeDef = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getComponentNode (aNodeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getComponentNode(" << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertVICnodeDef (env, aNodeDef);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getComponentParams
 * Signature: (I)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getComponentParams(JNIEnv *env, jobject jTreeMaintenance, jint aNodeID) {
  
  jobject itemVector;
  try {
    vector<OTDBparam> itemList =((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getComponentParams(aNodeID);
    vector<OTDBparam>::iterator itemIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBparam(env, *itemIterator));
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getComponentParams(" << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    saveComponentNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jVICnodeDef;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveComponentNode(JNIEnv *env, jobject jTreeMaintenance, jobject jVICnodeDef) {

  jboolean succes;
  
  try {
    VICnodeDef aVICnodeDef = convertjVICnodeDef (env, jVICnodeDef, jTreeMaintenance);
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->saveComponentNode(aVICnodeDef);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::saveComponentNode" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    isTopComponent
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_isTopComponent(JNIEnv *env, jobject jTreeMaintenance, jint aNodeID){
  
  jboolean succes;
  try {
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->isTopComponent(aNodeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::isTopComponent(" << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    deleteComponentNode
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteComponentNode  (JNIEnv *env, jobject jTreeMaintenance, jint aNodeID) {
  jboolean succes;
  try {
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->deleteComponentNode(aNodeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::deleteComponentNode(" << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getFullComponentName
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jVICnodeDef;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getFullComponentName  (JNIEnv *env, jobject jTreeMaintenance, jobject jVICnodeDef) {

  jstring jstr;
  try {
    VICnodeDef aVICnodeDef = convertjVICnodeDef (env, jVICnodeDef,jTreeMaintenance);
     jstr = env->NewStringUTF(((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getFullComponentName(aVICnodeDef).c_str());
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getFullComponentName" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return jstr;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    buildTemplateTree
 * Signature: (IS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_buildTemplateTree(JNIEnv *env, jobject jTreeMaintenance, jint topNodeID, jshort aClassif) {

  jint treeID;
  try {
    treeID = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->buildTemplateTree (topNodeID, aClassif);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::buildTemplateTree(" << topNodeID << "," << aClassif << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return treeID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    newTemplateTree
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_newTemplateTree(JNIEnv *env, jobject jTreeMaintenance) {

  jint treeID;
  try {
    treeID = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->newTemplateTree();
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::newTemplateTree" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return treeID;
  
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    copyTemplateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_copyTemplateTree(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID) {

  jint treeID;
  try {
    treeID = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->copyTemplateTree (aTreeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::copyTemplateTree(" << aTreeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return treeID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getNode
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb3/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getNode(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jint aNodeID) {
  
  OTDBnode aNode;
  try {
    aNode = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getNode (aTreeID, aNodeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getNode(" << aTreeID << "," << aNodeID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBnode (env, aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getParam
 * Signature: (II)Lnl/astron/lofar/sas/otb/jotdb3/jOTDBparam;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getParam__II(JNIEnv *env, jobject jTreeMaintenance, jint treeID, jint paramID ) {

  OTDBparam aParam;
  try {
    aParam = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getParam (treeID, paramID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getParam(" << treeID << "," << paramID << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBparam (env, aParam);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getParam
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jOTDBnode;)Lnl/astron/lofar/sas/o
tb/jotdb3/jOTDBparam;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getParam__Lnl_astron_lofar_sas_otb_jotdb3_jOTDBnode_2  (JNIEnv *env , jobject jTreeMaintenance, jobject jOTDBnode) {

  OTDBparam aParam;
  OTDBnode aNode = convertjOTDBnode (env, jOTDBnode,jTreeMaintenance);
  try {
    aParam = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getParam(aNode);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getParam(OTDBnode) " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }


  return convertOTDBparam (env, aParam);
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    saveParam
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jOTDBparam;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveParam(JNIEnv *env, jobject jTreeMaintenance, jobject jParam) {
 
  jboolean succes;
  try {
    OTDBparam aParam = convertjOTDBparam(env,jParam,jTreeMaintenance);
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->saveParam (aParam);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::saveParam " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getItemList
 * Signature: (III)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getItemList__III(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jint topNode, jint depth) {

  jobject itemVector;
  try {
    vector<OTDBnode> itemList = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getItemList (aTreeID, topNode, depth);
    vector<OTDBnode>::iterator itemIterator;
  
    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
  
    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (env, *itemIterator));

  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getItemList(" << aTreeID << "," << topNode << "," << depth << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getItemList
 * Signature: (ILjava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getItemList__ILjava_lang_String_2(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jstring aNameFragment) {

  jobject itemVector;

  const char* nf = env->GetStringUTFChars (aNameFragment, 0);
  const string nameFragment (nf);
  
  try {
    vector<OTDBnode> itemList = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getItemList (aTreeID, nameFragment);
    vector<OTDBnode>::iterator itemIterator;
  
    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
  
    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBnode (env, *itemIterator));
  
    env->ReleaseStringUTFChars (aNameFragment, nf);
  
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getItemList(" << aTreeID << "," << nameFragment << ") " << ex.what() << endl; 

    env->ReleaseStringUTFChars (aNameFragment, nf);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return itemVector;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    dupNode
 * Signature: (IIS)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_dupNode(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jint orgNodeID, jshort newIndex) {
  
  jint anID;
  try {
    anID = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->dupNode (aTreeID, orgNodeID, newIndex);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::dupNode(" << aTreeID << "," << orgNodeID << "," << newIndex << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return anID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    addComponent
 * Signature: (IIILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_addComponent(JNIEnv *env, jobject jTreeMaintenance, jint compID, jint treeID, jint nodeID, jstring newName) {

  jint anID;
  
  const char* nN = env->GetStringUTFChars (newName, 0);
  const string newNameForComponent (nN);


  try {
    anID=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->addComponent(compID,treeID,nodeID,newNameForComponent);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::addComponent(" << compID << "," << treeID << "," << nodeID << "," << newNameForComponent << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return anID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    saveNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveNode(JNIEnv *env, jobject jTreeMaintenance, jobject jNode) {

  jboolean aB;
  try {
    OTDBnode aNode = convertjOTDBnode (env, jNode, jTreeMaintenance);
    aB=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->saveNode (aNode);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::saveNode" << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aB;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    saveNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_saveNodeList(JNIEnv *env, jobject jTreeMaintenance, jobject aNodeList) {

  jboolean succes;

  try {
    OTDBnode aNode;
    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_elementAt = env->GetMethodID(class_Vector, "elementAt", "(I)Ljava/lang/Object;");
    jmethodID mid_Vector_size = env->GetMethodID(class_Vector, "size", "()I");	  
  
    for (int i = 0; i < env->CallIntMethod (aNodeList, mid_Vector_size); i++) {	       
      aNode = convertjOTDBnode (env, env->CallObjectMethod (aNodeList, mid_Vector_elementAt, i),jTreeMaintenance);
      succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->saveNode (aNode);
      if (!succes)
	return succes;
    }
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::saveNodeList" << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    deleteNode
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jOTDBnode;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteNode(JNIEnv *env, jobject jTreeMaintenance, jobject jNode) {

  jboolean aB ;
  try {
    jclass class_jOTDBnode = env->FindClass ("nl/astron/lofar/sas/otb/jotdb3/jOTDBnode");
    jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
    jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
  
    // Get original OTDB node
    OTDBnode aNode = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getNode (env->GetIntField (jNode, fid_jOTDBnode_itsTreeID), env->GetIntField (jNode, fid_jOTDBnode_itsNodeID));
    aB=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->deleteNode(aNode);
  
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::deleteNode" << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aB;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    deleteNodeList
 * Signature: (Ljava/util/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteNodeList(JNIEnv *env, jobject jTreeMaintenance, jobject jNodeList) {
  jboolean succes;
  try {
    OTDBnode aNode;
    // Construct java Vector
    jobject jNode;
    jclass class_Vector = env->FindClass ("java/util/Vector");
    jmethodID mid_Vector_elementAt = env->GetMethodID (class_Vector, "elementAt", "(I)Ljava/lang/Object;");
    jmethodID mid_Vector_size = env->GetMethodID (class_Vector, "size", "()I");	  
  
    jclass class_jOTDBnode = env->FindClass ("nl/astron/lofar/sas/otb/jotdb3/jOTDBnode");
    jfieldID fid_jOTDBnode_itsTreeID = env->GetFieldID (class_jOTDBnode, "itsTreeID", "I");
    jfieldID fid_jOTDBnode_itsNodeID = env->GetFieldID (class_jOTDBnode, "itsNodeID", "I");
  
    for (int i = 0; i < env->CallIntMethod (jNodeList, mid_Vector_size); i++) {	  
      jNode = env->CallObjectMethod (jNodeList, mid_Vector_elementAt, i);
      // Get original OTDB node
      aNode = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getNode (env->GetIntField (jNode, fid_jOTDBnode_itsTreeID), env->GetIntField (jNode, fid_jOTDBnode_itsNodeID));
      succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->deleteNode (aNode);
      if (!succes)
	return succes;
    }
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::deleteNodeList" << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
	   
  return succes;	  
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    checkTreeConstraints
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeaintenance_checkTreeConstraints(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jint topNode) {
  jboolean aB;
  try {
  //  aB=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->checkTreeConstraints (aTreeID, topNode);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::checkTreeConstraints(" << aTreeID << "," << topNode << ") "<< ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aB;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    instanciateTree
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_instanciateTree(JNIEnv *env, jobject jTreeMaintenance, jint baseTree) {
  jint anID;
  try {
    anID=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->instanciateTree (baseTree);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::instantiateTree(" << baseTree << ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return anID;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMantenance
 * Method:    pruneTree
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_pruneTree(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jshort pruningLevel) {

  jboolean aB;
  try {
    aB= ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->pruneTree (aTreeID, pruningLevel);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::pruneTree(" << aTreeID << "," << pruningLevel << ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aB;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    exportTree
 * Signature: (IILjava/lang/String;IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_exportTree(JNIEnv *env, jobject jTreeMaintenance, jint treeID, jint topItem, jstring aName, jint outputFormat, jboolean folded) {

  jboolean isCopy;
  jboolean succes;
  const char* name = env->GetStringUTFChars (aName, &isCopy);
  try {
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->exportTree (treeID, topItem, name, (TreeMaintenance::formatType)outputFormat , folded);
    env->ReleaseStringUTFChars (aName, name);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::exportTree(" << treeID << "," << topItem << "," << name << "," << (TreeMaintenance::formatType)outputFormat << "," << folded << ") "<< ex.what() << endl; 
    env->ReleaseStringUTFChars (aName, name);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    deleteTree
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_deleteTree(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID) {
  jboolean succes;
  try {
    succes=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->deleteTree (aTreeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::deleteTree(" << aTreeID << ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    getTopNode
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb3/jOTDBnode;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_getTopNode(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID) {

  OTDBnode aNode;
  try {
    aNode = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->getTopNode (aTreeID);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::getTopNode(" << aTreeID << ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBnode (env, aNode);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    setMomInfo
 * Signature: (IILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setMomInfo(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jint aMomID, jstring aCampaign) {
  jboolean isCopy;
  jboolean succes;  
  const char* name = env->GetStringUTFChars (aCampaign, &isCopy);
  try {
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->setMomInfo (aTreeID, aMomID, name);
    env->ReleaseStringUTFChars (aCampaign, name);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::setMomInfo(" << aTreeID << "," << aMomID << "," << name <<  ") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aCampaign, name);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    setClassification
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setClassification(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jshort aClassification) {
  jboolean succes;
  try {
    succes=((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->setClassification (aTreeID, aClassification);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::setClassification(" << aTreeID << "," << aClassification <<  ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  
  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    setTreeState
 * Signature: (IS)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setTreeState(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jshort aState) {
  jboolean aB;
  try {
    ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->setTreeState (aTreeID, aState);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::setTreeState(" << aTreeID << "," << aState <<  ") " << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  
  return aB;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    setDescription
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setDescription(JNIEnv *env, jobject jTreeMaintenance, jint aTreeID, jstring aDesc) {
  jboolean isCopy;
  jboolean succes;
  const char* desc = env->GetStringUTFChars (aDesc, &isCopy);
  try {
    succes = ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->setDescription (aTreeID, desc);
    env->ReleaseStringUTFChars (aDesc, desc);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::setDescription(" << aTreeID << "," << desc <<  ") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aDesc, desc);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  

  return succes;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    setSchedule
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_setSchedule(JNIEnv *env, jobject jTreeMaintenance, jint treeID, jstring aStartTime, jstring anEndTime) {
  const char* bd = env->GetStringUTFChars (aStartTime, 0);
  const char* ed = env->GetStringUTFChars (anEndTime, 0);
  const string startTime (bd);
  const string endTime (ed);
  const ptime ts (time_from_string (startTime));
  const ptime te (time_from_string (endTime));
  jboolean aB;
  try {
    aB= ((TreeMaintenance*)getCObjectPtr(env,jTreeMaintenance,"_TreeMaintenance"))->setSchedule(treeID,ts,te);
    env->ReleaseStringUTFChars (aStartTime, bd);
    env->ReleaseStringUTFChars (anEndTime, ed);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::setSchedule(" << treeID << "," << ts << "," << te <<  ") " << ex.what() << endl; 
    env->ReleaseStringUTFChars (aStartTime, bd);
    env->ReleaseStringUTFChars (anEndTime, ed);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  
  return aB;
}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance_errorMsg(JNIEnv *env, jobject jTreeMaintenance) {
  jstring aS;
  try {
    aS = env->NewStringUTF(((OTDBconnection*)getCObjectPtr(env,jTreeMaintenance,"_OTDBconnection"))->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::errorMsg" << ex.what() << endl; 
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }  
  return aS;

}


