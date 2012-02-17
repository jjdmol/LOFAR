#include "SupervisedTask.h"
#include <GCF/PAL/GCF_Answer.h>
#include "TST_Protocol.ph"

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;
using namespace TM;
using namespace PAL;
  namespace Test
  {
    
Task::Task(GCFTask& a, string taskName) : 
  GCFTask((State)&Task::initial, taskName),
  _application(a),
  _proxy(*this),
  _answer(*this)
{
  // register the protocol for debugging purposes
  registerProtocol(F_PML_PROTOCOL, F_PML_PROTOCOL_signalnames);
      // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames);  
}

Task::~Task()
{
}

int Task::initial(GCFEvent& e, GCFPortInterface& p)
{
  return _application.dispatch(e, p);
}

void Task::propSubscribed(const string& propName)
{
  GCFPropAnswerEvent  e(F_SUBSCRIBED);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void Task::propUnsubscribed(const string& propName)
{
  GCFPropAnswerEvent  e(F_UNSUBSCRIBED);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}

void Task::propValueChanged(const string& propName, const GCFPValue& value)
{
  GCFPropValueEvent e(F_VCHANGEMSG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}

void Task::propValueSet(const string& propName)
{
  GCFPropAnswerEvent  e(F_VSETRESP);
  e.pPropName = propName.c_str();
  _application.dispatch(e, _port);
}


void Task::valueGet(const string& propName, const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _application.dispatch(e, _port);
}
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
