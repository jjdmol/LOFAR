#include "Application1.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_MyProperty.h>
#include <math.h>
#include <stdio.h>
#include "TST_Protocol.ph"
#include <Suite/suite.h>

static string sTaskName = "TA1";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  Test("TestApplication1"),
  _supTask1(*this, "ST1"),
  _supTask2(*this, "ST2"),
  _counter(0),
  _curRemoteTestNr(0),
  _propertySetA1("A_B",   propertySetA1, &_supTask1.getAnswerObj()),
  _propertySetE1("A_L",   propertySetE1, &_supTask1.getAnswerObj()),
  _propertySetXX("A_X",   propertySetE1, &_supTask1.getAnswerObj()),
  _propertySetB1("A_C",   propertySetB1, &_supTask1.getAnswerObj()),
  _propertySetB2("A_D",   propertySetB2, &_supTask2.getAnswerObj()),
  _propertySetB3("A_E",   propertySetB3, &_supTask2.getAnswerObj()),
  _ePropertySetAC("A_C",  propertySetB1, &_supTask2.getAnswerObj()),   
  _ePropertySetAL("A_L",  propertySetB1, &_supTask2.getAnswerObj())   
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
      _supTask1.getPort().init(_supTask1, "client", GCFPortInterface::SAP, TST_PROTOCOL);
      _supTask2.getPort().init(_supTask2, "client", GCFPortInterface::SAP, TST_PROTOCOL);

      NEXT_TEST(1_1)
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
      TESTC(1 == _supTask1.getPort().open());
      break;

    case F_TIMER:
      TESTC(1 == _supTask1.getPort().open());
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
      NEXT_TEST(1_2)
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
        TESTC(1 == _supTask1.getPort().open());
      if (&p == &_supTask2.getPort())
        TESTC(1 == _supTask2.getPort().open());
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
          TESTC(1 == _supTask2.getPort().open());
      }        
      break;
    }

    case F_DISCONNECTED:
      if (closing)
      {
        _supTask2.getPort().init(_supTask2, "client", GCFPortInterface::SAP, TST_PROTOCOL);
        TESTC(1 == _supTask1.getPort().open());
        closing = false;
      }
      else
      {
        if (&p == &_supTask1.getPort())
          _supTask1.getPort().setTimer(1.0); // try again after 1 second
        if (&p == &_supTask2.getPort())
          _supTask2.getPort().setTimer(1.0); // try again after 1 second
      }
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
      NEXT_TEST(2_1);
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
      if (TESTC(_propertySetA1.enable() == GCF_NO_ERROR))
      {
        _supTask1.getPort().setTimer(10.0);
      }
      else
      {
        FAIL_AND_DONE("PML could not enable propset A1");
      }      
      break;

    case F_MYPS_ENABLED:
    {
      _supTask1.getPort().cancelAllTimers();

      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetA1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetA1.isEnabled());
      TESTC(_supTask1.getProxy().exists("A_B_temp"));
      NEXT_TEST(2_2);
      break;
    }  
    case F_TIMER:
      FAIL_AND_DONE("Gets not response on enable request in time.");
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
      if (TESTC(_propertySetA1.disable() == GCF_NO_ERROR))
      {
        _supTask1.getPort().setTimer(10.0);
      }
      else
      {
        FAIL_AND_DONE("PML could not disable propset A1");
      }      
      break;

    case F_MYPS_DISABLED:
    {
      _supTask1.getPort().cancelAllTimers();

      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetA1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(!_propertySetA1.isEnabled());
      TESTC(!_supTask1.getProxy().exists("A_B_temp"));
      NEXT_TEST(2_3);
      break;
    }  
    case F_TIMER:
      FAIL_AND_DONE("Gets not response on disable request in time.");
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
      if (TESTC(_propertySetB1.enable() == GCF_NO_ERROR))
      {
        if (!TESTC(_propertySetB1.enable() != GCF_NO_ERROR))
        {
          FAIL_AND_DONE("This should not be possible");
        }
      }
      else
      {
        FAIL_AND_DONE("PML could not enable propset B1");
      }      
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetB1.isEnabled());
      TESTC(_supTask1.getProxy().exists("A_C_temp"));
      if (TESTC(_propertySetB1.enable() != GCF_NO_ERROR))
      {
        TSTTestreadyEvent r;
        r.testnr = 203;
        if (_supTask1.getPort().isConnected())
          _supTask1.getPort().send(r);        
        NEXT_TEST(2_5);
      }
      else
      {
        FAIL_AND_DONE("This should not be possible");
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
      if (!TESTC(_propertySetA1.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset A1");
      }      
      else if (!TESTC(_propertySetB2.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset B2");
      }      
      else if (!TESTC(_propertySetB3.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset B3");
      }      
      else
      {
        _supTask1.getPort().setTimer(10.0);
      }
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
          TESTC(strcmp(pResponse->pScope, _propertySetA1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(_propertySetA1.isEnabled());
          TESTC(_supTask1.getProxy().exists("A_B_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_D") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB2.getScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(_propertySetB2.isEnabled());
          TESTC(_supTask2.getProxy().exists("A_D_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_E") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB3.getScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(_propertySetB3.isEnabled());
          TESTC(_supTask2.getProxy().exists("A_E_temp"));
        }
      }
      
      if (_counter == 0)
      {
        _counter = 4;
        _supTask1.getPort().cancelAllTimers();        
        if (!TESTC(_propertySetA1.disable() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not disable propset A1");
        }      
        else if (!TESTC(_propertySetB1.disable() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not disable propset B1");
        }      
        else if (!TESTC(_propertySetB2.disable() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not disable propset B2");
        }      
        else if (!TESTC(_propertySetB3.disable() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not disable propset B3");
        }        
        else
        {
          _supTask1.getPort().setTimer(10.0);
        }
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
          TESTC(strcmp(pResponse->pScope, _propertySetA1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(!_propertySetA1.isEnabled());
          TESTC(!_supTask1.getProxy().exists("A_B_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_C") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB1.getScope().c_str()) == 0);
          TESTC(&p == &_supTask1.getPort());
          TESTC(!_propertySetB1.isEnabled());
          TESTC(!_supTask1.getProxy().exists("A_C_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_D") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB2.getScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(!_propertySetB2.isEnabled());
          TESTC(!_supTask2.getProxy().exists("A_D_temp"));
        }
        else if (strcmp(pResponse->pScope, "A_E") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetB3.getScope().c_str()) == 0);
          TESTC(&p == &_supTask2.getPort());
          TESTC(!_propertySetB3.isEnabled());
          TESTC(!_supTask2.getProxy().exists("A_E_temp"));
        }
      }      
      if (_counter == 0)
      {
        _supTask1.getPort().cancelAllTimers();        
        TSTTestreadyEvent r;
        r.testnr = 205;
        _supTask1.getPort().send(r);
        NEXT_TEST(3_4);
      }      
      break;
    }
    case F_TIMER:
      FAIL_AND_DONE("Gets not response on disable request in time.");
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
      TESTC(!_supTask1.getProxy().exists("A_X"));
      if (!TESTC(_propertySetXX.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset XX");
      }      
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetXX.getScope().c_str()) == 0);
        TESTC(pResponse->result != GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(!_propertySetXX.isEnabled());
      TESTC(!_supTask1.getProxy().exists("A_X"));
      NEXT_TEST(4_1);
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
      if (!TESTC(_propertySetB1.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset B1");
      }      
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetB1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask1.getPort());
      TESTC(_propertySetB1.isEnabled());
      TESTC(_supTask1.getProxy().exists("A_C_temp"));
      TSTTestreadyEvent r;
      r.testnr = 401;
      _supTask1.getPort().send(r);
      NEXT_TEST(4_3);
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
          if (!TESTC(_ePropertySetAC.load() == GCF_NO_ERROR))
          {
            FAIL_AND_DONE("PML could not load propset A_C");
          }      
          break;
        case 403:
          if (!TESTC(_ePropertySetAC.unload() == GCF_NO_ERROR))
          {
            FAIL_AND_DONE("PML could not unload propset A_C");
          }
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
      if (!TESTC(_ePropertySetAC.load() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not load propset A_C");
      }      
      break;

    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      if (TESTC(_ePropertySetAC.isLoaded()))
      {
        TSTTestreadyEvent r;
        r.testnr = 402;
        _supTask1.getPort().send(r);        
        if (_curRemoteTestNr != 403) 
        {
          delayedUnLoad = true;
          break;
        }
        if (!TESTC(_ePropertySetAC.unload() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not unload propset A_C");
        }                
      }      
      else
      {
        FAIL_AND_DONE("PA could not load propset A_C");
      }
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
            
      if (TESTC(!_ePropertySetAC.isLoaded()))
      {
        TSTTestreadyEvent r;
        r.testnr = 403;
        _supTask1.getPort().send(r);        
        NEXT_TEST(5_1);
      }      
      else
      {
        FAIL_AND_DONE("PA could not unload propset A_C");
      }
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
      if (!TESTC(_propertySetE1.enable() == GCF_NO_ERROR))
      {
        FAIL_AND_DONE("PML could not enable propset E1");
      }      
      break;

    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetE1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
                  
      if (TESTC(_propertySetE1.isEnabled()))
      {
        if (!TESTC(_ePropertySetAL.load() == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PML could not load propset A_L");
        }
      }
      else
      {
        FAIL_AND_DONE("PA could not enable propset E1");
      }
      break;
    }    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetE1.getScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }      
      if (TESTC(_ePropertySetAL.isLoaded()))
      {
        _propertySetE1.configure("e1");
      }
      else
      {
        FAIL_AND_DONE("PA could not load propset A_L");
      }
      break;
    }    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pResponse = (GCFConfAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC(strcmp(pResponse->pScope, _propertySetE1.getScope().c_str()) == 0);
        TESTC(strcmp(pResponse->pApcName, "e1") == 0);
        if (!TESTC(pResponse->result == GCF_NO_ERROR))
        {
          FAIL_AND_DONE("PA could not configure the propset A_L");
        }
        else
        {
          TSTTestreadyEvent r;
          r.testnr = 501;
          _supTask1.getPort().send(r);        
          NEXT_TEST(6_1);
        }
      }
      else
      {
        FAIL_AND_DONE("PA could not configure the propset A_L");
      }
      break;
    }    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test6_1(GCFEvent& e, GCFPortInterface& p)
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
      
      FINISHED;
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/*
GCFEvent::TResult Application::test302(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false;
  static unsigned int nrOfValueChanges = 0;
  
  switch (e.signal)
  {
    case F_ENTRY:
      _apcT1.setAnswer(&_supTask1.getAnswerObj());
      if (_apcT1.load(false) != GCF_NO_ERROR)
      {
        failed(302);
        TRAN(Application::test303);
      }
      break;

    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_C") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        CHECK_DB;
        GCFPVInteger iv(22);
        if (_supTask2.getProxy().setPropValue("A_C_P1", iv) != GCF_NO_ERROR)
        {
          failed(302);
          TRAN(Application::test303);
        }
        else if (_supTask2.getProxy().requestPropValue("A_C_P1") != GCF_NO_ERROR)
        {
          failed(302);
          TRAN(Application::test303);
        }      
        else
          _counter = 1;
      }
      break;
    }      
    case F_VGETRESP:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:  
          if ((pResponse->pValue->getType() == GCFPValue::LPT_INTEGER) &&
              (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
              (((GCFPVInteger*)pResponse->pValue)->getValue() == 22) &&
              (&p == &_supTask2.getPort()))
          {
            GCFPVChar cv(22);
            if (_supTask2.getProxy().setPropValue("A_C_P2", cv) != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }
            else if (_supTask2.getProxy().requestPropValue("A_C_P2") != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }      
            else
              _counter++;
          }
          else
          {
            failed(302);
            TRAN(Application::test303);
          }
          break;
        case 2:  
          if ((pResponse->pValue->getType() == GCFPValue::LPT_CHAR) &&
              (strcmp(pResponse->pPropName, "A_C_P2") == 0) &&
              (((GCFPVChar*)pResponse->pValue)->getValue() == 22) &&
              (&p == &_supTask2.getPort()))
          {
            GCFPVDouble dv(22.0);
            if (_supTask2.getProxy().setPropValue("A_C_P3", dv) != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }
            else if (_supTask2.getProxy().requestPropValue("A_C_P3") != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }      
            else
              _counter++;
          }
          else
          {
            failed(302);
            TRAN(Application::test303);
          }
          break;
        case 3:  
          if ((pResponse->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
              (strcmp(pResponse->pPropName, "A_C_P3") == 0) &&
              (((GCFPVChar*)pResponse->pValue)->getValue() == 22.0) &&
              (&p == &_supTask2.getPort()))
          {
            if (!ready)
            {
              ready = true;
            }
            else if (nrOfValueChanges == 3)
            {
              passed(302);
              TRAN(Application::test303);
            }
          }
          else
          {
            failed(302);
            TRAN(Application::test303);
          }
          break;
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((strncmp(pResponse->pPropName, "A_C_P", 5) == 0) &&
          (&p == &_supTask1.getPort()))
      {
        nrOfValueChanges++;
        if (!ready)
        {
          ready = true;
        }
        else if (nrOfValueChanges == 3)
        {
          passed(302);
          TRAN(Application::test303);
        }
      }
      break;
    } 
    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 302;
      _supTask1.getPort().send(r);
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test303(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_curRemoteTestNr == 303) 
      {
        GCFPVInteger iv(22);
        if (_supTask1.getProxy().setPropValue("A_C_P1", iv) != GCF_NO_ERROR)
        {
          failed(303);
          TRAN(Application::test304);
        }
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
          (&p == &_supTask1.getPort()))
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
    case F_ENTRY:
      _counter = 0;
      if (_apcT3.load(false) != GCF_NO_ERROR)
      {
        failed(304);
        TRAN(Application::test305);
      }
      break;

    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_H") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        char propName[] = "A_H_J_P00";
        for (unsigned int i = 0; i <= 9; i++)
        {
          propName[7] = i + '0';
          for (unsigned int j = 0; j <= 9; j++)
          {
            propName[8] = j + '0';
            if (_supTask1.getProxy().subscribeProp(propName) == GCF_NO_ERROR)
            {
              _counter++;
            }
          }
        }        
      }
      else
      {
        failed(304);
        TRAN(Application::test305);
      }
      break;
    }
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strncmp(pResponse->pPropName, "A_H_J_P", 7) == 0) &&
          (&p == &_supTask1.getPort()))
      {
        _counter--;
        if (_counter == 0)
        {
          TSTTestreadyEvent r;
          r.testnr = 304;
          _supTask1.getPort().send(r);
        }
      }
      else
      {
        failed(304);
        TRAN(Application::test305);
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
          (strncmp(pResponse->pPropName, "A_H_J_P", 7) == 0) &&
          (&p == &_supTask1.getPort()))
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
          cerr << "Propvalue: " << dv * 100 << " PropNr: " << propNr << endl;
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
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
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

  static GCFProperty propertyAHJP00_1("A_H_J_P00");
  propertyAHJP00_1.setAnswer(&_supTask1.getAnswerObj());
  static GCFProperty propertyAHJP00_2("A_H_J_P00");
  propertyAHJP00_2.setAnswer(&_supTask2.getAnswerObj());

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 305) break;
      _counter = 0;
      if (propertyAHJP00_1.subscribe() != GCF_NO_ERROR)
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;
  
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pPropName, "A_H_J_P00") == 0))
      {
        if (&p == &_supTask1.getPort())
        {
          if (propertyAHJP00_2.subscribe() != GCF_NO_ERROR)
          {
            failed(305);
            TRAN(Application::test306);
          }
        }
        else if (&p == &_supTask2.getPort())
        {
          TSTTestreadyEvent r;
          r.testnr = 305;
          _supTask1.getPort().send(r);
        }
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
      if ((pResponse->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
          (strcmp(pResponse->pPropName, "A_H_J_P00") == 0) &&
          (((GCFPVDouble*)pResponse->pValue)->getValue() == 3.12))
      {
        _counter++;
      }
      else
      {
        nrOfFaults++;
      }
      if (_counter == 3 )
      {
        passed(305);
        TRAN(Application::test306);
      }
      else if (_counter + nrOfFaults == 3)
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
  
  static GCFProperty propertyACP1("A_C_P1");
  propertyACP1.setAnswer(&_supTask1.getAnswerObj());
  
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 306) break;
      
      if (propertyACP1.subscribe() != GCF_NO_ERROR)
      {
        failed(306);
        TRAN(Application::test501);
      }
      break;
  
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pResponse = static_cast<GCFPropAnswerEvent*>(&e);
      assert(pResponse);
      _counter = 0;
      if ((strcmp(pResponse->pPropName, "A_C_P1") == 0))
      {
        cerr << "Send nr " << _counter << " to A_K_P1" << endl;
        GCFPVInteger iv(_counter);
        if (_supTask1.getProxy().setPropValue("A_K_P1", iv) != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::test501);
        }
      }
      else
      {
        failed(306);
        TRAN(Application::test501);
      }
      break;
    }

    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      if ((pResponse->pValue->getType() == GCFPValue::LPT_INTEGER) &&
          (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
          ((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == _counter))
      {   
        nrOfSucceded++;
      }
      else
      {
        nrOfFaults++;
      }
      cerr << "Received nr " << (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() << " from A_C_P1(" << _counter << ")" << endl;
      _counter++;
      if (nrOfSucceded == 1000)
      {
        passed(306);
        TRAN(Application::test501);
      }
      else if (nrOfSucceded + nrOfFaults == 1000)
      {
        failed(306);
        TRAN(Application::test501);
      }
      else
      {
        cerr << "Send nr " << _counter << " to A_K_P1" << endl;
        GCFPVInteger iv(_counter);
        if (_supTask1.getProxy().setPropValue("A_K_P1", iv) != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::test501);
        }
      }        
      break;
    }

    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 306;
      _supTask1.getPort().send(r);
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult Application::test501(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  static GCFApc apc1("ApcTRT", "B_RT1", &_supTask1.getAnswerObj());
  static GCFApc apc2("ApcTRT", "B_RT2", &_supTask1.getAnswerObj());

  switch (e.signal)
  {
    case F_ENTRY:
      apc1.load(false);
      _counter = 0;
      break;

    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "B_RT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR))
      {          
        _supTask1.getPort().setTimer(40.0);
      }
      break;
    }  
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "B_RT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR))
      {          
        //if (apc2.unload() != GCF_NO_ERROR)
        {
          TSTTestreadyEvent r;
          r.testnr = 501;
          if (_supTask1.getPort().isConnected())
            _supTask1.getPort().send(r);
          passed(501);
          TRAN(Application::finished);        
        }
      }
      else
      {
        TSTTestreadyEvent r;
        r.testnr = 501;
        if (_supTask1.getPort().isConnected())
          _supTask1.getPort().send(r);
        passed(501);
        TRAN(Application::finished);        
      }
      break;
    }  
    case F_TIMER:
    {
      GCFTimerEvent* pTIM = static_cast<GCFTimerEvent*>(&e);
      assert(pTIM);
      srand(pTIM->sec * 1000000 + pTIM->usec);
      int maxSeqNr = 20 + (int) (80.0 * rand() / (RAND_MAX + 1.0));
      GCFPVInteger maxSeqNrV(maxSeqNr);
      _supTask1.getProxy().setPropValue("B_RT1_maxSeqNr", maxSeqNrV);
      if (_counter >= 3)
      {
        int maxSeqNr = 20 + (int) (80.0 * rand() / (RAND_MAX + 1.0));
        maxSeqNrV.setValue(maxSeqNr);
        //_supTask1.getProxy().setPropValue("B_RT2_maxSeqNr", maxSeqNrV);
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
        apc1.unload();
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
      if (_supTask1.getPort().isConnected())
      //  _supTask1.getPort().send(r);
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
