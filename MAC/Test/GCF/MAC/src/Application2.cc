#include <lofar_config.h>

#include "Application2.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <math.h>
#include <stdio.h>
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
static string sTaskName = "TA2";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  Test("TestApplication2"),
  _supTask3(*this, "ST3"),
  _counter(0),
  _curRemoteTestNr(0),
  _pSTPort1(0),
  _pSTPort2(0),
  _propertySetC1("A_H_I",     "TTypeC", PS_CAT_TEMPORARY, &_supTask3.getAnswerObj()),
  _propertySetD1("A_H",       "TTypeD", PS_CAT_TEMPORARY, &_supTask3.getAnswerObj()),  
  _propertySetB4("A_K",       "TTypeB", PS_CAT_PERMANENT, &_supTask3.getAnswerObj(), GCFMyPropertySet::USE_DB_DEFAULTS),
  _ePropertySetAC(REMOTESYS2"A_C", "TTypeB", &_supTask3.getAnswerObj()), 
  _ePropertySetAE(REMOTESYS2"A_E", "TTypeB", &_supTask3.getAnswerObj()), 
  _ePropertySetAH("A_H",      "TTypeD", &_supTask3.getAnswerObj()), 
  _ePropertySetAK("A_K",      "TTypeB", &_supTask3.getAnswerObj()) 
{
    // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames); 
  
  _propertySetC1.initProperties(propertiesSC1);
  _propertySetD1.initProperties(propertiesSD1);
  _propertySetB4.initProperties(propertiesSB4); 
}

Application::~Application()
{
  if (_pSTPort1) delete _pSTPort1;
  if (_pSTPort2) delete _pSTPort2;
}

