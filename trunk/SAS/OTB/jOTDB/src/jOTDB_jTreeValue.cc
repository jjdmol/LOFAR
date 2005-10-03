//#  jOTDB_jTreeValue.cc: Manages the connection with the OTDB database.
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
#include <jOTDB/jOTDB_jOTDBconnection.h>
#include <OTDB/TreeValue.h>
#include <OTDB/OTDBvalue.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <string>
#include <iostream>
#include <jOTDB/jOTDB_jOTDBcommon.h>
#include <jOTDB/jOTDB_jTreeValue.h>

using namespace boost::posix_time;
using namespace LOFAR::OTDB;

TreeValue* treeval;

namespace LOFAR 
{
   namespace jOTDB
     {

	JNIEXPORT void JNICALL Java_jOTDB_jTreeValue_initTreeValue
	  (JNIEnv *, jobject)
	  {
	     OTDBconn = getConnection ();
	  }
	
	JNIEXPORT jobject JNICALL Java_jOTDB_jTreeValue_searchInPeriod
	  (JNIEnv *env, jobject, jint aTreeID, jint topNode, jint depth, jstring beginDate, jstring endDate, jboolean mostRecentOnly)
	  {
	     const char* bd = env->GetStringUTFChars (beginDate, 0);
	     const char* ed = env->GetStringUTFChars (endDate, 0);
	     const string beginTime (bd);
	     const string endTime (ed);

	     const ptime ts (time_from_string (beginTime));
	     const ptime te (time_from_string (endTime));
	     vector<OTDBvalue> valueList = treeval->searchInPeriod (topNode, depth, ts, te, mostRecentOnly);
	     vector<OTDBvalue>::iterator valueIterator;
	 
	     // get treevalues
	     treeval = new TreeValue (OTDBconn, aTreeID);

	     // Construct java Vector
	     jobject valueVector;
	     jclass class_Vector = env->FindClass("java/util/Vector");
	     jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");
	     valueVector = env->NewObject(class_Vector, mid_Vector_cons);
	     jmethodID mid_Vector_add = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");
	     
	     for (valueIterator = valueList.begin(); valueIterator != valueList.end(); valueIterator++)
	       env->CallObjectMethod(valueVector, mid_Vector_add, convertOTDBvalue (env, *valueIterator));
	     
	     env->ReleaseStringUTFChars (beginDate, bd);
	     env->ReleaseStringUTFChars (endDate, ed);	     
	     
	     return valueVector;	     
	  }	

	jobject convertOTDBvalue (JNIEnv *env, OTDBvalue aValue)
	  {
	     jobject jvalue;
	     jclass class_jOTDBvalue = env->FindClass ("jOTDB/jOTDBvalue");
	     jmethodID mid_jOTDBvalue_cons = env->GetMethodID (class_jOTDBvalue, "<init>", "(I)V");
	     jvalue = env->NewObject (class_jOTDBvalue, mid_jOTDBvalue_cons, aValue.nodeID ());
	     
	     jfieldID fid_jOTDBvalue_name = env->GetFieldID (class_jOTDBvalue, "name", "Ljava/lang/String;");
	     jfieldID fid_jOTDBvalue_value = env->GetFieldID (class_jOTDBvalue, "value", "Ljava/lang/String;");
	     jfieldID fid_jOTDBvalue_time = env->GetFieldID (class_jOTDBvalue, "time", "Ljava/lang/String;");
	     
	     env->SetObjectField (jvalue, fid_jOTDBvalue_name, env->NewStringUTF (aValue.name.c_str ()));
	     env->SetObjectField (jvalue, fid_jOTDBvalue_value, env->NewStringUTF (aValue.value.c_str ()));
	     env->SetObjectField (jvalue, fid_jOTDBvalue_time, env->NewStringUTF (to_simple_string(aValue.time).c_str ()));
	     
	     return jvalue;	 
	 }

     } // namespace jOTDB
} // namespace LOFAR
