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
#define SRAPCNAME         "ApcStationReceptor"
#define SRGAPCNAME        "ApcStationReceptorGroup"

#define VTNAME            "VT1"
#define SBFNAME           "BF1"
#define SRGNAME           "SRG1"
#define BSNAME            "BeamServer"
#define LDSNAME           "LogicalDeviceScheduler"


#define PROPERTY_LDS_COMMAND          "PAC_LogicalDeviceScheduler_command"
#define PROPERTY_LDS_STATUS           "PAC_LogicalDeviceScheduler_status"
#define PROPERTY_LDS_WG_FREQUENCY     "PAC_LogicalDeviceScheduler_WaveFormGenerator_frequency"
#define PROPERTY_LDS_WG_AMPLITUDE     "PAC_LogicalDeviceScheduler_WaveFormGenerator_amplitude"
#define PROPERTY_LDS_WG_SAMPLEPERIOD  "PAC_LogicalDeviceScheduler_WaveFormGenerator_samplePeriod"
#define PROPERTY_VT_COMMAND           "PAC_VT1_command"
#define PROPERTY_VT_STATUS            "PAC_VT1_status"
#define PROPERTY_SBF_COMMAND          "PAC_VT1_BF1_command"
#define PROPERTY_SBF_STATUS           "PAC_VT1_BF1_status"
#define PROPERTY_SBF_DIRECTIONTYPE    "PAC_VT1_BF1_directionType"
#define PROPERTY_SBF_DIRECTIONANGLE1  "PAC_VT1_BF1_directionAngle1"
#define PROPERTY_SBF_DIRECTIONANGLE2  "PAC_VT1_BF1_directionAngle2"

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

const TProperty propertiesLDS[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READABLE_PROP, ""},
  {"WaveFormGenerator_frequency", GCFPValue::LPT_DOUBLE, GCF_WRITABLE_PROP, "1500000.0"},
  {"WaveFormGenerator_amplitude", GCFPValue::LPT_UNSIGNED, GCF_WRITABLE_PROP, "128"},
};

const TPropertySet propertySetLDS = 
{
  4, "PAC_LogicalDeviceScheduler", propertiesLDS
};

