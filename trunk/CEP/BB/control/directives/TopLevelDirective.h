#ifndef TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6
#define TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6
#include "Strategy.h"
#include "StrategicDirective.h"

//##ModelId=3F3A5F9D00BB
template <Strategy>
class TopLevelDirective
{
  public:
    //##ModelId=3F44F0D6039B
    StrategicDirective campaign;
  private:
    //##ModelId=3F4DCA4801E4
    //##Documentation
    //## The textual db id of the textual representation of the directive. It
    //## starts with 0 followed by a series of dot seperated sequence numbers
    //## i.e. 0.23.1.5.0.
    //## 
    //## Top level is always 0. It is just there for clairity.
    string scriptID;


};



#endif /* TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6 */
