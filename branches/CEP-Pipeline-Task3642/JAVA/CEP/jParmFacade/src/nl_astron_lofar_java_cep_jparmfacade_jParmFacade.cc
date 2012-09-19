//#  nl_astron_lofar_java_cep_jparmfacade_jParmFacade.cc: Manages the 
//#              connection with the parameter database.
//#
//#  Copyright (C) 2005-2007
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

#include <jni.h>
#include <jParmFacade/nl_astron_lofar_java_cep_jparmfacade_jParmFacade.h>
#include <jParmFacade/nl_astron_lofar_java_cep_jparmfacade_jCommon.h>
#include <ParmFacade/ParmFacade.h>
#include <iostream>
#include <string>

using namespace LOFAR::ParmDB;
using namespace std;

ParmFacade* theirPF;


/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    getRange
 * Signature: (Ljava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getRange  (JNIEnv *env, jobject obj, jstring parmNamePattern) {

  jboolean isCopy;
  jobject rangeVector;

  // create the connection with the ParmDB
  setParmDBConnection(env,obj);

  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  vector<double> rangeList;
  try {
    rangeList = theirPF->getRange(pattern);

    env->ReleaseStringUTFChars (parmNamePattern, pattern);

    vector<double>::iterator rangeIterator;
    
    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    rangeVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
    
    // Double
    jobject jDouble;
    jclass class_Double = env->FindClass ("java/lang/Double");
    jmethodID mid_Double_cons = env->GetMethodID (class_Double, "<init>", "(D)V");
    
    for (rangeIterator = rangeList.begin(); rangeIterator != rangeList.end(); rangeIterator++) {
      jDouble = env->NewObject (class_Double, mid_Double_cons, *rangeIterator);
      
      env->CallObjectMethod(rangeVector, mid_Vector_add, jDouble);
    }
  } catch (exception &ex) {
    string aStr= (string)ex.what();
    cout << "Exception during getRange("<< pattern << "): "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),aStr.c_str());
  }
  

  return rangeVector;
}


