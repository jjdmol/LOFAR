#ifndef TASK_H
#define TASK_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/GCF_Defines.h>
#include "PropertyProxy.h"
#include "Answer.h"
#include <Common/lofar_iostream.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace Test
  {
    
class Task : public TM::GCFTask
{
  public:
    Task(TM::GCFTask& a, string taskName);
    ~Task();
    
    TM::GCFTCPPort& getPort() {return _port;}
    PropertyProxy& getProxy() {return _proxy;}
    Answer& getAnswerObj() {return _answer;}
    
    void handleAnswer(TM::GCFEvent& answer) 
      { _application.dispatch(answer, _port); }
  
  private: // state methods
    int initial(TM::GCFEvent& e, TM::GCFPortInterface& p);

  private: // methods called by the PropertyProxy
    friend class PropertyProxy;
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);
    void propValueChanged(const string& propName, const Common::GCFPValue& value);
    void propValueSet(const string& propName);
    void valueGet(const string& propName, const Common::GCFPValue& value);    
    
  private:
    TM::GCFTCPPort _port;
    TM::GCFTask& _application; 
    PropertyProxy _proxy;   
    Answer _answer;
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
