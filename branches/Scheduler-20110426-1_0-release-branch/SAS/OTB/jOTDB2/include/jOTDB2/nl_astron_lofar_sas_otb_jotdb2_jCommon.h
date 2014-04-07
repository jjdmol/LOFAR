//#  jCommon.h: Holds a static OTDBconnection.
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
// Hold a static OTDBconnection, to be
// shared between the different JNI implementation
// Also holds the Class converters for this connection
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

using namespace LOFAR::OTDB;

jobject convertOTDBnode      (JNIEnv *env, OTDBnode aNode);
jobject convertOTDBvalue     (JNIEnv *env, OTDBvalue aValue);
jobject convertOTDBtree      (JNIEnv *env, OTDBtree aTree);
jobject convertTreeState     (JNIEnv *env, TreeState aTreeState);
jobject convertVICnodeDef    (JNIEnv *env, VICnodeDef aNodeDef);
jobject convertOTDBparam     (JNIEnv *env, OTDBparam aParam);
jobject convertCampaignInfo  (JNIEnv *env, CampaignInfo aCampaignInfo);

OTDBnode     convertjOTDBnode     (JNIEnv *env, jobject jNode);
VICnodeDef   convertjVICnodeDef   (JNIEnv *env, jobject jNode);
OTDBparam    convertjOTDBparam    (JNIEnv *env, jobject jParam);
OTDBvalue    convertjOTDBvalue    (JNIEnv *env, jobject jvalue);
CampaignInfo convertjCampaignInfo (JNIEnv *env, jobject jCampaignInfo);

// Used in TreeValue
void  setTreeValConnection(JNIEnv *env, jobject callerObject);
void setErrorMsg(JNIEnv *env, jobject callerObject);

#endif
