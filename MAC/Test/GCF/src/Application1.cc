#include "Application1.h"
#include "Defines.h"
#include <SAL/GCF_PVInteger.h>
#include <SAL/GCF_PVChar.h>
#include <SAL/GCF_PVDouble.h>
#include <math.h>
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
  _curRemoteTestNr(0)
{
    // register the protocol for debugging purposes
  registerProtocol(TST_PROTOCOL, TST_PROTOCOL_signalnames);  
}

GCFEvent::TResult Application::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
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
    case F_ENTRY_SIG:
      if (_supTask1.loadMyProperties(propertySetA1) != GCF_NO_ERROR)
      {
        failed(101);
        TRAN(Application::test102);
      }
      else
      {
        _supTask1.getPort().setTimer(5.0);
      }      
      break;

    case F_MYLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
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
    case F_TIMER_SIG:
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
    case F_ENTRY_SIG:
      if (_supTask1.unloadMyProperties(propertySetA1.scope) != GCF_NO_ERROR)
      {
        failed(102);
        TRAN(Application::test103);        
      } 
      else
      {
        _supTask1.getPort().setTimer(1.0);
      } 
      break;

    case F_MYUNLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
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
    
    case F_TIMER_SIG:
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
    case F_ENTRY_SIG:
      if (_supTask1.loadMyProperties(propertySetB1) != GCF_NO_ERROR)
      {
        failed(103);
        TRAN(Application::test104);
      }
      else
      {
        _counter = 1;
        if (_supTask1.loadMyProperties(propertySetB1) == GCF_NO_ERROR)
        {
          _counter = 2;
        }
        _supTask1.getPort().setTimer(1.0);
      }      
      break;

    case F_MYLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
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
    case F_TIMER_SIG:
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
    case F_ENTRY_SIG:
      if (_supTask1.loadMyProperties(propertySetA1) != GCF_NO_ERROR)
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

    case F_MYLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_supTask1.loadMyProperties(propertySetB2) != GCF_NO_ERROR)
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
              (&p == &_supTask1.getPort()))
          {
            if (_supTask2.loadMyProperties(propertySetB3) != GCF_NO_ERROR)
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
            if (_supTask2.loadMyProperties(propertySetB4) != GCF_NO_ERROR)
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
            if (_supTask1.unloadMyProperties(propertySetA1.scope) != GCF_NO_ERROR)
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
    case F_MYUNLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 5:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_supTask1.unloadMyProperties(propertySetB2.scope) != GCF_NO_ERROR)
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
              (&p == &_supTask1.getPort()))
          {
            if (_supTask2.unloadMyProperties(propertySetB3.scope) != GCF_NO_ERROR)
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
            if (_supTask2.unloadMyProperties(propertySetB4.scope) != GCF_NO_ERROR)
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
    case F_TIMER_SIG:
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
    case F_ENTRY_SIG:
      if (_supTask1.loadMyProperties(propertySetA1) != GCF_NO_ERROR)
      {
        failed(105);
        TRAN(Application::test201);
      }
      else
        _counter = 1;
      break;

    case F_MYLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetA1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask1.getPort()))
          {
            if (_supTask1.loadMyProperties(propertySetB2) != GCF_NO_ERROR)
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
              (&p == &_supTask1.getPort()))
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
    case F_TIMER_SIG:
      if (!ready) {failed(105); ready = true;}
      else TRAN(Application::test201);
      break;

    case F_EXIT_SIG:
    {
      TSTTestreadyEvent r(105);
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

GCFEvent::TResult Application::test201(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      skiped(201);
      TRAN(Application::test202);
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test202(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      skiped(202);
      TRAN(Application::test203);
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test203(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      skiped(203);
      TRAN(Application::test204);
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test204(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      skiped(204);
      TRAN(Application::test205);
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
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
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      if (_curRemoteTestNr == 0)
        _curRemoteTestNr = pIndication->testnr;
      else
        break;
    }
    case F_ENTRY_SIG:
      if (_curRemoteTestNr == 0) break;
      if (_supTask2.loadAPC("ApcT1-D", "A_C") != GCF_NO_ERROR)
      {
        failed(205);
        TRAN(Application::test206);
      }
      _counter = 1;
      break;

    case F_APCLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1-D") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask2.getPort()))
          {
			CHECK_DB;
            
            if (_supTask2.unloadAPC("ApcT1", "A_C") != GCF_NO_ERROR)
            {
              failed(204);
              TRAN(Application::test205);
            }
            else
            {
              TSTTestreadyEvent r(205);
              _supTask1.getPort().send(r);
            }
          }
          _counter++;
          break;
      }
      break;
    }      
      
    case F_APCUNLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
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

    case F_EXIT_SIG:
    {
      TSTTestreadyEvent r(205);
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
    case F_ENTRY_SIG:
      _counter = 0;
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
      if (_supTask2.loadAPC("ApcT1-D", "A_C") != GCF_NO_ERROR)
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
    case F_APCLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 1)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1-D") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask2.getPort()))
        {
          CHECK_DB
          
          if (_supTask2.unloadAPC("ApcT1", "A_C") != GCF_NO_ERROR)
          {
            failed(206);
            TRAN(Application::test207);
          }
          else
          {
            TSTTestreadyEvent r(206);
            _supTask1.getPort().send(r);
            _counter++;
          }
        }          
      }
      break;
    }      
      
    case F_APCUNLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 2)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask2.getPort()))
        {
          CHECK_DB
          passed(206);          
        }
        else
          failed(206);
        TRAN(Application::test207);  
      }
      break;
    }      

    case F_EXIT_SIG:
    {
      TSTTestreadyEvent r(206);
      _supTask1.getPort().send(r);
      break;
    }
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test207(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      skiped(207);
      TRAN(Application::test208);
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
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
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
    }
    case F_ENTRY_SIG:
      if (_curRemoteTestNr != 208) break;

      if (_supTask1.loadAPC("ApcT3", "A_H") != GCF_NO_ERROR)
      {
        failed(208);
        TRAN(Application::test301);
      }
      else
      {
        _counter = 1;
      }      
      break;
    case F_APCLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 1)
      {
        if ((strcmp(pResponse->pScope, "A_H") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask1.getPort()))
        {
          CHECK_DB
          
          if (_supTask1.unloadAPC("ApcT3", "A_H") != GCF_NO_ERROR)
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
      
    case F_APCUNLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
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

GCFEvent::TResult Application::test209(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test210(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
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

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      if (_supTask1.unloadMyProperties(propertySetB1.scope) != GCF_NO_ERROR)
      {
        failed(301);
        TRAN(Application::test302);
      }
      else
        _counter = 1;
      break;

    case F_MYUNLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        if (_supTask1.loadMyProperties(propertySetB1) != GCF_NO_ERROR)
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

    case F_MYLOADED_SIG:
    {
      GCFPMLMYPAnswerEvent* pResponse = static_cast<GCFPMLMYPAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        if (_supTask1.get("A_C_P1") != GCF_NO_ERROR)
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

    case F_VGETRESP_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:    
          if ((pResponse->pValue->getType() == GCFPValue::INTEGER_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
              (((GCFPVInteger*)pResponse->pValue)->getValue() == 11) &&
              (&p == &_supTask1.getPort()))
          {
            GCFPVInteger iv(12);
            if (_supTask1.set("A_C_P1", iv ) != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else if (_supTask1.get("A_C_P1") != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else
              _counter = 2;            
          }
          else
          {
            failed(301);
            TRAN(Application::test302);
          }
          break;
        case 2:
          if ((pResponse->pValue->getType() == GCFPValue::INTEGER_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
              (((GCFPVInteger*)pResponse->pValue)->getValue() == 12) &&
              (&p == &_supTask1.getPort()))
          {
            if (_supTask1.get("A_C_P2") != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else
              _counter = 3;
          }
          else
          {
            failed(301);
            TRAN(Application::test302);
          }
          break;
        case 3:    
          if ((pResponse->pValue->getType() == GCFPValue::CHAR_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P2") == 0) &&
              (((GCFPVChar*)pResponse->pValue)->getValue() == 25) &&
              (&p == &_supTask1.getPort()))
          {
            GCFPVChar cv(26);
            if (_supTask1.set("A_C_P2", cv ) != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else if (_supTask1.get("A_C_P2") != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else
              _counter = 4;            
          }
          else
          {
            failed(301);
            TRAN(Application::test302);
          }
          break;
        case 4:
          if ((pResponse->pValue->getType() == GCFPValue::CHAR_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P2") == 0) &&
              (((GCFPVChar*)pResponse->pValue)->getValue() == 26) &&
              (&p == &_supTask1.getPort()))
          {
            if (_supTask1.get("A_C_P3") != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else
              _counter = 5;
          }
          else
          {
            failed(301);
            TRAN(Application::test302);
          }
          break;
        case 5:    
          if ((pResponse->pValue->getType() == GCFPValue::DOUBLE_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P3") == 0) &&
              (((GCFPVDouble*)pResponse->pValue)->getValue() == 33.3) &&
              (&p == &_supTask1.getPort()))
          {
            GCFPVDouble dv(33.4);
            if (_supTask1.set("A_C_P3", dv ) != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else if (_supTask1.get("A_C_P3") != GCF_NO_ERROR)
            {
              failed(301);
              TRAN(Application::test302);
            }
            else
              _counter = 6;
          }
          else
          {
            failed(301);
            TRAN(Application::test302);
          }
          break;
        case 6:
          if ((pResponse->pValue->getType() == GCFPValue::DOUBLE_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P3") == 0) &&
              (((GCFPVDouble*)pResponse->pValue)->getValue() == 33.4) &&
              (&p == &_supTask1.getPort()))
          {
            passed(301);
          }
          else
          {
            failed(301);
          }
          TRAN(Application::test302);
          break;
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
    case F_ENTRY_SIG:
      if (_supTask1.loadAPC("ApcT1", "A_C") != GCF_NO_ERROR)
      {
        failed(302);
        TRAN(Application::test303);
      }
      break;

    case F_APCLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pScope, "A_C") == 0) &&
          (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
          (pResponse->result == GCF_NO_ERROR) &&
          (&p == &_supTask1.getPort()))
      {
        CHECK_DB
        GCFPVInteger iv(22);
        if (_supTask2.set("A_C_P1", iv) != GCF_NO_ERROR)
        {
          failed(302);
          TRAN(Application::test303);
        }
        else if (_supTask2.get("A_C_P1") != GCF_NO_ERROR)
        {
          failed(302);
          TRAN(Application::test303);
        }      
        else
          _counter = 1;
      }
      break;
    }      
    case F_VGETRESP_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:  
          if ((pResponse->pValue->getType() == GCFPValue::INTEGER_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
              (((GCFPVInteger*)pResponse->pValue)->getValue() == 22) &&
              (&p == &_supTask2.getPort()))
          {
            GCFPVChar cv(22);
            if (_supTask2.set("A_C_P2", cv) != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }
            else if (_supTask2.get("A_C_P2") != GCF_NO_ERROR)
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
          if ((pResponse->pValue->getType() == GCFPValue::CHAR_VAL) &&
              (strcmp(pResponse->pPropName, "A_C_P2") == 0) &&
              (((GCFPVChar*)pResponse->pValue)->getValue() == 22) &&
              (&p == &_supTask2.getPort()))
          {
            GCFPVDouble dv(22.0);
            if (_supTask2.set("A_C_P3", dv) != GCF_NO_ERROR)
            {
              failed(302);
              TRAN(Application::test303);
            }
            else if (_supTask2.get("A_C_P3") != GCF_NO_ERROR)
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
          if ((pResponse->pValue->getType() == GCFPValue::DOUBLE_VAL) &&
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
    case F_VCHANGEMSG_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
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
    case F_EXIT_SIG:
    {
      TSTTestreadyEvent r(302);
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
    case F_ENTRY_SIG:
      break;

    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
      if (_curRemoteTestNr == 303) 
      {
        GCFPVInteger iv(22);
        if (_supTask1.set("A_C_P1", iv) != GCF_NO_ERROR)
        {
          failed(303);
          TRAN(Application::test304);
        }
      }
      break;
    }
    case F_VCHANGEMSG_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::INTEGER_VAL) &&
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
    case F_ENTRY_SIG:
      _counter = 0;
      if (_supTask1.loadAPC("ApcT3", "A_H") != GCF_NO_ERROR)
      {
        failed(304);
        TRAN(Application::test305);
      }
      break;

    case F_APCLOADED_SIG:
    {
      GCFPMLAPCAnswerEvent* pResponse = static_cast<GCFPMLAPCAnswerEvent*>(&e);
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
            if (_supTask1.getProxy().subscribe(propName) == GCF_NO_ERROR)
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
    case F_SUBSCRIBED_SIG:
    {
      GCFPMLAnswerEvent* pResponse = static_cast<GCFPMLAnswerEvent*>(&e);
      assert(pResponse);
      if ((strncmp(pResponse->pPropName, "A_H_J_P", 7) == 0) &&
          (&p == &_supTask1.getPort()))
      {
        _counter--;
        if (_counter == 0)
        {
          TSTTestreadyEvent r(304);
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
    case F_VCHANGEMSG_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::DOUBLE_VAL) &&
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
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
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

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
    }
    case F_ENTRY_SIG:
      if (_curRemoteTestNr != 305) break;
      _counter = 0;
      if (_supTask1.getProxy().subscribe("A_H_J_P00") != GCF_NO_ERROR)
      {
        failed(305);
        TRAN(Application::test306);
      }
      break;
  
    case F_SUBSCRIBED_SIG:
    {
      GCFPMLAnswerEvent* pResponse = static_cast<GCFPMLAnswerEvent*>(&e);
      assert(pResponse);
      if ((strcmp(pResponse->pPropName, "A_H_J_P00") == 0))
      {
        if (&p == &_supTask1.getPort())
        {
          if (_supTask2.getProxy().subscribe("A_H_J_P00") != GCF_NO_ERROR)
          {
            failed(305);
            TRAN(Application::test306);
          }
        }
        else if (&p == &_supTask2.getPort())
        {
          TSTTestreadyEvent r(305);
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

    case F_VCHANGEMSG_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      if ((pResponse->pValue->getType() == GCFPValue::DOUBLE_VAL) &&
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
  
  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent* pIndication = static_cast<TSTTestreadyEvent*>(&e);
      assert(pIndication);
      _curRemoteTestNr = pIndication->testnr;
    }
    case F_ENTRY_SIG:
      if (_curRemoteTestNr != 306) break;
      
      if (_supTask1.getProxy().subscribe("A_C_P1") != GCF_NO_ERROR)
      {
        failed(306);
        TRAN(Application::test403);
      }
      break;
  
    case F_SUBSCRIBED_SIG:
    {
      GCFPMLAnswerEvent* pResponse = static_cast<GCFPMLAnswerEvent*>(&e);
      assert(pResponse);
      _counter = 0;
      if ((strcmp(pResponse->pPropName, "A_C_P1") == 0))
      {
        GCFPVInteger iv(_counter);
        if (_supTask1.set("A_K_P1", iv) != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::test403);
        }
      }
      else
      {
        failed(306);
        TRAN(Application::test403);
      }
      break;
    }

    case F_VCHANGEMSG_SIG:
    {
      GCFPMLValueEvent* pResponse = static_cast<GCFPMLValueEvent*>(&e);
      assert(pResponse);
      if (pResponse->internal) break;
      if ((pResponse->pValue->getType() == GCFPValue::INTEGER_VAL) &&
          (strcmp(pResponse->pPropName, "A_C_P1") == 0) &&
          (((GCFPVInteger*)pResponse->pValue)->getValue() == _counter))
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
        TRAN(Application::test403);
      }
      else if (nrOfSucceded + nrOfFaults == 1000)
      {
        failed(306);
        TRAN(Application::test403);
      }
      else
      {
        GCFPVInteger iv(_counter);
        if (_supTask1.set("A_K_P1", iv) != GCF_NO_ERROR)
        {
          failed(306);
          TRAN(Application::test403);
        }
      }        
      break;
    }

    case F_EXIT_SIG:
    {
      TSTTestreadyEvent r(306);
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
    case F_ENTRY_SIG:
      _supTask1.getPort().init(_supTask1, "client", GCFPortInterface::SAP, TST_PROTOCOL);
      _supTask1.getPort().open();
      break;

    case F_TIMER_SIG:
      _supTask1.getPort().open();
      break;

    case F_CONNECTED_SIG:
    {
      TSTTestreqEvent r(401);
      _supTask1.getPort().send(r);
      break;
    }
    case F_DISCONNECTED_SIG:
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
    case F_ENTRY_SIG:
      closing = true;
      break;

    case F_TIMER_SIG:
      if (&p == &_supTask1.getPort())
        _supTask1.getPort().open();
      if (&p == &_supTask2.getPort())
        _supTask2.getPort().open();
      break;

    case F_CONNECTED_SIG:
    {
      if (_supTask1.getPort().isConnected())
      {
        if (_supTask2.getPort().isConnected())
        {
          TSTTestreqEvent r(402);
          _supTask1.getPort().send(r);
        }
        else
          _supTask2.getPort().open();
      }        
      break;
    }

    case F_DISCONNECTED_SIG:
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
        TSTTestreqEvent r(402);
        _supTask2.getPort().send(r);
      }
      else
      {
        TSTTestreadyEvent r(402);
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

GCFEvent::TResult Application::test403(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
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
