//#  GCF_RTMyPropertySet.cc: 
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

#include <GCF/PALlight/GCF_RTMyPropertySet.h>
#include <GCF/PALlight/GCF_RTMyProperty.h>
#include <GPM_RTController.h>
#include <GCF/PALlight/GCF_RTAnswer.h>
#include <GCF/Utils.h>
#include <Common/lofar_iostream.h>
#include <sstream>
using std::istringstream;
using std::ostringstream;
using std::ifstream;

#include <Common/lofar_fstream.h>
#include <sys/types.h>
#include <unistd.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;
  namespace RTCPMLlight 
  {
TMACValueType macValueTypes[] = 
{
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

GCFRTMyPropertySet::GCFRTMyPropertySet(const char* name,
                                       const char* type,
                                       TPSCategory category,
                                       GCFRTAnswer* pAnswerObj) : 
  _scope(name),
  _type(type),
  _category(category),
  _pAnswerObj(pAnswerObj),
  _state(S_DISABLED),
  _dummyProperty(*this),
  _counter(0),
  _missing(0)
{
  GCFRTMyProperty* pProperty;
  TPropertyInfo* pPropInfo;
  
  if (!isValidScope(_scope.c_str()))
  {
    LOG_WARN(formatString ( 
        "Scope %s meets not the name convention! Set to \"\"",
        _scope.c_str()));
    _scope = "";
  }
  readTypeFile();
  for (TPropInfoList::iterator iter = _propSetInfo.begin();
       iter != _propSetInfo.end(); ++iter)
  { 
    pPropInfo = &(*iter);
    pProperty = new GCFRTMyProperty(*pPropInfo, *this);
    addProperty(pPropInfo->propName, *pProperty);
  }
  if (_pAnswerObj)
  {
    setAnswer(_pAnswerObj);
  }
  _pController = GPMRTController::instance();  
  ASSERT(_pController);
}  

GCFRTMyPropertySet::~GCFRTMyPropertySet ()
{
  ASSERT(_pController);
  // delete this set from the controller permanent
  // this means no response will be send to this object
  // on response of the PA
  _pController->unregisterScope(*this); 
  clearAllProperties();  

  _pController->deletePropSet(*this);  
  GPMRTController::release(); 
  _pController = 0;
}
  
TGCFResult GCFRTMyPropertySet::enable ()
{
  TGCFResult result(GCF_NO_ERROR);
    
  LOG_INFO(formatString ( 
      "REQ: Enable property set '%s'",
      getScope().c_str()));
  if (_state != S_DISABLED)
  {
    wrongState("enable");
    result = GCF_WRONG_STATE;
  }
  else
  {
    ASSERT(_pController);
    TPMResult pmResult = _pController->registerScope(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _state = S_ENABLING;
    }
    else if (pmResult == PM_SCOPE_ALREADY_EXISTS)
    {
      LOG_ERROR(formatString ( 
          "Property set with scope %s already exists in this Application",
          getScope().c_str()));    
      result = GCF_SCOPE_ALREADY_REG;
    }
  }
  return result;
}

TGCFResult GCFRTMyPropertySet::disable ()
{
  TGCFResult result(GCF_NO_ERROR);
  
  LOG_INFO(formatString ( 
      "REQ: Disable property set '%s'",
      getScope().c_str()));
  switch (_state)
  {
    case S_LINKED:
      GCFRTMyProperty* pProperty;
      for (TPropertyList::iterator iter = _properties.begin();
           iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        ASSERT(pProperty);
        if (pProperty->isLinked())
        {
          pProperty->unlink();
        }
      }
      // intentional fall through
    case S_ENABLED:
    {
      ASSERT(_pController);
  
      TPMResult pmResult = _pController->unregisterScope(*this);
      ASSERT(pmResult == PM_NO_ERROR);  
      _state = S_DISABLING;
      
      break;
    }
    default:
      wrongState("disable");
      result = GCF_WRONG_STATE;
      break;
  }
  return result;
}

void GCFRTMyPropertySet::scopeRegistered (TGCFResult result)
{
  ASSERT(_state == S_ENABLING);
  LOG_INFO(formatString ( 
      "PA-RESP: Property set '%s' is enabled%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));
  if (result == GCF_NO_ERROR)
  {
    _state = S_ENABLED;
  }
  else
  {
    _state = S_DISABLED;
  }

  dispatchAnswer(F_MYPS_ENABLED, result);
}

void GCFRTMyPropertySet::scopeUnregistered (TGCFResult result)
{
  ASSERT(_state == S_DISABLING);
   
  LOG_INFO(formatString ( 
      "Property set '%s' is disabled%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));

  _state = S_DISABLED;
  dispatchAnswer(F_MYPS_DISABLED, result);
}

void GCFRTMyPropertySet::linkProperties()
{
  ASSERT(_pController);
  list<string> propsToSubscribe;
  switch (_state)
  {
    case S_ENABLED:
    {
      GCFRTMyProperty* pProperty(0);

      ASSERT(_counter == 0);
      _missing = 0;
      for(TPropertyList::iterator iter = _properties.begin(); 
          iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        ASSERT(pProperty);
        pProperty->link();
        if (pProperty->testAccessMode(GCF_WRITABLE_PROP))
        {
          LOG_DEBUG(formatString(
              "Property '%s' must be subscribed in PI. Add to list.",
              pProperty->getName().c_str()));
          propsToSubscribe.push_back(pProperty->getName());
        }     
      }      
      _state = S_LINKED;
      _pController->propertiesLinked(getScope(), propsToSubscribe, PI_NO_ERROR);
      break;
    }

    case S_DISABLED:
    case S_DISABLING:
      _pController->propertiesLinked(getScope(), propsToSubscribe, PI_PS_GONE);
      break;

    default:
      wrongState("linkProperties");
      _pController->propertiesLinked(getScope(), propsToSubscribe, PI_WRONG_STATE);
      break;
  }
}

void GCFRTMyPropertySet::unlinkProperties()
{
  ASSERT(_pController);
  switch (_state)
  {
    case S_DISABLED:
    case S_DISABLING:
      _pController->propertiesUnlinked(getScope(), PI_PS_GONE);
      break;

    case S_LINKED:
    {
      _state = S_ENABLED;
      GCFRTMyProperty* pProperty(0);
      for (TPropertyList::iterator iter = _properties.begin(); 
           iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        if (pProperty) pProperty->unlink();
      }  
      _pController->propertiesUnlinked(getScope(), PI_NO_ERROR);
      break;
    }
    default:
      wrongState("unlinkProperties");
      _pController->propertiesUnlinked(getScope(), PI_WRONG_STATE);
      break;
  }
}

GCFRTMyProperty* GCFRTMyPropertySet::getProperty (const string& propName) const
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

GCFRTMyProperty& GCFRTMyPropertySet::operator[] (const string& propName)
{ 
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (!pProperty)
  {
    pProperty = &_dummyProperty;
  }
  return *pProperty;
}

TGCFResult GCFRTMyPropertySet::setValue (const string& propName, 
                                         const GCFPValue& value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}
                             
TGCFResult GCFRTMyPropertySet::setValue (const string& propName, 
                                       const string& value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

GCFPValue* GCFRTMyPropertySet::getValue (const string& propName)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->getValue();
  }
  else 
  {
    return 0;
  }
}
                                     
GCFPValue* GCFRTMyPropertySet::getOldValue (const string& propName)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->getOldValue();
  }
  else 
  {
    return 0;
  }
}                                         

void GCFRTMyPropertySet::setAnswer (GCFRTAnswer* pAnswerObj)
{
  GCFRTMyProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    ASSERT(pProperty);
    pProperty->setAnswer(pAnswerObj);
  }
  _pAnswerObj = pAnswerObj;
}

bool GCFRTMyPropertySet::exists (const string& propName) const
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  return (pProperty != 0);
}

void GCFRTMyPropertySet::valueSet(const string& propName, const GCFPValue& value) const
{
  ASSERT(_pController);
  _pController->valueSet(propName, value);  
}

void GCFRTMyPropertySet::valueChanged(string propName, const GCFPValue& value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  ASSERT(pProperty);
  pProperty->valueChanged(value);
}

void GCFRTMyPropertySet::dispatchAnswer(unsigned short sig, TGCFResult result)
{
  if (_pAnswerObj != 0)
  {
    GCFPropSetAnswerEvent e(sig);
    e.pScope = _scope.c_str();
    e.result = result;
    _pAnswerObj->handleAnswer(e);
  }
}

void GCFRTMyPropertySet::addProperty(const string& propName, GCFRTMyProperty& prop)
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

bool GCFRTMyPropertySet::cutScope(string& propName) const 
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

void GCFRTMyPropertySet::clearAllProperties()
{
  GCFRTMyProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    ASSERT(pProperty);
    delete pProperty;
  }
}

