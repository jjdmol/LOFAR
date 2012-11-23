//#  CEPPropertySet.cc: 
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

#include <lofar_config.h>

#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/PALlight/CEPProperty.h>
#include <PIClient.h>
#include <GCF/Utils.h>
#include <Common/lofar_fstream.h>
using std::ifstream;

#define ASCII_LINE_SIZE 1024

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace CEPPMLlight 
  {

CEPPropertySet::CEPPropertySet(const char* name,
                               const char* type,
                               TPSCategory category) : 
  _scope(name),
  _type(type),
  _category(category),
  _state(S_DISABLED),
  _dummyProperty(*this)
{
  CEPProperty* pProperty;
  TPropertyInfo* pPropInfo;
  
  if (!isValidScope(_scope.c_str()))
  {
    LOG_WARN(formatString ( 
        "Scope %s meets not the name convention! Set to \"\"",
        _scope.c_str()));
    _scope = "";
  }
  
  TPropInfoList propInfoList;
  readTypeFile(propInfoList);

  for (TPropInfoList::iterator iter = propInfoList.begin();
       iter != propInfoList.end(); ++iter)
  { 
    pPropInfo = &(*iter);
    pProperty = new CEPProperty(*pPropInfo, *this);
    addProperty(pPropInfo->propName, *pProperty);
  }
  _pClient = PIClient::instance();  // get the singleton instance
  ASSERT(_pClient);
}  

CEPPropertySet::~CEPPropertySet ()
{
  ASSERT(_pClient);
  // delete this set from the PIClient administration permanent
  if (_state != S_DISABLED && _state != S_DISABLING)
  {
    _pClient->unregisterScope(*this, (_state == S_ENABLING));
  }
  _pClient->deletePropSet(*this);  

  clearAllProperties();  

  PIClient::release(); 
  _pClient = 0;
}
  
bool CEPPropertySet::enable ()
{
  TGCFResult result(GCF_NO_ERROR);
    
  LOG_INFO(formatString ( 
      "REQ: Enable property set '%s'",
      getScope().c_str()));

  _stateMutex.lock();
  if (_state != S_DISABLED)
  {
    wrongState("enable");
    result = GCF_WRONG_STATE;
  }
  else
  {
    ASSERT(_pClient);
    
    if (_pClient->registerScope(*this))
    {
      _state = S_ENABLING;
    }
    else 
    {
      LOG_ERROR(formatString ( 
          "Property set with scope %s already exists in this Application",
          getScope().c_str()));    
      result = GCF_SCOPE_ALREADY_REG;
    }
  }
  _stateMutex.unlock();
  return (result == GCF_NO_ERROR);
}

bool CEPPropertySet::disable ()
{
  TGCFResult result(GCF_NO_ERROR);
  
  LOG_INFO(formatString ( 
      "REQ: Disable property set '%s'",
      getScope().c_str()));

  _stateMutex.lock();
      
  switch (_state)
  {
    case S_LINKED:
    case S_ENABLED:
    case S_ENABLING:
      ASSERT(_pClient);
  
      _pClient->unregisterScope(*this, (_state == S_ENABLING));
      _state = S_DISABLING;      
      break;

    default:
      wrongState("disable");
      result = GCF_WRONG_STATE;
      break;
  }
  _stateMutex.unlock();
  return (result == GCF_NO_ERROR);
}

void CEPPropertySet::scopeRegistered (bool succeed)
{
  _stateMutex.lock();
 
  ASSERT(_state == S_ENABLING);
  LOG_INFO(formatString ( 
      "PA-RESP: Property set '%s' is enabled%s",
      getScope().c_str(), 
      (succeed ? "" : " (with errors)")));

  _state = (succeed ? S_ENABLED : S_DISABLED);
  
  _stateMutex.unlock();
}

void CEPPropertySet::scopeUnregistered (bool succeed)
{
  _stateMutex.lock();

  ASSERT(_state == S_DISABLING);
   
  LOG_INFO(formatString ( 
      "Property set '%s' is disabled%s",
      getScope().c_str(), 
      (succeed ? "" : " (with errors)")));

  _state = S_DISABLED;

  _stateMutex.unlock();
}

void CEPPropertySet::linkProperties()
{
  ASSERT(_pClient);

  _stateMutex.lock();
  
  switch (_state)
  {
    case S_ENABLED:
      _state = S_LINKED;
      _pClient->propertiesLinked(getScope(), PI_NO_ERROR);
      break;

    case S_DISABLED:
    case S_DISABLING:
      _pClient->propertiesLinked(getScope(), PI_PS_GONE);
      break;

    default:
      wrongState("linkProperties");
      _pClient->propertiesLinked(getScope(), PI_WRONG_STATE);
      break;
  }
  _stateMutex.unlock();
}

void CEPPropertySet::unlinkProperties()
{
  ASSERT(_pClient);

  _stateMutex.lock();

  switch (_state)
  {
    case S_DISABLED:
    case S_DISABLING:
      _pClient->propertiesUnlinked(getScope(), PI_PS_GONE);
      break;

    case S_LINKED:
      _state = S_ENABLED;
      _pClient->propertiesUnlinked(getScope(), PI_NO_ERROR);
      break;

    default:
      wrongState("unlinkProperties");
      _pClient->propertiesUnlinked(getScope(), PI_WRONG_STATE);
      break;
  }

  _stateMutex.unlock();
}

void CEPPropertySet::connectionLost()
{
  ASSERT(_pClient);
  _stateMutex.lock();
  
  switch (_state)
  {
    case S_DISABLED:
      break;

    case S_DISABLING:
      _state = S_DISABLED;
      break;

    default:
      _pClient->deletePropSet(*this);
      _pClient->registerScope(*this);
      _state = S_ENABLING;
      break;
  }
  _stateMutex.unlock();
}

CEPProperty* CEPPropertySet::getProperty (const string& propName) const
{
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::const_iterator iter = _properties.find(shortPropName);
  
  if (iter != _properties.end())
  {
    return iter->second;
  }
  else
  {
    return 0;
  }
}

CEPProperty& CEPPropertySet::operator[] (const string& propName)
{ 
  CEPProperty* pProperty = getProperty(propName);
  if (!pProperty)
  {
    pProperty = &_dummyProperty;
  }
  return *pProperty;
}

bool CEPPropertySet::setValue (const string& propName, 
                                     const GCFPValue& value)
{
  CEPProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    LOG_WARN(formatString (
        "Property '%s' not in this set (%s)! Value not set!",
        propName.c_str(),
        _scope.c_str()));
    return false;
  }
}
                             
