#ifndef TASK_H
#define TASK_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/GCF_Defines.h>
#include "PropertyProxy.h"
#include "Answer.h"
#include <Common/lofar_iostream.h>

class Task : public GCFTask
{
  public:
    Task(GCFTask& a, string taskName);
    ~Task();
    
    inline GCFTCPPort& getPort() {return _port;}
    inline PropertyProxy& getProxy() {return _proxy;}
    inline Answer& getAnswerObj() {return _answer;}
    
    inline void handleAnswer(GCFEvent& answer) 
      { _application.dispatch(answer, _port); }
  
  private: // state methods
    int initial(GCFEvent& e, GCFPortInterface& p);

  private: // methods called by the PropertyProxy
    friend class PropertyProxy;
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);
    void propValueChanged(const string& propName, const GCFPValue& value);
    void valueGet(const string& propName, const GCFPValue& value);    
    
  private:
    GCFTCPPort _port;
    GCFTask& _application; 
    PropertyProxy _proxy;   
    Answer _answer;
};
#endif
