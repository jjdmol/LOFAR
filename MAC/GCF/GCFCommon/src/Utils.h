#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

class Utils
{
  public:
  
    Utils();
    ~Utils();
    
    static unsigned int packString(const string& value, 
                                   char* buffer, 
                                   unsigned int maxBufferSize);
                                  
    static unsigned int unpackString(char* pStringData, 
                             string& value);
   
    static unsigned short getStringDataLength(char* pStringData);
    static const unsigned int SLEN_FIELD_SIZE = 3;
    static unsigned int packPropertyList(list<string>& propertyList, 
                                         char* buffer, 
                                         unsigned int maxBufferSize);
    static void unpackPropertyList(char* pListData, list<string>& propertyList);
};

#endif // UTILS_H