bool CEPPropertySet::setValue (const string& propName, 
                                     const string& value)
{
  CEPProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    LOG_WARN(formatString (
        "Property '%s' not in this set (%s)! Value not set!",
        propName.c_str(),
        _scope.c_str()));
    return false;
  }
}

GCFPValue* CEPPropertySet::getValue (const string& propName)
{
  CEPProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->getValue();
  }
  else 
  {
    return 0;
  }
}
                                     
bool CEPPropertySet::exists (const string& propName) const
{
  CEPProperty* pProperty = getProperty(propName);
  return (pProperty != 0);
}

void CEPPropertySet::valueSet(const string& propName, const GCFPValue& value)
{
  ASSERT(_pClient);
  // a user has changed the property value and monitoring is switched on
  // changed value will be forward to the PIClient
  _pClient->valueSet(*this, propName, value);  
}

void CEPPropertySet::addProperty(const string& propName, CEPProperty& prop)
{
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::iterator iter = _properties.find(shortPropName);
  if (iter == _properties.end())
  {
    _properties[shortPropName] = &prop;
  }
  else
  {
    LOG_DEBUG(formatString ( 
      "Property %s already existing in scope '%s'",
      shortPropName.c_str(),
      _scope.c_str()));
  }
}

bool CEPPropertySet::cutScope(string& propName) const 
{
  bool scopeFound(false);
  
  if (propName.find(_scope) == 0)
  {
    // plus 1 means erase the GCF_PROP_NAME_SEP after scope too
    propName.erase(0, _scope.size() + 1); 
    scopeFound = true;
  }
  
  return scopeFound;
}

void CEPPropertySet::clearAllProperties()
{
  CEPProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    ASSERT(pProperty);
    delete pProperty;
  }
}

void CEPPropertySet::wrongState(const char* request)
{
  const char* stateString[] =
  {
    "DISABLED",
    "DISABLING",
    "ENABLING",
    "ENABLED",
    "LINKED",
  };
  LOG_WARN(formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        getScope().c_str(),
        stateString[_state]));  
}

//------------------------------------------------------------------------------
//  READ TYPE INFORMATION FILE PART
//------------------------------------------------------------------------------

