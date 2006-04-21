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
#include <ParmFacade/ParmFacade.h>
#include <iostream>

using namespace LOFAR::ParmDB;
using namespace std;

ParmFacade* theirPF;

/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    initParmFacade
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_initParmFacade(JNIEnv *env, jobject, jstring tableName) {
  const char* tablename = env->GetStringUTFChars(tableName, 0);
  const string t(tablename);

  try {
    theirPF = new ParmFacade(t);
  } catch (exception &ex) {
    cout << "Exception during new ParmFacade(" << t << ") : "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
    //    env->ExceptionClear();
  }
  env->ReleaseStringUTFChars(tableName,tablename);
}

/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    getRange
 * Signature: (Ljava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getRange  (JNIEnv *env, jobject, jstring parmNamePattern) {

  jboolean isCopy;
  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  vector<double> rangeList;
  try {
    rangeList = theirPF->getRange(pattern);

  } catch (exception &ex) {
    cout << "Exception during getRange("<< pattern << "): "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
    //    env->ExceptionClear();
  }
  
  env->ReleaseStringUTFChars (parmNamePattern, pattern);

  vector<double>::iterator rangeIterator;

  // Construct java Vector
  jobject rangeVector;
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
  return rangeVector;
}


/*
 * Class:     nl_astron_lofar_java_cep_jParmFacade_jparmfacade
 * Method:    getNames
 * Signature: (Ljava/lang/String;)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getNames  (JNIEnv *env, jobject, jstring parmNamePattern) {

  jboolean isCopy;
  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  vector<string> nameList;
  try {
    nameList = theirPF->getNames(pattern);
  } catch (exception &ex) {
    cout << "Exception during getNames("<< pattern << "): "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
    //    env->ExceptionClear();
  }

  env->ReleaseStringUTFChars (parmNamePattern, pattern);

  vector<string>::iterator nameIterator;

  // Construct java Vector
  jobject nameVector;
  jclass class_Vector = env->FindClass("java/util/Vector");
  jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
  nameVector = env->NewObject(class_Vector, mid_Vector_cons);
  jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");

  jstring jstr;
  for (nameIterator = nameList.begin(); nameIterator != nameList.end(); nameIterator++) {
    jstr = env->NewStringUTF (((string)*nameIterator).c_str());
    env->CallObjectMethod(nameVector, mid_Vector_add, jstr);
  }
  return nameVector;
}



/*
 * Class:     nl_astron_lofar_java_cep_jparmfacade_jParmFacade
 * Method:    getValues
 * Signature: (Ljava/lang/String;DDIDDI)Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_java_cep_jparmfacade_jParmFacade_getValues (JNIEnv *env, jobject, jstring parmNamePattern, jdouble startx, jdouble endx, jint nx, jdouble starty, jdouble endy, jint ny) {

  jboolean isCopy;
  const char* pattern = env->GetStringUTFChars (parmNamePattern, &isCopy);
  map<string,vector<double> > valMap;
  try {
    valMap = theirPF->getValues(pattern,startx,endx,nx,starty,endy,ny);
  } catch (exception ex) {
    cout << "Exception during getValues("<< pattern << "," << startx << ","
	 << endx << "," << nx << "," << starty << "," << endy << "," << ny 
	 << ") : "<< ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
    //    env->ExceptionClear();
  }

  env->ReleaseStringUTFChars (parmNamePattern, pattern);



  // Construct java Map
  jobject result;
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
 
  return result;
}

