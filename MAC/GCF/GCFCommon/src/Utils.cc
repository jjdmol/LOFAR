#include <GCF/Utils.h>
#include <stdio.h>
#include <assert.h>
#include <GCF/GCF_Defines.h>

Utils::Utils()
{}
Utils::~Utils()
{}

bool Utils::isValidPropName(const char* propName)
{
  bool result(true);
  assert(propName);
  char doubleSep[] = {GCF_PROP_NAME_SEP, GCF_PROP_NAME_SEP, 0};
  if (propName[0] == GCF_PROP_NAME_SEP || propName[strlen(propName) - 1] == GCF_PROP_NAME_SEP )
  {
    result = false;
  }
  else if (strstr(propName, doubleSep) != 0)
  {
    result = false;
  }
  else
  {
    for(unsigned short i = 0; i < strlen(propName); i++)
    {
      if (!isalnum(propName[i]) && propName[i] != GCF_PROP_NAME_SEP)
      {
        result = false;
        break;
      }
    }
  }
  return true;// TODO: use regular expression to find out the name meets the convention
}
