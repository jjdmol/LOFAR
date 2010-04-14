//#  jCommon.h:
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

#ifndef LOFAR_JOTDB_COMMON_H
#define LOFAR_JOTDB_COMMON_H

// \file
// holds the Class converters for this connection
//
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <jni.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBvalue.h>
#include <OTDB/OTDBtree.h>
#include <OTDB/Campaign.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/TreeState.h>
#include <OTDB/TreeValue.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/CampaignInfo.h>
#include <OTDB/TreeMaintenance.h>
#include <string>
#include <map>


extern std::map<std::string,void *> theirC_ObjectMap;


void* getCObjectPtr(JNIEnv *env,jobject anObject,std::string aClassName);
LOFAR::OTDB::OTDBconnection* getConnection(std::string aName);
std::string getOwnerExt(JNIEnv *env,jobject anObject);

jobject convertOTDBnode      (JNIEnv *env, LOFAR::OTDB::OTDBnode aNode);
jobject convertOTDBvalue     (JNIEnv *env, LOFAR::OTDB::OTDBvalue aValue);
jobject convertOTDBtree      (JNIEnv *env, LOFAR::OTDB::OTDBtree aTree);
jobject convertTreeState     (JNIEnv *env, LOFAR::OTDB::TreeState aTreeState);
jobject convertVICnodeDef    (JNIEnv *env, LOFAR::OTDB::VICnodeDef aNodeDef);
jobject convertOTDBparam     (JNIEnv *env, LOFAR::OTDB::OTDBparam aParam);
jobject convertCampaignInfo  (JNIEnv *env, LOFAR::OTDB::CampaignInfo aCampaignInfo);

LOFAR::OTDB::OTDBnode     convertjOTDBnode     (JNIEnv *env, jobject jNode, jobject jTreeMaintenance);
LOFAR::OTDB::VICnodeDef   convertjVICnodeDef   (JNIEnv *env, jobject jNode, jobject jTreeMaintenance);
LOFAR::OTDB::OTDBparam    convertjOTDBparam    (JNIEnv *env, jobject jParam, jobject jTreeMaintenance);
LOFAR::OTDB::OTDBvalue    convertjOTDBvalue    (JNIEnv *env, jobject jvalue);
LOFAR::OTDB::CampaignInfo convertjCampaignInfo (JNIEnv *env, jobject jCampaignInfo,jobject jCampaign);

// Used in TreeValue
void  setTreeValConnection(JNIEnv *env, jobject callerObject);
void setErrorMsg(JNIEnv *env, jobject callerObject);

#endif
