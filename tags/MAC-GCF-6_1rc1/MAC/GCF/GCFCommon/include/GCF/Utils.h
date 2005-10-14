#ifndef UTILS_H
#define UTILS_H

#include <Common/lofar_string.h>
#include <Common/lofar_list.h>
#include <set>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common 
  {

class Utils
{
  public:
  
    Utils();
    ~Utils();
       
    static void convListToString(std::string& listString, 
                                 const std::list<std::string>& stringList);
    static void convStringToList(std::list<std::string>& stringList, 
                                 const std::string& listString);
    static void convSetToString(std::string& setString, 
                                const std::set<std::string>& stringSet);
    static void convStringToSet(std::set<std::string>& stringSet, 
                                const std::string& setString);
    static bool isValidPropName(const char* propName);    
    static bool isValidScope(const char* scopeName);
};

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif // UTILS_H
