#ifndef TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6
#define TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6
#include "control/Strategy.h"
#include "StrategicDirective.h"
#include "HigherDirective.h"

//##ModelId=3F3A5F9D00BB
class TopLevelDirective : public HigherDirective, public Directive
{
 public:
  TopLevelDirective(const std::string & initID = "start");
 private:
    //##ModelId=3F4DCA4801E4
    //##Documentation
    //## The textual db id of the textual representation of the directive. It
    //## starts with 0 followed by a series of dot seperated sequence numbers
    //## i.e. 0.23.1.5.0.
    //## 
    //## Top level is always 0. It is just there for clairity.
    const std::string scriptID;
};



#endif /* TOPLEVELDIRECTIVE_H_HEADER_INCLUDED_C0C449E6 */
