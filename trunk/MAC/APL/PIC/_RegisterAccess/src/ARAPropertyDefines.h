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

const int N_RACKS               = 6;
const int N_SUBRACKS_PER_RACK   = 4;
const int N_BOARDS_PER_SUBRACK  = 1;
const int N_APS_PER_BOARD       = 4;
const int N_RCUS_PER_AP         = 2;
const int N_RCUS                = N_RCUS_PER_AP*
                                  N_APS_PER_BOARD*
                                  N_BOARDS_PER_SUBRACK*
                                  N_SUBRACKS_PER_RACK*
                                  N_RACKS;

const char APC_Station[]        = "ApcStationType";
const char APC_Rack[]           = "ApcRackType";
const char APC_SubRack[]        = "ApcSubRackType";
const char APC_Board[]          = "ApcBoardType";
const char APC_Ethernet[]       = "ApcEthernetType";
const char APC_FPGA[]           = "ApcFPGAType";
const char APC_RCU[]            = "ApcRCUType";
const char APC_ADCStatistics[]  = "ApcADCStatisticsType";
const char APC_Maintenance[]    = "ApcMaintenanceType";
const char APC_Alert[]          = "ApcAlertType";

const char SCOPE_PIC[] =                                              "PIC";
const char SCOPE_PIC_RackN[] =                                        "PIC_Rack%d";
const char SCOPE_PIC_RackN_SubRackN[] =                               "PIC_Rack%d_SubRack%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN[] =                        "PIC_Rack%d_SubRack%d_Board%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_ETH[] =                    "PIC_Rack%d_SubRack%d_Board%d_ETH";
const char SCOPE_PIC_RackN_SubRackN_BoardN_BP[] =                     "PIC_Rack%d_SubRack%d_Board%d_BP";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN[] =                    "PIC_Rack%d_SubRack%d_Board%d_AP%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN[] =               "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics[] = "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_ADCStatistics";
const char SCOPE_PIC_Maintenance[] =                                  "PIC_Maintenance";
const char SCOPE_PIC_RackN_Maintenance[] =                            "PIC_Rack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_Maintenance[] =                   "PIC_Rack%d_SubRack%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance[] =            "PIC_Rack%d_SubRack%d_Board%d_Maintenance";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance[] =   "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d_Maintenance";
const char SCOPE_PIC_RackN_Alert[] =                                  "PIC_Rack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_Alert[] =                         "PIC_Rack%d_SubRack%d_Alert";
const char SCOPE_PIC_RackN_SubRackN_BoardN_Alert[] =                  "PIC_Rack%d_SubRack%d_Board%d_Alert";

// the following constants cannot be defined as const char because they are used
// as char* elsewhere
#define PROPNAME_STATUS          "status"
#define PROPNAME_PACKETSRECEIVED "packetsReceived"
#define PROPNAME_PACKETSERROR    "packetsError"
#define PROPNAME_LASTERROR       "lastError"
#define PROPNAME_FFI0            "ffi0"
#define PROPNAME_FFI1            "ffi1"
#define PROPNAME_FFI2            "ffi2"
#define PROPNAME_ALIVE           "alive"
#define PROPNAME_TEMPERATURE     "temperature"
#define PROPNAME_VERSION         "version"
#define PROPNAME_RCUHERR         "RCUHerr"
#define PROPNAME_RCUVERR         "RCUVerr"
#define PROPNAME_RINGERR         "ringErr"
#define PROPNAME_STATSREADY      "statsReady"
#define PROPNAME_OVERFLOW        "overflow"
#define PROPNAME_FROMTIME        "fromTime"
#define PROPNAME_TOTIME          "toTime"


const TProperty PROPS_Station[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_PIC = 
{
  1, "PIC", PROPS_Station
};

const TProperty PROPS_Rack[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_Racks[] = 
{
  {1, "Rack1", PROPS_Rack},
  {1, "Rack2", PROPS_Rack},
  {1, "Rack3", PROPS_Rack},
  {1, "Rack4", PROPS_Rack},
  {1, "Rack5", PROPS_Rack},
  {1, "Rack6", PROPS_Rack},
};

const TProperty PROPS_SubRack[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_SubRacks[] = 
{
  {1, "SubRack1", PROPS_SubRack},
  {1, "SubRack2", PROPS_SubRack},
  {1, "SubRack3", PROPS_SubRack},
  {1, "SubRack4", PROPS_SubRack},
};

const TProperty PROPS_Board[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_Boards[] = 
{
  {1, "Board1", PROPS_Board},
};

const TProperty PROPS_Ethernet[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_PACKETSRECEIVED, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_PACKETSERROR, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_LASTERROR, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FFI0, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FFI1, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FFI2, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_ETH = 
{
  7, "ETH", PROPS_Ethernet
};

const TProperty PROPS_FPGA[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_ALIVE, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_TEMPERATURE, GCFPValue::LPT_DOUBLE, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0.0"},
  {PROPNAME_VERSION, GCFPValue::LPT_STRING, GCF_READABLE_PROP | GCF_WRITABLE_PROP, ""},
};

const TPropertySet PROPSET_BP = 
{
  4, "BP", PROPS_FPGA
};

const TPropertySet PROPSET_APs[] = 
{
  {4, "AP1", PROPS_FPGA},
  {4, "AP2", PROPS_FPGA},
  {4, "AP3", PROPS_FPGA},
  {4, "AP4", PROPS_FPGA},
};

const TProperty PROPS_RCU[] =
{
  {PROPNAME_RCUHERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_RCUVERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_RINGERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_STATSREADY, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
};

const TPropertySet PROPSET_RCUs[] = 
{
  {4, "RCU1", PROPS_RCU},
  {4, "RCU2", PROPS_RCU},
};

const TProperty PROPS_ADCStatistics[] =
{
  {PROPNAME_OVERFLOW, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
};

const TPropertySet PROPSET_ADCStatistics = 
{
  1, "ADCStatistics", PROPS_ADCStatistics
};

const TProperty PROPS_Maintenance[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FROMTIME, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_TOTIME, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_Maintenance = 
{
  3, "Maintenance", PROPS_Maintenance
};

const TProperty PROPS_Alert[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_Alert = 
{
  1, "Alert", PROPS_Alert
};

#endif // ARAPropertyDefines_H
