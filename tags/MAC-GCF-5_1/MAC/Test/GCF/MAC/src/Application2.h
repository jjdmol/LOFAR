#ifndef APPLICATION_H
#define APPLICATION_H

#include <GCF/TM/GCF_Task.h>
#include "SupervisedTask.h"
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <Suite/test.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace TM
  {
class GCFEvent;
class GCFPortInterface;
  }
  namespace Test
  {

class Application : public TM::GCFTask, public ::Test
{
  public:
    Application();
    virtual ~Application();
    
    void run();
    
  private: 
    TM::GCFEvent::TResult initial(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test1_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test1_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test2_5(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test3_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test3_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test3_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_4(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test5_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_4(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_5(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_6(TM::GCFEvent& e, TM::GCFPortInterface& p);

    TM::GCFEvent::TResult finished(TM::GCFEvent& e, TM::GCFPortInterface& p);
    
  private:
    Task _supTask3;
    unsigned int _counter;

    TM::GCFTCPPort _port;
    unsigned int _curRemoteTestNr;

    TM::GCFTCPPort* _pSTPort1;
    TM::GCFTCPPort* _pSTPort2;

    PAL::GCFMyPropertySet _propertySetC1;
    PAL::GCFMyPropertySet _propertySetD1;
    PAL::GCFMyPropertySet _propertySetB4;
    
    PAL::GCFExtPropertySet _ePropertySetAC;
    PAL::GCFExtPropertySet _ePropertySetAE;
    PAL::GCFExtPropertySet _ePropertySetAH;
    PAL::GCFExtPropertySet _ePropertySetAK;
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
