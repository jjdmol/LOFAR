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
#include <GCF/Utils.h>
#include "GSA_Service.h"
#include "GSA_Defines.h"
#include <Manager.hxx>

string GCFPVSSInfo::_sysName = "";
string GCFPVSSInfo::_projName = "";
unsigned int GCFPVSSInfo::_lastSysNr = 0;

TMACValueType macValueTypes[] = {
  NO_LPT,           // DPELEMENT_NOELEMENT,
  NO_LPT,           // DPELEMENT_RECORD,
  NO_LPT,           // DPELEMENT_ARRAY,
  LPT_DYNCHAR,      // DPELEMENT_DYNCHAR,
  LPT_DYNUNSIGNED,  // DPELEMENT_DYNUINT,
  LPT_DYNINTEGER,   // DPELEMENT_DYNINT,
  LPT_DYNDOUBLE,    // DPELEMENT_DYNFLOAT,
  LPT_DYNBOOL,      // DPELEMENT_DYNBIT,
  NO_LPT,           // DPELEMENT_DYN32BIT, not yet
  LPT_DYNSTRING,    // DPELEMENT_DYNTEXT,
  NO_LPT,           // DPELEMENT_DYNTIME, not yet
  NO_LPT,           // DPELEMENT_CHARARRAY,
  NO_LPT,           // DPELEMENT_UINTARRAY,
  NO_LPT,           // DPELEMENT_INTARRAY,
  NO_LPT,           // DPELEMENT_FLOATARRAY,
  NO_LPT,           // DPELEMENT_BITARRAY,
  NO_LPT,           // DPELEMENT_32BITARRAY,
  NO_LPT,           // DPELEMENT_TEXTARRAY,
  NO_LPT,           // DPELEMENT_TIMEARRAY,
  LPT_CHAR,         // DPELEMENT_CHAR,
  LPT_UNSIGNED,     // DPELEMENT_UINT,
  LPT_INTEGER,      // DPELEMENT_INT,
  LPT_DOUBLE,       // DPELEMENT_FLOAT,
  LPT_BOOL,         // DPELEMENT_BIT,
  NO_LPT,           // DPELEMENT_32BIT, not yet
  LPT_STRING,       // DPELEMENT_TEXT,
  NO_LPT,           // DPELEMENT_TIME, not yet
  NO_LPT,           // DPELEMENT_DPID,
  NO_LPT,           // DPELEMENT_NOVALUE,
  NO_LPT,           // DPELEMENT_DYNDPID,
  NO_LPT,           // DPELEMENT_DYNCHARARRAY,
  NO_LPT,           // DPELEMENT_DYNUINTARRAY,
  NO_LPT,           // DPELEMENT_DYNINTARRAY,
  NO_LPT,           // DPELEMENT_DYNFLOATARRAY,
  NO_LPT,           // DPELEMENT_DYNBITARRAY,
  NO_LPT,           // DPELEMENT_DYN32BITARRAY,
  NO_LPT,           // DPELEMENT_DYNTEXTARRAY,
  NO_LPT,           // DPELEMENT_DYNTIMEARRAY,
  NO_LPT,           // DPELEMENT_DYNDPIDARRAY,
  NO_LPT,           // DPELEMENT_DPIDARRAY,
  NO_LPT,           // DPELEMENT_NOVALUEARRAY,
  NO_LPT,           // DPELEMENT_TYPEREFERENCE,
  NO_LPT,           // DPELEMENT_LANGTEXT,
  NO_LPT,           // DPELEMENT_LANGTEXTARRAY,
  NO_LPT,           // DPELEMENT_DYNLANGTEXT,
  NO_LPT,           // DPELEMENT_DYNLANGTEXTARRAY,
  LPT_BLOB,         // DPELEMENT_BLOB,
  NO_LPT,           // DPELEMENT_BLOBARRAY,
  LPT_DYNBLOB,      // DPELEMENT_DYNBLOB,
  NO_LPT,           // DPELEMENT_DYNBLOBARRAY,
};

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
      _sysName += (const char*) sysName;
    }
  }
  return _sysName;
}

