#ifndef APPLICATION_H
#define APPLICATION_H

#include <GCF/TM/GCF_Task.h>
#include "SupervisedTask.h"
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <Suite/test.h>

class GCFEvent;
class GCFPortInterface;

class Application : public GCFTask , public Test
{
  public:
    Application();
    void run();
    
  private: 
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test1_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test1_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test2_5(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test3_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test3_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test3_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_4(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test5_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_3(GCFEvent& e, GCFPortInterface& p);

    GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
    
  private:
    Task _supTask3;
    unsigned int _counter;

    GCFTCPPort _port;
    unsigned int _curRemoteTestNr;

    GCFTCPPort* _pSTPort1;
    GCFTCPPort* _pSTPort2;

    GCFMyPropertySet _propertySetC1;
    GCFMyPropertySet _propertySetD1;
    GCFMyPropertySet _propertySetB4;
    
    GCFExtPropertySet _ePropertySetAC;
    GCFExtPropertySet _ePropertySetAE;
    GCFExtPropertySet _ePropertySetAH;
    GCFExtPropertySet _ePropertySetAK;
};
#endif
