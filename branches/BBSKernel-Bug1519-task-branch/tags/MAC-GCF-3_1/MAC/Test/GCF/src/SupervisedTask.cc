#include "SupervisedTask.h"
#include <GCF/GCF_Answer.h>
#include "TST_Protocol.ph"

SupervisedTask::SupervisedTask(GCFTask& a, string taskName) : 
  GCFTask((State)&SupervisedTask::initial, taskName),
  _application(a),
  _proxy(*this),
  _answer(*this)
{
  // register the protocol for debugging purposes
  registerProtocol(F_PML_PROTOCOL, F_PML_PROTOCOL_signalnames);
      // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames);  
}

SupervisedTask::~SupervisedTask()
{
}

int SupervisedTask::initial(GCFEvent& e, GCFPortInterface& p)
{
  return _application.dispatch(e, p);
}

void SupervisedTask::propSubscribed(const string& propName)
{
  GCFPropAnswerEvent  e(F_SUBSCRIBED);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void SupervisedTask::propUnsubscribed(const string& propName)
{
  GCFPropAnswerEvent  e(F_UNSUBSCRIBED);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void SupervisedTask::propValueChanged(const string& propName, const GCFPValue& value)
{
  GCFPropValueEvent e(F_VCHANGEMSG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}

void SupervisedTask::valueGet(const string& propName, const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}
