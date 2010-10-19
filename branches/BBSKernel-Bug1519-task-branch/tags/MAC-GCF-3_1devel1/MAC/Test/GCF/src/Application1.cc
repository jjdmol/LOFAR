#include "Application1.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_Property.h>
#include <GCF/GCF_MyProperty.h>
#include <math.h>
#include <stdio.h>
#define DECLARE_SIGNAL_NAMES
#include "TST_Protocol.ph"

#ifdef WAIT_FOR_INPUT
#define CHECK_DB \
          { \
            string input; \
            cout << "Check SCADA DB content: "; \
            cin >> input; \
          }
#else
#define CHECK_DB {}
#endif
static string sTaskName = "TA1";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  _supTask1(*this, "ST1"),
  _supTask2(*this, "ST2"),
  _passed(0),
  _failed(0),
  _counter(0),
  _curRemoteTestNr(0),
  _propertySetA1(propertySetA1, &_supTask1.getAnswerObj()),
  _propertySetE1(propertySetE1, &_supTask1.getAnswerObj()),
  _propertySetB1(propertySetB1, &_supTask1.getAnswerObj()),
  _propertySetB2(propertySetB2, &_supTask2.getAnswerObj()),
  _propertySetB3(propertySetB3, &_supTask2.getAnswerObj()),
  _propertySetB4(propertySetB4, &_supTask2.getAnswerObj()),
  _apcT1("ApcT1", "A_C", &_supTask2.getAnswerObj()),
  _apcT3("ApcT3", "A_H", &_supTask1.getAnswerObj())  
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
      TRAN(Application::test401);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test101(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetA1.load() != GCF_NO_ERROR)
      {
        failed(101);
        TRAN(Application::test102);
      }
      else
      {
        _supTask1.getPort().setTimer(5.0);
      }      
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if (ready) { TRAN(Application::test102); break;}
      if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        passed(101);
      }
      else
      {
        failed(101);
      }
      ready = true;
      break;
    }  
    case F_TIMER:
      if (!ready) {failed(101); ready = true;}
      else TRAN(Application::test102);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test102(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetA1.unload() != GCF_NO_ERROR)
      {
        failed(102);
        TRAN(Application::test103);        
      } 
      else
      {
        _supTask1.getPort().setTimer(1.0);
      } 
      break;

    case F_MYPUNLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if (ready) { TRAN(Application::test103); break;}
      if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        passed(102);
      }
      else
      {
        failed(102);
      }      
      ready = true;
      break;
    }
    
    case F_TIMER:
      if (!ready) {failed(102); ready = true;}
      else TRAN(Application::test103);
      break;
      
   default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test103(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetB1.load() != GCF_NO_ERROR)
      {
        failed(103);
        TRAN(Application::test104);
      }
      else
      {
        _counter = 1;
        if (_propertySetB1.load() == GCF_NO_ERROR)
        {
          _counter = 2;
        }
        _supTask1.getPort().setTimer(1.0);
      }      
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if (ready) { TRAN(Application::test103); break;}
      if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        passed(103);
      }
      else
      {
        failed(103);
      }      
      if (_counter > 0) _counter--;
      if (_counter == 0)
        ready = true;
      break;
    }  
    case F_TIMER:
      if (!ready) {failed(103); ready = true;}
      else TRAN(Application::test104);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test104(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetA1.load() != GCF_NO_ERROR)
      {
        failed(104);
        TRAN(Application::test105);
      }
      else
      {
        _supTask1.getPort().setTimer(10.0);
      }      
      _counter = 1;
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_propertySetB2.load() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
          
        case 2:
          if ((strcmp(pResponse->pScope, propertySetB2.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (_propertySetB3.load() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
          
        case 3:
          if ((strcmp(pResponse->pScope, propertySetB3.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (_propertySetB4.load() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;

        case 4:
          if ((strcmp(pResponse->pScope, propertySetB4.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (_propertySetA1.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
      }
      break;
    }      
    case F_MYPUNLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 5:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_propertySetB2.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
          
        case 6:
          if ((strcmp(pResponse->pScope, propertySetB2.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (_propertySetB3.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
          
        case 7:
          if ((strcmp(pResponse->pScope, propertySetB3.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (_propertySetB4.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
          }
          _counter++;
          break;
          
        case 8:
          if ((strcmp(pResponse->pScope, propertySetB4.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            if (!ready)
            {
              passed(104);
              ready = true;
            }
            else TRAN(Application::test105);            
          }
          break;          
      }
      break;
    }  
    case F_TIMER:
      if (!ready) {failed(104); ready = true;}
      else TRAN(Application::test105);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test105(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetA1.load() != GCF_NO_ERROR)
      {
        failed(105);
        TRAN(Application::test201);
      }
      else
        _counter = 1;
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_propertySetB2.load() != GCF_NO_ERROR)
            {
              failed(105);
              TRAN(Application::test201);
            }
            else
              _counter++;            
          }
          break;
          
        case 2:
          if ((strcmp(pResponse->pScope, propertySetB2.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            passed(105);
          }
          else 
            failed(105);
          TRAN(Application::test201);            
          break;          
      }
      break;
    }  
    case F_TIMER:
      if (!ready) {failed(105); ready = true;}
      else TRAN(Application::test201);
      break;

    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 105;
      if (_supTask1.getPort().isConnected())
        _supTask1.getPort().send(r);
      break;
    }  
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test201(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(201);
      TRAN(Application::test202);
      break;

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

GCFEvent::TResult Application::test202(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(202);
      TRAN(Application::test203);
      break;

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

GCFEvent::TResult Application::test203(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(203);
      TRAN(Application::test204);
      break;

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

GCFEvent::TResult Application::test204(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(204);
      TRAN(Application::test205);
      break;

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

GCFEvent::TResult Application::test205(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      if (_curRemoteTestNr == 0)
        _curRemoteTestNr = indicationIn.testnr;
        //intentional fall through
      else
        break;
    }
    case F_ENTRY:
      if (_curRemoteTestNr == 0) break;
      if (_apcT1.load() != GCF_NO_ERROR)
      {
        failed(205);
        TRAN(Application::test206);
      }
      _counter = 1;
      break;

    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
			      CHECK_DB;
            if (_apcT1.unload() != GCF_NO_ERROR)
            {
              failed(204);
              TRAN(Application::test205);
            }
            else
            {
              TSTTestreadyEvent r;
              r.testnr = 205;
              _supTask1.getPort().send(r);
            }
          }
          _counter++;
          break;
      }
      break;
    }      
      
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 2:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
            CHECK_DB
            passed(205);
          }
          else
            failed(205);
          TRAN(Application::test206);
          break;
      }
      break;
    }      

    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 205;
      _supTask1.getPort().send(r);
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test206(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _counter = 0;
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_apcT1.load() != GCF_NO_ERROR)
      {
        failed(206);
        TRAN(Application::test207);
      }
      else
      {
        _counter = 1;
      }      
      break;
    }
    case F_APCLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 1)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask2.getPort()))
        {
          CHECK_DB;          
          if (_apcT1.unload() != GCF_NO_ERROR)
          {
            failed(206);
            TRAN(Application::test207);
          }
          else
          {
            TSTTestreadyEvent r;
            r.testnr = 206;
            _supTask1.getPort().send(r);
            _counter++;
          }
        }          
      }
      break;
    }      
      
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 2)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask2.getPort()))
        {
          CHECK_DB;
          passed(206);          
        }
        else
          failed(206);
        TRAN(Application::test207);  
      }
      break;
    }      

    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 206;
      _supTask1.getPort().send(r);
      break;
    }
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test207(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(207);
      TRAN(Application::test208);
      break;

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

GCFEvent::TResult Application::test208(GCFEvent& e, GCFPortInterface& p)
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
      if (_curRemoteTestNr != 208) break;
      if (_apcT3.load(false) != GCF_NO_ERROR)
      {
        failed(208);
        TRAN(Application::test301);
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
      if (_counter == 1)
      {
        if ((strcmp(pResponse->pScope, "A_H") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask1.getPort()))
        {
          CHECK_DB
          
          if (_apcT3.unload() != GCF_NO_ERROR)
          {
            failed(208);
            TRAN(Application::test301);
          }
          else
          {
            _counter++;
          }
        }  
        else        
        {
          failed(208);
          TRAN(Application::test301);
        }
      }
      break;
    }      
      
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 2)
      {
        if ((strcmp(pResponse->pScope, "A_H") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask1.getPort()))
        {
          CHECK_DB
          passed(208);
        }
        else
          failed(208);
        TRAN(Application::test301);  
      }
      break;
    }      

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test209(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test210(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test301(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  GCFPValue* pValue;
  
  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetB1.unload() != GCF_NO_ERROR)
      {
        failed(301);
        TRAN(Application::test302);
      }
      else
        _counter = 1;
      break;

    case F_MYPUNLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        if (_propertySetB1.load() != GCF_NO_ERROR)
        {
          failed(301);
          TRAN(Application::test302);
        }     
      }
      else
      {
        failed(301);
        TRAN(Application::test302);
      }      
      break;
    }

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        // A_C_P1 section start
        pValue = _propertySetB1.getValue("P1"); // its a clone
        assert(pValue);
        if (((GCFPVInteger*)pValue)->getValue() != 11)
        {          
          failed(301);
          TRAN(Application::test302);
        }
        delete pValue;
        GCFPVInteger iv(12);
        if (_propertySetB1.setValue("P1", iv) != GCF_NO_ERROR)
        {
          failed(301);
          TRAN(Application::test302);
        }
        pValue = _propertySetB1.getValue("P1");  // its a clone
        assert(pValue);
        if (((GCFPVInteger*)pValue)->getValue() != 12)
        {          
          failed(301);
          TRAN(Application::test302);
        }
        delete pValue;
        
        // A_C_P2 section start
        GCFMyProperty* pProperty(0);
        pProperty = static_cast<GCFMyProperty*>(_propertySetB1.getProperty("A_C_P2"));
        assert(pProperty);      
        pValue = pProperty->getValue();  // its a clone
        assert(pValue);
        if (((GCFPVChar*)pValue)->getValue() != 25)
        {          
          failed(301);
          TRAN(Application::test302);
          break;
        }
        delete pValue;
        GCFPVChar cv(26);
        if (_propertySetB1.setValue("P2", cv) != GCF_NO_ERROR)
        {
          failed(301);
          TRAN(Application::test302);
          break;
        }
        pValue = _propertySetB1.getValue("P2");  // its a clone
        assert(pValue);
        if (((GCFPVChar*)pValue)->getValue() != 26)
        {          
          failed(301);
          TRAN(Application::test302);
          break;
        }
        delete pValue;
        // A_C_P3 section start
        pValue = _propertySetB1.getValue("A_C_P3");
        assert(pValue);
        if (((GCFPVDouble*)pValue)->getValue() != 33.3)
        {          
          failed(301);
          TRAN(Application::test302);
          break;
        }
        delete pValue;
        GCFPVDouble dv(33.4);
        if (_propertySetB1.setValue("P3", dv) != GCF_NO_ERROR)
        {
          failed(301);
          TRAN(Application::test302);
          break;
        }
        pValue = _propertySetB1.getValue("P3");  // its a clone
        assert(pValue);
        if (((GCFPVDouble*)pValue)->getValue() != 33.4)
        {          
          failed(301);
          TRAN(Application::test302);
          break;
        }
        delete pValue;

        passed(301);
        TRAN(Application::test302);
      }
      else
      {
        failed(301);
        TRAN(Application::test302);
      }
      break;
    }  

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

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

