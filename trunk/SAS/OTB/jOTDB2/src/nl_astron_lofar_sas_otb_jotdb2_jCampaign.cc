//#  jCampaign.cc: Maintenance on CampaihnInfo.
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
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCampaign.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCommon.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.h>
#include <OTDB/Campaingn.h>
#include <OTDB/CampaignInfo.h>
#include <iostream>
#include <string>

using namespace LOFAR::OTDB;
using namespace std;

extern OTDBconnection*  theirConn;
Campaign* theirCampaign;

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    initCampaign
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_initCampaign  (JNIEnv *env, jobject) {

  try {
    theirCampaign = new Campaign(theirConn);
  } catch (exception &ex) {
    cout << "Exception during new Campaign"<< ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    getCampaign
 * Signature: (Ljava/lang/String;)Lnl/astron/lofar/sas/otb/jotdb2/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_getCampaign__Ljava_lang_String_2
  (JNIEnv *env, jobject, jstring name) {
  const char* name;
  try {
    CampaingInfo aCampaignInfo;
    jboolean isCopy;
    name = env->GetStringUTFChars (aName, &isCopy);
    aCampaignInfo = theirTM->getCampaign(name);
    env->ReleaseStringUTFChars (aName, name);
  } catch (exception &ex) {
    cout << "Exception during Campaign::getCampaign(" << name << ") " << ex.what() << endl;
    env->ReleaseStringUTFChars (aName, name);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }


  return convertCampaignInfo (env, aCampaignInfo);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    getCampaign
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb2/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_getCampaign__I
  (JNIEnv *env, jobject, jint anId) {
  try {
    CampaingInfo aCampaignInfo;
    jboolean isCopy;
    aCampaignInfo = theirTM->getCampaign(anId);
  } catch (exception &ex) {
    cout << "Exception during Campaign::getCampaign(" << anId << ") " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertCampaignInfo (env, aCampaignInfo);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    getCampaignList
 * Signature: ()Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_getCampaignList
  (JNIEnv *env, jobject){
  jobject itemVector;
  try {
    jboolean isCopy;
    vector<CampaignInfo> itemList = theirTM->getCampaignList();

    vector<CampaignInfo>:iterator itemIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (itemIterator = itemList.begin(); itemIterator != itemList.end(); itemIterator++)
      env->CallObjectMethod(itemVector, mid_Vector_add, convertCampaignInfo (env, *itemIterator));
  } catch (exception &ex) {
    cout << "Exception during Campaign::getCampaignList() " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return itemVector;

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    saveCampaign
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb2/jCampaignInfo;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_saveCampaign
  (JNIEnv *env, jobject, jobject jCampaignInfo){
  jint retVal;

  try {
    CampaignInfo aCampaignInfo = convertjCampaignInfo (env, jCampaignInfo);
    retVal = theirCampaign->saveCampaign(aCampaignInfo);
  } catch (exception &ex) {
    cout << "Exception during TreeMaintenance::saveCampaign" << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_errorMsg
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jCampaign
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jCampaign_errorMsg(JNIEnv *env, jobject) {
  jstring aS;
  try {
    aS = env->NewStringUTF(theirConn->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during Campaign::errorMsg" << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return aS;

}