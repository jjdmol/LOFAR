#ifndef SUPERVISEDTASK_H
#define SUPERVISEDTASK_H

#include <GCF/GCF_Task.h>
#include <GCF/GCF_TCPPort.h>
#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_Defines.h>
#include "PropertyProxy.h"
#include "Answer.h"
#include <Common/lofar_iostream.h>

class SupervisedTask : public GCFTask
{
  public:
    SupervisedTask(GCFTask& a, string taskName);
    ~SupervisedTask();
    
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
