#include "selfcal.h"

void parameter::setName(char *newname)
{
    name = newname;
}
const char * parameter::getName(void)const
{
    return name;
}
void parameter::setValue(double newvalue)
{
    value = newvalue;
}
const double parameter::getValue(void)const
{
    return value;
}
void parameter::setDelta(double newdelta)
{
    delta = newdelta;
}
const double parameter::getDelta(void)const
{
    return delta;
}
