#include "Utils.h"
#include <stdio.h>

Utils::Utils()
{}
Utils::~Utils()
{}

unsigned int Utils::unpackString(char* pStringData, string& value)
{
  unsigned int stringLength(0);
  sscanf(pStringData, "%03x", &stringLength);
  value.assign(pStringData + SLEN_FIELD_SIZE, stringLength);
  return stringLength + SLEN_FIELD_SIZE;
}

unsigned int Utils::packString(const string& value, char* buffer, unsigned int maxBufferSize)
{
  unsigned int neededBufLength = value.size() + SLEN_FIELD_SIZE;
  assert(neededBufLength < maxBufferSize);
  sprintf(buffer, "%03x%s", value.size(), value.c_str());
  return neededBufLength;
}

unsigned short Utils::getStringDataLength(char* pStringData)
{
  unsigned int scopeNameLength(0);
  sscanf(pStringData, "%03x", &scopeNameLength);
  return scopeNameLength + SLEN_FIELD_SIZE;
}

unsigned int Utils::packPropertyList(list<string>& propertyList, char* buffer, unsigned int maxBufferSize)
{
  string allPropNames;
  for (list<string>::iterator iter = propertyList.begin(); 
       iter != propertyList.end(); ++iter)
  {
    allPropNames += *iter;
    allPropNames += '|';
  }
  unsigned int neededBufLength = allPropNames.size() + SLEN_FIELD_SIZE;
  
  assert(neededBufLength < maxBufferSize);
  
  sprintf(buffer, "%03x%s", allPropNames.length(), allPropNames.c_str());
  return neededBufLength;
}

void Utils::unpackPropertyList(char* pListData, list<string>& propertyList)
{
  unsigned int dataLength;
  char* pPropertyData;
  sscanf(pListData, "%03x", &dataLength);
  pPropertyData = pListData + 3;
  propertyList.clear();
  if (dataLength > 0)
  {
    string propName;
    char* pPropName = strtok(pPropertyData, "|");
    while (pPropName && dataLength > 0)
    {
      propName = pPropName;      
      pPropName = strtok(NULL, "|");
      dataLength -= (propName.size() + 1);
      propertyList.push_back(propName);
    }
  }
}