const TProperty primaryPropertiesVT[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READABLE_PROP, ""},
  {"startTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
  {"stopTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
};

const TPropertySet primaryPropertySetVT1 = 
{
  4, "PAC_VT1", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT2 = 
{
  4, "PAC_VT2", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT3 = 
{
  4, "PAC_VT3", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT4 = 
{
  4, "PAC_VT4", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT5 = 
{
  4, "PAC_VT5", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT6 = 
{
  4, "PAC_VT6", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT7 = 
{
  4, "PAC_VT7", primaryPropertiesVT
};

const TPropertySet primaryPropertySetVT8 = 
{
  4, "PAC_VT8", primaryPropertiesVT
};

const TProperty propertiesSR[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READABLE_PROP, ""},
  {"startTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
  {"stopTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
  {"filter", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},  // 1,2,3,4
  {"antenna", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"}, // 0=LBA, 1=HBA
  {"power", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"}, // 0=off, 1=on
};

const TPropertySet propertySetSR1 = 
{
  7, "PAC_RCU1", propertiesSR
};

const TPropertySet propertySetSR2 = 
{
  7, "PAC_RCU2", propertiesSR
};

const TPropertySet propertySetSR3 = 
{
  7, "PAC_RCU3", propertiesSR
};

const TPropertySet propertySetSR4 = 
{
  7, "PAC_RCU4", propertiesSR
};

const TPropertySet propertySetSR5 = 
{
  7, "PAC_RCU5", propertiesSR
};

const TPropertySet propertySetSR6 = 
{
  7, "PAC_RCU6", propertiesSR
};

const TPropertySet propertySetSR7 = 
{
  7, "PAC_RCU7", propertiesSR
};

const TPropertySet propertySetSR8 = 
{
  7, "PAC_RCU8", propertiesSR
};

const TProperty propertiesSRG[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READABLE_PROP, ""},
  {"startTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
  {"stopTime", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},
  {"filter", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"},  // 1,2,3,4
  {"antenna", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"}, // 0=LBA, 1=HBA
  {"power", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "0"}, // 0=off, 1=on
};

const TPropertySet propertySetSRG1 = 
{
  7, "PAC_SRG1", propertiesSRG
};

const TPropertySet propertySetSRG2 = 
{
  7, "PAC_SRG2", propertiesSRG
};

const TPropertySet propertySetSRG3 = 
{
  7, "PAC_SRG3", propertiesSRG
};

const TPropertySet propertySetSRG4 = 
{
  7, "PAC_SRG4", propertiesSRG
};

const TProperty primaryPropertiesSBF[] =
{
  {"command", GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, ""},
  {"status", GCFPValue::LPT_STRING, GCF_READABLE_PROP, ""},
  {"directionType",   GCFPValue::LPT_STRING, GCF_WRITABLE_PROP, "LMN"},
  {"directionAngle1", GCFPValue::LPT_DOUBLE, GCF_WRITABLE_PROP, "0.0"},
  {"directionAngle2", GCFPValue::LPT_DOUBLE, GCF_WRITABLE_PROP, "0.0"},
};

const TPropertySet primaryPropertySetSBF1 = 
{
  5, "PAC_VT1_BF1", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF2 = 
{
  5, "PAC_VT2_BF2", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF3 = 
{
  5, "PAC_VT3_BF3", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF4 = 
{
  5, "PAC_VT4_BF4", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF5 = 
{
  5, "PAC_VT5_BF5", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF6 = 
{
  5, "PAC_VT6_BF6", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF7 = 
{
  5, "PAC_VT7_BF7", primaryPropertiesSBF
};

const TPropertySet primaryPropertySetSBF8 = 
{
  5, "PAC_VT8_BF8", primaryPropertiesSBF
};

const TProperty propertiesBeamServer[] =
{
  {"power000_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power000_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power001_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power001_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power002_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power002_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power003_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power003_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power004_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power004_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power005_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power005_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power006_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power006_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power007_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power007_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power008_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power008_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power009_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power009_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power010_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power010_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power011_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power011_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power012_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power012_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power013_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power013_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power014_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power014_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power015_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power015_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power016_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power016_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power017_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power017_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power018_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power018_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power019_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power019_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power020_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power020_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power021_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power021_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power022_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power022_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power023_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power023_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power024_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power024_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power025_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power025_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power026_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power026_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power027_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power027_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power028_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power028_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power029_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power029_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power030_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power030_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power031_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power031_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power032_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power032_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power033_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power033_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power034_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power034_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power035_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power035_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power036_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power036_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power037_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power037_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power038_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power038_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power039_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power039_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power040_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power040_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power041_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power041_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power042_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power042_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power043_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power043_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power044_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power044_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power045_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power045_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power046_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power046_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power047_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power047_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power048_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power048_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power049_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power049_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power050_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power050_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power051_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power051_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power052_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power052_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power053_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power053_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power054_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power054_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power055_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power055_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power056_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power056_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power057_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power057_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power058_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power058_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power059_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power059_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power060_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power060_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power061_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power061_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power062_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power062_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power063_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power063_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power064_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power064_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power065_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power065_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power066_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power066_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power067_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power067_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power068_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power068_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power069_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power069_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power070_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power070_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power071_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power071_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power072_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power072_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power073_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power073_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power074_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power074_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power075_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power075_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power076_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power076_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power077_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power077_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power078_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power078_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power079_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power079_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power080_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power080_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power081_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power081_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power082_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power082_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power083_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power083_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power084_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power084_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power085_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power085_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power086_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power086_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power087_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power087_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power088_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power088_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power089_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power089_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power090_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power090_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power091_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power091_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power092_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power092_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power093_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power093_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power094_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power094_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power095_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power095_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power096_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power096_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power097_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power097_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power098_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power098_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power099_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power099_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power100_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power100_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power101_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power101_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power102_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power102_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power103_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power103_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power104_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power104_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power105_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power105_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power106_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power106_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power107_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power107_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power108_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power108_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power109_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power109_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power110_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power110_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power111_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power111_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power112_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power112_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power113_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power113_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power114_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power114_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power115_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power115_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power116_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power116_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power117_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power117_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power118_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power118_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power119_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power119_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"power120_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power120_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power121_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power121_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power122_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power122_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power123_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power123_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power124_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power124_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power125_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power125_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power126_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power126_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power127_x", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  {"power127_y", GCFPValue::LPT_DOUBLE, GCF_READWRITE_PROP, ""},
  
  {"seqnr", GCFPValue::LPT_UNSIGNED, GCF_READWRITE_PROP, ""},
};

const TPropertySet propertySetBeamServer = 
{
  257, "PAC_BeamServer", propertiesBeamServer
};


#endif
