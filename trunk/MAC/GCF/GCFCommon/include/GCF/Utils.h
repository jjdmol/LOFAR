#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

class Utils
{
  public:
  
    Utils();
    ~Utils();
       
    static bool isValidPropName(const char* propName);    
};

#endif // UTILS_H
