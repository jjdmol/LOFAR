#ifndef SUPERVISEDTASK_H
#define SUPERVISEDTASK_H

#include <PML/GCF_SupTask.h>
#include <TM/Socket/GCF_TCPPort.h>
#include <TM/GCF_TMProtocols.h>
#include <GCFCommon/GCF_Defines.h>
#include "PropertyProxy.h"

class SupervisedTask : public GCFSupervisedTask
{
  public:
    SupervisedTask(GCFTask& a, string taskName);
    ~SupervisedTask();
    
    inline GCFTCPPort& getPort() {return _port;}
    inline PropertyProxy& getProxy() {return _proxy;}    
  
  private: // state methods
    int initial(GCFEvent& e, GCFPortInterface& p);

  private: // methods called by the PropertyProxy
    friend class PropertyProxy;
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);

  private: // definition of abstract methods of GCFSupervisedTask
    void valueChanged(const string& propName, const GCFPValue& value);
    void propValueChanged(const string& propName, const GCFPValue& value);
    void valueGet(const string& propName, const GCFPValue& value);
    void apcLoaded(const string& apcName, const string& scope, TGCFResult result);
    void apcUnloaded(const string& apcName, const string& scope, TGCFResult result);
    void apcReloaded(const string& apcName, const string& scope, TGCFResult result);
    void myPropertiesLoaded(const string& scope, TGCFResult result);
    void myPropertiesUnloaded(const string& scope, TGCFResult result);
    
  private:
    GCFTCPPort _port;
    GCFTask& _application; 
    PropertyProxy _proxy;   
};

// only for internal use
struct GCFPMLValueEvent : public GCFEvent
{
  GCFPMLValueEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPMLValueEvent);
  }
  const GCFPValue* pValue;
  const char* pPropName;
  bool internal;
};

struct GCFPMLAnswerEvent : public GCFEvent
{
  GCFPMLAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPMLAnswerEvent);
  }
  const char* pPropName;
};

struct GCFPMLAPCAnswerEvent : public GCFEvent
{
  GCFPMLAPCAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPMLAPCAnswerEvent);
  }
  const char* pApcName;
  const char* pScope;
  TGCFResult result;
};

struct GCFPMLMYPAnswerEvent : public GCFEvent
{
  GCFPMLMYPAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPMLMYPAnswerEvent);
  }
  const char* pScope;
  TGCFResult result;
};

enum {
  F_PML_PROTOCOL = F_GCF_PROTOCOL,
};
/**
 * F_PML_PROTOCOL signals
 */
enum {
  F_SUBSCRIBED_ID = 1,                                 
  F_UNSUBSCRIBED_ID,      
  F_VCHANGEMSG_ID,               
  F_VGETRESP_ID,        
  F_APCLOADED_ID,
  F_APCUNLOADED_ID,
  F_APCRELOADED_ID,
  F_MYLOADED_ID,
  F_MYUNLOADED_ID,
};

#define F_SUBSCRIBED_SIG    F_SIGNAL(F_PML_PROTOCOL, F_SUBSCRIBED_ID,    F_IN)
#define F_UNSUBSCRIBED_SIG  F_SIGNAL(F_PML_PROTOCOL, F_UNSUBSCRIBED_ID,  F_IN)
#define F_VCHANGEMSG_SIG    F_SIGNAL(F_PML_PROTOCOL, F_VCHANGEMSG_ID,    F_IN)
#define F_VGETRESP_SIG      F_SIGNAL(F_PML_PROTOCOL, F_VGETRESP_ID,      F_IN)
#define F_APCLOADED_SIG     F_SIGNAL(F_PML_PROTOCOL, F_APCLOADED_ID,     F_IN)
#define F_APCUNLOADED_SIG   F_SIGNAL(F_PML_PROTOCOL, F_APCUNLOADED_ID,   F_IN)
#define F_APCRELOADED_SIG   F_SIGNAL(F_PML_PROTOCOL, F_APCRELOADED_ID,   F_IN)
#define F_MYLOADED_SIG      F_SIGNAL(F_PML_PROTOCOL, F_MYLOADED_ID,      F_IN)
#define F_MYUNLOADED_SIG    F_SIGNAL(F_PML_PROTOCOL, F_MYUNLOADED_ID,    F_IN)

extern const char* F_PML_PROTOCOL_signalnames[];

#endif
