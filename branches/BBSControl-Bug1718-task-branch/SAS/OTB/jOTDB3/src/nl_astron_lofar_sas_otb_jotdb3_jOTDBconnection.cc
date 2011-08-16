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
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>
#include <OTDB/OTDBconnection.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <string>
#include <iostream>


using namespace boost::posix_time;
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

  try {
    OTDBconnection* aPtr = new OTDBconnection(u, p, d, h);
    if ( !aPtr) {
      env->ThrowNew(env->FindClass("java/lang/Exception"),"Error creating OTDBconnection");
      return;
    }

    jclass class_jOTDBconn = env->GetObjectClass (jOTDBconnection);
    jfieldID fid_jOTDBconn_name = env->GetFieldID (class_jOTDBconn, "itsName", "Ljava/lang/String;");
    
    // itsName
    jstring str = (jstring)env->GetObjectField (jOTDBconnection, fid_jOTDBconn_name);
    jboolean isCopy;
    const char* n = env->GetStringUTFChars (str, &isCopy);
    const string name (n);

 

    theirC_ObjectMap[name+"_OTDBconnection"]=(void *)aPtr;


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
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
}

JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_isConnected(JNIEnv *env, jobject jOTDBconnection) {
  jboolean connected;

  try {
    connected = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->isConnected();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connected "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}


JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_connect(JNIEnv *env, jobject jOTDBconnection) {
  jboolean connected;
  try {
    connected = (jboolean)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->connect();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::connect "<< ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return connected;
}

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_disconnect(JNIEnv *env, jobject jOTDBconnection) {

  try {
    ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->disconnect();

    jclass class_jOTDBconn = env->GetObjectClass (jOTDBconnection);
    jfieldID fid_jOTDBconn_name = env->GetFieldID (class_jOTDBconn, "itsName", "Ljava/lang/String;");
    
    // itsName
    jstring str = (jstring)env->GetObjectField (jOTDBconnection, fid_jOTDBconn_name);
    jboolean isCopy;
    const char* n = env->GetStringUTFChars (str, &isCopy);
    const string name (n);

    std::map<std::string,void *>::iterator iter = theirC_ObjectMap.find(name+"_OTDBconnection");
    if( iter != theirC_ObjectMap.end() ) 
       theirC_ObjectMap.erase(iter);
    else
       cout << "Key is not in myMap" << '\n';

  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::disconnect "<< ex.what() << endl;
    
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

  jobject itemVector;
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

  jobject itemVector;
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

  jobject itemVector;
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

  jobject itemVector;
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

  jobject itemVector;
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

  jobject itemVector;
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

  jobject statesVector;
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

  jobject statesVector;
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

  jobject statesVector;
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

  jobject statesVector;
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
  jobject itemVector;
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
  jobject itemVector;
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
  jobject itemVector;
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
  jobject itemVector;
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

  jobject treeVector;
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

  jobject treeVector;
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

  jobject treeVector;
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

JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_newGroupID(JNIEnv *env, jobject jOTDBconnection) {
  jint token;
  try {
    token = (jint)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->newGroupID();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::newGroupID " << ex.what() << endl;

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return token;
}


JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_errorMsg(JNIEnv *env, jobject jOTDBconnection) {
  jstring jstr;
  try {
    jstr = env->NewStringUTF(((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->errorMsg().c_str());
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::errorMsg " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getAuthToken(JNIEnv *env, jobject jOTDBconnection) {
  jint token;
  try {
    token = (jint)((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getAuthToken();
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getAuthToken " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return token;
}

JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getDBName(JNIEnv *env, jobject jOTDBconnection) {
  jstring jstr;
  try {
        string aString= ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getDBName();
        jstr = env->NewStringUTF(aString.c_str());
  } catch (exception &ex) {
        cout << "Exception during OTDBconnection::getDBName " << ex.what() << endl;
    
	env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }
  return jstr;
}


