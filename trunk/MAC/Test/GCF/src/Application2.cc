#include "Application2.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <math.h>
#include <stdio.h>
#include "TST_Protocol.ph"
#include <Suite/suite.h>

static string sTaskName = "TA2";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  Test("TestApplication2"),
  _supTask3(*this, "ST3"),
  _counter(0),
  _curRemoteTestNr(0),
  _pSTPort1(0),
  _pSTPort2(0),
  _propertySetC1("A_H_I", propertySetC1, &_supTask3.getAnswerObj()),
  _propertySetD1("A_H",   propertySetD1, &_supTask3.getAnswerObj()),  
  _propertySetB4("A_K",   propertySetB4, &_supTask3.getAnswerObj(), GCFMyPropertySet::USE_DB_DEFAULTS),
  _ePropertySetAC("A_C",  propertySetB1, &_supTask3.getAnswerObj()), 
  _ePropertySetAE("A_E",  propertySetB1, &_supTask3.getAnswerObj()), 
  _ePropertySetAH("A_H",  propertySetD1, &_supTask3.getAnswerObj()), 
  _ePropertySetAK("A_K",  propertySetB4, &_supTask3.getAnswerObj()) 
{
    // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames);  
}

GCFEvent::TResult Application::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      _supTask3.getPort().init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
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
      _supTask3.getPort().init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
      TESTC(1 == _supTask3.getPort().open());
      break;

    case F_TIMER:
      TESTC(1 == _supTask3.getPort().open());
      break;

    case F_CONNECTED:
      break;

    case F_DISCONNECTED:
      if (&p == &_supTask3.getPort())
        _supTask3.getPort().setTimer(1.0); // try again after 1 second
      break;

    case TST_TESTREQ:
    {
      TSTTestrespEvent r;
      r.testnr = 101;
      TESTC(_supTask3.getPort().send(r) == SIZEOF_EVENT(r));
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
      _supTask3.getPort().close();
      break;

    case F_TIMER:
      TESTC(1 == _port.open());
      break;

    case F_CONNECTED:
      _counter = 0;
      break;
      
    case F_ACCEPT_REQ:
      if (_pSTPort1 == 0)
      {
        _pSTPort1 = new GCFTCPPort();
        _pSTPort1->init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
        TESTC(0 == _port.accept(*_pSTPort1));
      }
      else
      {
        _pSTPort2 = new GCFTCPPort();
        _pSTPort2->init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
        TESTC(0 == _port.accept(*_pSTPort2));
      }
      break;
      
    case F_DISCONNECTED:
      if (closing)
      {
        _port.init(_supTask3, "server", GCFPortInterface::MSPP, TST_PROTOCOL);
        TESTC(1 == _port.open());
        closing = false;
      }
      else
      {
        if (&p == &_port)
          _port.setTimer(1.0); // try again after 1 second
      }      
      break;

    case F_CLOSED:
      _port.init(_supTask3, "server", GCFPortInterface::MSPP, TST_PROTOCOL);
      TESTC(1 == _port.open());
      closing = false;
      break;

    case TST_TESTREQ:
    {
      TSTTestrespEvent r;
      r.testnr = 102;
      _counter++;
      if (_counter == 1)
      {
        if (TESTC(_pSTPort1))
          TESTC(_pSTPort1->send(r) == SIZEOF_EVENT(r));
        else
          ABORT_TESTS;
      }
      else if (_counter == 2)
      {
        if (TESTC(_pSTPort2))
          TESTC(_pSTPort2->send(r) == SIZEOF_EVENT(r));
        else
          ABORT_TESTS;
      }
      break;
    }  
    case TST_TESTREADY:
    {
      TSTTestreadyEvent r(e);
      assert(_pSTPort1);
      if (TESTC(_pSTPort1))
      {
        TESTC(_pSTPort1->send(r) == SIZEOF_EVENT(r));
        NEXT_TEST(2_5, "Enable/Disable mulitple temp. prop. sets");
      }  
      else
        ABORT_TESTS;
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
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through        
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 203) break;
      _counter = 2;
      TESTC_ABORT_ON_FAIL(_propertySetC1.enable() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_propertySetD1.enable() == GCF_NO_ERROR);
      _supTask3.getPort().setTimer(10.0);
      break;

    case F_MYPS_ENABLED:
    {
      _counter--;
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
        if (strcmp(pResponse->pScope, "A_H_I") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetC1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(_propertySetC1.isEnabled());
          TESTC(_supTask3.getProxy().exists("A_H_I_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_H") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetD1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(_propertySetD1.isEnabled());
          TESTC(_supTask3.getProxy().exists("A_H_temp"));
        }
      }
      
      if (_counter == 0)
      {
        _counter = 2;
        _supTask3.getPort().cancelAllTimers();        
        TESTC_ABORT_ON_FAIL(_propertySetC1.disable() == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_propertySetD1.disable() == GCF_NO_ERROR);
        _supTask3.getPort().setTimer(10.0);
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
        if (strcmp(pResponse->pScope, "A_H_I") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetC1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(!_propertySetC1.isEnabled());
          TESTC(!_supTask3.getProxy().exists("A_H_I_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_H") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetD1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(!_propertySetD1.isEnabled());
          TESTC(!_supTask3.getProxy().exists("A_H_temp"));
        }
      }      
      if (_counter == 0)
      {
        _supTask3.getPort().cancelAllTimers();        
        NEXT_TEST(3_1, "Enable permanent prop. set");
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

GCFEvent::TResult Application::test3_1(GCFEvent& e, GCFPortInterface& p)
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
      if (_curRemoteTestNr != 205) break;           
      TESTC_ABORT_ON_FAIL(_propertySetB4.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask3.getPort());
      TESTC(_propertySetB4.isEnabled());
      TESTC(!_supTask3.getProxy().exists("A_K_temp"));
      NEXT_TEST(3_2, "Disable permanent prop. set");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test3_2(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_propertySetB4.disable() == GCF_NO_ERROR);
      break;

    case F_MYPS_DISABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask3.getPort());
      TESTC(!_propertySetB4.isEnabled());
      TESTC(!_supTask3.getProxy().exists("A_K_temp"));
      NEXT_TEST(3_3, "Disable permanent prop. set, which was not loaded");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test3_3(GCFEvent& e, GCFPortInterface& /*p*/)
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
      if (_curRemoteTestNr != 205) break;           
      TESTC_ABORT_ON_FAIL(_propertySetB4.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(_propertySetB4.disable() == GCF_NO_ERROR);
      break;
    }
    
    case F_MYPS_DISABLED:   
    {   
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }      
      
      TESTC_ABORT_ON_FAIL(_propertySetB4.disable() != GCF_NO_ERROR);
      TSTTestreadyEvent r;
      r.testnr = 303;
      _pSTPort1->send(r);        
      NEXT_TEST(4_1, "Load property set");
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_1(GCFEvent& e, GCFPortInterface& /*p*/)
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
      if (_curRemoteTestNr != 401) break;           
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR);
      _supTask3.getPort().setTimer(10.0);
      break;

    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      _supTask3.getPort().cancelAllTimers();        
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.isLoaded());
      NEXT_TEST(4_2, "Unload prop. set");
      break;
    }
    
    case F_TIMER:
      FAIL_AND_ABORT("Gets not response on load request in time.");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_2(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.unload() == GCF_NO_ERROR);
      _supTask3.getPort().setTimer(10.0);
      break;

    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      _supTask3.getPort().cancelAllTimers();        
      TESTC_ABORT_ON_FAIL(!_ePropertySetAC.isLoaded());

      TSTTestreadyEvent r;
      r.testnr = 402;
      _pSTPort1->send(r);        

      NEXT_TEST(4_3, "Check property usecount");
      break;
    }
    
    case F_TIMER:
      FAIL_AND_ABORT("Gets not response on unload request in time.");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool delayedUnload = false;

  switch (e.signal)
  {
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR);
      break;

    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.isLoaded());
      TSTTestreadyEvent r;
      r.testnr = 403;
      _pSTPort1->send(r); 
             
      if (_curRemoteTestNr != 402) 
      {
        delayedUnload = true;
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(!_ePropertySetAC.isLoaded());
      NEXT_TEST(4_4, "Stability; Reload multiple prop. sets");
      break;
    }

    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (!delayedUnload) break;           
      if (_curRemoteTestNr != 402) break;  
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.unload() == GCF_NO_ERROR);
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test4_4(GCFEvent& e, GCFPortInterface& /*p*/)
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
      if (_curRemoteTestNr != 403) break;           
      TESTC_ABORT_ON_FAIL(_propertySetB4.enable() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_propertySetD1.enable() == GCF_NO_ERROR);
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      if (_propertySetB4.isEnabled() && _propertySetD1.isEnabled())
      {
        _counter = 0;
        TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR); 
        TESTC_ABORT_ON_FAIL(_ePropertySetAH.load() == GCF_NO_ERROR); 
        TESTC_ABORT_ON_FAIL(_ePropertySetAK.load() == GCF_NO_ERROR);
      }            
      break;
    }

    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      if (_ePropertySetAC.isLoaded() && 
          _ePropertySetAH.isLoaded() && 
          _ePropertySetAK.isLoaded())
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetAC.unload() == GCF_NO_ERROR); 
        TESTC_ABORT_ON_FAIL(_ePropertySetAH.unload() == GCF_NO_ERROR); 
        TESTC_ABORT_ON_FAIL(_ePropertySetAK.unload() == GCF_NO_ERROR);
      }
      break;    
    }
    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      if (!_ePropertySetAC.isLoaded() && 
          !_ePropertySetAH.isLoaded() && 
          !_ePropertySetAK.isLoaded())
      {
        _counter++;
        if (_counter < 100)
        {
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR); 
          TESTC_ABORT_ON_FAIL(_ePropertySetAH.load() == GCF_NO_ERROR); 
          TESTC_ABORT_ON_FAIL(_ePropertySetAK.load() == GCF_NO_ERROR);
        }
        else
        {
          NEXT_TEST(5_3, "Configure prop. set with a non-existing APC file");
        }
      }
      break;   
    }
    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 404;
      if (_pSTPort1->isConnected())
        _pSTPort1->send(r);
      break;
    } 
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test5_3(GCFEvent& e, GCFPortInterface& /*p*/)
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
      if (_curRemoteTestNr != 501) break;           
      _ePropertySetAK.configure("e2");
      break;
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pResponse = (GCFConfAnswerEvent*)(&e);
      TESTC_ABORT_ON_FAIL(pResponse);
      TESTC(strcmp(pResponse->pScope, _ePropertySetAK.getScope().c_str()) == 0);
      TESTC(strcmp(pResponse->pApcName, "e2") == 0); 
      TESTC_ABORT_ON_FAIL(pResponse->result != GCF_NO_ERROR);
      NEXT_TEST(6_3, "Subscribe to property in DB, receive changes");
      break;
    }    
    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 503;
      if (_pSTPort1->isConnected())
        _pSTPort1->send(r);
      break;
    } 
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static GCFExtProperty* pProperty = 0;
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 602) break;           
      TESTC_ABORT_ON_FAIL(_ePropertySetAE.load() == GCF_NO_ERROR);
      break;
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAE.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      TESTC_ABORT_ON_FAIL(_ePropertySetAE.isLoaded());
      pProperty = (GCFExtProperty*) (&_ePropertySetAE["P1"]);
      pProperty->subscribe();
      break;
    }    
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = (GCFPropAnswerEvent*)(&e);
      TESTC(strcmp(pResponse->pPropName, "A_E.P1") == 0);
      TESTC_ABORT_ON_FAIL(pProperty->isSubscribed());
      TSTTestreadyEvent r;
      r.testnr = 603;
      assert(_pSTPort1);
      _pSTPort1->send(r);        
      break;
    }    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      TESTC(pResponse->pValue->getType() == LPT_INTEGER);
      TESTC(strcmp(pResponse->pPropName, "A_E.P1") == 0);
      TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == 22);
      FINISH;
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/*GCFEvent::TResult Application::test303(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static GCFProperty propertyACP1("A_C_P1");
  propertyACP1.setAnswer(&_supTask3.getAnswerObj());
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 302) break;      
      if (_apcT1.load(false) != GCF_NO_ERROR)
      {
        failed(303);
        TRAN(Application::test304);
      }
      else
      {
        _counter = 1;
      }      
      break;
    
    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_C") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask3.getPort()))
      {           
        if (propertyACP1.subscribe() != GCF_NO_ERROR)
        {
          failed(303);
          TRAN(Application::test304);            
        }
      }
      else
      {
        failed(303);
        TRAN(Application::test304);            
      }
      break;
    }      

    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
          (&p == &_supTask3.getPort()))
      {
        TSTTestreadyEvent r;
        r.testnr = 303;
        assert(_pSTPort1);
        _pSTPort1->send(r);        
      }
      else
      {
        failed(303);
        TRAN(Application::test304);            
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::LPT_INTEGER) &&
          (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
          (((GCFPVInteger*)pResponse->pValue)->getValue() == 22) &&
          (&p == &_supTask3.getPort()))
      {
        passed(303);
        TRAN(Application::test304);
      }
      else
      {
        failed(303);
        TRAN(Application::test304);            
      }
      break;
    }    
    case F_EXIT:
      propertyACP1.unsubscribe();
      break;
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test304(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned int nrOfFaults = 0;

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
      _counter = 0;
      if (_curRemoteTestNr != 304) break;
      char propName[] = "A_H_J_P00";
      GCFPVDouble dv(0.0);
      for (unsigned int i = 0; i <= 9; i++)
      {
        propName[7] = i + '0';
        for (unsigned int j = 0; j <= 9; j++)
        {
          propName[8] = j + '0';
          dv.setValue(((double) (i * 10 + j)) / 100.0 );
          if (_supTask3.getProxy().setPropValue(propName, dv) == GCF_NO_ERROR)
          {
            _counter++;
          }
        }
      }
      if (_counter != 100)
      {
        failed(304);
        TRAN(Application::test305);
      }
      else
        _counter = 0;
      break; 
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
          (strncmp(pResponse->pPropName, "A_H_J_P", 7) == 0) &&
          (&p == &_supTask3.getPort()))
      {
        double dv = ((GCFPVDouble*)pResponse->pValue)->getValue();
        unsigned int propNr = atoi(pResponse->pPropName + 7);
        unsigned int doubleVal = (unsigned int) floor((dv * 100.0) + 0.5);
        if (doubleVal == propNr)
        {
          _counter++;
        }
        else 
        {
          double dv = ((GCFPVDouble*)pResponse->pValue)->getValue();
          cerr << "Propvalue: " << dv << " PropNr: " << propNr << endl;
          nrOfFaults++;
        }
      }
      else
      {
        cerr << "Propname fails" << pResponse->pPropName << endl;
        nrOfFaults++;
      }
      if (_counter == 100 )
      {
        passed(304);
        TRAN(Application::test305);
      }
      else if (_counter + nrOfFaults == 100)
      {
        failed(304);
        TRAN(Application::test305);
      }
      break;
    }    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test305(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned int nrOfFaults = 0;

  static GCFProperty propertyAHJP00("A_H_J_P00");
  propertyAHJP00.setAnswer(&_supTask3.getAnswerObj());

  switch (e.signal)
  {
    case F_ENTRY:
      _counter = 0;
      if (_apcT3.load(false) != GCF_NO_ERROR)
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;

    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_H") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask3.getPort()))
      {
        if (propertyAHJP00.subscribe() != GCF_NO_ERROR)
        {
          failed(305);
          TRAN(Application::test306);
        }
      }
      else
      {
        failed(305);
        TRAN(Application::test306);          
      }
      break;
    }  
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pPropName, "A_H_J_P00") == 0) &&
          (&p == &_supTask3.getPort()))
      {
        TSTTestreadyEvent r;
        r.testnr = 305;
        assert(_pSTPort1);
        _pSTPort1->send(r);        
      }
      else
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;
    }
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_curRemoteTestNr == 305)
      {
        GCFPVDouble dv(3.12);
        _counter = 0;
        if (propertyAHJP00.setValue(dv) != GCF_NO_ERROR)
        {
          failed(305);
          TRAN(Application::test306);          
        }
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
          (strcmp(pResponse->pPropName, "A_H_J_P00") == 0) &&
          (((GCFPVDouble*)pResponse->pValue)->getValue() == 3.12) &&
          (&p == &_supTask3.getPort()))
      {
        _counter++;
      }
      else
      {
        nrOfFaults++;
      }
      if (_counter == 2 )
      {
        passed(305);
        TRAN(Application::test306);
      }
      else if (_counter + nrOfFaults == 2)
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test306(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned int nrOfFaults = 0;
  static unsigned int nrOfSucceded = 0;  
  
  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetB4.load() != GCF_NO_ERROR)
      {
        failed(306);
        TRAN(Application::finished);
      }
      break;
  
    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, propertySetB4.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask3.getPort()))
      {
        if (_apcT1K.load(false) != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::finished);
        }
        else
          _counter = 1;
      }
      else
      {
        failed(306);
        TRAN(Application::finished);
      }      
      break;
    }
    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_K") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask3.getPort()))
      {           
        if (_supTask3.getProxy().subscribeProp("A_K_P1") != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::finished);
        }
      }
      else
      {
        failed(306);
        TRAN(Application::finished);
      }   
      break;
    }
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pPropName, "A_K_P1") == 0) &&
          (&p == &_supTask3.getPort()))
      {
        TSTTestreadyEvent r;
        r.testnr = 306;
        _counter = 0;
        assert(_pSTPort1);
        _pSTPort1->send(r);        
      }
      else
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      if ((pResponse->pValue->getType() == GCFPValue::LPT_INTEGER) &&
          (strcmp(pResponse->pPropName, "A_K_P1") == 0) &&
          ((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == _counter) &&
          (&p == &_supTask3.getPort()))
      {   
        nrOfSucceded++;
      }
      else
      {
        nrOfFaults++;
      }
      cerr << "Received nr " << (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() << " from A_K_P1 (" << _counter << ")" << endl;
      _counter++;
      GCFPVInteger iv;
      iv.copy(*(pResponse->pValue));
      cerr << "Send nr " << iv.getValue() << " to A_K_P1 (" << _counter << ")" << endl;
      if (_supTask3.getProxy().setPropValue("A_C_P1", iv) != GCF_NO_ERROR)
      {
        failed(306);
        TRAN(Application::finished);
      }
      break;
    }

    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_curRemoteTestNr != 306) break;
      if (nrOfSucceded == 1000)
      {
        passed(306);
        TRAN(Application::finished);
      }
      else if (nrOfSucceded + nrOfFaults == 1000)
      {
        failed(306);
        TRAN(Application::finished);
      }      
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}
*/
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
      if (_curRemoteTestNr != 999) break;           
      TSTTestreadyEvent r;
      r.testnr = 999;
      if (_pSTPort1->isConnected())
      {
      //  _pSTPort1->send(r);
      }
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