void GCFRTMyPropertySet::setAllAccessModes(TAccessMode mode, bool on)
{
  GCFRTMyProperty* pProperty;
  for(TPropertyList::iterator iter = _properties.begin(); 
      iter != _properties.end(); ++iter)
  {
    pProperty = iter->second;
    ASSERT(pProperty);
    pProperty->setAccessMode(mode, on);    
  }
}

void GCFRTMyPropertySet::initProperties(const TPropertyConfig config[])
{
  GCFRTMyProperty* pProperty;
  unsigned int i = 0;
  while (config[i].propName != 0)
  {
    pProperty = getProperty(config[i].propName);
    if (pProperty)
    {
      if (config[i].defaultValue)
      {
        pProperty->setValue(config[i].defaultValue);
      }
      if (~config[i].accessMode & GCF_READABLE_PROP)
        pProperty->setAccessMode(GCF_READABLE_PROP, false);    
      if (~config[i].accessMode & GCF_WRITABLE_PROP)
        pProperty->setAccessMode(GCF_WRITABLE_PROP, false);    
    }
    i++;
  }
}

void GCFRTMyPropertySet::wrongState(const char* request)
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

void buildTypeStructTree(const string path, 
                         ifstream&    pvssAsciiFile, 
                         char*        curAsciiLine, 
                         list<TPropertyInfo>& propInfos);

