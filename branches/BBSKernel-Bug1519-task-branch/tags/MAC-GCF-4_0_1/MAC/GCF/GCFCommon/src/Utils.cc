#include <GCF/Utils.h>
#include <stdio.h>
#include <assert.h>
#include <GCF/GCF_Defines.h>

Utils::Utils()
{}
Utils::~Utils()
{}

void Utils::getPropertyListString(string& propListString, 
                                  const list<string>& propertyList)
{
  propListString.clear();
  for (list<string>::const_iterator iter = propertyList.begin(); 
       iter != propertyList.end(); ++iter)
  {
    propListString += *iter;
    propListString += '|';
  }
}

void Utils::getPropertyListFromString(list<string>& propertyList, 
                                      const string& propListString)
{
  unsigned int dataLength = propListString.length();
  char propertyData[dataLength + 1];
  memcpy(propertyData, propListString.c_str(), dataLength);
  propertyData[dataLength] = 0;
  propertyList.clear();
  if (dataLength > 0)
  {
    string propName;
    char* pPropName = strtok(propertyData, "|");
    while (pPropName && dataLength > 0)
    {
      propName = pPropName;      
      pPropName = strtok(NULL, "|");
      dataLength -= (propName.size() + 1);
      propertyList.push_back(propName);
    }
  }
}

bool Utils::isValidPropName(const char* propName)
{
  bool result(true);
  assert(propName);
  char doubleSep[] = {GCF_PROP_NAME_SEP, GCF_PROP_NAME_SEP, 0};
  if (propName[0] == GCF_PROP_NAME_SEP || propName[strlen(propName) - 1] == GCF_PROP_NAME_SEP )
  {
    result = false;
  }
  else if (strstr(propName, doubleSep) != 0)
  {
    result = false;
  }
  else
  {
    for(unsigned short i = 0; i < strlen(propName); i++)
    {
      if (!isalnum(propName[i]) && propName[i] != GCF_PROP_NAME_SEP)
      {
        result = false;
        break;
      }
    }
  }
  return true;// TODO: use regular expression to find out the name meets the convention
}
