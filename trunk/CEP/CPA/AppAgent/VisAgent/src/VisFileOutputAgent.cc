#include "VisFileOutputAgent.h"

//##ModelId=3E2C29B00086
int VisFileOutputAgent::state() const
{
  return fileState() != FILEERROR 
          ? SUCCESS 
          : ERROR;
}

//##ModelId=3E2C29B40120
string VisFileOutputAgent::stateString() const
{
  return fileState() == FILEERROR 
         ? "ERROR " + errorString()
         :  "OK (" + fileStateString() + ")";
}

