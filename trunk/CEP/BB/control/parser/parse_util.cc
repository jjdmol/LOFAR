
#include <iostream>
#include <fstream>
#include <sstream>
#include "parser/selfparse.h"

#include "blackboard/debug.h"

std::istream *selfparseStream;

#ifdef __cplusplus
extern "C" {
#endif

int selfparseGetChars(char * buf, int max_size)
{
   int result = 0;
   char c;

   std::ostringstream os;
   os << "trying to get " << max_size << " chars.";
   DEBUG(os.str());

   memset(buf,0,max_size);

   if(selfparseStream->eof())
   {
      return '\0';
   }

   do
   {
      selfparseStream->unsetf(std::ios_base::skipws);
      *selfparseStream >> c;
          //      if (c != '\0')
      buf[result] = c;
   } while ((++result < (max_size-1)) &&
            !selfparseStream->eof()
           );

   os << "resulting buf of length " << result;
   DEBUG(os.str());

   return result;
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  char * branch = ("start");
  unsigned int subBranch = (0);

  static char * branchNumber = 0;
  void newSiblings(void)
  {
    TRACE t("newSiblings(void)");
    subBranch = 0;
  }
  char * calculateBrancheNumber()
  {
    char subBstr[6];
    int rc = snprintf(subBstr, 5, "%u", subBranch++);
    if( rc > 5 )
    {
      report ("branch number truncated");
    }
    if(branchNumber)
    {
      free(branchNumber);
    }
          //   branchNumber = new char [ strlen(branch) + strlen(subBstr) + 2];
    branchNumber = (char *)(malloc( strlen(branch) + strlen(subBstr) + 2));
      
    sprintf(branchNumber,"%s.%s",branch,subBstr);
      
    return branchNumber;
  }

#ifdef __cplusplus
}
#endif
