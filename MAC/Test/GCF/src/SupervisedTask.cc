#include "SupervisedTask.h"
#include "TST_Protocol.ph"

/**
 * F_PVSS_PROTOCOL signal names
 */
const char* F_PML_PROTOCOL_signalnames[] =
{
  "F_PML_PROTOCOL: invalid signal",
  "F_SUBCRIBED_SIG (IN)",
  "F_UNSUBCRIBED_SIG (IN)",
  "F_VCHANGEMSG_SIG (IN)",
  "F_VGETRESP_SIG (IN)",
  "F_APCLOADED_SIG (IN)",
  "F_APCUNLOADED_SIG (IN)",
  "F_APCRELOADED_SIG (IN)",
  "F_MYLOADED_SIG (IN)",
  "F_MYUNLOADED_SIG (IN)",
};

SupervisedTask::SupervisedTask(GCFTask& a, string taskName) : 
  GCFSupervisedTask((State)&SupervisedTask::initial, taskName),
  _application(a),
  _proxy(*this)
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
  GCFPMLAnswerEvent  e(F_SUBSCRIBED_SIG);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void SupervisedTask::propUnsubscribed(const string& propName)
{
  GCFPMLAnswerEvent  e(F_UNSUBSCRIBED_SIG);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void SupervisedTask::propValueChanged(const string& propName, const GCFPValue& value)
{
  GCFPMLValueEvent e(F_VCHANGEMSG_SIG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}

void SupervisedTask::valueChanged(const string& propName, const GCFPValue& value)
{
  GCFPMLValueEvent e(F_VCHANGEMSG_SIG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = true;
  _application.dispatch(e, _port);
}

void SupervisedTask::valueGet(const string& propName, const GCFPValue& value)
{
  GCFPMLValueEvent e(F_VGETRESP_SIG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}

void SupervisedTask::apcLoaded(const string& apcName, const string& scope, TGCFResult result)
{
  GCFPMLAPCAnswerEvent e(F_APCLOADED_SIG);
  e.pScope = scope.c_str();
  e.pApcName = apcName.c_str();
  e.result = result;
  _application.dispatch(e, _port);
}

void SupervisedTask::apcUnloaded(const string& apcName, const string& scope, TGCFResult result)
{
  GCFPMLAPCAnswerEvent e(F_APCUNLOADED_SIG);
  e.pScope = scope.c_str();
  e.pApcName = apcName.c_str();
  e.result = result;
  _application.dispatch(e, _port);
}

void SupervisedTask::apcReloaded(const string& apcName, const string& scope, TGCFResult result)
{
  GCFPMLAPCAnswerEvent e(F_APCRELOADED_SIG);
  e.pScope = scope.c_str();
  e.pApcName = apcName.c_str();
  e.result = result;
  _application.dispatch(e, _port);
}

void SupervisedTask::myPropertiesLoaded(const string& scope, TGCFResult result)
{
  GCFPMLMYPAnswerEvent e(F_MYLOADED_SIG);
  e.pScope = scope.c_str();
  e.result = result;
  _application.dispatch(e, _port);
}

void SupervisedTask::myPropertiesUnloaded(const string& scope, TGCFResult result)
{
  GCFPMLMYPAnswerEvent e(F_MYUNLOADED_SIG);
  e.pScope = scope.c_str();
  e.result = result;
  _application.dispatch(e, _port);
}

