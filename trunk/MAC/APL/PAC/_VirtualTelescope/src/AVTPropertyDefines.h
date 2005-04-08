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

#ifndef AVTPropertyDefines_H
#define AVTPropertyDefines_H

namespace LOFAR
{
  
namespace AVT
{

const char SCOPESEPARATOR[] = "_";
const char PROPERTYSEPARATOR[] = ".";
  
const char APC_SBF[]                      = "ApcStationBeamformer.xml";
const char APC_VT[]                       = "ApcVirtualTelescope.xml";
const char APC_SR[]                       = "ApcStationReceptor.xml";
const char APC_SRG[]                      = "ApcStationReceptorGroup.xml";
const char APC_LogicalDeviceScheduler[]   = "ApcLogicalDeviceScheduler.xml";
const char APC_WaveformGenerator[]        = "ApcWaveformGenerator.xml";

const char SCOPE_PAC[]                        = "PAC";
const char SCOPE_PAC_VTn[]                    = "PAC_VT%d";
const char SCOPE_PAC_VTn_BFn[]                = "PAC_VT%d_BF%d";
const char SCOPE_PAC_SRGn[]                   = "PAC_SRG%d";
const char SCOPE_PAC_SRn[]                    = "PAC_SR%d";
const char SCOPE_PAC_LogicalDeviceScheduler[] = "PAC_LogicalDeviceScheduler";
const char SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator[] = "PAC_LogicalDeviceScheduler_WaveFormGenerator";

const char SCOPE_PIC[] =                                              "PIC";
const char SCOPE_PIC_RackN[] =                                        "PIC_Rack%d";
const char SCOPE_PIC_RackN_SubRackN[] =                               "PIC_Rack%d_SubRack%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN[] =                        "PIC_Rack%d_SubRack%d_Board%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_ETH[] =                    "PIC_Rack%d_SubRack%d_Board%d_ETH";
const char SCOPE_PIC_RackN_SubRackN_BoardN_BP[] =                     "PIC_Rack%d_SubRack%d_Board%d_BP";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN[] =                    "PIC_Rack%d_SubRack%d_Board%d_AP%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN[] =               "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d";
const char SCOPE_PIC_Maintenance[] =                                  "PIC_Maintenance";
const char SCOPE_PIC_RackN_Maintenance[] =                            "PIC_Rack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_Maintenance[] =                   "PIC_Rack%d_SubRack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance[] =            "PIC_Rack%d_SubRack%d_Board%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance[] =   "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_Maintenance";
const char SCOPE_PIC_RackN_Alert[] =                                  "PIC_Rack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_Alert[] =                         "PIC_Rack%d_SubRack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Alert[] =                  "PIC_Rack%d_SubRack%d_Board%d_Alert";

const string TYPE_LCU_PIC_Maintenance   = "TLcuPicMaintenance";
const string TYPE_LCU_PIC_RCU           = "TLcuPicRCU";


const string TYPE_LCU_PAC                         = "TLcuPac";
const string TYPE_LCU_PAC_LogicalDevice           = "TLcuPacLogicalDevice";
const string TYPE_LCU_PAC_VT                      = "TLcuPacVT";
const string TYPE_LCU_PAC_BF                      = "TLcuPacSBF";
const string TYPE_LCU_PAC_SRG                     = "TLcuPacSRG";
const string TYPE_LCU_PAC_SR                      = "TLcuPacSR";
const string TYPE_LCU_PAC_LogicalDeviceScheduler  = "TLcuPacLogicalDeviceScheduler";
const string TYPE_LCU_PAC_WaveformGenerator       = "TLcuPacWaveformGenerator";

// the following constants cannot be defined as const char because they are used
// as char* elsewhere
#define PROPNAME_COMMAND          "command"
#define PROPNAME_STATUS           "status"
#define PROPNAME_STARTTIME        "startTime"
#define PROPNAME_STOPTIME         "stopTime"
#define PROPNAME_SRGNAME          "srgName"
#define PROPNAME_BFNAME           "bfName"
#define PROPNAME_FREQUENCY        "frequency"
#define PROPNAME_AMPLITUDE        "amplitude"
#define PROPNAME_DIRECTIONTYPE    "directionType"
#define PROPNAME_DIRECTIONANGLE1  "directionAngle1"
#define PROPNAME_DIRECTIONANGLE2  "directionAngle2"
#define PROPNAME_FILTER           "filter"
#define PROPNAME_ANTENNA          "antenna"
#define PROPNAME_POWER            "power"
#define PROPNAME_SUBBANDSTART     "subbandStart"
#define PROPNAME_SUBBANDEND       "subbandEnd"

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

#define PROPERTYCONFIG_BEGIN(_name_) \
  const LOFAR::GCF::Common::TPropertyConfig _name_[] = \
  {
    
#define PROPERTYCONFIG_ITEM(_propname_,_flags_,_default_) \
    {_propname_,_flags_,_default_},
  
#define PROPERTYCONFIG_END \
    {0,0,0} \
  };

PROPERTYCONFIG_BEGIN(PROPS_LogicalDeviceScheduler)
PROPERTYCONFIG_ITEM(PROPNAME_COMMAND, GCF_WRITABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_WaveformGenerator)
PROPERTYCONFIG_ITEM(PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "1500000.0")
PROPERTYCONFIG_ITEM(PROPNAME_AMPLITUDE, GCF_WRITABLE_PROP, "128")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_VT)
PROPERTYCONFIG_ITEM(PROPNAME_COMMAND, GCF_WRITABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_SRGNAME, GCF_READABLE_PROP, "SRG1")
PROPERTYCONFIG_ITEM(PROPNAME_BFNAME, GCF_READABLE_PROP, "BF1")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_SR)
PROPERTYCONFIG_ITEM(PROPNAME_COMMAND, GCF_WRITABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FILTER, GCF_WRITABLE_PROP, "0")  // 1,2,3,4
PROPERTYCONFIG_ITEM(PROPNAME_ANTENNA, GCF_WRITABLE_PROP, "0") // 0=LBA, 1=HBA
PROPERTYCONFIG_ITEM(PROPNAME_POWER, GCF_WRITABLE_PROP, "0") // 0=off, 1=on
PROPERTYCONFIG_ITEM(PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "110.0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_SRG)
PROPERTYCONFIG_ITEM(PROPNAME_COMMAND, GCF_WRITABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FILTER, GCF_WRITABLE_PROP, "0")  // 1,2,3,4
PROPERTYCONFIG_ITEM(PROPNAME_ANTENNA, GCF_WRITABLE_PROP, "0") // 0=LBA, 1=HBA
PROPERTYCONFIG_ITEM(PROPNAME_POWER, GCF_WRITABLE_PROP, "0") // 0=off, 1=on
PROPERTYCONFIG_ITEM(PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "110.0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_SBF)
PROPERTYCONFIG_ITEM(PROPNAME_COMMAND, GCF_WRITABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_DIRECTIONTYPE,   GCF_WRITABLE_PROP, "LMN")
PROPERTYCONFIG_ITEM(PROPNAME_DIRECTIONANGLE1, GCF_WRITABLE_PROP, "0.0")
PROPERTYCONFIG_ITEM(PROPNAME_DIRECTIONANGLE2, GCF_WRITABLE_PROP, "0.0")
PROPERTYCONFIG_ITEM(PROPNAME_SUBBANDSTART, GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_SUBBANDEND, GCF_WRITABLE_PROP, "127")
PROPERTYCONFIG_END

};//AVT
};//LOFAR

#endif //AVTPropertyDefines_h