GCFEvent::TResult Application::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      _supTask3.getPort().init(_supTask3, MAC_SVCMASK_GCFTEST_ST3SERVER, GCFPortInterface::SPP, TST_PROTOCOL);
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
    case F_TIMER:
      TESTC(_supTask3.getPort().open());
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
      TESTC(_port.open());
      break;

    case F_CONNECTED:
      _counter = 0;
      break;
      
    case F_ACCEPT_REQ:
      if (_pSTPort1 == 0)
      {
        _pSTPort1 = new GCFTCPPort();
        _pSTPort1->init(_supTask3, MAC_SVCMASK_GCFTEST_ST3SERVER, GCFPortInterface::SPP, TST_PROTOCOL);
        TESTC(_port.accept(*_pSTPort1));
      }
      else
      {
        _pSTPort2 = new GCFTCPPort();
        _pSTPort2->init(_supTask3, MAC_SVCMASK_GCFTEST_ST3SERVER, GCFPortInterface::SPP, TST_PROTOCOL);
        TESTC(_port.accept(*_pSTPort2));
      }
      break;
      
    case F_DISCONNECTED:
      if (closing)
      {
        _port.init(_supTask3, MAC_SVCMASK_GCFTEST_ST3PROVIDER, GCFPortInterface::MSPP, TST_PROTOCOL);
        TESTC(_port.open());
        closing = false;
      }
      else
      {
        if (&p == &_port)
          _port.setTimer(1.0); // try again after 1 second
      }      
      break;

    case F_CLOSED:
      _port.init(_supTask3, MAC_SVCMASK_GCFTEST_ST3PROVIDER, GCFPortInterface::MSPP, TST_PROTOCOL);
      TESTC(_port.open());
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
          TESTC(strcmp(pResponse->pScope, _propertySetC1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(_propertySetC1.isEnabled());
          TESTC_DESCR(GCFPVSSInfo::propExists("A_H_I__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_H") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetD1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(_propertySetD1.isEnabled());
          TESTC_DESCR(GCFPVSSInfo::propExists("A_H__enabled"), "may fail");
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
          TESTC(strcmp(pResponse->pScope, _propertySetC1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(!_propertySetC1.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_H_I__enabled"), "may fail");
        }
        else if (strcmp(pResponse->pScope, "A_H") == 0)
        {
          TESTC(strcmp(pResponse->pScope, _propertySetD1.getFullScope().c_str()) == 0);
          TESTC(&p == &_supTask3.getPort());
          TESTC(!_propertySetD1.isEnabled());
          TESTC_DESCR(!GCFPVSSInfo::propExists("A_H__enabled"), "may fail");
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
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask3.getPort());
      TESTC(_propertySetB4.isEnabled());
      TESTC_DESCR(!GCFPVSSInfo::propExists("A_K__enabled"), "may fail");
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
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
      TESTC(&p == &_supTask3.getPort());
      TESTC(!_propertySetB4.isEnabled());
      TESTC_DESCR(!GCFPVSSInfo::propExists("A_K__enabled"), "may fail");
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
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getFullScope().c_str()) == 0);
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
        TESTC(strcmp(pResponse->pScope, _propertySetB4.getFullScope().c_str()) == 0);
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

  static bool loadedInTime = false;
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
        TESTC(pResponse->result == GCF_NO_ERROR);
      }
            
      _supTask3.getPort().cancelAllTimers();
      loadedInTime = true;        
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.isLoaded());
      _counter = 0;
      if (_ePropertySetAC.exists("P1"))
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P1") == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P2") == GCF_NO_ERROR);
        TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P3") == GCF_NO_ERROR);
      }
      else
      {
        _supTask3.getPort().setTimer(0.0);
      }
      break;
    }
    case F_VGETRESP:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      const TPropertyConfig* pPropInfo;
      GCFPValue* pOrigValue(0);
      if (strstr(pResponse->pPropName, "A_C.P1") > 0)
      {
        pPropInfo = &propertiesSB1[0];
        pOrigValue = GCFPValue::createMACTypeObject(pResponse->pValue->getType());
        if (pPropInfo->defaultValue)
        {
          pOrigValue->setValue(pPropInfo->defaultValue);
        }
        TESTC(((GCFPVInteger*)pResponse->pValue)->getValue() == ((GCFPVInteger*)pOrigValue)->getValue());        
      }
      else if (strstr(pResponse->pPropName, "A_C.P2") > 0)
      {
        pPropInfo = &propertiesSB1[1];        
        pOrigValue = GCFPValue::createMACTypeObject(pResponse->pValue->getType());
        if (pPropInfo->defaultValue)
        {
          pOrigValue->setValue(pPropInfo->defaultValue);
        }
        TESTC(((GCFPVDouble*)pResponse->pValue)->getValue() == ((GCFPVDouble*)pOrigValue)->getValue());        
      }
      else if (strstr(pResponse->pPropName, "A_C.P3") > 0)
      {
        pPropInfo = &propertiesSB1[2];  
        pOrigValue = GCFPValue::createMACTypeObject(pResponse->pValue->getType());
        if (pPropInfo->defaultValue)
        {
          pOrigValue->setValue(pPropInfo->defaultValue);
        }
        TESTC(((GCFPVString*)pResponse->pValue)->getValue() == ((GCFPVString*)pOrigValue)->getValue());        
      }
      if (pOrigValue)
      {
        delete pOrigValue;
      }
      NEXT_TEST(4_2, "Unload prop. set");
      break;
    }
    
    case F_TIMER:
      if (loadedInTime)
      {
        if (GCFPVSSInfo::propExists(REMOTESYS2"A_C.P1"))
        {
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P1") == GCF_NO_ERROR);
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P2") == GCF_NO_ERROR);
          TESTC_ABORT_ON_FAIL(_ePropertySetAC.requestValue("P3") == GCF_NO_ERROR);
        }
        else
        {
          _counter++;
          if (_counter < 20)
          {
            _supTask3.getPort().setTimer(0.0);
          }
          else
          {
            FAIL_AND_ABORT("DP "REMOTESYS2"A_C not known to this app in time.");
          }
        }        
      }
      else
      {
        FAIL_AND_ABORT("Gets not response on load request in time.");
      }
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAC.getFullScope().c_str()) == 0);
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
      TESTC(strcmp(pResponse->pScope, _ePropertySetAK.getFullScope().c_str()) == 0);
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
        TESTC(strcmp(pResponse->pScope, _ePropertySetAE.getFullScope().c_str()) == 0);
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
      TESTC(strstr(pResponse->pPropName, "A_E.P1") > 0);
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
      TESTC(strstr(pResponse->pPropName, "A_E.P1") > 0);
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
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
    {
      if (_curRemoteTestNr != 603) break;
      _counter = 0;
      char propName[] = "J.P00";
      GCFPVDouble dv(0.0);
      for (unsigned int i = 0; i <= 9; i++)
      {
        propName[3] = i + '0';
        for (unsigned int j = 0; j <= 9; j++)
        {
          propName[4] = j + '0';
          dv.setValue(((double) (i * 10 + j)) / 100.0 );
          if (TESTC(_propertySetD1.setValue(propName, dv) == GCF_NO_ERROR))
          {
            _counter++;
          }
        }
      }
      TESTC_ABORT_ON_FAIL(_counter == 100);
      _counter = 0;
      break; 
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
      TESTC(pResponse->internal);
      TESTC(strstr(pResponse->pPropName, "A_H.J.P") > 0);

      unsigned int expectedVal = atoi(pResponse->pPropName + GCFPVSSInfo::getLocalSystemName().length() + 8);
      double dv = ((GCFPVDouble*)pResponse->pValue)->getValue();
      unsigned int receivedVal = (unsigned int) floor((dv * 100.0) + 0.5);
      TESTC(receivedVal == expectedVal);
      _counter++;
      cerr << "Received prop. val: " << receivedVal << " Expected prop. val: " << expectedVal << endl;

      if (_counter == 100 )
      {
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


GCFEvent::TResult Application::test6_5(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_curRemoteTestNr == 605)
      {
        GCFPVDouble dv(3.12);
        _counter = 0;
        TESTC_ABORT_ON_FAIL(_propertySetD1["J.P00"].setValue(dv) == GCF_NO_ERROR);
      }
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 604) break;
      _counter = 0;
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.load() == GCF_NO_ERROR);
      break;

    case F_EXTPS_LOADED:
    {
      _counter = 0;
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.isLoaded());
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.subscribeProp("A_H.J.P00") == GCF_NO_ERROR);
      break;
    }  
    case F_SUBSCRIBED:
    {
      TESTC_ABORT_ON_FAIL(_ePropertySetAH.isPropSubscribed("J.P00"));
      TSTTestreadyEvent r;
      r.testnr = 604;
      assert(_pSTPort1);
      _pSTPort1->send(r);        
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      
      TESTC(pResponse->pValue->getType() == LPT_DOUBLE);
      TESTC(strstr(pResponse->pPropName, "A_H.J.P00") > 0);
      TESTC_ABORT_ON_FAIL(((GCFPVDouble*)pResponse->pValue)->getValue() == 3.12);
      _counter++;
      if (_counter == 2)
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
    case F_ENTRY:
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.load() == GCF_NO_ERROR);
      TESTC_ABORT_ON_FAIL(_ePropertySetAK.load() == GCF_NO_ERROR);
      break;
  
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pResponse = (GCFPropSetAnswerEvent*)(&e);
      if (TESTC(pResponse))
      {
        TESTC_ABORT_ON_FAIL(pResponse->result == GCF_NO_ERROR);
      }
      if (_ePropertySetAC.isLoaded() && _ePropertySetAK.isLoaded())
      {
        TESTC_ABORT_ON_FAIL(_ePropertySetAK.subscribeProp("P1") == GCF_NO_ERROR);
      }
      break;
    }
    case F_SUBSCRIBED:
    {
      TESTC_ABORT_ON_FAIL(_ePropertySetAK.isPropSubscribed("P1"));
      _counter = 0;
      TSTTestreadyEvent r;
      r.testnr = 606;
      assert(_pSTPort1);
      _pSTPort1->send(r);        
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      assert(strstr(pResponse->pPropName, "A_K.P1") > 0);
      TESTC((unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() == _counter);
      cerr << "Received nr " << (unsigned int)((GCFPVInteger*)pResponse->pValue)->getValue() << " from A_K.P1 (" << _counter << ")" << endl;
      _counter++;
      GCFPVInteger iv;
      iv.copy(*(pResponse->pValue));
      cerr << "Send nr " << iv.getValue() << " to "REMOTESYS2"A_C.P1 (" << _counter << ")" << endl;
      TESTC_ABORT_ON_FAIL(_ePropertySetAC.setValue("P1", iv) == GCF_NO_ERROR);
      if (_counter == 1000)
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
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
  TM::GCFTask::init(argc, argv);
  
  LOG_INFO("MACProcessScope: GCF.TEST.MAC.App2");

  Suite s("GCF Test", &cerr);
  s.addTest(new LOFAR::GCF::Test::Application);
  s.run();
  s.report();
  s.free();
  
  return 0;  
}
