
#include <iostream>
#include <fstream>
#include "parser/selfparse.h"

#define DEBUG(x) if(debug_flag_on) std::cout << x << std::endl;
static bool debug_flag_on(true);

std::istream *selfparseStream;

#ifdef __cplusplus
extern "C" {
#endif

int selfparseGetChars(char * buf, int max_size)
{
   int result = 0;
   char c;

   DEBUG("trying to get " << max_size << " chars." );

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

   DEBUG("resulting buf of length " << result << ": " << buf);

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

   char * calculateBrancheNumber()
   {
      static char * branchNumber = 0;
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
