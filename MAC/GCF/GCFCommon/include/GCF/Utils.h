#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

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
};

#endif // UTILS_H
