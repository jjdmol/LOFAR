//#  ARAPropertyDefines.h: common defines for the AVT package
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

#ifndef ARAPropertyDefines_H
#define ARAPropertyDefines_H

#include "GCF/GCF_Defines.h"
#include "GCF/GCF_PValue.h"

namespace LOFAR
{
  
namespace ARA
{
  
const int STATUS_OK = 0;
const int STATUS_ERROR = 1;

/*
const int N_RACKS               = 1; // 6
const int N_SUBRACKS_PER_RACK   = 1; // 4
const int N_BOARDS_PER_SUBRACK  = 1;
const int N_APS_PER_BOARD       = 1;
const int N_RCUS_PER_AP         = 2;
const int N_RCUS                = N_RCUS_PER_AP*
                                  N_APS_PER_BOARD*
                                  N_BOARDS_PER_SUBRACK*
                                  N_SUBRACKS_PER_RACK*
                                  N_RACKS;
*/

const char APC_Station[]        = "ApcStationType";
const char APC_Rack[]           = "ApcRackType";
const char APC_SubRack[]        = "ApcSubRackType";
const char APC_Board[]          = "ApcBoardType";
const char APC_Ethernet[]       = "ApcEthernetType";
const char APC_FPGA[]           = "ApcFPGAType";
const char APC_RCU[]            = "ApcRCUType";
const char APC_LFA[]            = "ApcLFAType";
const char APC_HFA[]            = "ApcHFAType";
const char APC_ADCStatistics[]  = "ApcADCStatisticsType";
const char APC_Maintenance[]    = "ApcMaintenanceType";
const char APC_Alert[]          = "ApcAlertType";
const char APC_MEPStatus[]      = "ApcMEPStatusType";
const char APC_SYNCStatus[]     = "ApcSYNCStatusType";
const char APC_BoardRCUStatus[] = "ApcBoardRCUStatusType";

const char SCOPE_PIC[] =                                              "PIC";
const char SCOPE_PIC_RackN[] =                                        "PIC_Rack%d";
const char SCOPE_PIC_RackN_SubRackN[] =                               "PIC_Rack%d_SubRack%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN[] =                        "PIC_Rack%d_SubRack%d_Board%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus[] =              "PIC_Rack%d_SubRack%d_Board%d_MEPStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_ETH[] =                    "PIC_Rack%d_SubRack%d_Board%d_ETH";
const char SCOPE_PIC_RackN_SubRackN_BoardN_BP[] =                     "PIC_Rack%d_SubRack%d_Board%d_BP";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN[] =                    "PIC_Rack%d_SubRack%d_Board%d_AP%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus[] =         "PIC_Rack%d_SubRack%d_Board%d_AP%d_SYNCStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus[] =     "PIC_Rack%d_SubRack%d_Board%d_AP%d_BoardRCUStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN[] =               "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA[] =           "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_LFA";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA[] =           "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_HFA";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics[] = "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_ADCStatistics";
const char SCOPE_PIC_Maintenance[] =                                  "PIC_Maintenance";
const char SCOPE_PIC_RackN_Maintenance[] =                            "PIC_Rack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_Maintenance[] =                   "PIC_Rack%d_SubRack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance[] =            "PIC_Rack%d_SubRack%d_Board%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance[] =   "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA_Maintenance[] =  "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_LFA_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA_Maintenance[] =  "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_HFA_Maintenance";
const char SCOPE_PIC_RackN_Alert[] =                                  "PIC_Rack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_Alert[] =                         "PIC_Rack%d_SubRack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Alert[] =                  "PIC_Rack%d_SubRack%d_Board%d_Alert";

const char TYPE_LCU_PIC[]               = "TLcuPic";
const char TYPE_LCU_PIC_Maintenance[]   = "TLcuPicMaintenance";
const char TYPE_LCU_PIC_Rack[]          = "TLcuPicRack";
const char TYPE_LCU_PIC_Alert[]         = "TLcuPicAlert";
const char TYPE_LCU_PIC_SubRack[]       = "TLcuPicSubRack";
const char TYPE_LCU_PIC_Board[]         = "TLcuPicBoard";
const char TYPE_LCU_PIC_MEPStatus[]     = "TLcuPicMEPStatus";
const char TYPE_LCU_PIC_SYNCStatus[]    = "TLcuPicSYNCStatus";
const char TYPE_LCU_PIC_BoardRCUStatus[]= "TLcuPicBoardRCUStatus";
const char TYPE_LCU_PIC_Ethernet[]      = "TLcuPicEthernet";
const char TYPE_LCU_PIC_FPGA[]          = "TLcuPicFPGA";
const char TYPE_LCU_PIC_RCU[]           = "TLcuPicRCU";
const char TYPE_LCU_PIC_ADCStatistics[] = "TLcuPicADCStatistics";
const char TYPE_LCU_PIC_LFA[]           = "TLcuPicLFA";
const char TYPE_LCU_PIC_HFA[]           = "TLcuPicHFA";
const char TYPE_LCU_PAC_LogicalDeviceScheduler[]  = "TLcuPacLogicalDeviceScheduler";

const GCF::Common::TPSCategory PSCAT_LCU_PIC               = GCF::Common::PS_CAT_PERMANENT;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_Alert         = GCF::Common::PS_CAT_PERM_AUTOLOAD;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_Maintenance   = GCF::Common::PS_CAT_PERMANENT;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_Rack          = GCF::Common::PS_CAT_PERM_AUTOLOAD;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_SubRack       = GCF::Common::PS_CAT_PERM_AUTOLOAD;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_Board         = GCF::Common::PS_CAT_PERM_AUTOLOAD;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_MEPStatus     = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_SYNCStatus    = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_BoardRCUStatus= GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_Ethernet      = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_FPGA          = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_RCU           = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_ADCStatistics = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_LFA           = GCF::Common::PS_CAT_TEMPORARY;
const GCF::Common::TPSCategory PSCAT_LCU_PIC_HFA           = GCF::Common::PS_CAT_TEMPORARY;


// the following constants cannot be defined as const char because they are used
// as char* elsewhere
#define PROPNAME_STATUS          "status"
#define PROPNAME_VOLTAGE15       "voltage15"
#define PROPNAME_VOLTAGE33       "voltage33"
#define PROPNAME_FRAMESRECEIVED  "framesReceived"
#define PROPNAME_FRAMESERROR     "framesError"
#define PROPNAME_LASTERROR       "lastError"
#define PROPNAME_FFI0            "ffi0"
#define PROPNAME_FFI1            "ffi1"
#define PROPNAME_FFI2            "ffi2"
#define PROPNAME_FPGAFFI0        "fpgaffi0"
#define PROPNAME_FPGAFFI1        "fpgaffi1"
#define PROPNAME_ALIVE           "alive"
#define PROPNAME_TEMPERATURE     "temperature"
#define PROPNAME_VERSION         "version"
#define PROPNAME_OVERFLOW        "overflow"
#define PROPNAME_VDDVCCEN        "VddVccEn"
#define PROPNAME_VHENABLE        "VhEnable"
#define PROPNAME_VLENABLE        "VlEnable"
#define PROPNAME_FILSELB         "filSelB"
#define PROPNAME_FILSELA         "filSelA"
#define PROPNAME_BANDSEL         "bandSel"
#define PROPNAME_HBAENABLE       "HBAEnable"
#define PROPNAME_LBAENABLE       "LBAEnable"
#define PROPNAME_FROMTIME        "fromTime"
#define PROPNAME_TOTIME          "toTime"
#define PROPNAME_STATISTICSSUBBANDPOWER "statisticsSubbandPower"
#define PROPNAME_STATISTICSBEAMLETPOWER "statisticsBeamletPower"
#define PROPNAME_SEQNR           "seqnr"
#define PROPNAME_ERROR           "error"
#define PROPNAME_SAMPLECOUNT     "sampleCount"
#define PROPNAME_SYNCCOUNT       "syncCount"
#define PROPNAME_ERRORCOUNT      "errorCount"
#define PROPNAME_NOFOVERFLOW     "nofOverflow"


#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

#define PROPERTYCONFIG_BEGIN(_name_) \
  const LOFAR::GCF::Common::TPropertyConfig _name_[] = \
  {
    
#define PROPERTYCONFIG_ITEM(_propname_,_flags_,_default_) \
    {_propname_,_flags_,_default_},
  
#define PROPERTYCONFIG_END \
    {0,0,0} \
  };

PROPERTYCONFIG_BEGIN(PROPS_Station)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_STATISTICSSUBBANDPOWER, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATISTICSBEAMLETPOWER, GCF_READABLE_PROP, "")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_Rack)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READWRITE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_SubRack)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_Board)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_VOLTAGE15, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_VOLTAGE33, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI0, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI1, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FPGAFFI0, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FPGAFFI1, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_VERSION, GCF_READABLE_PROP, "")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_Ethernet)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FRAMESRECEIVED, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FRAMESERROR, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_LASTERROR, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI0, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI1, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI2, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_FPGA)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_TEMPERATURE, GCF_READABLE_PROP, "0.0")
PROPERTYCONFIG_ITEM(PROPNAME_VERSION, GCF_READABLE_PROP, "")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_RCU)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_VDDVCCEN, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_VHENABLE, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_VLENABLE, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_FILSELB, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_FILSELA, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_BANDSEL, GCF_WRITABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_HBAENABLE, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_LBAENABLE, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_ITEM(PROPNAME_STATISTICSSUBBANDPOWER, GCF_READABLE_PROP, "")
PROPERTYCONFIG_ITEM(PROPNAME_STATISTICSBEAMLETPOWER, GCF_READABLE_PROP, "")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_BoardRCUStatus)
PROPERTYCONFIG_ITEM(PROPNAME_FFI0, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI1, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_LFA)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_HFA)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_ADCStatistics)
PROPERTYCONFIG_ITEM(PROPNAME_OVERFLOW, GCF_READABLE_PROP, "false")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_MEPStatus)
PROPERTYCONFIG_ITEM(PROPNAME_SEQNR, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_ERROR, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FFI0,   GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_SYNCStatus)
PROPERTYCONFIG_ITEM(PROPNAME_SAMPLECOUNT, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_SYNCCOUNT,   GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_ERRORCOUNT,  GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_Maintenance)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_FROMTIME, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_ITEM(PROPNAME_TOTIME, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_END

PROPERTYCONFIG_BEGIN(PROPS_Alert)
PROPERTYCONFIG_ITEM(PROPNAME_STATUS, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0")
PROPERTYCONFIG_END

};

} // namespace LOFAR

#endif // ARAPropertyDefines_H