void GCFRTMyPropertySet::readTypeFile()
{
  ifstream    pvssAsciiFile;

  char buffer[200];
  system("chmod 777 genTypeInfo");
  sprintf(buffer, "./genTypeInfo %d %s", getpid(), _type.c_str());
  system(buffer);
  //# Try to pen the file
  sprintf(buffer, "typeInfo_%d.dpl.tmp", getpid());
  pvssAsciiFile.open(buffer, ifstream::in);
  
  char asciiLine[1024];
  bool typesSectionFound(false);
  bool typeFound(false);

  //# Read the file line by line and convert it to Key Value pairs.
  while (pvssAsciiFile.getline (asciiLine, 1024))
  {
    if (typesSectionFound)
    {
      if (strlen(asciiLine) == 0) // end of typeSection
      {
        break;
      }
      else if (strncmp(asciiLine, _type.c_str(), _type.length()) == 0)      
      {
        typeFound = true;
      }
      if (typeFound)
      {
        buildTypeStructTree("", pvssAsciiFile, asciiLine, _propSetInfo);
        break;        
      }
    }
    if (strncmp(asciiLine, "TypeName", 8) == 0)
    {
      typesSectionFound = true;
    }
  }
  pvssAsciiFile.close();
  sprintf(buffer, "rm -f typeInfo_%d.dpl.tmp", getpid());
  system(buffer);
}

void buildTypeStructTree(const string path, 
                         ifstream&    pvssAsciiFile, 
                         char*        curAsciiLine, 
                         list<TPropertyInfo>& propInfos)
{
  string propName = path;
  static char asciiLine[1024];
  static bool rootElementFound = false;
  static bool readWithBuild = false;
  unsigned int elType, elTypeSeq;  
  
  char* elName(0);
  int nrOfScanned = sscanf(curAsciiLine, "%as%d#%d", &elName, &elType, &elTypeSeq);
  if (elTypeSeq == 1 || nrOfScanned < 3)
  {
    if (rootElementFound) 
    {
      readWithBuild = true; 
      return;
    }
    rootElementFound = true;
  }
  else
  {
    if (strcmp(elName, "__internal") == 0)
    {
      delete [] elName;
      return;
    }
    if (elType != 41) //type ref
    {
      if (propName.length() > 0) propName += '.';
      propName += elName;  
    }
  }
  if (elName)
    delete [] elName;
  if (elType == 1 || elType == 41) // structure or type ref
  {
    while (pvssAsciiFile.getline (asciiLine, 1024) && !readWithBuild)
    {
      buildTypeStructTree(propName, pvssAsciiFile, asciiLine, propInfos);
    }
  }
  else
  {
    if (macValueTypes[elType] != NO_LPT)
    {
      if (isValidPropName(propName.c_str()))
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
}
  } // namespace RTCPMLlight
 } // namespace GCF
} // namespace LOFAR
