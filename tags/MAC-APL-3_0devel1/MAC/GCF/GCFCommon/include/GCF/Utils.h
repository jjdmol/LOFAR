#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

using std::string;
using std::list;

class Utils
{
  public:
  
    Utils();
    ~Utils();
       
    static void convListToString(string& listString, 
                                      const list<string>& stringList);
    static void convStringToList(list<string>& stringList, 
                                          const string& listString);
    static bool isValidPropName(const char* propName);    
    static bool isValidScope(const char* scopeName);
};

#endif // UTILS_H
