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
