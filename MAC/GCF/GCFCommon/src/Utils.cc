#include "Utils.h"
#include <stdio.h>
#include <assert.h>

Utils::Utils()
{}
Utils::~Utils()
{}

unsigned int Utils::unpackString(char* pStringData, string& value)
{
  unsigned int stringLength(0);
  memcpy((void *) &stringLength, pStringData, SLEN_FIELD_SIZE);
  value.clear();
  value.append(pStringData + SLEN_FIELD_SIZE, stringLength);
  return stringLength + SLEN_FIELD_SIZE;
}

unsigned int Utils::packString(const string& value, char* buffer, unsigned int maxBufferSize)
{
  unsigned int neededBufLength = value.size() + SLEN_FIELD_SIZE;
  assert(neededBufLength < maxBufferSize);
  unsigned int stringLength(value.size());
  memcpy(buffer, (void *) &stringLength, SLEN_FIELD_SIZE);
  memcpy(buffer + SLEN_FIELD_SIZE, (void *) value.c_str(), value.size());
  return neededBufLength;
}

unsigned short Utils::getStringDataLength(char* pStringData)
{
  unsigned int stringLength(0);
  memcpy((void *) &stringLength, pStringData, SLEN_FIELD_SIZE);
  return stringLength + SLEN_FIELD_SIZE;
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
  
  return packString(allPropNames, buffer, maxBufferSize);
}

void Utils::unpackPropertyList(char* pListData, list<string>& propertyList)
{
  unsigned int dataLength;
  memcpy((void *) &dataLength, pListData, SLEN_FIELD_SIZE);
  char propertyData[dataLength + 1];
  memcpy(propertyData, pListData + SLEN_FIELD_SIZE, dataLength);
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
