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

#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>
#include <OTDB/Campaign.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/ParamTypeConv.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/UnitConv.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <iostream>


using namespace boost::posix_time;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace std;


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection
 * Method:    initOTDBconnection
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;);
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_initOTDBconnection(JNIEnv *env, jobject jOTDBconnection, jstring username, jstring passwd, jstring database, jstring hostname) {
  const char* user = env->GetStringUTFChars(username, 0);
  const char* pass = env->GetStringUTFChars(passwd, 0);
  const char* db = env->GetStringUTFChars(database, 0);
  const char* hn = env->GetStringUTFChars(hostname, 0);
  const string u (user);
  const string p (pass);
  const string d (db);
  const string h (hn);
  const char* n(0);
  jstring str(0);

  try {
    OTDBconnection* aPtr = new OTDBconnection(u, p, d, h);
    if ( !aPtr) {
      env->ThrowNew(env->FindClass("java/lang/Exception"),"Error creating OTDBconnection");
      return;
    }

    jclass class_jOTDBconn = env->GetObjectClass (jOTDBconnection);
    jfieldID fid_jOTDBconn_name = env->GetFieldID (class_jOTDBconn, "itsName", "Ljava/lang/String;");
    
    // itsName
    str = (jstring)env->GetObjectField (jOTDBconnection, fid_jOTDBconn_name);
    jboolean isCopy;
    n = env->GetStringUTFChars (str, &isCopy);
    const string name (n);

    std::map<std::string,void *>::iterator iter;    

    theirC_ObjectMap.erase(name+"_OTDBconnection");
    theirC_ObjectMap[name+"_OTDBconnection"]=(void *)aPtr;

    LOG_DEBUG("after connect: theirC_ObjectMap contains: ");
    for (iter=theirC_ObjectMap.begin();iter!=theirC_ObjectMap.end(); ++iter) {
        LOG_DEBUG_STR(iter->second << " " << iter->first);
    }

    env->ReleaseStringUTFChars (str, n);

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
    env->ReleaseStringUTFChars(str, n);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_isConnected(JNIEnv *env, jobject jOTDBconnection) {
  jboolean connected(0);

  try {
    connected = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->isConnected();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connected "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}


JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_connect(JNIEnv *env, jobject jOTDBconnection) {
  jboolean connected(0);
  try {
    connected = (jboolean)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->connect();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connect "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_disconnect(JNIEnv *env, jobject jOTDBconnection) {
    jstring str(0);
    const char* n(0);
  try {
    std::map<std::string,void *>::iterator iter;

    ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->disconnect();

    jclass class_jOTDBconn = env->GetObjectClass (jOTDBconnection);
    jfieldID fid_jOTDBconn_name = env->GetFieldID (class_jOTDBconn, "itsName", "Ljava/lang/String;");
    
    // itsName
    str = (jstring)env->GetObjectField (jOTDBconnection, fid_jOTDBconn_name);
    jboolean isCopy;
    n = env->GetStringUTFChars (str, &isCopy);
    const string name (n);
    env->ReleaseStringUTFChars(str, n);

    bool found = false;
    std::map<std::string,void *>::iterator itr;
    std::map<std::string,void *>::iterator end;
    itr = theirC_ObjectMap.begin();
    end = theirC_ObjectMap.end();
    while (itr != end) {
        std::string n = itr->first;
        if (itr->first.find(name)!= string::npos ){
            LOG_DEBUG_STR( " found match " << itr->first);
            if (!found) found=true;
            std::map<std::string,void *>::iterator tmpitr = itr;
            itr++;
            // free memory
            vector<string> spl = StringUtil::split(tmpitr->first,'_');
            int cnt = spl.size();
            string objectclass = spl[cnt-1];
            bool flag = false;
            if (objectclass=="Campaign") {
                delete (Campaign*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="ClassifConv") {
                delete (ClassifConv*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="OTDBconnection") {
                delete (OTDBconnection*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="ParamTypeConv") {
                delete (ParamTypeConv*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="TreeMaintenance") {
                delete (TreeMaintenance*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="TreeStateConv") {
                delete (TreeStateConv*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="TreeTypeConv") {
                delete (TreeTypeConv*)(tmpitr->second);
                flag = true;
            } else if (objectclass=="UnitConv") {
                delete (UnitConv*)(tmpitr->second);
                flag = true;
            }
            if (!flag) {
                LOG_ERROR_STR(itr->first << " Failed to free memory");
            }
            theirC_ObjectMap.erase(tmpitr);
        } else {
            itr++;
        }
    }
    if (!found) {
         LOG_DEBUG_STR(name << " not found in theirC_ObjectMap");
    }

    LOG_DEBUG("after disconnect: theirC_ObjectMap contains: ");
    for (iter=theirC_ObjectMap.begin();iter!=theirC_ObjectMap.end(); ++iter) {
        LOG_DEBUG_STR(iter->second << " " << iter->first);
    }

  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::disconnect "<< ex.what() << endl;
    env->ReleaseStringUTFChars(str, n);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeInfo__IZ (JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID) {
  OTDBtree aTree;
  try {
    aTree = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeInfo((int)treeID, isMomID);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeInfo(" << treeID << "," << isMomID << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBtree(env, aTree);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeInfo__I(JNIEnv *env, jobject jOTDBconnection, jint treeID) {
  OTDBtree aTree;
  try {
    aTree = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeInfo((int)treeID);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeInfo(" << treeID <<  ") " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBtree(env, aTree);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType, jint groupID,
jstring processType, jstring processSubtype, jstring strategy) {
  const char* pt = env->GetStringUTFChars (processType, 0);
  const char* pst = env->GetStringUTFChars (processSubtype, 0);
  const char* st = env->GetStringUTFChars (strategy, 0);

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType, classifType, groupID, pt,pst, st);
    vector<OTDBtree>::iterator treeIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBtree(env, *treeIterator));
    }
    env->ReleaseStringUTFChars(processType, pt);
    env->ReleaseStringUTFChars(processSubtype, pst);
    env->ReleaseStringUTFChars(strategy, st);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << "," << classifType << "," << groupID << "," << processType << "," << processSubtype << "," << strategy << ") " << ex.what() << endl;
    
    env->ReleaseStringUTFChars(processType, pt);
    env->ReleaseStringUTFChars(processSubtype, pst);
    env->ReleaseStringUTFChars(strategy, st);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType, jint groupID,
jstring processType, jstring processSubtype) {
  const char* pt = env->GetStringUTFChars (processType, 0);
  const char* pst = env->GetStringUTFChars (processSubtype, 0);

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType, classifType, groupID, pt, pst);
    vector<OTDBtree>::iterator treeIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBtree(env, *treeIterator));
    }
    env->ReleaseStringUTFChars(processType, pt);
    env->ReleaseStringUTFChars(processSubtype, pst);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << "," << classifType << "," << groupID << "," << processType << "," << processSubtype << ") " << ex.what() << endl;

    env->ReleaseStringUTFChars(processType, pt);
    env->ReleaseStringUTFChars(processSubtype, pst);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSILjava_lang_String_2 (JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType, jint groupID,
jstring processType) {
  const char* pt = env->GetStringUTFChars (processType, 0);

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType, classifType, groupID, pt);
    vector<OTDBtree>::iterator treeIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertOTDBtree(env, *treeIterator));
    }
    env->ReleaseStringUTFChars(processType, pt);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << "," << classifType << "," << groupID << "," << processType << ") " << ex.what() << endl;

    env->ReleaseStringUTFChars(processType, pt);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SSI (JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType, jint groupID) {

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType, classifType, groupID);
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
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << "," << classifType << "," << groupID <<") " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__SS (JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType) {

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType, classifType);
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


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList__S (JNIEnv *env, jobject jOTDBconnection, jshort treeType) {

  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeList(treeType);
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
    cout << "Exception during OTDBconnection::getTreeList(" << treeType << ") " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZLjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID, jstring beginDate, jstring endDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  jobject statesVector(0);
  vector<TreeState> states;
  try {

    states = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                          ->getStateList(treeID, isMomID,time_from_string(bd), time_from_string(ed));
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

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZLjava_lang_String_2 (JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID, jstring beginDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);

  jobject statesVector(0);
  vector<TreeState> states;
  try {

    states = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                          ->getStateList(treeID, isMomID,time_from_string(bd));
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
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateList(" << treeID << "," << isMomID << "," <<
    time_from_string(bd) << ") "<< ex.what() << endl;

    env->ReleaseStringUTFChars(beginDate, bd);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(statesVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__IZ(JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID) {

  jobject statesVector(0);
  vector<TreeState> states;
  try {

    states = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                          ->getStateList(treeID, isMomID);
    vector<TreeState>::iterator statesIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    statesVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (statesIterator = states.begin(); statesIterator != states.end(); statesIterator++) {
      env->CallObjectMethod(statesVector, mid_Vector_add,convertTreeState(env, *statesIterator ));
    }


  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateList(" << treeID << "," << isMomID << ") "<< ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(statesVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList__I (JNIEnv *env, jobject jOTDBconnection, jint treeID) {

  jobject statesVector(0);
  vector<TreeState> states;
  try {

    states = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                          ->getStateList(treeID);
    vector<TreeState>::iterator statesIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    statesVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (statesIterator = states.begin(); statesIterator != states.end(); statesIterator++) {
      env->CallObjectMethod(statesVector, mid_Vector_add,convertTreeState(env, *statesIterator ));
    }


  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateList(" << treeID << ") "<< ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(statesVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getDefaultTemplates(JNIEnv *env, jobject jOTDBconnection){
  jobject itemVector(0);
  try {
    vector<DefaultTemplate> templates = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getDefaultTemplates();
    vector<DefaultTemplate>::iterator templateIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");

    itemVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");


    for (templateIterator = templates.begin(); templateIterator != templates.end(); templateIterator++) {
      env->CallObjectMethod(itemVector, mid_Vector_add, convertDefaultTemplate(env, *templateIterator));
    }
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getDefaultTamplates() " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getExecutableTrees__S (JNIEnv *env, jobject jOTDBconnection, jshort classifType) {
  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getExecutableTrees(classifType);
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
    cout << "Exception during OTDBconnection::getExecutableTrees(" << classifType << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getExecutableTrees__ (JNIEnv *env, jobject jOTDBconnection) {
  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getExecutableTrees();
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
    cout << "Exception during OTDBconnection::getExecutableTrees() " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeGroup(JNIEnv *env, jobject jOTDBconnection, jint groupType, jint periodInMinutes) {
  jobject itemVector(0);
  try {
    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeGroup(groupType,periodInMinutes);
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
    cout << "Exception during OTDBconnection::getTreeGroup(" << groupType << ","<< periodInMinutes <<") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());

  }

  return(itemVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod(JNIEnv *env, jobject jOTDBconnection, jshort treeType , jstring beginDate, jstring endDate){
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  jobject treeVector(0);
  try {

    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                       ->getTreesInPeriod(treeType,time_from_string(bd), time_from_string(ed));
    vector<OTDBtree>::iterator treeIterator;
    

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    treeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(treeVector, mid_Vector_add,convertOTDBtree(env, *treeIterator ));
    }

    env->ReleaseStringUTFChars(beginDate, bd);
    env->ReleaseStringUTFChars(endDate, ed);  
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateListTreesInPeriod(" << treeType << "," <<
    time_from_string(bd) << "," << time_from_string(ed) << ") "<< ex.what() << endl;
    
    env->ReleaseStringUTFChars(beginDate, bd);
    env->ReleaseStringUTFChars(endDate, ed);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(treeVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod(JNIEnv *env, jobject jOTDBconnection, jshort treeType , jstring beginDate){
  const char* bd = env->GetStringUTFChars (beginDate, 0);

  jobject treeVector(0);
  try {

    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                       ->getTreesInPeriod(treeType,time_from_string(bd));
    vector<OTDBtree>::iterator treeIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    treeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(treeVector, mid_Vector_add,convertOTDBtree(env, *treeIterator ));
    }

    env->ReleaseStringUTFChars(beginDate, bd);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateListTreesInPeriod(" << treeType << "," <<
    time_from_string(bd) <<  ") "<< ex.what() << endl;

    env->ReleaseStringUTFChars(beginDate, bd);
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(treeVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreesInPeriod(JNIEnv *env, jobject jOTDBconnection, jshort treeType){

  jobject treeVector(0);
  try {

    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                       ->getTreesInPeriod(treeType);
    vector<OTDBtree>::iterator treeIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    treeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(treeVector, mid_Vector_add,convertOTDBtree(env, *treeIterator ));
    }

  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getStateListTreesInPeriod(" << treeType << ") "<< ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(treeVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getModifiedTrees__Ljava_lang_String_2S
  (JNIEnv *env, jobject jOTDBconnection, jstring after, jshort treeType) {
  jobject treeVector(0);

  const char* ad = env->GetStringUTFChars (after, 0);
  try {

    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                       ->getModifiedTrees(time_from_string(ad),treeType);
    vector<OTDBtree>::iterator treeIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    treeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(treeVector, mid_Vector_add,convertOTDBtree(env, *treeIterator ));
    }
    env->ReleaseStringUTFChars(after, ad);

  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getModifiedTrees(" << after << "," << treeType << ") "<< ex.what() << endl;
    env->ReleaseStringUTFChars(after, ad);

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(treeVector);
}

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getModifiedTrees__Ljava_lang_String_2
  (JNIEnv *env, jobject jOTDBconnection, jstring after) {
  jobject treeVector(0);

  const char* ad = env->GetStringUTFChars (after, 0);
  try {

    vector<OTDBtree> trees = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
                       ->getModifiedTrees(time_from_string(ad));
    vector<OTDBtree>::iterator treeIterator;


    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    treeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    for (treeIterator = trees.begin(); treeIterator != trees.end(); treeIterator++) {
      env->CallObjectMethod(treeVector, mid_Vector_add,convertOTDBtree(env, *treeIterator ));
    }
    env->ReleaseStringUTFChars(after, ad);

  } catch (exception &ex) {

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return(treeVector);
}



JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_newGroupID(JNIEnv *env, jobject jOTDBconnection) {
  jint token(0);
  try {
    token = (jint)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->newGroupID();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::newGroupID " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return token;
}


JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_errorMsg(JNIEnv *env, jobject jOTDBconnection) {
  jstring jstr(0);
  try {
    jstr = env->NewStringUTF(((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::errorMsg " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getAuthToken(JNIEnv *env, jobject jOTDBconnection) {
  jint token(0);
  try {
    token = (jint)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getAuthToken();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getAuthToken " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return token;
}

JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getDBName(JNIEnv *env, jobject jOTDBconnection) {
  jstring jstr(0);
  try {
        string aString= ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getDBName();
        jstr = env->NewStringUTF(aString.c_str());
  } catch (exception &ex) {
        cout << "Exception during OTDBconnection::getDBName " << ex.what() << endl;
    
	env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


