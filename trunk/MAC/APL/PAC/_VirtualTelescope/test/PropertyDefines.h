//#  PropertyDefines.h: common defines for the AVT package
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
//#  $Id$

#ifndef PropertyDefines_H
#define PropertyDefines_H

string sSBFName           = "AVTTestSBF1";
const string sSBFAPCName  = "AVTTestSBFAPC";
const string sSBFAPCScope = "AVTTestSBFAPCScope";
string sVTName            = "AVTTestVT1";
const string sVTAPCName   = "AVTTestVTAPC";
const string sVTAPCScope  = "AVTTestVTAPCScope";
string sBSName            = "BeamServer";

string ssep      = "_";
string scopeLCU1 = "LCU1";
string scopeLDS  = "LogicalDeviceScheduler";
string scopeVT1  = "VT1";
string scopeSBF1 = "SBF1";

string propertyLDScommand = scopeLDS+ssep+string("command");
string propertyLDSstatus  = scopeLDS+ssep+string("status");
string propertyVTcommand  = scopeVT1+ssep+string("command");
string propertyVTstatus   = scopeVT1+ssep+string("status");
string propertySBFcommand = scopeSBF1+ssep+string("command");
string propertySBFstatus  = scopeSBF1+ssep+string("status");
string propertySBFdirectionType   = scopeSBF1+ssep+string("directionType");
string propertySBFdirectionAngle1 = scopeSBF1+ssep+string("directionAngle1");
string propertySBFdirectionAngle2 = scopeSBF1+ssep+string("directionAngle2");

string scopedPropertyLDScommand = scopeLCU1+ssep+propertyLDScommand;
string scopedPropertyLDSstatus  = scopeLCU1+ssep+propertyLDSstatus;
string scopedPropertyVTcommand  = scopeLCU1+ssep+propertyVTcommand;
string scopedPropertyVTstatus   = scopeLCU1+ssep+propertyVTstatus;
string scopedPropertySBFcommand = scopeLCU1+ssep+scopeVT1+ssep+propertySBFcommand;
string scopedPropertySBFstatus  = scopeLCU1+ssep+scopeVT1+ssep+propertySBFstatus;
string scopedPropertySBFdirectionType   = scopeLCU1+ssep+scopeVT1+ssep+propertySBFdirectionType;
string scopedPropertySBFdirectionAngle1 = scopeLCU1+ssep+scopeVT1+ssep+propertySBFdirectionAngle1;
string scopedPropertySBFdirectionAngle2 = scopeLCU1+ssep+scopeVT1+ssep+propertySBFdirectionAngle2;

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

const TProperty propertiesLDS[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
};

const TPropertySet propertySetLDS = 
{
  2, "PAC_LogicalDeviceScheduler", propertiesLDS
};

const TProperty primaryPropertiesVT[] =
{
  {"start-time", GCFPValue::LPT_INTEGER, GCF_READWRITE_PROP, "0"},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
};

const TPropertySet primaryPropertySetVT = 
{
  2, "PAC_VT", primaryPropertiesVT
};

const TProperty primaryPropertiesSBF[] =
{
  {"directionType",   GCFPValue::LPT_STRING, GCF_READWRITE_PROP, "AZEL"},
  {"directionAngle1", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, "0.0"},
  {"directionAngle2", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, "0.0"},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
};

const TPropertySet primaryPropertySetSBF = 
{
  2, "PAC_VT_SBF", primaryPropertiesSBF
};


#endif
