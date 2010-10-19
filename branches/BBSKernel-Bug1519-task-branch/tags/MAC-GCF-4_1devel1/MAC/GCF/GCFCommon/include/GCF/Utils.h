#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

class Utils
{
  public:
  
    Utils();
    ~Utils();
       
    static void getPropertyListString(string& propListString, 
                                      const list<string>& propertyList);
    static void getPropertyListFromString(list<string>& propertyList, 
                                          const string& propListString);
    static bool isValidPropName(const char* propName);    
};

#endif // UTILS_H
