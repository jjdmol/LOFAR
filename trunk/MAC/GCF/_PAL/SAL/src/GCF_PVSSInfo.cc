//#  GCF_PVSSInfo.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include <GCF/PAL/GCF_PVSSInfo.h>
#include "GSA_Service.h"
#include "GSA_Defines.h"

#include <Manager.hxx>

string GCFPVSSInfo::_sysName = "";
string GCFPVSSInfo::_projName = "";
unsigned int GCFPVSSInfo::_lastSysNr = 0;

bool GCFPVSSInfo::propExists(const string& dpeName)
{
  DpIdentifier dpId;
  CharString dpePvssName(dpeName.c_str());
  if (Manager::getId(dpePvssName, dpId) == PVSS_FALSE)
    return false;
  else
    return true;
}

bool GCFPVSSInfo::typeExists (const string& dpTypeName)
{
  CharString pvssTypeName(dpTypeName.c_str());
  DpTypeId dpTypeId; 
  return (Manager::getTypeId(pvssTypeName, dpTypeId) == PVSS_TRUE);
}

const string& GCFPVSSInfo::getLocalSystemName()
{
  if (_sysName.length() == 0)
  {
    CharString sysName;
    if (Manager::getSystemName(Resources::getSystem(), sysName) == PVSS_TRUE)
    {      
      _sysName = (const char*) sysName;
    }
  }
  return _sysName;
}

const string GCFPVSSInfo::getSystemName(unsigned int sysnr)
{
  CharString sysName;
  if (Manager::getSystemName(sysnr, sysName) == PVSS_TRUE)
  {      
    return (const char*) sysName;
  }
  return "";
}

const string& GCFPVSSInfo::getProjectName()
{
  if (_projName.length() == 0)
  {
    CharString projName = Resources::getProjectName();
    _projName = (const char*) projName;
  }
  return _projName;
}

unsigned int GCFPVSSInfo::getLastEventSysId()
{
  return _lastSysNr;
}

unsigned int GCFPVSSInfo::getSysId(const string& name)
{
  string::size_type index = name.find(':');
  if (index > name.length())
  {
    index = name.length();
  }
  CharString sysName(name.c_str(), index);
  SystemNumType sysNr;
  Manager::getSystemId(sysName, sysNr);
  return sysNr;
}