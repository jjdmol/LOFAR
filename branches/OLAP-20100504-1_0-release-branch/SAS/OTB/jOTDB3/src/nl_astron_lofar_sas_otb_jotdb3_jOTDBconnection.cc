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


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeInfo(JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID) {
  OTDBtree aTree;
  try {
    aTree = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))->getTreeInfo((int)treeID, isMomID);
  } catch (exception &ex) {
    cout << "Exception during OTDBconnection::getTreeInfo(" << treeID << "," << isMomID << ") " << ex.what() << endl;
    
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return convertOTDBtree(env, aTree);
}


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getStateList(JNIEnv *env, jobject jOTDBconnection, jint treeID, jboolean isMomID, jstring beginDate, jstring endDate) {
  const char* bd = env->GetStringUTFChars (beginDate, 0);
  const char* ed = env->GetStringUTFChars (endDate, 0);

  jobject statesVector;
  try {

    vector<TreeState> states = ((OTDBconnection*)getCObjectPtr(env,jOTDBconnection,"_OTDBconnection"))
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


JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getTreeList(JNIEnv *env, jobject jOTDBconnection, jshort treeType, jshort classifType) {
  
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

JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection_getExecutableTrees(JNIEnv *env, jobject jOTDBconnection, jshort classifType) {
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