GCFEvent::TResult Application::test306(GCFEvent& e, GCFPortInterface& /*p*/)
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

GCFEvent::TResult Application::test401(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _supTask1.getPort().init(_supTask1, "client", GCFPortInterface::SAP, TST_PROTOCOL);
      _supTask1.getPort().open();
      break;

    case F_TIMER:
      _supTask1.getPort().open();
      break;

    case F_CONNECTED:
    {
      TSTTestreqEvent r;
      r.testnr = 401;
      _supTask1.getPort().send(r);
      break;
    }
    case F_DISCONNECTED:
      if (&p == &_supTask1.getPort())
        _supTask1.getPort().setTimer(1.0); // try again after 1 second
      break;

    case TST_TESTRESP:
    {
      passed(401);
      TRAN(Application::test402);
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test402(GCFEvent& e, GCFPortInterface& p)
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
        _supTask1.getPort().open();
      if (&p == &_supTask2.getPort())
        _supTask2.getPort().open();
      break;

    case F_CONNECTED:
    {
      if (_supTask1.getPort().isConnected())
      {
        if (_supTask2.getPort().isConnected())
        {
          TSTTestreqEvent r;
          r.testnr = 402;
          _supTask1.getPort().send(r);
        }
        else
          _supTask2.getPort().open();
      }        
      break;
    }

    case F_DISCONNECTED:
      if (closing)
      {
        _supTask2.getPort().init(_supTask2, "client", GCFPortInterface::SAP, TST_PROTOCOL);
        _supTask1.getPort().open();
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
        r.testnr = 402;
        _supTask2.getPort().send(r);
      }
      else
      {
        TSTTestreadyEvent r;
        r.testnr = 402;
        _supTask1.getPort().send(r);
      }
      break;

    case TST_TESTREADY:
      passed(402);
      TRAN(Application::test101);
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test501(GCFEvent& e, GCFPortInterface& /*p*/)
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
        if (apc2.unload() != GCF_NO_ERROR)
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
        _supTask1.getProxy().setPropValue("B_RT2_maxSeqNr", maxSeqNrV);
      }
      if (_counter < 10)
      {
        if (_counter == 3)
        {
          apc2.load(false);
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

GCFEvent::TResult Application::finished(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      fprintf(stderr, "Ready with tests: passed %d, failed %d\n", _passed, _failed);
      GCFTask::stop();
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);
  
  Application a;

  a.start();

  GCFTask::run();
  
  return 0;  
}
