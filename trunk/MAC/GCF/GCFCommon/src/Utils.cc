#include <lofar_config.h>

#include <GCF/Utils.h>
#include <stdio.h>
#include <unistd.h>

using std::set;

namespace LOFAR {
 namespace GCF {
  namespace Common {

//
// myHostname(giveFullname)
//
string myHostname(bool	giveFullName)
{
	char	fullhostname[300];
	if (gethostname(fullhostname, 300) != 0) {
		return ("localhost");
	}

	if (!giveFullName) {
		char*	dot = strchr(fullhostname, '.');
		if (dot) {
			*dot='\0';
		}
	}

	return (fullhostname);
}

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


  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
