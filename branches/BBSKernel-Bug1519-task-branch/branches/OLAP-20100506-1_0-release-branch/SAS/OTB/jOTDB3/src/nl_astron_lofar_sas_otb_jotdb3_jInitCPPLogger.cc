//#  jInitCPPLogger.cc: Initialize the c++ logger for OTDB
//#
//#  Copyright (C) 2002-2006
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
//#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>
#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jInitCPPLogger.h>


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jInitCPPLogger
 * Method:    initLogger
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jInitCPPLogger_initLogger(JNIEnv * env, jobject, jstring aLogName) {
  jboolean isCopy;
  const char* logname = env->GetStringUTFChars(aLogName, &isCopy);
  INIT_LOGGER(logname);
  env->ReleaseStringUTFChars (aLogName,logname);
}

