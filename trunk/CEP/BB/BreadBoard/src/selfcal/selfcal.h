#include <strings.h>

class parameter
{
      char * name;
      double value;
      double delta;
   public:
      void setName(char * newname);
      const char * getName(void)const;
};