/*
 * Class:     nl_astron_lofar_java_cep_jParmFacade_jparmfacade
 * Method:    getNames
 * Signature: (Ljava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getNames  (JNIEnv *env, jobject obj, jstring parmNamePattern) {

  jboolean isCopy;
  jobject nameVector;

  // create the connection with the ParmDB
  setParmDBConnection(env,obj);

  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  vector<string> nameList;
  try {
    nameList = theirPF->getNames(pattern);

    env->ReleaseStringUTFChars (parmNamePattern, pattern);

    vector<string>::iterator nameIterator;

    // Construct java Vector
    jclass class_Vector = env->FindClass("java/util/Vector");
    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
    nameVector = env->NewObject(class_Vector, mid_Vector_cons);
    jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

    jstring jstr;
    for (nameIterator = nameList.begin(); nameIterator != nameList.end(); nameIterator++) {
      jstr = env->NewStringUTF (((string)*nameIterator).c_str());
      env->CallObjectMethod(nameVector, mid_Vector_add, jstr);
    }
  } catch (exception &ex) {
    string aStr= (string)ex.what();
    cout << "Exception during getNames("<< pattern << "): "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),aStr.c_str());
  }

  return nameVector;
}

/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    getValues
 * Signature: (Ljava/lang/String;DDIDDI)Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getValues (JNIEnv *env, jobject obj, jstring parmNamePattern, jdouble startx, jdouble endx, jint nx, jdouble starty, jdouble endy, jint ny) {

  jboolean isCopy;
  jobject result;

  // create the connection with the ParmDB
  setParmDBConnection(env,obj);


  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  map<string,vector<double> > valMap;
  try {
    valMap = theirPF->getValues(pattern,startx,endx,nx,starty,endy,ny);
    env->ReleaseStringUTFChars (parmNamePattern, pattern);

    // Construct java Map
    jclass mapClass, doubleClass, vectorClass;
    jmethodID mapInit, mapPut, vectorAdd, doubleInit, vectorInit;
    
    mapClass = env->FindClass("java/util/HashMap");
    mapInit = env->GetMethodID(mapClass, "<init>", "()V");
    result = env->NewObject(mapClass, mapInit);
    mapPut= env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    
    // Construct java Double
    jobject jDouble;
    doubleClass = env->FindClass ("java/lang/Double");
    doubleInit = env->GetMethodID (doubleClass, "<init>", "(D)V");
    
    
    // Loop through map and convert to HashMap
    
    for (map<string,vector<double> >::const_iterator valIter=valMap.begin();
         valIter != valMap.end();
         valIter++) {
      
      // Construct java Vector
      jobject valVector;
      vectorClass = env->FindClass("java/util/Vector");
      vectorInit = env->GetMethodID(vectorClass, "<init>", "()V");
      valVector = env->NewObject(vectorClass, vectorInit);
      vectorAdd = env->GetMethodID(vectorClass, "add", "(Ljava/lang/Object;)Z");
      
      
      for (vector<double>::const_iterator iter=valIter->second.begin();
	   iter != valIter->second.end();
	   iter++) {
	
        jDouble = env->NewObject (doubleClass, doubleInit, *iter);
	
	env->CallObjectMethod(valVector, vectorAdd, jDouble);
      }
      
      
      env->CallObjectMethod(result, mapPut, env->NewStringUTF(valIter->first.c_str()),valVector);
      
    }
  } catch (exception &ex) {
    cout << "Exception during getValues("<< pattern << "," << startx << ","
	 << endx << "," << nx << "," << starty << "," << endy << "," << ny 
	 << ") : "<< ex.what() << endl;
    string aStr= (string)ex.what();
    jclass newExcCls = env->FindClass("java/lang/Exception");
    if (newExcCls == 0) { 
      cout << "Unable to find the new exception class, give up." << endl;
      //      env->ReleaseStringUTFChars (parmNamePattern, pattern);
      return result;
    }

    env->ThrowNew(newExcCls,aStr.c_str());
  }
  
  return result;
}

/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    getHistory
 * Signature: (Ljava/lang/String;DDIDDI)Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getHistory (JNIEnv *env, jobject obj, jstring parmNamePattern, jdouble startx, jdouble endx, jdouble starty, jdouble endy, jdouble startSolveTime, jdouble endSolveTime) {

  jboolean isCopy;
  jobject result;

  // create the connection with the ParmDB
  setParmDBConnection(env,obj);


  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  map<string,vector<double> > valMap;
  try {
    valMap = theirPF->getHistory(pattern,startx,endx,starty,endy,startSolveTime,endSolveTime);
    env->ReleaseStringUTFChars (parmNamePattern, pattern);

    // Construct java Map
    jclass mapClass, doubleClass, vectorClass;
    jmethodID mapInit, mapPut, vectorAdd, doubleInit, vectorInit;
    
    mapClass = env->FindClass("java/util/HashMap");
    mapInit = env->GetMethodID(mapClass, "<init>", "()V");
    result = env->NewObject(mapClass, mapInit);
    mapPut= env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    
    // Construct java Double
    jobject jDouble;
    doubleClass = env->FindClass ("java/lang/Double");
    doubleInit = env->GetMethodID (doubleClass, "<init>", "(D)V");
    
    
    // Loop through map and convert to HashMap
    
    for (map<string,vector<double> >::const_iterator valIter=valMap.begin();
         valIter != valMap.end();
         valIter++) {
      
      // Construct java Vector
      jobject valVector;
      vectorClass = env->FindClass("java/util/Vector");
      vectorInit = env->GetMethodID(vectorClass, "<init>", "()V");
      valVector = env->NewObject(vectorClass, vectorInit);
      vectorAdd = env->GetMethodID(vectorClass, "add", "(Ljava/lang/Object;)Z");
      
      
      for (vector<double>::const_iterator iter=valIter->second.begin();
	   iter != valIter->second.end();
	   iter++) {
	
        jDouble = env->NewObject (doubleClass, doubleInit, *iter);
	
	env->CallObjectMethod(valVector, vectorAdd, jDouble);
      }
      
      
      env->CallObjectMethod(result, mapPut, env->NewStringUTF(valIter->first.c_str()),valVector);
      
    }
  } catch (exception &ex) {
    cout << "Exception during getHistory("<< pattern << "," << startx << ","
	 << endx << "," << starty << "," << endy << "," << startSolveTime << ","
         << endSolveTime << ") : "<< ex.what() << endl;
    string aStr= (string)ex.what();
    jclass newExcCls = env->FindClass("java/lang/Exception");
    if (newExcCls == 0) { 
      cout << "Unable to find the new exception class, give up." << endl;
      //      env->ReleaseStringUTFChars (parmNamePattern, pattern);
      return result;
    }

    env->ThrowNew(newExcCls,aStr.c_str());
  }
  
  return result;
}


void  setParmDBConnection(JNIEnv *env, jobject callerObject) {

  // get the  callerclass
  jclass jPF=env->GetObjectClass(callerObject);

  // get the methodID
  jfieldID id_PFID = env->GetFieldID (jPF, "itsParmFacadeDB","Ljava/lang/String;");

  // get the value
  jstring nstr = (jstring)env->GetObjectField (callerObject, id_PFID);

  const char* n = env->GetStringUTFChars (nstr, 0);
  const string name (n);
  // create the connection with the c++ ParmFacade
  cout << "Connect to :" << name << endl;
  theirPF=new ParmFacade(name);
  env->ReleaseStringUTFChars (nstr, n);
}