unsigned int GCFPVSSInfo::getLocalSystemId()
{
  return Resources::getSystem();
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
    _projName += (const char*) projName;
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
  // workaround - start
  DpIdentifier dpId;
  Manager::getId(sysName + ".", dpId);
  sysNr = dpId.getSystem();
  // workaround - end
  /*
  DpIdentificationResult res;
  if ((res = Manager::getDpIdentificationPtr()->getSystemId((const char*)sysName, sysNr)) != DpIdentOK)
  {
    LOG_ERROR(formatString(
        "PVSS could not find system nr for '%s', error %d",
        (const char*) sysName,
        res));    
    sysNr = 0;
  }*/
  return sysNr;
}

unsigned int GCFPVSSInfo::getManNum()
{
  return Resources::getManNum();
}

void buildTypeStructTree(const string path, const DpType* pType, const DpElementId elId, list<TPropertyInfo>& propInfos);

TGCFResult GCFPVSSInfo::getTypeStruct(const string& typeName, list<TPropertyInfo>& propInfos, unsigned int sysNr)
{
  TGCFResult result(GCF_NO_ERROR);
  propInfos.clear();
  
  CharString pvssTypeName = typeName.c_str();
  DpTypeId typeId;
  DpType* pType;
  if (Manager::getTypeId(pvssTypeName, typeId, sysNr) == PVSS_FALSE)
  {
    string sysName = getSystemName(sysNr);
    LOG_ERROR(formatString(
        "PVSS could not find type %s on system %s", 
        typeName.c_str(),
        sysName.c_str()));
    result = GCF_PVSS_ERROR;
  }
  else if ((pType = Manager::getTypeContainerPtr()->getTypePtr(typeId)) == 0)
  {
    LOG_ERROR(formatString(
        "PVSS internal error on type information (%s:%s)",
        getSystemName(sysNr).c_str(), 
        typeName.c_str()));
    result = GCF_PVSS_ERROR;
  }
  else
  {
    DpElementId elId = pType->getRootElement();
    string path;
    buildTypeStructTree(path, pType, elId, propInfos);
  }  
  
  return result;  
}

void buildTypeStructTree(const string path, 
                         const DpType* pType, 
                         const DpElementId elId, 
                         list<TPropertyInfo>& propInfos)
{
  string propName = path;
  DpElementType elType = pType->getTypeNodePtr(elId)->getElementType();
  if (elId != pType->getRootElement())
  {
    char* elName;
    Manager::getDpIdentificationPtr()->getElementName(pType->getName(), elId, elName);
    if (strcmp(elName, "__internal") == 0)
    {
      delete [] elName;
      return;
    }
    if (elType != DPELEMENT_TYPEREFERENCE)
    {
      if (propName.length() > 0) propName += '.';
      propName += elName;
    }
    delete [] elName; 
  }
  if (elType != DPELEMENT_RECORD && elType != DPELEMENT_TYPEREFERENCE)
  {
    if (macValueTypes[elType] != NO_LPT)
    {
      
      if (Utils::isValidPropName(propName.c_str()))
      {
        TPropertyInfo propInfo;
        propInfo.propName = propName;
        propInfo.type = macValueTypes[elType];
        propInfos.push_back(propInfo);
      }
      else
      {
        LOG_WARN(formatString ( 
            "Property name %s meets not the name convention! Not add!!!",
            propName.c_str()));
      }
    }
    else
    {
      LOG_ERROR(formatString(
          "TypeElement type %d (see DpElementType.hxx) is unknown to GCF (%s). No add!!!",
          elType, 
          propName.c_str()));      
    }
  } 
  else
  {
    DynPtrArrayIndex nrOfChilds = pType->getSonNumber(elId);
    DpElementId childElId;
    for (DynPtrArrayIndex i = 0; i < nrOfChilds; i++)
    {
      childElId = pType->getSon(elId, i);
      buildTypeStructTree(propName, pType, childElId, propInfos);
    }
  }
}

