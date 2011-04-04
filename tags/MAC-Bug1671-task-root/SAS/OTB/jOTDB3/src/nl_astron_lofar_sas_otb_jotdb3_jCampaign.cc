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
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCampaign.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>
#include <OTDB/Campaign.h>
#include <OTDB/CampaignInfo.h>
#include <iostream>
#include <string>

using namespace LOFAR::OTDB;
using namespace std;


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    initCampaign
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_initCampaign  (JNIEnv *env, jobject jCampaign) {

  string name = getOwnerExt(env,jCampaign);



  try {
    OTDBconnection* aConn=getConnection(name);
    Campaign* aCampaign = new Campaign(aConn);
    theirC_ObjectMap[name+"_Campaign"]=(void*)aCampaign;

  } catch (exception &ex) {
    cout << "Exception during new Campaign"<< ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaign
 * Signature: (Ljava/lang/String;)Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaign__Ljava_lang_String_2
  (JNIEnv *env, jobject jCampaign, jstring aName) {
  const char* name;
  CampaignInfo aCampaignInfo;
  jboolean isCopy;

  try {
    name = env->GetStringUTFChars (aName, &isCopy);
    aCampaignInfo = ((Campaign*)getCObjectPtr(env,jCampaign,"_Campaign"))->getCampaign(name);
    env->ReleaseStringUTFChars (aName, name);
  } catch (exception &ex) {
    cout << "Exception during Campaign::getCampaign(" << aName << ") " << ex.what() << endl;
    env->ReleaseStringUTFChars (aName, name);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }


  return convertCampaignInfo (env, aCampaignInfo);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaign
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaign__I
  (JNIEnv *env, jobject jCampaign, jint anId) {
  CampaignInfo aCampaignInfo;
  try {
    aCampaignInfo = ((Campaign*)getCObjectPtr(env,jCampaign,"_Campaign"))->getCampaign(anId);
  } catch (exception &ex) {
    cout << "Exception during Campaign::getCampaign(" << anId << ") " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertCampaignInfo (env, aCampaignInfo);
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaignList
 * Signature: ()Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaignList
  (JNIEnv *env, jobject jCampaign){
  jobject itemVector;
  try {
    vector<CampaignInfo> itemList = ((Campaign*)getCObjectPtr(env,jCampaign,"_Campaign"))->getCampaignList();

    vector<CampaignInfo>::iterator itemIterator;

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
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    saveCampaign
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_saveCampaign
  (JNIEnv *env, jobject jCampaign, jobject jCampaignInfo){
  jint retVal;

  try {
    CampaignInfo aCampaignInfo = convertjCampaignInfo (env, jCampaignInfo,jCampaign);
    retVal = ((Campaign*)getCObjectPtr(env,jCampaign,"_Campaign"))->saveCampaign(aCampaignInfo);
  } catch (exception &ex) {
    cout << "Exception during Campaign::saveCampaign" << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return retVal;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_errorMsg
  (JNIEnv *, jobject jCampaign);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_errorMsg(JNIEnv *env, jobject jCampaign) {
  jstring aS;
  try {
    aS = env->NewStringUTF(((OTDBconnection*)getCObjectPtr(env,jCampaign,"_OTDBconnection"))->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during Campaign::errorMsg" << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return aS;

}
