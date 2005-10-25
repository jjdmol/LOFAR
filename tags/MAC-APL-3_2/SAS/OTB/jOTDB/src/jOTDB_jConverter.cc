//#  jOTDB_jConverter.cc: 
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
#include <jOTDB/jOTDB_jConverter.h>
#include <string>
#include <iostream>
#include <jOTDB/jOTDB_jOTDBcommon.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/ParamTypeConv.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/UnitConv.h>

using namespace LOFAR::OTDB;

static ClassifConv* classifConv;
static ParamTypeConv* paramTypeConv;
static TreeStateConv* treeStateConv;
static TreeTypeConv* treeTypeConv;
static UnitConv* unitConv;

namespace LOFAR 
{
   namespace jOTDB
     {
	JNIEXPORT void JNICALL Java_jOTDB_jConverter_initConverter
	  (JNIEnv *, jobject)
	  {
	     OTDBconn = getConnection ();
	     classifConv = new ClassifConv (OTDBconn);
	     paramTypeConv = new ParamTypeConv (OTDBconn);
	     treeStateConv = new TreeStateConv (OTDBconn);
	     treeTypeConv = new TreeTypeConv (OTDBconn);
	     unitConv = new UnitConv (OTDBconn);
	  }
	
	
	JNIEXPORT jshort JNICALL Java_jOTDB_jConverter_getClassif__Ljava_lang_String_2
	  (JNIEnv *env, jobject, jstring aConv)
	  {
	     const char* chars = env->GetStringUTFChars (aConv, 0);
	     const string str (chars);
	     
	     short ret = classifConv->get (str);
	     
	     env->ReleaseStringUTFChars (aConv, chars);	     

	     return ret;
	  }
	
	JNIEXPORT jstring JNICALL Java_jOTDB_jConverter_getClassif__S
	  (JNIEnv *env, jobject, jshort aConv)
	  {
	     jstring jstr = env->NewStringUTF (classifConv->get(aConv).c_str());
	     return jstr;
	  }
	
	JNIEXPORT jshort JNICALL Java_jOTDB_jConverter_getParamType__Ljava_lang_String_2
	  (JNIEnv *env, jobject, jstring aConv)
	  {
	     const char* chars = env->GetStringUTFChars (aConv, 0);
	     const string str (chars);
	     
	     short ret = paramTypeConv->get (str);
	     
	     env->ReleaseStringUTFChars (aConv, chars);	     

	     return ret;
	  }
	
	JNIEXPORT jstring JNICALL Java_jOTDB_jConverter_getParamType__S
	  (JNIEnv *env, jobject, jshort aConv)
	  {
	     jstring jstr = env->NewStringUTF (paramTypeConv->get(aConv).c_str());
	     return jstr;
	  }
	
	JNIEXPORT jshort JNICALL Java_jOTDB_jConverter_getTreeState__Ljava_lang_String_2
	  (JNIEnv *env, jobject, jstring aConv)
	  {
	     const char* chars = env->GetStringUTFChars (aConv, 0);
	     const string str (chars);
	     
	     short ret = treeStateConv->get (str);
	     
	     env->ReleaseStringUTFChars (aConv, chars);	     
	     
	     return ret;
	  }
	
	JNIEXPORT jstring JNICALL Java_jOTDB_jConverter_getTreeState__S
	  (JNIEnv *env, jobject, jshort aConv)
	  {
	     jstring jstr = env->NewStringUTF (treeStateConv->get(aConv).c_str());
	     return jstr;
	  }
	
	JNIEXPORT jshort JNICALL Java_jOTDB_jConverter_getTreeType__Ljava_lang_String_2
	  (JNIEnv *env, jobject, jstring aConv)
	  {
	     const char* chars = env->GetStringUTFChars (aConv, 0);
	     const string str (chars);
	     
	     short ret = treeTypeConv->get (str);
	     
	     env->ReleaseStringUTFChars (aConv, chars);	     

	     return ret;
	  }
	
	JNIEXPORT jstring JNICALL Java_jOTDB_jConverter_getTreeType__S
	  (JNIEnv *env, jobject, jshort aConv)
	  {
	     jstring jstr = env->NewStringUTF (treeTypeConv->get(aConv).c_str());
	     return jstr;
	  }
	
	JNIEXPORT jshort JNICALL Java_jOTDB_jConverter_getUnit__Ljava_lang_String_2
	  (JNIEnv *env, jobject, jstring aConv)
	  {
	     const char* chars = env->GetStringUTFChars (aConv, 0);
	     const string str (chars);
	     
	     short ret = unitConv->get (str);
	     
	     env->ReleaseStringUTFChars (aConv, chars);	     

	     return ret;
	  }
	
	JNIEXPORT jstring JNICALL Java_jOTDB_jConverter_getUnit__S
	  (JNIEnv *env, jobject, jshort aConv)
	  {
	     jstring jstr = env->NewStringUTF (unitConv->get(aConv).c_str());
	     return jstr;
	  }
     } // namespace jOTDB
} // namespace LOFAR
