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

#define SBFAPCNAME        "ApcStationBeamformer"
#define VTAPCNAME         "ApcVirtualTelescope"

#define VTNAME            "VT1"
#define SBFNAME           "BF1"
#define SRGNAME           "SRG1"
#define BSNAME            "BeamServer"
#define LDSNAME           "LogicalDeviceScheduler"


#define PROPERTY_LDS_COMMAND          "LogicalDeviceScheduler_command"
#define PROPERTY_LDS_STATUS           "LogicalDeviceScheduler_status"
#define PROPERTY_VT_COMMAND           "VT1_command"
#define PROPERTY_VT_STATUS            "VT1_status"
#define PROPERTY_SBF_COMMAND          "VT1_BF1_command"
#define PROPERTY_SBF_STATUS           "VT1_BF1_status"
#define PROPERTY_SBF_DIRECTIONTYPE    "VT1_BF1_directionType"
#define PROPERTY_SBF_DIRECTIONANGLE1  "VT1_BF1_directionAngle1"
#define PROPERTY_SBF_DIRECTIONANGLE2  "VT1_BF1_directionAngle2"

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

const TProperty propertiesLDS[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"WaveFormGenerator_frequency", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, "1000000.0"},
  {"WaveFormGenerator_amplitude", GCFPValue::LPT_UNSIGNED, GCF_READWRITE_PROP, "128"},
  {"WaveFormGenerator_samplePeriod", GCFPValue::LPT_UNSIGNED, GCF_READWRITE_PROP, "2"},
};

const TPropertySet propertySetLDS = 
{
  5, "LogicalDeviceScheduler", propertiesLDS
};

const TProperty primaryPropertiesVT[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"startTime", GCFPValue::LPT_INTEGER, GCF_READWRITE_PROP, "0"},
};

const TPropertySet primaryPropertySetVT = 
{
  3, "VT1", primaryPropertiesVT
};

const TProperty primaryPropertiesSBF[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READWRITE_PROP, ""},
  {"directionType",   GCFPValue::LPT_STRING, GCF_READWRITE_PROP, "AZEL"},
  {"directionAngle1", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, "0.0"},
  {"directionAngle2", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, "0.0"},
};

const TPropertySet primaryPropertySetSBF = 
{
  5, "VT1_BF1", primaryPropertiesSBF
};


#endif
