#include <GCF/Utils.h>
#include <stdio.h>
#include <assert.h>
#include <GCF/GCF_Defines.h>

Utils::Utils()
{}
Utils::~Utils()
{}

void Utils::convListToString(string& listString, 
                                  const list<string>& stringList)
{
  listString.clear();
  for (list<string>::const_iterator iter = stringList.begin(); 
       iter != stringList.end(); ++iter)
  {
    listString += *iter;
    listString += '|';
  }
}

void Utils::convStringToList(list<string>& stringList, 
                                      const string& listString)
{
  unsigned int dataLength = listString.length();
  char data[dataLength + 1];
  memcpy(data, listString.c_str(), dataLength);
  data[dataLength] = 0;
  stringList.clear();
  if (dataLength > 0)
  {
    string stringListItem;
    char* pStringListItem = strtok(data, "|");
    while (pStringListItem && dataLength > 0)
    {
      stringListItem = pStringListItem;      
      pStringListItem = strtok(NULL, "|");
      dataLength -= (stringListItem.size() + 1);
      stringList.push_back(stringListItem);
    }
  }
}

bool Utils::isValidPropName(const char* propName)
{
  bool valid(true);
  assert(propName);
  char doubleSep[] = {GCF_PROP_NAME_SEP, GCF_PROP_NAME_SEP, 0};
  unsigned int length = strlen(propName);
  if (propName[0] == GCF_PROP_NAME_SEP || propName[length - 1] == GCF_PROP_NAME_SEP )
  {
    valid = false;
  }
  else if (strstr(propName, doubleSep) != 0)
  {
    valid = false;
  }
  else
  {
    char refInd[] = "__";
    char* refIndPos = strstr(propName, refInd);
    if (refIndPos != 0)
    {
      if (refIndPos > propName)
      {
        if (*(refIndPos - 1) != GCF_PROP_NAME_SEP)
        {
          // ref indication may only found at begin or after a GCF_PROP_NAME_SEP
          valid = false;
        }
      }
      if (valid && strchr(refIndPos, GCF_PROP_NAME_SEP) > 0)
      {
        // ref indication may not used in struct name
        valid = false;
      }
    }
    for (unsigned short i = 0; valid && i < length; i++)
    {
      if (!isalnum(propName[i]) && propName[i] != GCF_PROP_NAME_SEP)
      {
        valid = false;
      }
    }
  }
  return valid;
}

bool Utils::isValidScope(const char* scopeName)
{
  bool valid(true);
  assert(scopeName);
  char doubleSep[] = {GCF_SCOPE_NAME_SEP, GCF_SCOPE_NAME_SEP, 0};
  unsigned int length = strlen(scopeName);
  char* sysNameSep = strchr(scopeName, ':');
  if (sysNameSep > 0)
  {
    scopeName = sysNameSep + 1;
  }
  if (scopeName[0] == GCF_SCOPE_NAME_SEP || scopeName[length - 1] == GCF_SCOPE_NAME_SEP )
  {
    valid = false;
  }
  else if (strstr(scopeName, doubleSep) != 0)
  {
    valid = false;
  }
  else
  {
    for(unsigned short i = 0; valid && i < length; i++)
    {
      if (!isalnum(scopeName[i]) && scopeName[i] != GCF_SCOPE_NAME_SEP)
      {
        valid = false;
      }
    }
  }
  return valid;
}
