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

const char SCOPE_PIC[] =                                              "PIC";
const char SCOPE_PIC_RackN[] =                                        "PIC_Rack%d";
const char SCOPE_PIC_RackN_SubRackN[] =                               "PIC_Rack%d_SubRack%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN[] =                        "PIC_Rack%d_SubRack%d_Board%d";
const char SCOPE_PIC_RackN_SubRackN_BoardN_MEPReadStatus[] =          "PIC_Rack%d_SubRack%d_Board%d_MEPReadStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_MEPWriteStatus[] =         "PIC_Rack%d_SubRack%d_Board%d_MEPWriteStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_SYNCStatus[] =             "PIC_Rack%d_SubRack%d_Board%d_SYNCStatus";
const char SCOPE_PIC_RackN_SubRackN_BoardN_ETH[] =                    "PIC_Rack%d_SubRack%d_Board%d_ETH";
const char SCOPE_PIC_RackN_SubRackN_BoardN_BP[] =                     "PIC_Rack%d_SubRack%d_Board%d_BP";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN[] =                    "PIC_Rack%d_SubRack%d_Board%d_AP%d";
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

const char TYPE_PIC[]           = "TLOFAR_PIC";
const char TYPE_Maintenance[]   = "TLOFAR_Maintenance";
const char TYPE_Rack[]          = "TLOFAR_Rack";
const char TYPE_Alert[]         = "TLOFAR_Alert";
const char TYPE_SubRack[]       = "TLOFAR_SubRack";
const char TYPE_Board[]         = "TLOFAR_Board";
const char TYPE_MEPStatus[]     = "TLOFAR_MEPStatus";
const char TYPE_SYNCStatus[]    = "TLOFAR_SYNCStatus";
const char TYPE_ETH[]           = "TLOFAR_ETH";
const char TYPE_BP[]            = "TLOFAR_BP";
const char TYPE_AP[]            = "TLOFAR_AP";
const char TYPE_RCU[]           = "TLOFAR_RCU";
const char TYPE_ADCStatistics[] = "TLOFAR_ADCStatistics";
const char TYPE_LFA[]           = "TLOFAR_LFA";
const char TYPE_HFA[]           = "TLOFAR_HFA";


// the following constants cannot be defined as const char because they are used
// as char* elsewhere
#define PROPNAME_STATUS          "status"
#define PROPNAME_VOLTAGE15       "voltage15"
#define PROPNAME_VOLTAGE22       "voltage22"
#define PROPNAME_FFI             "ffi"
#define PROPNAME_PACKETSRECEIVED "packetsReceived"
#define PROPNAME_PACKETSERROR    "packetsError"
#define PROPNAME_LASTERROR       "lastError"
#define PROPNAME_FFI0            "ffi0"
#define PROPNAME_FFI1            "ffi1"
#define PROPNAME_FFI2            "ffi2"
#define PROPNAME_ALIVE           "alive"
#define PROPNAME_TEMPERATURE     "temperature"
#define PROPNAME_VERSION         "version"
#define PROPNAME_RCUPWR          "rcuPwr"
#define PROPNAME_HBAPWR          "hbaPwr"
#define PROPNAME_LBAPWR          "lbaPwr"
#define PROPNAME_FILTER          "filter"
#define PROPNAME_OVERFLOW        "overflow"
#define PROPNAME_FROMTIME        "fromTime"
#define PROPNAME_TOTIME          "toTime"
#define PROPNAME_STATISTICSSUBBANDMEAN  "statisticsSubbandMean"
#define PROPNAME_STATISTICSSUBBANDPOWER "statisticsSubbandPower"
#define PROPNAME_STATISTICSBEAMLETMEAN  "statisticsBeamletMean"
#define PROPNAME_STATISTICSBEAMLETPOWER "statisticsBeamletPower"
#define PROPNAME_SEQNR           "seqnr"
#define PROPNAME_ERROR           "error"
#define PROPNAME_CLOCKCOUNT      "clockCount"
#define PROPNAME_COUNT           "count"
#define PROPNAME_ERRORS          "errors"

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

const TPropertyConfig PROPS_Station[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
  {PROPNAME_STATISTICSSUBBANDMEAN, GCF_READABLE_PROP, ""},
  {PROPNAME_STATISTICSSUBBANDPOWER, GCF_READABLE_PROP, ""},
  {PROPNAME_STATISTICSBEAMLETMEAN, GCF_READABLE_PROP, ""},
  {PROPNAME_STATISTICSBEAMLETPOWER, GCF_READABLE_PROP, ""},
};

const TPropertyConfig PROPS_Rack[] =
{
  {PROPNAME_STATUS, GCF_READWRITE_PROP, "0"},
};

const TPropertyConfig PROPS_SubRack[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
};

const TPropertyConfig PROPS_Board[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
  {PROPNAME_VOLTAGE15, GCF_READABLE_PROP, "0"},
  {PROPNAME_VOLTAGE22, GCF_READABLE_PROP, "0"},
  {PROPNAME_FFI, GCF_READABLE_PROP, "0"},
  {PROPNAME_VERSION, GCF_READABLE_PROP, ""},
};

const TPropertyConfig PROPS_Ethernet[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
  {PROPNAME_PACKETSRECEIVED, GCF_READABLE_PROP, "0"},
  {PROPNAME_PACKETSERROR, GCF_READABLE_PROP, "0"},
  {PROPNAME_LASTERROR, GCF_READABLE_PROP, "0"},
  {PROPNAME_FFI0, GCF_READABLE_PROP, "0"},
  {PROPNAME_FFI1, GCF_READABLE_PROP, "0"},
  {PROPNAME_FFI2, GCF_READABLE_PROP, "0"},
};

const TPropertyConfig PROPS_FPGA[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
  {PROPNAME_ALIVE, GCF_READABLE_PROP, "false"},
  {PROPNAME_TEMPERATURE, GCF_READABLE_PROP, "0.0"},
  {PROPNAME_VERSION, GCF_READABLE_PROP, ""},
};

const TPropertyConfig PROPS_RCU[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
  {PROPNAME_OVERFLOW, GCF_READABLE_PROP, "false"},
  {PROPNAME_RCUPWR, GCF_READABLE_PROP, "false"},
  {PROPNAME_HBAPWR, GCF_READABLE_PROP, "false"},
  {PROPNAME_LBAPWR, GCF_READABLE_PROP, "false"},
  {PROPNAME_FILTER, GCF_READABLE_PROP, "0"},
};

const TPropertyConfig PROPS_LFA[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
};

const TPropertyConfig PROPS_HFA[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP, "0"},
};

const TPropertyConfig PROPS_ADCStatistics[] =
{
  {PROPNAME_OVERFLOW, GCF_READABLE_PROP, "false"},
};

const TPropertyConfig PROPS_MEPStatus[] =
{
  {PROPNAME_SEQNR, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_ERROR, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FFI,   GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertyConfig PROPS_SYNCStatus[] =
{
  {PROPNAME_CLOCKCOUNT, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_COUNT,      GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_ERRORS,     GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertyConfig PROPS_Maintenance[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FROMTIME, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
  {PROPNAME_TOTIME, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

const TPropertyConfig PROPS_Alert[] =
{
  {PROPNAME_STATUS, GCF_READABLE_PROP | GCF_WRITABLE_PROP, "0"},
};

};

#endif // ARAPropertyDefines_H
