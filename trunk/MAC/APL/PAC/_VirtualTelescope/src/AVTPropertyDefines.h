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

namespace ARA
{
  
const char APC_SBF[]  = "ApcStationBeamformer";
const char APC_VT[]   = "ApcVirtualTelescope";
const char APC_SR[]   = "ApcStationReceptor";
const char APC_SRG[]  = "ApcStationReceptorGroup";
const char APC_LogicalDeviceScheduler[]   = "ApcLogicalDeviceScheduler";
const char APC_WaveformGenerator[]        = "ApcWaveformGenerator";

const char SCOPE_PAC[]                        = "PAC";
const char SCOPE_PAC_VTn[]                    = "PAC_VT%d";
const char SCOPE_PAC_VTn_BFn[]                = "PAC_VT%d_BF%d";
const char SCOPE_PAC_SRGn[]                   = "PAC_SRG%d";
const char SCOPE_PAC_SRn[]                    = "PAC_SR%d";
const char SCOPE_PAC_LogicalDeviceScheduler[] = "PAC_LogicalDeviceScheduler";
const char SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator[] = "PAC_LogicalDeviceScheduler_WaveFormGenerator";

const string TYPE_PAC                     = "TLOFAR_PAC";
const string TYPE_LogicalDevice           = "TLOFAR_LogicalDevice";
const string TYPE_VT                      = "TLOFAR_VT";
const string TYPE_BF                      = "TLOFAR_BF";
const string TYPE_SRG                     = "TLOFAR_SRG";
const string TYPE_SR                      = "TLOFAR_SR";
const string TYPE_LogicalDeviceScheduler  = "TLOFAR_LogicalDeviceScheduler";
const string TYPE_WaveformGenerator       = "TLOFAR_WaveformGenerator";

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

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

const TPropertyConfig PROPS_LogicalDeviceScheduler[] =
{
  {PROPNAME_COMMAND, GCF_WRITABLE_PROP, ""},
  {PROPNAME_STATUS, GCF_READABLE_PROP, ""},
};

const TPropertyConfig PROPS_WaveformGenerator[] =
{
  {PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "1500000.0"},
  {PROPNAME_AMPLITUDE, GCF_WRITABLE_PROP, "128"},
};

const TPropertyConfig PROPS_VT[] =
{
  {PROPNAME_COMMAND, GCF_WRITABLE_PROP, ""},
  {PROPNAME_STATUS, GCF_READABLE_PROP, ""},
  {PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_SRGNAME, GCF_READABLE_PROP, "SRG1"},
  {PROPNAME_BFNAME, GCF_READABLE_PROP, "BF1"},
};

const TPropertyConfig PROPS_SR[] =
{
  {PROPNAME_COMMAND, GCF_WRITABLE_PROP, ""},
  {PROPNAME_STATUS, GCF_READABLE_PROP, ""},
  {PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FILTER, GCF_WRITABLE_PROP, "0"},  // 1,2,3,4
  {PROPNAME_ANTENNA, GCF_WRITABLE_PROP, "0"}, // 0=LBA, 1=HBA
  {PROPNAME_POWER, GCF_WRITABLE_PROP, "0"}, // 0=off, 1=on
  {PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "110.0"},
};

const TPropertyConfig PROPS_SRG[] =
{
  {PROPNAME_COMMAND, GCF_WRITABLE_PROP, ""},
  {PROPNAME_STATUS, GCF_READABLE_PROP, ""},
  {PROPNAME_STARTTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_STOPTIME, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_FILTER, GCF_WRITABLE_PROP, "0"},  // 1,2,3,4
  {PROPNAME_ANTENNA, GCF_WRITABLE_PROP, "0"}, // 0=LBA, 1=HBA
  {PROPNAME_POWER, GCF_WRITABLE_PROP, "0"}, // 0=off, 1=on
  {PROPNAME_FREQUENCY, GCF_WRITABLE_PROP, "110.0"},
};

const TPropertyConfig PROPS_SBF[] =
{
  {PROPNAME_COMMAND, GCF_WRITABLE_PROP, ""},
  {PROPNAME_STATUS, GCF_READABLE_PROP, ""},
  {PROPNAME_DIRECTIONTYPE,   GCF_WRITABLE_PROP, "LMN"},
  {PROPNAME_DIRECTIONANGLE1, GCF_WRITABLE_PROP, "0.0"},
  {PROPNAME_DIRECTIONANGLE2, GCF_WRITABLE_PROP, "0.0"},
  {PROPNAME_SUBBANDSTART, GCF_WRITABLE_PROP, "0"},
  {PROPNAME_SUBBANDEND, GCF_WRITABLE_PROP, "127"},
};


#endif
