#include "VisFileAgentBase.h"

//##ModelId=3DFDFC07004C
string VisFileAgentBase::fileStateString() const
{
  switch( state_ )
  {
    case FILECLOSED:  return "FILECLOSED";
    case HEADER:      return "HEADER";
    case DATA:        return "DATA";
    case ENDFILE:     return "ENDFILE";
    case FILEERROR:   return "ERROR";
    default:          return "unknown";
  }
}

//##ModelId=3DF9FECE01CA
void VisFileAgentBase::setErrorState(const string &msg)
{
  state_ = FILEERROR;
  errmsg_ = msg;
}
