#include <lofar_config.h>

#include <GCF/Utils.h>
#include <stdio.h>
#include <unistd.h>
#include <GCF/GCF_Defines.h>

using std::set;

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common 
  {


void convListToString(string& listString, 
                                  const list<string>& stringList)
{
  listString.clear();
  for (list<string>::const_iterator iter = stringList.begin(); 
       iter != stringList.end(); ++iter) {
    listString += *iter;
    listString += '|';
  }
}

void convStringToList(list<string>& stringList, 
                                      const string& listString)
{
  unsigned int dataLength = listString.length();
  char data[dataLength + 1];
  memcpy(data, listString.c_str(), dataLength);
  data[dataLength] = 0;
  stringList.clear();
  if (dataLength > 0) {
    string stringListItem;
    char* pStringListItem = strtok(data, "|");
    while (pStringListItem && dataLength > 0) {
      stringListItem = pStringListItem;      
      pStringListItem = strtok(NULL, "|");
      dataLength -= (stringListItem.size() + 1);
      stringList.push_back(stringListItem);
    }
  }
}

void convSetToString(string& setString, 
                                  const set<string>& stringSet)
{
  setString.clear();
  for (set<string>::const_iterator iter = stringSet.begin(); 
       iter != stringSet.end(); ++iter) {
    setString += *iter;
    setString += '|';
  }
}

void convStringToSet(set<string>& stringSet, 
                                      const string& setString)
{
  unsigned int dataLength = setString.length();
  char data[dataLength + 1];
  memcpy(data, setString.c_str(), dataLength);
  data[dataLength] = 0;
  stringSet.clear();
  if (dataLength > 0) {
    string stringSetItem;
    char* pStringSetItem = strtok(data, "|");
    while (pStringSetItem && dataLength > 0) {
      stringSetItem = pStringSetItem;      
      pStringSetItem = strtok(NULL, "|");
      dataLength -= (stringSetItem.size() + 1);
      stringSet.insert(stringSetItem);
    }
  }
}

bool isValidPropName(const char* propName)
{
  bool valid(true);
  ASSERT(propName);
  char doubleSep[] = {GCF_PROP_NAME_SEP, GCF_PROP_NAME_SEP, 0};
  unsigned int length = strlen(propName);
  if (propName[0] == GCF_PROP_NAME_SEP || propName[length - 1] == GCF_PROP_NAME_SEP ) {
    valid = false;
  }
  else if (strstr(propName, doubleSep) != 0) {
    valid = false;
  }
  else {
    char refInd[] = "__";
    char* refIndPos = strstr(propName, refInd);
    if (refIndPos != 0) {
      if (refIndPos > propName) {
        if (*(refIndPos - 1) != GCF_PROP_NAME_SEP) {
          // ref indication may only found at begin or after a GCF_PROP_NAME_SEP
          valid = false;
        }
      }
      if (valid && strchr(refIndPos, GCF_PROP_NAME_SEP) > 0) {
        // ref indication may not used in struct name
        valid = false;
      }
    }
    for (unsigned short i = 0; valid && i < length; i++) {
      if (refIndPos > 0 && ((propName + i) == refIndPos)) {
        i += 2; // skip the ref indicator
      }
      if (!isalnum(propName[i]) && propName[i] != GCF_PROP_NAME_SEP) {
        valid = false;
      }
    }
  }
  return valid;
}

bool isValidScope(const char* scopeName)
{
  bool valid(true);
  ASSERT(scopeName);
  char doubleSep[] = {GCF_SCOPE_NAME_SEP, GCF_SCOPE_NAME_SEP, 0};
  unsigned int length = strlen(scopeName);
  char* sysNameSep = strchr(scopeName, ':');
  if (sysNameSep > 0) {
    length -= (sysNameSep + 1 - scopeName);
    scopeName = sysNameSep + 1;
  }
  if (scopeName[0] == GCF_SCOPE_NAME_SEP || scopeName[length - 1] == GCF_SCOPE_NAME_SEP ) {
    valid = false;
  }
  else if (strstr(scopeName, doubleSep) != 0) {
    valid = false;
  }
  else {
    for(unsigned short i = 0; valid && i < length; i++) {
      if (!isalnum(scopeName[i]) && scopeName[i] != GCF_SCOPE_NAME_SEP) {
        valid = false;
      }
    }
  }
  return valid;
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
