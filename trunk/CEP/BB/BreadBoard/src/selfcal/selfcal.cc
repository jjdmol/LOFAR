#include "selfcal.h"

void parameter::setName(char *newname)
{
    name = newname;
}
const char * parameter::getName(void)const
{
    return name;
}
