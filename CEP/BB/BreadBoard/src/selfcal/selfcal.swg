%module selfcal
%{
#include "selfcal.h"
#include <string>
%}
class parameter
{
      char * name;
      double value;
      double delta;
public:
      void setName(char * newname);
      char *getName(void)const;
};

