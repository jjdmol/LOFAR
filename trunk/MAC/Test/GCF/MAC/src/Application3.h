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
    virtual ~Application();
    void run();    
    
  private: 
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test7_3(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test7_4(GCFEvent& e, GCFPortInterface& p);

    GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
    
  private:
    Task _supTask;
    unsigned int _seqNr;
    unsigned int _counter;
    // Task
    GCFExtPropertySet _ePropertySetC;
    
};
#endif
