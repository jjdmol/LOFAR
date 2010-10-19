#include "Application3.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <math.h>
#include <stdio.h>
#include "TST_Protocol.ph"
#include <Suite/suite.h>

using std::cerr;
using std::endl;

static string sTaskName = "TA";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  Test("TestApplication3"),
  _supTask(*this, "ST"),
  _counter(0),
  _ePropertySetC("C",      "TTypeF", &_supTask.getAnswerObj()) 
{
}

Application::~Application()
{
}

GCFEvent::TResult Application::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      system("killall CEPPing");
      sleep(2);
      _supTask.getPort().init(_supTask, "server", GCFPortInterface::SPP, 0); // dumy
      NEXT_TEST(7_3, "CEP-PMLlight (Fase 1)");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test7_3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(system("./CEPPing 2&> CEPPing.out &") != -1);
      TESTC_ABORT_ON_FAIL(system("./CEPEcho 2&> CEPEcho.out &") != -1);
      _supTask.getPort().setTimer(2.0);      
      break;

    case F_TIMER:
      TESTC_ABORT_ON_FAIL(_ePropertySetC.load() == GCF_NO_ERROR);
      break;
    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (pResponse->result != GCF_NO_ERROR)
      {
        if (strcmp(pResponse->pScope, "LCU1:C") == 0)
        {
          TESTC_ABORT_ON_FAIL(_ePropertySetC.load() == GCF_NO_ERROR);
        }
      }
      else if (_ePropertySetC.isLoaded())
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetC.subscribeProp("sn") == GCF_NO_ERROR);
      }
      break;
    } 
    case F_SUBSCRIBED:
      _ePropertySetC.isPropSubscribed("sn");
      break;
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      assert(pResponse->pValue->getType() == LPT_INTEGER);
      if (strstr(pResponse->pPropName, "LCU1:C.sn") > 0)
      {
        _seqNr = (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue();
        NEXT_TEST(7_4, "CEP-PMLlight (Fase 2)");
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult Application::test7_4(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
      _supTask.getPort().setTimer(20.0);
      _counter = 0;
      break;

    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      assert(pResponse->pValue->getType() == LPT_INTEGER);
      if (strstr(pResponse->pPropName, "LCU1:C.sn") > 0)
      {
        _seqNr = (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue();
        TESTC(_seqNr < 200);
      }
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (pResponse->result != GCF_NO_ERROR)
      {
        if (strcmp(pResponse->pScope, "LCU1:C") == 0)
        {
          _ePropertySetC.load(); // retry          
        }
      }
      else
      {
        TESTC(_ePropertySetC.subscribeProp("sn") == GCF_NO_ERROR);
      }
      break;
    } 
    case F_SUBSCRIBED:    
      TESTC(_ePropertySetC.isPropSubscribed("sn"));
      break;
    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(pResponse->result == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(strcmp(pResponse->pScope, "LCU1:C") == 0);
      _ePropertySetC.load(); // reload      
      break;
    }
    case F_SERVER_GONE:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(pResponse->result == GCF_NO_ERROR);
      TESTC(_seqNr > 190);
      TESTC_ABORT_ON_FAIL(strcmp(pResponse->pScope, "LCU1:C") == 0);
      _ePropertySetC.load(); // reload
      break;
    }
    case F_TIMER:
    {
      _counter++;
      if (_counter < 20)
      {
        _ePropertySetC.unload();  
        _supTask.getPort().setTimer(20.0);      
      }
      else
      {
        FINISH;
      }      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::finished(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      system("killall CEPPing");
      GCFTask::stop();    
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void Application::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);
  
  Suite s("GCF Test", &cerr);
  s.addTest(new Application);
  s.run();
  s.report();
  s.free();
  
  return 0;  
}
