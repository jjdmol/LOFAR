#ifndef APPLICATION_H
#define APPLICATION_H

#include <GCF/TM/GCF_Task.h>
#include "SupervisedTask.h"
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
    TM::GCFEvent::TResult test7_3(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult test7_4(TM::GCFEvent& e, TM::GCFPortInterface& p);

    TM::GCFEvent::TResult finished(TM::GCFEvent& e, TM::GCFPortInterface& p);
    
  private:
    Task _supTask;
    unsigned int _seqNr;
    unsigned int _counter;
    // Task
    PAL::GCFExtPropertySet _ePropertySetC;
    
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
