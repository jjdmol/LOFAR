#include "Application2.h"
#include "Defines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_Property.h>
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

static string sTaskName = "TA2";

Application::Application() :
  GCFTask((State)&Application::initial, sTaskName),
  _supTask3(*this, "ST3"),
  _passed(0),
  _failed(0),
  _counter(0),
  _curRemoteTestNr(0),
  _pSTPort1(0),
  _pSTPort2(0),
  _propertySetC1(propertySetC1, &_supTask3.getAnswerObj()),
  _propertySetD1(propertySetD1, &_supTask3.getAnswerObj()),
  _propertySetB4(propertySetB4, &_supTask3.getAnswerObj()),
  _apcT1("ApcT1", "A_C", &_supTask3.getAnswerObj()),
  _apcT1K("ApcT1", "A_K", &_supTask3.getAnswerObj()),
  _apcT1a("ApcT1a", "A_C", &_supTask3.getAnswerObj()),
  _apcT1b("ApcT1b", "A", &_supTask3.getAnswerObj()),
  _apcT3("ApcT3", "A_H", &_supTask3.getAnswerObj())
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
      TRAN(Application::test401);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test101(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(101);
      TRAN(Application::test102);
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

GCFEvent::TResult Application::test102(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(102);
      TRAN(Application::test103);
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

GCFEvent::TResult Application::test103(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(103);
      TRAN(Application::test104);
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

GCFEvent::TResult Application::test104(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static bool ready = false; 

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
      if (_propertySetC1.load() != GCF_NO_ERROR)
      {
        failed(104);
        TRAN(Application::test105);
      }
      else
      {
        _supTask3.getPort().setTimer(10.0);
        _counter = 1;
      }      
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetC1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_propertySetD1.load() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
            else _counter++;
          }          
          break;

        case 2:
          if ((strcmp(pResponse->pScope, propertySetD1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_propertySetC1.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
            else _counter++;
          }
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
        case 3:
          if ((strcmp(pResponse->pScope, propertySetC1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_propertySetD1.unload() != GCF_NO_ERROR)
            {
              failed(104);
              TRAN(Application::test105);
            }
            else _counter++;
          }
          break;
          
        case 4:
          if ((strcmp(pResponse->pScope, propertySetD1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (!ready)
            {
              passed(104);
              ready = true;
            }
            else TRAN(Application::test105)
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

  switch (e.signal)
  {
    case F_ENTRY:
      if (_propertySetC1.load() != GCF_NO_ERROR)
      {
        failed(105);
        TRAN(Application::test201);
      }
      else
      {
        _counter = 1;
      }      
      break;

    case F_MYPLOADED:
    {
      GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, propertySetC1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_propertySetD1.load() != GCF_NO_ERROR)
            {
              failed(105);
              TRAN(Application::test201);
            }
            else _counter++;
          }          
          break;

        case 2:
          if ((strcmp(pResponse->pScope, propertySetD1.scope) == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
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

GCFEvent::TResult Application::test201(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      //intentional fall through
    }
    case F_ENTRY:
      if (_curRemoteTestNr != 105) break;
      if (_apcT1.load() != GCF_NO_ERROR)
      {
        failed(201);
        TRAN(Application::test202);
      }
      else
      {
        _supTask3.getPort().setTimer(15.0);
        _counter = 1;
      }      
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
              (&p == &_supTask3.getPort()))
          {
            if (_apcT1.unload() != GCF_NO_ERROR)
            {
              failed(201);
              TRAN(Application::test202);
            }
            else _counter++;
          }
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
              (&p == &_supTask3.getPort()))
          {
            if (!ready)
            {
              passed(201);
              ready = true;
            }
            else TRAN(Application::test202);            
          }
          break;          
      }
      break;
    }  
    case F_TIMER:
      if (!ready) {failed(201); ready = true;}
      else TRAN(Application::test202);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Application::test202(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false; 

  switch (e.signal)
  {
    case F_ENTRY:
      if (_apcT1.load(false) != GCF_NO_ERROR)
      {
        failed(202);
        TRAN(Application::test203);
      }
      else
      {
        _supTask3.getPort().setTimer(10.0);
        _counter = 1;
      }      
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
              (&p == &_supTask3.getPort()))
          {           
            CHECK_DB
            if (!ready)
            {
              passed(202);
              ready = true;
            }
            else TRAN(Application::test203);
          }
          break;
      }
      break;
    }      
    case F_TIMER:
      if (!ready) {failed(202); ready = true;}
      else TRAN(Application::test203);
      break;

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
    case F_ENTRY:    
      if (_apcT1.unload() != GCF_NO_ERROR)
      {
        failed(203);
        TRAN(Application::test204);
      }
      else
      {
        _counter = 1;
      }      
      break;

    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 1)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask3.getPort()))
        {           
          CHECK_DB
          passed(203);          
        }
        else 
          failed(203);

        TRAN(Application::test204);
      }
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
    case F_ENTRY:
      if (_apcT1.load() != GCF_NO_ERROR)
      {
        failed(204);
        TRAN(Application::test205);
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
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_apcT1.reload() != GCF_NO_ERROR)
            {
              failed(204);
              TRAN(Application::test205);
            }
            else _counter++;
          }
          break;
      }
      break;
    }      
    case F_APCRELOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 2:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            passed(204);
          }
          else
            failed(204);
          TRAN(Application::test205);
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