// Type conversion matrix
TMACValueType macValueTypes[] = 
{
  /*
  MAC value types      PVSS type name               PVSS type id
  */
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // structure                    1
  NO_LPT,           // <not specified by pvss>      0
  LPT_DYNCHAR,      // dyn. character array         3
  LPT_DYNUNSIGNED,  // dyn. unsigned array          4
  LPT_DYNINTEGER,   // dyn. integer array           5
  LPT_DYNDOUBLE,    // dyn. float array             6
  LPT_DYNBOOL,      // dyn. bit array               7
  NO_LPT,           // dyn. bit pattern-array       8
  LPT_DYNSTRING,    // dyn. text array              9
  NO_LPT,           // dyn. time array              10
  NO_LPT,           // character structure          11
  NO_LPT,           // integer structure            12
  NO_LPT,           // unsigned structure           13
  NO_LPT,           // float structure              14
  NO_LPT,           // bit32                        15
  NO_LPT,           // bit32 structure              16
  NO_LPT,           // text structure               17
  NO_LPT,           // time structure               18
  LPT_CHAR,         // character                    19
  LPT_UNSIGNED,     // unsigned                     20
  LPT_INTEGER,      // integer                      21
  LPT_DOUBLE,       // float                        22
  LPT_BOOL,         // bit                          23
  NO_LPT,           // bit pattern                  24
  LPT_STRING,       // text                         25
  NO_LPT,           // time                         26
  NO_LPT,           // identifier                   27
  NO_LPT,           // dyn. identifier              29
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // identifier array             39
  NO_LPT,           // <not specified by pvss>      0
  NO_LPT,           // type reference               41
  NO_LPT,           // multilingual text            42
  NO_LPT,           // multilingual text structure  43
  NO_LPT,           // dyn. description array       44
  NO_LPT,           // <not specified by pvss>      0
  LPT_BLOB,         // blob                         46
  NO_LPT,           // blob structure               47
  LPT_DYNBLOB,      // dyn. blob array              48
};

// forward declaration
void buildTypeStructTree(const string path, 
                         ifstream&    pvssAsciiFile, 
                         char*        curAsciiLine, 
                         CEPPropertySet::TPropInfoList& propInfos);

void CEPPropertySet::readTypeFile(TPropInfoList& propInfos)
{
  ifstream    pvssAsciiFile;

  char buffer[200];
  // Try to open the type information file
  sprintf(buffer, "typeStruct_%s.dpl", _type.c_str());
  pvssAsciiFile.open(buffer, ifstream::in);
  
  if (!pvssAsciiFile)
  {
    LOG_ERROR(formatString( 
        "File '%s' with type information could not be opend. No property set could be setup.",
        buffer));
    return;
  }
  
  char asciiLine[ASCII_LINE_SIZE];
  bool typesSectionFound(false);

  // Read the file line by line and convert to element names (property names).
  while (pvssAsciiFile.getline (asciiLine, ASCII_LINE_SIZE))
  {
    if (typesSectionFound)
    {
      if (strlen(asciiLine) == 0) // end of typesSection
      {
        break;
      }
      else if (strncmp(asciiLine, _type.c_str(), _type.length()) == 0)      
      {
        // now we are ready to read all elements/properties (element by element)
        buildTypeStructTree("", pvssAsciiFile, asciiLine, propInfos);
      }
    }
    if (strncmp(asciiLine, "TypeName", 8) == 0)
    {
      // now we are ready to search the type (information, property list)
      typesSectionFound = true;
    }
  }
  pvssAsciiFile.close();
}

void buildTypeStructTree(const string path, 
                         ifstream&    pvssAsciiFile, 
                         char*        curAsciiLine, 
                         CEPPropertySet::TPropInfoList& propInfos)
{
  string propName = path;
  static char asciiLine[ASCII_LINE_SIZE];
  static bool rootElementFound = false;
  static bool readyWithType = false;
  unsigned int elType, elTypeSeq;  
  
  static char elName[256];
  
  int nrOfScanned = sscanf(curAsciiLine, "%s%d#%d", elName, &elType, &elTypeSeq);
  // first element entry found or empty ascii line found (end of type information section)
  if (elTypeSeq == 1 || nrOfScanned < 3)
  {
    if (rootElementFound) 
    {
      // type information is complete, because start of new type information 
      // sequence is found (seqnr 1) or end of type information section is found
      readyWithType = true; 
      return;
    }
    rootElementFound = true;
  }
  else
  {
    // a new level of the property name is found, add the level only if not a 
    // pvss ref type
    if (elType != 41) // 41 == type ref
    {
      // add only a propname level seperator if always a level is set
      if (propName.length() > 0) propName += GCF_PROP_NAME_SEP; 
      // add the propname level
      propName += elName;  
    }
  }
      
  switch (elType)
  {
    case 1:   // structure
    case 11:  // character structure
    case 12:  // integer structure
    case 13:  // unsigned structure
    case 14:  // float structure
    case 16:  // bit32 structure
    case 17:  // text structure
    case 18:  // time structure
    case 41:  // type ref
    case 47:  // blob structure
      // a new propname level will entered
      while (pvssAsciiFile.getline (asciiLine, ASCII_LINE_SIZE) && !readyWithType)
      {
        buildTypeStructTree(propName, pvssAsciiFile, asciiLine, propInfos);
      }
      break;
      
    default:
      if (macValueTypes[elType] != NO_LPT)
      {
        if (isValidPropName(propName.c_str()))
        {
          // we have a valid propname so add it to the list
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
            "TypeElement typeID %d (see DpElementType.hxx) is unknown to GCF (%s). Not add!!!",
            elType, 
            propName.c_str()));      
      }
      break;
  }
}
  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR

