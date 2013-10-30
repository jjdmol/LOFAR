#include <lofar_config.h>

#include "Application1.h"
#include "Defines.h"
#include <RTC/RTDefines.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_MyProperty.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <csignal>
#include "TST_Protocol.ph"
#include <Suite/suite.h>

using std::cerr;
using std::endl;

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;   
using namespace TM;
using namespace PAL;
  namespace Test
  {
static string sTaskName = "TA1";

const TPropertyInfo propertyAHJP00Def(REMOTESYS1"A_H.J.P00", LPT_DOUBLE);

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  Test("TestApplication1"),
  _supTask1(*this, "ST1"),
  _supTask2(*this, "ST2"),
  _counter(0),
  _curRemoteTestNr(0),
  _propertySetA1("A_B",         "TTypeA", PS_CAT_TEMPORARY, &_supTask1.getAnswerObj()),
  _propertySetE1("A_L",         "TTypeE", PS_CAT_PERM_AUTOLOAD, &_supTask1.getAnswerObj()),
  _propertySetXX("A_X",         "TTypeE", PS_CAT_PERMANENT, &_supTask1.getAnswerObj()),
  _propertySetB1("A_C",         "TTypeB", PS_CAT_TEMPORARY, &_supTask1.getAnswerObj()),
  _propertySetB2("A_D",         "TTypeB", PS_CAT_TEMPORARY, &_supTask2.getAnswerObj()),
  _propertySetB3("A_E",         "TTypeB", PS_CAT_TEMPORARY, &_supTask2.getAnswerObj()),
  _ePropertySetAC("A_C",        "TTypeB", &_supTask2.getAnswerObj()),   
  _ePropertySetAD("A_D",        "TTypeB", &_supTask2.getAnswerObj()),   
  _ePropertySetAH(REMOTESYS1"A_H",   "TTypeD", &_supTask2.getAnswerObj()),   
  _ePropertySetAL("A_L",        "TTypeE", &_supTask2.getAnswerObj()),   
  _ePropertySetBRD1("B_A_BRD1", "TTypeF", &_supTask1.getAnswerObj()),
  _ePropertySetBRD2("B_A_BRD2", "TTypeG", &_supTask1.getAnswerObj())   
{
    // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames);  

  _propertySetA1.initProperties(propertiesSA1);
  _propertySetE1.initProperties(propertiesSE1);
  _propertySetB1.initProperties(propertiesSB1);
  _propertySetB2.initProperties(propertiesSB2);
  _propertySetB3.initProperties(propertiesSB3);
}

Application::~Application()
{
  // the following values do not end up in the PVSS database
  TESTC(_propertySetA1.setValue("F.P4", "99") == GCF_NO_ERROR);
  TESTC(_propertySetE1.setValue("P5", "99") == GCF_NO_ERROR);
  TESTC(_propertySetXX.setValue("P5", "99") == GCF_NO_ERROR);
  TESTC(_propertySetB1.setValue("P1", "99") == GCF_NO_ERROR);
  TESTC(_propertySetB2.setValue("P1", "99") == GCF_NO_ERROR);
  TESTC(_propertySetB3.setValue("P1", "99") == GCF_NO_ERROR);
  
}

void Application::abort()
{
  FINISH;
}

GCFEvent::TResult Application::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      system("killall RTPing");
      system("killall RTEcho");
      NEXT_TEST(1_1, "Port connection, one client, one server");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test1_1(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _supTask1.getPort().init(_supTask1, "client", GCFPortInterface::SAP, TST_PROTOCOL);
      TESTC(_supTask1.getPort().open());
      break;

    case F_TIMER:
      TESTC(_supTask1.getPort().open());
      break;

    case F_CONNECTED:
    {
      TSTTestreqEvent r;
      r.testnr = 101;
      TESTC(_supTask1.getPort().send(r) == SIZEOF_EVENT(r));
      break;
    }
    case F_DISCONNECTED:
      if (&p == &_supTask1.getPort())
        _supTask1.getPort().setTimer(1.0); // try again after 1 second
      break;

    case TST_TESTRESP:
    {
      NEXT_TEST(1_2, "Port connection, multiple client, one server");
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test1_2(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool closing = true;
  
  switch (e.signal)
  {
    case F_ENTRY:
      closing = true;
      break;

    case F_TIMER:
      if (&p == &_supTask1.getPort())
        TESTC(_supTask1.getPort().open());
      if (&p == &_supTask2.getPort())
        TESTC(_supTask2.getPort().open());
      break;

    case F_CONNECTED:
    {
      if (_supTask1.getPort().isConnected())
      {
        if (_supTask2.getPort().isConnected())
        {
          TSTTestreqEvent r;
          r.testnr = 102;
          TESTC(_supTask1.getPort().send(r) == SIZEOF_EVENT(r));
        }
        else
          TESTC(_supTask2.getPort().open());
      }        
      break;
    }

    case F_DISCONNECTED:
      if (closing)
      {
        _supTask1.getPort().close();
      }
      else
      {
        if (&p == &_supTask1.getPort())
          _supTask1.getPort().setTimer(1.0); // try again after 1 second
        if (&p == &_supTask2.getPort())
          _supTask2.getPort().setTimer(1.0); // try again after 1 second
      }
      break;
    
    case F_CLOSED:
      _supTask1.getPort().init(_supTask1, "mclient", GCFPortInterface::SAP, TST_PROTOCOL);
      _supTask2.getPort().init(_supTask2, "mclient", GCFPortInterface::SAP, TST_PROTOCOL);   
      TESTC(_supTask1.getPort().open());
      closing = false;
      break;
    
    case TST_TESTRESP:
      if (&p == &_supTask1.getPort())
      {
        TSTTestreqEvent r;
        r.testnr = 102;
        TESTC(_supTask2.getPort().send(r) == SIZEOF_EVENT(r));
      }
      else
      {
        TSTTestreadyEvent r;
        r.testnr = 102;
        TESTC(_supTask1.getPort().send(r) == SIZEOF_EVENT(r));
      }
      break;

    case TST_TESTREADY:
      NEXT_TEST(2_1, "Enable property set");
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test2_1(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_propertySetA1.enable() == GCF_NO_ERROR);
      _supTask1.getPort().setTimer(10.0);
      break;

    case F_MYPS_ENABLED:
    {
      _supTask1.getPort().cancelAllTimers();

      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetA1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
        TESTC(_propertySetA1.setValue("F.P4", "21") == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetA1.isEnabled());
      TESTC_DESCR(GCFPVSSInfo::propExists("A_B__enabled"), "may fail");
      NEXT_TEST(2_2, "Disable property set");
      break;
    }  
    case F_TIMER:
      FAIL_AND_ABORT("Gets not response on enable request in time.");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test2_2(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_propertySetA1.disable() == GCF_NO_ERROR);
      _supTask1.getPort().setTimer(10.0);
      break;

    case F_MYPS_DISABLED:
    {
      _supTask1.getPort().cancelAllTimers();

      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetA1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(!_propertySetA1.isEnabled());
      TESTC_DESCR(!GCFPVSSInfo::propExists("A_B__enabled"), "may fail");
      NEXT_TEST(2_3, "Recognize that a property set is alrady in use");
      break;
    }  
    case F_TIMER:
      FAIL_AND_ABORT("Gets not response on disable request in time.");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test2_3(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_propertySetB1.enable() == GCF_NO_ERROR);
      TESTC_DESCR_ABORT_ON_FAIL((_propertySetB1.enable() != GCF_NO_ERROR), "Should not be possible");
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
        TESTC(_propertySetB1.setValue("P1", "23") == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetB1.isEnabled());
      TESTC_DESCR(GCFPVSSInfo::propExists("A_C__enabled"), "may fail");
      if (TESTC(_propertySetB1.enable() != GCF_NO_ERROR))
      {
        TSTTestreadyEvent r;
        r.testnr = 203;
        if (_supTask1.getPort().isConnected())
          _supTask1.getPort().send(r);        
        NEXT_TEST(2_5, "Enable/Disable mulitple temp. prop. sets");
      }
      else
      {
        ABORT_TESTS;
      }
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test2_5(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _counter = 3;
      TESTC_ABORT_ON_FAIL(_propertySetA1.enable() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_propertySetB2.enable() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_propertySetB3.enable() == GCF_NO_ERROR);
      _supTask1.getPort().setTimer(10.0);
      break;

    case F_MYPS_ENABLED:
    {
      _counter--;
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
        if (strcmp(pResponse->pScope, "A_B") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetA1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(_propertySetA1.isEnabled());
          TESTC_DESCR(GCFPVSSInfo::propExists("A_B__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_D") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB2.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(_propertySetB2.isEnabled());
          TESTC_DESCR(GCFPVSSInfo::propExists("A_D__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_E") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB3.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(_propertySetB3.isEnabled());
          TESTC(_propertySetB3.setValue("P1", "25") == GCF_NO_ERROR);
          TESTC_DESCR(GCFPVSSInfo::propExists("A_E__enabled"), "may fail");
        }
      }
      
      if (_counter == 0)
      {
        _counter = 4;
        _supTask1.getPort().cancelAllTimers();        
        TESTC_ABORT_ON_FAIL(_propertySetA1.disable() == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_propertySetB1.disable() == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_propertySetB2.disable() == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_propertySetB3.disable() == GCF_NO_ERROR);
        _supTask1.getPort().setTimer(10.0);
      }
      break;
    }  
    case F_MYPS_DISABLED:
    {
      _counter--;
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
        if (strcmp(pResponse->pScope, "A_B") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetA1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(!_propertySetA1.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_B__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_C") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(!_propertySetB1.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_C__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_D") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB2.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(!_propertySetB2.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_D__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_E") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB3.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(!_propertySetB3.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_E__enabled"), "may fail");
        }
      }      
      if (_counter == 0)
      {
        _supTask1.getPort().cancelAllTimers();        
        TSTTestreadyEvent r;
        r.testnr = 205;
        _supTask1.getPort().send(r);
        NEXT_TEST(3_4, "Enable permanent prop. set, which does not exists");
      }      
      break;
    }
    case F_TIMER:
      FAIL_AND_ABORT("Gets not response on disable request in time.");
      break;
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test3_4(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 303) break;           
      TESTC(!GCFPVSSInfo::propExists("A_X"));
      TESTC_ABORT_ON_FAIL(_propertySetXX.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetXX.getFullScope().c_str()) == 0);
        TESTC(pResponse->result != GCF_NO_ERROR);
        TESTC(_propertySetXX.setValue("P5", "34") == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(!_propertySetXX.isEnabled());
      TESTC(!GCFPVSSInfo::propExists("A_X"));
      NEXT_TEST(4_1, "Load property set (prepare for TestApp2)");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_1(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_propertySetB1.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetB1.isEnabled());
      TESTC_DESCR(GCFPVSSInfo::propExists("A_C__enabled"), "may fail");
      TSTTestreadyEvent r;
      r.testnr = 401;
      _supTask1.getPort().send(r);
      NEXT_TEST(4_3, "Check property set usecount");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static bool delayedUnLoad = false;
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (!delayedUnLoad) break; 
                
      delayedUnLoad = false; 
      switch (_curRemoteTestNr)
      {
        case 402:
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR);
          break;
        case 403:
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.unload() == GCF_NO_ERROR);
          break;
      }
      break;
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 402) 
      {
        delayedUnLoad = true;
        break;
      }
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR);
      break;

    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.isLoaded());
      TSTTestreadyEvent r;
      r.testnr = 402;
      _supTask1.getPort().send(r);        
      if (_curRemoteTestNr != 403) 
      {
        delayedUnLoad = true;
        break;
      }
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.unload() == GCF_NO_ERROR);
      break;
    }

    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(!_ePropertySetAC.isLoaded());

      TSTTestreadyEvent r;
      r.testnr = 403;
      _supTask1.getPort().send(r);        

      NEXT_TEST(5_1, "Configure prop. set");
      break;
    }


    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test5_1(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 404) break;           
      TESTC_ABORT_ON_FAIL(_propertySetE1.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetE1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
                  
      TESTC_ABORT_ON_FAIL(_propertySetE1.isEnabled());
      TESTC_ABORT_ON_FAIL(_ePropertySetAL.load() == GCF_NO_ERROR);
      break;
    }    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetE1.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }      
      TESTC_ABORT_ON_FAIL(_ePropertySetAL.isLoaded());
      _propertySetE1.configure("e1");
      break;
    }    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pResponse = (GCFConfAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(pResponse);
      TESTC(strcmp(pResponse->pScope, _propertySetE1.getFullScope().c_str()) == 0);
      TESTC(strcmp(pResponse->pApcName, "e1") == 0);
      TESTC_ABORT_ON_FAIL(pResponse->result == GCF_NO_ERROR);
      TESTC(_propertySetE1.setValue("P5", "51") == GCF_NO_ERROR);

      TSTTestreadyEvent r;
      r.testnr = 501;
      _supTask1.getPort().send(r);        

      NEXT_TEST(6_1, "Read/Write properties in RAM");
      break;
    }    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_1(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
    {
      if (_curRemoteTestNr != 503) break;           
      GCFPValue* pValue;
  
      // A_D.P1 section start    
      pValue = _propertySetB2.getValue("P1"); // its a clone
      assert(pValue);
      TESTC(((GCFPVInteger*)pValue)->getValue() == 12);
      delete pValue;

      GCFPVInteger iv(13);
      TESTC(_propertySetB2.setValue("P1", iv) == GCF_NO_ERROR);

      pValue = _propertySetB2.getValue("P1");  // its a clone
      assert(pValue);
      TESTC(((GCFPVInteger*)pValue)->getValue() == 13);
      delete pValue;
      
      // A_D.P2 section start
      GCFMyProperty* pProperty(0);
      pProperty = static_cast<GCFMyProperty*>(_propertySetB2.getProperty("A_D.P2"));
      assert(pProperty);      

      pValue = pProperty->getValue();  // its a clone
      assert(pValue);
      TESTC(((GCFPVChar*)pValue)->getValue() == 26);
      delete pValue;

      GCFPVChar cv(27);
      TESTC(_propertySetB2.setValue("P2", cv) == GCF_NO_ERROR);

      pValue = _propertySetB2.getValue("P2");  // its a clone
      assert(pValue);
      TESTC(((GCFPVChar*)pValue)->getValue() == 27);
      delete pValue;

      // A_D.P3 section start
      pValue = _propertySetB2.getValue("A_D.P3"); // its a clone
      assert(pValue);
      TESTC(((GCFPVDouble*)pValue)->getValue() == 34.3);
      delete pValue;

      GCFPVDouble dv(35.4);
      TESTC(_propertySetB2.setValue("P3", dv) == GCF_NO_ERROR);

      pValue = _propertySetB2.getValue("P3");  // its a clone
      assert(pValue);
      TESTC(((GCFPVDouble*)pValue)->getValue() == 35.4);
      delete pValue;
      
      NEXT_TEST(6_2, "Read/write properties in Database");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_2(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned int receivedChanges = 0;
  switch (e.signal)
  {
    case F_ENTRY:
    {
      TESTC_ABORT_ON_FAIL(_propertySetB2.enable() == GCF_NO_ERROR);
      break;
    }
    case F_MYPS_ENABLED:
    {
      TESTC_ABORT_ON_FAIL(_ePropertySetAD.load() == GCF_NO_ERROR);
      break;
    }
    case F_EXTPS_LOADED:
    {
      TESTC_ABORT_ON_FAIL(_ePropertySetAD.isLoaded());
      if (_supTask2.getProxy().requestPropValue("A_D.P1") == GCF_NO_ERROR)
      {
        TESTC(_supTask2.getProxy().requestPropValue("A_D.P2") == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_supTask2.getProxy().requestPropValue("A_D.P3") == GCF_NO_ERROR);
      }
      else
      {
        _supTask1.getPort().setTimer(0.0);
      }
      _counter = 0;
      break;
    }
    case F_VGETRESP:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);

      switch (_counter)
      {
        case 0:  
        case 1:
        case 2:
          if (strstr(pResponse->pPropName, "A_D.P1") > 0)
          {
            TESTC(pResponse->pValue->getType() == LPT_INTEGER);
            TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == 13);            
            GCFPVInteger iv(22);
            TESTC_ABORT_ON_FAIL(_supTask2.getProxy().setPropValue("A_D.P1", iv) == GCF_NO_ERROR);
            TESTC_ABORT_ON_FAIL(_supTask2.getProxy().requestPropValue("A_D.P1") == GCF_NO_ERROR);
          }
          else if (strstr(pResponse->pPropName, "A_D.P2") > 0)          
          {
            TESTC(pResponse->pValue->getType() == LPT_CHAR);
            TESTC(((GCFPVChar*)pResponse->pValue)->getValue() == 27);            
            GCFPVChar cv(28);
            TESTC_ABORT_ON_FAIL(_ePropertySetAD.setValue("A_D.P2", cv) == GCF_NO_ERROR);
            TESTC_ABORT_ON_FAIL(_ePropertySetAD.requestValue("P2") == GCF_NO_ERROR);
          }
          else if (strstr(pResponse->pPropName, "A_D.P3") > 0)          
          {
            TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
            TESTC(((GCFPVDouble*)pResponse->pValue)->getValue() == 35.4);            
            GCFPVDouble dv(22.0);
            TESTC_ABORT_ON_FAIL(_ePropertySetAD["P3"].setValue(dv) == GCF_NO_ERROR);
            TESTC_ABORT_ON_FAIL(_ePropertySetAD["P3"].requestValue() == GCF_NO_ERROR);           
          }
          else
          {
            FAIL_AND_ABORT("Wrong property name was received!");
          }
          _counter++;
          break;
        case 3:  
        case 4:  
        case 5:  
          if (strstr(pResponse->pPropName, "A_D.P1") > 0)
          {
            TESTC(pResponse->pValue->getType() == LPT_INTEGER);
            TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == 22);            
            _counter++;
          }
          else if (strstr(pResponse->pPropName, "A_D.P2") > 0)          
          {
            TESTC(pResponse->pValue->getType() == LPT_CHAR);
            TESTC(((GCFPVChar*)pResponse->pValue)->getValue() == 28);            
            _counter++;
          }
          else if (strstr(pResponse->pPropName, "A_D.P3") > 0)          
          {
            TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
            TESTC(((GCFPVDouble*)pResponse->pValue)->getValue() == 22.0);            
            _counter++;
          }
          if (_counter == 6 && receivedChanges == 3)
          {
            NEXT_TEST(6_3, "Subscribe to property in DB, receive changes");
          }
          break;
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      TESTC(pResponse->internal);
      if (strstr(pResponse->pPropName, "A_D.P1") > 0)
      {
        TESTC(pResponse->pValue->getType() == LPT_INTEGER);
        TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == 22);            
        receivedChanges++;
      }
      else if (strstr(pResponse->pPropName, "A_D.P2") > 0)          
      {
        TESTC(pResponse->pValue->getType() == LPT_CHAR);
        TESTC(((GCFPVChar*)pResponse->pValue)->getValue() == 28);            
        receivedChanges++;
      }
      else if (strstr(pResponse->pPropName, "A_D.P3") > 0)          
      {
        TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
        TESTC(((GCFPVDouble*)pResponse->pValue)->getValue() == 22.0);            
        receivedChanges++;
      }
      if (_counter == 6 && receivedChanges == 3)
      {
        NEXT_TEST(6_3, "Subscribe to property in DB, receive changes");
      }
      break;
    }  

    case F_TIMER:
      if (_supTask2.getProxy().requestPropValue("A_D.P1") == GCF_NO_ERROR)
      {
        TESTC(_supTask2.getProxy().requestPropValue("A_D.P2") == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_supTask2.getProxy().requestPropValue("A_D.P3") == GCF_NO_ERROR);
        _counter = 0;
      }
      else
      {
        _counter++;
        if (_counter < 20) _supTask1.getPort().setTimer(0.0);
        else FAIL_AND_ABORT("DP A_D not available in time");
      }
      break;
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      TESTC_ABORT_ON_FAIL(_propertySetB3.enable() == GCF_NO_ERROR);
      break;
    }
    case F_MYPS_ENABLED:
    {
      TSTTestreadyEvent r;
      r.testnr = 602;
      _supTask1.getPort().send(r);        
      break;
    }
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      GCFPVInteger iv(22);
      TESTC_ABORT_ON_FAIL(_propertySetB3["P1"].setValue(iv) == GCF_NO_ERROR);
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      TESTC(pResponse->internal);
      TESTC(strstr(pResponse->pPropName, "A_E.P1") > 0);
      TESTC(pResponse->pValue->getType() == LPT_INTEGER);
      TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == 22);            
      NEXT_TEST(6_4, "Subscribe to multiple properties, test if changes are received with valid information");
      break;
    }  
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_4(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.load() == GCF_NO_ERROR);
      break;
    }
    case F_EXTPS_LOADED:
    {
      _counter = 0;
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.isLoaded());
      char propName[] = "J.P00";
      for (unsigned int i = 0; i <= 9; i++)
      {
        propName[3] = i + '0';
        for (unsigned int j = 0; j <= 9; j++)
        {
          propName[4] = j + '0';
          if (TESTC_DESCR(_ePropertySetAH.subscribeProp(propName) == GCF_NO_ERROR, propName))
          {
            _counter++;
          }
        }
      }        
      break;
    }
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = (GCFPropAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(strstr(pResponse->pPropName, "A_H.J.P") > 0);
      _counter--;
      if (_counter == 0)
      {
        TSTTestreadyEvent r;
        r.testnr = 603;
        _supTask1.getPort().send(r);
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);

      TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
      TESTC(!pResponse->internal);
      TESTC(strstr(pResponse->pPropName, "A_H.J.P") > 0);
      unsigned int expectedVal = atoi(pResponse->pPropName + GCFPVSSInfo::getLocalSystemName().length() + 8);
      double dv = ((GCFPVDouble*)pResponse->pValue)->getValue();
      unsigned int receivedVal = (unsigned int) floor((dv * 100.0) + 0.5);
      TESTC(receivedVal == expectedVal);
      _counter++;
      cerr << "Received prop. val: " << receivedVal << " Expected prop. val: " << expectedVal << endl;
      LOG_KEYVALUE(pResponse->pPropName, *pResponse->pValue, KVL_ORIGIN_MAC);
      if (_counter == 100 )
      {
        char propName[] = "J.P00";
        for (unsigned int i = 0; i <= 9; i++)
        {
          propName[3] = i + '0';
          for (unsigned int j = 0; j <= 9; j++)
          {
            propName[4] = j + '0';
            TESTC_DESCR(_ePropertySetAH.unsubscribeProp(propName) == GCF_NO_ERROR, propName);
          }
        }        
        TSTTestreadyEvent r;
        r.testnr = 604;
        _supTask1.getPort().send(r);
        NEXT_TEST(6_5, "Property change is distributed to multiple subscribers");
      }
      break;
    }    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_5(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static GCFExtProperty propertyAHJP00(propertyAHJP00Def); 
  propertyAHJP00.setAnswer(&_supTask1.getAnswerObj());

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 604) break;
      _counter = 0;
      TESTC_ABORT_ON_FAIL(propertyAHJP00.subscribe() == GCF_NO_ERROR);      
      break;
  
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = (GCFPropAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(strstr(pResponse->pPropName, "A_H.J.P00") > 0);
      if (&p == &_supTask1.getPort())
      {
        TESTC_ABORT_ON_FAIL(propertyAHJP00.isSubscribed());
        TESTC_ABORT_ON_FAIL(_ePropertySetAH.subscribeProp("J.P00") == GCF_NO_ERROR);
      }
      else if (&p == &_supTask2.getPort())
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetAH.isPropSubscribed("J.P00"));
        TSTTestreadyEvent r;
        r.testnr = 605;
        _supTask1.getPort().send(r);
      }
      break;
    }

    case F_VCHANGEMSG:
    {
       GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);

      TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
      TESTC(!pResponse->internal);
      TESTC(strstr(pResponse->pPropName, "A_H.J.P") > 0);
      TESTC(((GCFPVDouble*)pResponse->pValue)->getValue() == 3.12);
      _counter++;
      if (_counter == 2 )
      {
//        NEXT_TEST(6_6, "Send and receive properties between tasks, test stability and performance");
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

GCFEvent::TResult Application::test6_6(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 606) break;
      
      TESTC_ABORT_ON_FAIL(_supTask1.getProxy().subscribeProp("A_C.P1") == GCF_NO_ERROR);
      break;
  
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = (GCFPropAnswerEvent*)(&e);
      assert(pResponse);
      _counter = 0;
      TESTC_ABORT_ON_FAIL(strstr(pResponse->pPropName, "A_C.P1") > 0);
      cerr << "Send nr " << _counter << " to "REMOTESYS1"A_K.P1" << endl;
      GCFPVInteger iv(_counter);
      TESTC_ABORT_ON_FAIL(_supTask1.getProxy().setPropValue(REMOTESYS1"A_K.P1", iv) == GCF_NO_ERROR);
      break;
    }

    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      assert(pResponse->pValue->getType() == LPT_INTEGER);
      assert(strstr(pResponse->pPropName, "A_C.P1") > 0);
      TESTC((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == _counter);
      cerr << "Received nr " << (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() << " from A_C.P1 (" << _counter << ")" << endl;
      _counter++;
      if (_counter == 1000)
      {
         NEXT_TEST(7_1, "access board properties");
      }
      else
      {
        cerr << "Send nr " << _counter << " to "REMOTESYS1"A_K.P1" << endl;
        GCFPVInteger iv(_counter);
        TESTC_ABORT_ON_FAIL(_supTask1.getProxy().setPropValue(REMOTESYS1"A_K.P1", iv) == GCF_NO_ERROR);
      }        
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult Application::test7_1(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(system("./RTPing -brdnr 1 2&> RTPing1.out &") != -1);
      TESTC_ABORT_ON_FAIL(system("./RTPing -brdnr 2 2&> RTPing2.out &") != -1);
      _supTask1.getPort().setTimer(2.0);      
      break;

    case F_TIMER:
      TESTC_ABORT_ON_FAIL(_ePropertySetBRD1.load() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_ePropertySetBRD2.load() == GCF_NO_ERROR);
      break;
    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (pResponse->result != GCF_NO_ERROR)
      {
        if (strcmp(pResponse->pScope, "LCU1:B_A_BRD1") == 0)
        {
          TESTC_ABORT_ON_FAIL(_ePropertySetBRD1.load() == GCF_NO_ERROR);
        }
        else
        {
          TESTC_ABORT_ON_FAIL(_ePropertySetBRD2.load() == GCF_NO_ERROR);
        }
      }
      else if (_ePropertySetBRD1.isLoaded() &&  _ePropertySetBRD2.isLoaded())
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetBRD1.subscribeProp("sn") == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_ePropertySetBRD2.subscribeProp("sn000") == GCF_NO_ERROR);        
      }
      break;
    } 
    case F_SUBSCRIBED:
      if (_ePropertySetBRD1.isPropSubscribed("sn") && _ePropertySetBRD2.isPropSubscribed("sn000"))
      {
        GCFPVInteger iv(10);
        _ePropertySetBRD1["max"].setValue(iv);
        iv.setValue(200);
        _ePropertySetBRD2["max"].setValue(iv);
      }
      break;
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      assert(pResponse->pValue->getType() == LPT_INTEGER);
      if (strstr(pResponse->pPropName, "B_A_BRD1.sn") > 0)
      {
        TESTC((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == 10);
      }
      else if (strstr(pResponse->pPropName, "B_A_BRD2.sn000") > 0)
      {
        TESTC((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == 200);
        NEXT_TEST(7_2, "follow the sequence numbers of both boards");
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult Application::test7_2(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(system("./RTEcho -brdnr 1 2&> RTEcho1.out &") != -1);
      TESTC_ABORT_ON_FAIL(system("./RTEcho -brdnr 2 2&> RTEcho2.out &") != -1);      
      _supTask1.getPort().setTimer(40.0);
      _counter = 0;
      break;
    case F_TIMER:
    {
      GCFTimerEvent* pTIM = (GCFTimerEvent*)(&e);
      srand(pTIM->sec * 1000000 + pTIM->usec);
      int maxSeqNr = 20 + (int) (80.0 * rand() / (RAND_MAX + 1.0));
      GCFPVInteger maxSeqNrV(maxSeqNr);
      _supTask1.getProxy().setPropValue("B_A_BRD1.max", maxSeqNrV);
      if (_counter >= 3)
      {
        int maxSeqNr = 20 + (int) (80.0 * rand() / (RAND_MAX + 1.0));
        maxSeqNrV.setValue(maxSeqNr);
        _supTask1.getProxy().setPropValue("B_A_BRD2.max", maxSeqNrV);
      }
      if (_counter < 7)
      {
        if (_counter == 3)
        {
          //apc2.load(false);
        }
        _counter++;
        _supTask1.getPort().setTimer(40.0);
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
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
    {
      system("killall RTPing");
      system("killall RTEcho");
      //if (_curRemoteTestNr != 999) break;           
      TSTTestreadyEvent r;
      r.testnr = 999;
      if (_supTask1.getPort().isConnected())
      {
        _supTask1.getPort().send(r);
      }

      // the following values end up in the PVSS database
      TESTC(_propertySetA1.setValue("F.P4", "97") == GCF_NO_ERROR);
      TESTC(_propertySetE1.setValue("P5", "97") == GCF_NO_ERROR);
      TESTC(_propertySetXX.setValue("P5", "97") == GCF_NO_ERROR);
      TESTC(_propertySetB1.setValue("P1", "97") == GCF_NO_ERROR);
      TESTC(_propertySetB2.setValue("P1", "97") == GCF_NO_ERROR);
      TESTC(_propertySetB3.setValue("P1", "97") == GCF_NO_ERROR);
      
      _supTask1.getPort().setTimer(1.0);
      break;
    }
      
    case F_TIMER:
      // the following values do not end up in the PVSS database
      TESTC(_propertySetA1.setValue("F.P4", "98") == GCF_NO_ERROR);
      TESTC(_propertySetE1.setValue("P5", "98") == GCF_NO_ERROR);
      TESTC(_propertySetXX.setValue("P5", "98") == GCF_NO_ERROR);
      TESTC(_propertySetB1.setValue("P1", "98") == GCF_NO_ERROR);
      TESTC(_propertySetB2.setValue("P1", "98") == GCF_NO_ERROR);
      TESTC(_propertySetB3.setValue("P1", "98") == GCF_NO_ERROR);
      sleep(5);
      GCFTask::stop();
      break;
    
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
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR

static LOFAR::GCF::Test::Application* pApp = 0;

void sigintHandler(/*@unused@*/int signum)
{
  LOG_INFO(LOFAR::formatString("SIGINT signal detected (%d)",signum));
  if(0 != pApp)
    pApp->abort();
}

int main(int argc, char* argv[])
{
  LOFAR::GCF::TM::GCFTask::init(argc, argv);

  LOG_INFO("MACProcessScope: GCF.TEST.MAC.App1");
    
  Suite s("GCF Test", &cerr);
  pApp = new LOFAR::GCF::Test::Application;
  s.addTest(pApp);
  
  /* install ctrl-c signal handler */
  (void)signal(SIGINT,sigintHandler);
  
  s.run();
  s.report();
  s.free();
  
  return 0;  
}
