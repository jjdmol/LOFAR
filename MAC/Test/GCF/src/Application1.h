#ifndef APPLICATION_H
#define APPLICATION_H

#include <GCF/TM/GCF_Task.h>
#include "SupervisedTask.h"
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <Suite/test.h>

class GCFEvent;
class GCFPortInterface;

class Application : public GCFTask, public Test
{
  public:
    Application();
    void run();    
    
  private: 
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test1_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test1_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test2_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test2_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test2_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test2_5(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test3_4(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test4_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test5_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_1(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_2(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_4(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_5(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test6_6(GCFEvent& e, GCFPortInterface& p);

    GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
    
  private:
    Task _supTask1;
    Task _supTask2;
    unsigned int _counter;
    unsigned int _curRemoteTestNr;
    // Task1
    GCFMyPropertySet _propertySetA1;
    GCFMyPropertySet _propertySetE1;
    GCFMyPropertySet _propertySetXX;
    // Task2
    GCFMyPropertySet _propertySetB1;
    GCFMyPropertySet _propertySetB2;
    GCFMyPropertySet _propertySetB3;

    GCFExtPropertySet _ePropertySetAC;
    GCFExtPropertySet _ePropertySetAD;
    GCFExtPropertySet _ePropertySetAH;
    GCFExtPropertySet _ePropertySetAL;
};
#endif
