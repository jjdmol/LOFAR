#ifndef VISAGENT_SRC_VISFILEOUTPUTAGENT_H_HEADER_INCLUDED_C26B922B
#define VISAGENT_SRC_VISFILEOUTPUTAGENT_H_HEADER_INCLUDED_C26B922B
    
#include <VisAgent/VisFileAgentBase.h>
#include <VisAgent/VisOutputAgent.h>

//##ModelId=3E02FF13009E
class VisFileOutputAgent : public VisOutputAgent, public VisFileAgentBase
{
  public:
    //##ModelId=3E2C29B00086
    //##Documentation
    //## Reports current state of agent. Default version always reports success
    virtual int state() const;

    //##ModelId=3E2C29B40120
    //##Documentation
    //## Reports current state as a string
    virtual string stateString() const;

};



#endif /* VISAGENT_SRC_VISFILEOUTPUTAGENT_H_HEADER_INCLUDED_C26B922B */