GCFEvent::TResult Application::test205(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool ready = false;
  static bool started = false;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      TSTTestreadyEvent r;
      r.testnr = 204;
      assert(_pSTPort1);
      _pSTPort1->send(r);      
      break;
    } 
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (started)
      {
        if (ready) TRAN(Application::test206)
        else ready = true;
      }
      else
      {
        started = true;
        if (_apcT1.unload() != GCF_NO_ERROR)
        {
          failed(205);
          TRAN(Application::test206);
        }
        else
        {
          _counter = 1;
        }      
      }
      break;
    }
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {    
            CHECK_DB
            passed(205);
          }
          else
            failed(205);
          if (ready) TRAN(Application::test206)
          else ready = true;          
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

GCFEvent::TResult Application::test206(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _counter = 0;
      if (_apcT1.load() != GCF_NO_ERROR)
      {
        failed(204);
        TRAN(Application::test205);
      }
      else
      {
        _counter = 1;
      }
      break;
      
    case TST_TESTREADY:
    {
      TSTTestreadyEvent indicationIn(e);
      _curRemoteTestNr = indicationIn.testnr;
      if (_counter == 2)
      {
        if (_apcT1.unload() != GCF_NO_ERROR)
        {
          failed(206);
          TRAN(Application::test207);
        }
        else _counter++;
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
            (&p == &_supTask3.getPort()))
        {
          if (_apcT1.reload() != GCF_NO_ERROR)
          {
            failed(206);
            TRAN(Application::test207);
          }
          else _counter++;
        }
      }
      break;
    }      
    case F_APCRELOADED:
    {
   
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 2)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask3.getPort()))
        {
          TSTTestreadyEvent r;
          r.testnr = 206;
          assert(_pSTPort1);
          _pSTPort1->send(r);      
        }
      }
      break;
    }
    case F_APCUNLOADED:
    {
      GCFAPCAnswerEvent* pResponse = static_cast<GCFAPCAnswerEvent*>(&e);
      assert(pResponse);
      if (_counter == 3)
      {
        if ((strcmp(pResponse->pScope, "A_C") == 0) &&
            (strcmp(pResponse->pApcName, "ApcT1") == 0) &&
            (pResponse->result == GCF_NO_ERROR) &&
            (&p == &_supTask3.getPort()))
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
      if (_apcT1a.load(false) != GCF_NO_ERROR)
      {
        failed(207);
        TRAN(Application::test208);
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
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1a") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_apcT1a.unload() != GCF_NO_ERROR)
            {
              failed(207);
              TRAN(Application::test208);
            }
            else _counter++;
          }
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
              (strcmp(pResponse->pApcName, "ApcT1a") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            passed(207);
          }
          else
            failed(207);
          TRAN(Application::test208);
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

GCFEvent::TResult Application::test208(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:      
      if (_apcT1b.load(false) != GCF_NO_ERROR)
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
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1b") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            if (_apcT1b.unload() != GCF_NO_ERROR)
            {
              failed(208);
              TRAN(Application::test301);
            }
            else _counter++;
          }
          else
          {
            failed(208);
            TRAN(Application::test301);
          }          
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
          if ((strcmp(pResponse->pScope, "A") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT1b") == 0) &&
              (pResponse->result == GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            passed(208);
          }
          else
            failed(208);
          TRAN(Application::test301);
          break;          
      }
      break;
    }  

    case F_EXIT:
    {
      TSTTestreadyEvent r;
      r.testnr = 208;
      assert(_pSTPort1);
      _pSTPort1->send(r);      
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

// Will be skipped due to problems in the design 
GCFEvent::TResult Application::test209(GCFEvent& e, GCFPortInterface& p)
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
      _apcT3.setScope("A_C");
      if (_apcT3.load(false) != GCF_NO_ERROR)
      {
        failed(209);
        TRAN(Application::test210);
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
      switch (_counter)
      {
        case 1:
          if ((strcmp(pResponse->pScope, "A_C") == 0) &&
              (strcmp(pResponse->pApcName, "ApcT3") == 0) &&
              (pResponse->result != GCF_NO_ERROR) &&
              (&p == &_supTask3.getPort()))
          {
            passed(209);
          }
          else
            failed(209);
          TRAN(Application::test210);
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

// Will be skipped due to combinations of other tests coverts this test
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

GCFEvent::TResult Application::test301(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(301);
      TRAN(Application::test302);
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

GCFEvent::TResult Application::test302(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      skipped(302);
      TRAN(Application::test303);
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

GCFEvent::TResult Application::test303(GCFEvent& e, GCFPortInterface& p)
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

GCFEvent::TResult Application::test401(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      _supTask3.getPort().init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
      _supTask3.getPort().open();
      break;

    case F_TIMER:
      _supTask3.getPort().open();
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
      r.testnr = 401;
      _supTask3.getPort().send(r);
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
      _supTask3.getPort().close();
      break;

    case F_TIMER:
      _port.open();
      break;

    case F_CONNECTED:
      _counter = 0;
      break;
      
    case F_ACCEPT_REQ:
      if (_pSTPort1 == 0)
      {
        _pSTPort1 = new GCFTCPPort();
        _pSTPort1->init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
        _port.accept(*_pSTPort1);
      }
      else
      {
        _pSTPort2 = new GCFTCPPort();
        _pSTPort2->init(_supTask3, "server", GCFPortInterface::SPP, TST_PROTOCOL);
        _port.accept(*_pSTPort2);
      }
      break;
      
    case F_DISCONNECTED:
      if (closing)
      {
        _port.init(_supTask3, "server", GCFPortInterface::MSPP, TST_PROTOCOL);
        _port.open();
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
      _port.open();
      closing = false;
      break;

    case TST_TESTREQ:
    {
      TSTTestrespEvent r;
      r.testnr = 402;
      _counter++;
      if (_counter == 1)
      {
        assert(_pSTPort1);
        _pSTPort1->send(r);
      }
      else if (_counter == 2)
      {
        assert(_pSTPort2);
        _pSTPort2->send(r);
      }
      break;
    }  
    case TST_TESTREADY:
    {
      TSTTestreadyEvent r(e);
      assert(_pSTPort1);
      _pSTPort1->send(r);      
      passed(402);
      TRAN(Application::test101);
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
      if (_curRemoteTestNr != 501) break;     
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
