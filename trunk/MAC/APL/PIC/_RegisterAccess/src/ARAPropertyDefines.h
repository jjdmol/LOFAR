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
const char SCOPE_PIC_Rack1[] =                                        "PIC_Rack1";
const char SCOPE_PIC_Rack1_SubRack1[] =                               "PIC_Rack1_SubRack1";
const char SCOPE_PIC_Rack1_SubRack1_Board1[] =                        "PIC_Rack1_SubRack1_Board1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_ETH[] =                    "PIC_Rack1_SubRack1_Board1_ETH";
const char SCOPE_PIC_Rack1_SubRack1_Board1_BP[] =                     "PIC_Rack1_SubRack1_Board1_BP";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1[] =                    "PIC_Rack1_SubRack1_Board1_AP1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2[] =                    "PIC_Rack1_SubRack1_Board1_AP2";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3[] =                    "PIC_Rack1_SubRack1_Board1_AP3";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4[] =                    "PIC_Rack1_SubRack1_Board1_AP4";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1[] =               "PIC_Rack1_SubRack1_Board1_AP1_RCU1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2[] =               "PIC_Rack1_SubRack1_Board1_AP1_RCU2";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1[] =               "PIC_Rack1_SubRack1_Board1_AP2_RCU1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2[] =               "PIC_Rack1_SubRack1_Board1_AP2_RCU2";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1[] =               "PIC_Rack1_SubRack1_Board1_AP3_RCU1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2[] =               "PIC_Rack1_SubRack1_Board1_AP3_RCU2";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1[] =               "PIC_Rack1_SubRack1_Board1_AP4_RCU1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2[] =               "PIC_Rack1_SubRack1_Board1_AP4_RCU2";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics[] = "PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics";
const char SCOPE_PIC_Maintenance[] =                                  "PIC_Maintenance";
const char SCOPE_PIC_Rack1_Maintenance[] =                            "PIC_Rack1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Maintenance[] =                   "PIC_Rack1_SubRack1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_Maintenance[] =            "PIC_Rack1_SubRack1_Board1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance[] =   "PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance";
const char SCOPE_PIC_Rack1_Alert[] =                                  "PIC_Rack1_Alert";
const char SCOPE_PIC_Rack1_SubRack1_Alert[] =                         "PIC_Rack1_SubRack1_Alert";
const char SCOPE_PIC_Rack1_SubRack1_Board1_Alert[] =                  "PIC_Rack1_SubRack1_Board1_Alert";

// the following constants cannot be defined as const char because they are used
// as char* elsewhere
#define PROPNAME_STATUS          "status"
#define PROPNAME_PACKETSRECEIVED "packetsReceived"
#define PROPNAME_PACKETSERROR    "packetsError"
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

const TPropertySet PROPSET_Rack1 = 
{
  1, "Rack1", PROPS_Rack
};

const TProperty PROPS_SubRack[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_SubRack1 = 
{
  1, "SubRack1", PROPS_SubRack
};

const TProperty PROPS_Board[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_Board1 = 
{
  1, "Board1", PROPS_Board
};

const TProperty PROPS_Ethernet[] =
{
  {PROPNAME_STATUS, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_PACKETSRECEIVED, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_PACKETSERROR, GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertySet PROPSET_ETH = 
{
  3, "ETH", PROPS_Ethernet
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

const TPropertySet PROPSET_AP1 = 
{
  4, "AP1", PROPS_FPGA
};

const TPropertySet PROPSET_AP2 = 
{
  4, "AP2", PROPS_FPGA
};

const TPropertySet PROPSET_AP3 = 
{
  4, "AP3", PROPS_FPGA
};

const TPropertySet PROPSET_AP4 = 
{
  4, "AP4", PROPS_FPGA
};

const TProperty PROPS_RCU[] =
{
  {PROPNAME_RCUHERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_RCUVERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_RINGERR, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
  {PROPNAME_STATSREADY, GCFPValue::LPT_BOOL, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "false"},
};

const TPropertySet PROPSET_RCU1 = 
{
  4, "RCU1", PROPS_RCU
};

const TPropertySet PROPSET_RCU2 = 
{
  4, "RCU2", PROPS_RCU
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
