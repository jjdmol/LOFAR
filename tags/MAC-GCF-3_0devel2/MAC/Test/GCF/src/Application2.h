#ifndef APPLICATION_H
#define APPLICATION_H

#include <GCF/GCF_Task.h>
#include "SupervisedTask.h"
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>

class GCFEvent;
class GCFPortInterface;

class Application : public GCFTask
{
  public:
    Application();
    
  private: 
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test101(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test102(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test103(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test104(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test105(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test201(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test202(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test203(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test204(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test205(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test206(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test207(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test208(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test209(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test210(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test301(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test302(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test303(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test304(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test305(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test306(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test401(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult test402(GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
    
    void passed(unsigned int passedTest)
    { 
      cerr << "Test " << passedTest << " passed\n";
      _passed++;
    }
    void failed(unsigned int failedTest)
    {
      cerr << "Test " << failedTest << " failed\n";
      _failed++;
    }
    void skipped(unsigned int skippedTest)
    {
      cerr << "Nothing to do in test " << skippedTest << " => skipped\n";
    }
  
  private:
    SupervisedTask _supTask3;
    unsigned char _passed;
    unsigned char _failed;
    unsigned int _counter;
    GCFTCPPort _port;
    unsigned int _curRemoteTestNr;
    GCFTCPPort* _pSTPort1;
    GCFTCPPort* _pSTPort2;
    GCFMyPropertySet _propertySetC1;
    GCFMyPropertySet _propertySetD1;
    GCFMyPropertySet _propertySetB4;
    GCFApc  _apcT1;
    GCFApc  _apcT1K;
    GCFApc  _apcT1a;
    GCFApc  _apcT1b;
    GCFApc  _apcT3;
};
#endif
