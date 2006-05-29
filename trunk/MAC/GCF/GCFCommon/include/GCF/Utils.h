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

void	convListToString(std::string& listString, 
                         const std::list<std::string>& stringList);
void	convStringToList(std::list<std::string>& stringList, 
                         const std::string& listString);
void	convSetToString(std::string& setString, 
                        const std::set<std::string>& stringSet);
void	convStringToSet(std::set<std::string>& stringSet, 
                        const std::string& setString);
bool	isValidPropName(const char* propName);    
bool	isValidScope   (const char* scopeName);
string 	myHostname	   (bool	giveFullName = false);

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif // UTILS_H
