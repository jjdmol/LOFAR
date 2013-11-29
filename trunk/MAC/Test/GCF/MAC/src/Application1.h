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
    void abort();
    
  private: 
    TM::GCFEvent::TResult initial(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test1_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test1_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test2_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test2_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test2_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test2_5(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test3_4(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test4_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test5_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_2(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_4(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_5(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test6_6(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test7_1(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test7_2(TM::GCFEvent& e, TM::GCFPortInterface& p);

    TM::GCFEvent::TResult finished(TM::GCFEvent& e, TM::GCFPortInterface& p);
    
  private:
    Task _supTask1;
    Task _supTask2;
    unsigned int _counter;
    unsigned int _curRemoteTestNr;
    // Task1
    PAL::GCFMyPropertySet _propertySetA1;
    PAL::GCFMyPropertySet _propertySetE1;
    PAL::GCFMyPropertySet _propertySetXX;
    // Task2
    PAL::GCFMyPropertySet _propertySetB1;
    PAL::GCFMyPropertySet _propertySetB2;
    PAL::GCFMyPropertySet _propertySetB3;

    PAL::GCFExtPropertySet _ePropertySetAC;
    PAL::GCFExtPropertySet _ePropertySetAD;
    PAL::GCFExtPropertySet _ePropertySetAH;
    PAL::GCFExtPropertySet _ePropertySetAL;
    PAL::GCFExtPropertySet _ePropertySetBRD1;
    PAL::GCFExtPropertySet _ePropertySetBRD2;
    
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
