//#  AVTTestTask.cc: Implementation of the Virtual Telescope test
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <stdio.h>
#include <math.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include <APLCommon/APL_Defines.h>
#include "AVTTestTask.h"
#include "../src/AVTPropertyDefines.h" 

#define DECLARE_SIGNAL_NAMES
#include "../src/LogicalDevice_Protocol.ph"
#include <ABS_Protocol.ph>


#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(AVTTestTask::test##_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
    TRAN(AVTTestTask::finished); \
  }

#define ABORT_TESTS \
  { \
    cout << "TESTS ABORTED due to an ERROR or terminated" << endl; \
    FINISH; \
  }

#define FAIL_AND_ABORT(_txt_) \
  { \
    FAIL(_txt_);  \
    ABORT_TESTS; \
  }

#define TESTC_ABORT_ON_FAIL(cond) \
  if (!TESTC(cond)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#define TESTC_DESCR_ABORT_ON_FAIL(cond, _descr_) \
  if (!TESTC_DESCR(cond, _descr_)) \
  { \
    ABORT_TESTS; \
    break; \
  }

using namespace LOFAR;
using namespace AVT;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

string AVTTestTask::m_taskName("AVTTest");
bool   AVTTestTask::m_sBeamServerOnly(false);
string gBSName("BeamServer");

AVTTestTask::AVTTestTask() :
  GCFTask((State)&AVTTestTask::initial, m_taskName),
  Test(m_taskName),
  m_answer(),
  m_beamserver(*this, gBSName, GCFPortInterface::MSPP, ABS_PROTOCOL),
  m_extPropsetLDS(SCOPE_PAC_LogicalDeviceScheduler, TYPE_LCU_PAC_LogicalDeviceScheduler.c_str(), &m_answer),
  m_extPropsetLDSWG(SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator, TYPE_LCU_PAC_WaveformGenerator.c_str(), &m_answer),
  m_extPropsetSBF1("PAC_VT1_BF1",TYPE_LCU_PAC_BF.c_str(),&m_answer),
  m_propsetLDloaded(false),
  m_propsetLDWGloaded(false),
  m_propsetSBFloaded(false),
  m_BEAMALLOC_received(false),
  m_BEAMFREE_received(false),
  m_BEAMPOINTTO_received(false),
  m_WGSETTINGS_received(false),
  m_WGENABLE_received(false),
  m_WGDISABLE_received(false),
  m_beamAngle1(0.0),
  m_beamAngle2(0.0),
  m_seqnr(0),
  m_client_list()
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  
  m_answer.setTask(this);
  
  m_extPropsetLDS.load();
  m_extPropsetLDSWG.load();
}

AVTTestTask::~AVTTestTask()
{
}

void AVTTestTask::run()
{
  start(); // make initial transition
  GCFTask::run();
}

GCFEvent::TResult AVTTestTask::initial(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_beamserver.open(); // start listening
      break;
    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&event);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        if(strstr(pPropAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator) != 0)
        {
          m_propsetLDWGloaded = true;
        }
        else if(strstr(pPropAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler) != 0)
        {
          m_propsetLDloaded = true;
        }
      }
      if(m_propsetLDloaded && m_propsetLDWGloaded)
      {
        if(m_sBeamServerOnly)
        {
          TRAN(AVTTestTask::beamServer);
        }
        else
        {
          TRAN(AVTTestTask::propertiesLoaded);
        }
      }
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTTestTask::propertiesLoaded(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::propertiesLoaded (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_TIMER: // intentional fall through
    case F_ENTRY:
      if(GCF_NO_ERROR!=m_extPropsetLDS.subscribeProp(PROPNAME_STATUS))
      {
        LOG_DEBUG(formatString("AVTTestTask(%s): unable to subscribe to LogicalDeviceProperties",getName().c_str()));
        m_beamserver.setTimer(1.0); // abuse the beamserver to set a timert
      }
      else
      {
        NEXT_TEST(1,"set antenna orhtogonalization weights");
      }
      break;

    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::propertiesLoaded, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set antenna orhtogonalization weights
 */
GCFEvent::TResult AVTTestTask::test1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test1 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      // ignore during first increments
      TESTC(true);
      NEXT_TEST(2,"create virtual telescope");
      break;

    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::test1, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: create virtual telescope
 */
GCFEvent::TResult AVTTestTask::test2(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test3 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status = handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status = GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT:
        break;

      case F_ENTRY:
      {
        // SCHEDULE <scheduleid>,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
        //          <frequency>,<subbands>,<direction>
        string cmd("SCHEDULE 1,");
        string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
        string times("0,0,"); // from now until forever
        string freq("110.0,");
        string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
        string direction("AZEL,0.0,0.0");
        GCFPVString command(cmd+devices+times+freq+subbands+direction);
        if(!TESTC(m_extPropsetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
        {
          NEXT_TEST(3,"initialize EPA waveform generator");
        }
        break;
      }
      
      case F_VCHANGEMSG:
      {
        GCFPropValueEvent* pResponse=static_cast<GCFPropValueEvent*>(&event);
        if(strstr(pResponse->pPropName, SCOPE_PAC_LogicalDeviceScheduler) == 0)
        {
          string statusValue=((GCFPVString*)pResponse->pValue)->getValue();
          LOG_DEBUG(formatString("AVTTestTask(%s)::test3 status changed (%s)",getName().c_str(),statusValue.c_str()));
          if(statusValue == string("Resumed"))
          {
          	// subscribe to PAC_VT1_BF1
        	  m_extPropsetSBF1.load();
          }
          else if(statusValue != string("Claimed") &&
                  statusValue != string("Prepared"))
          {
            TESTC(false);
            NEXT_TEST(3,"initialize EPA waveform generator");
          }
        }
        break;
      }
      
	    case F_EXTPS_LOADED:
	    {
	      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&event);
	      if(pPropAnswer->result == GCF_NO_ERROR)
	      {
	        if(strstr(pPropAnswer->pScope, "PAC_VT1_BF1") != 0)
	        {
	          m_propsetSBFloaded = true;
            TESTC(true);
            NEXT_TEST(3,"initialize EPA waveform generator");
	        }
	      }
	      else
        {
          TESTC(false);
          NEXT_TEST(3,"initialize EPA waveform generator");
        }
	      break;
	    }
      case F_DISCONNECTED:
      {
        TESTC(false);
        NEXT_TEST(3,"initialize EPA waveform generator");
        break;
      }
              
      default:
        LOG_DEBUG(formatString("AVTTestTask(%s)::test3, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  return status;
}

/* 
 * Test case: initialize EPA waveform generator
 */
GCFEvent::TResult AVTTestTask::test3(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test2 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status=handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status=GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT:
        break;
  
      case F_ENTRY:
      {
        m_WGSETTINGS_received=false;
        GCFPVDouble frequency(1000000.0);
        GCFPVUnsigned amplitude(128);
        if(!TESTC(m_extPropsetLDSWG.setValue(PROPNAME_FREQUENCY,frequency) != GCF_NO_ERROR ||
            m_extPropsetLDSWG.setValue(PROPNAME_AMPLITUDE,amplitude) != GCF_NO_ERROR))
        {
          NEXT_TEST(4,"select beamlets");
        }
        else
        {
          // check if the beamserver receives the messages
          m_beamserver.setTimer(1.0);
        }
        break;
      }

      case F_TIMER:
      {
        if(m_WGSETTINGS_received==true)
        {
          m_WGSETTINGS_received=false;
          TESTC(true);
          NEXT_TEST(4,"select beamlets");
        }
        else
        {
          m_beamserver.setTimer(1.0);
        }
        break;
      }
      
      case F_DISCONNECTED:
      {
        TESTC(false);
        NEXT_TEST(4,"select beamlets");
        break;
      }
              
      default:
        LOG_DEBUG(formatString("AVTTestTask(%s)::test2, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  return status;
}

/* 
 * Test case: select beamlets
 */
GCFEvent::TResult AVTTestTask::test4(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test4 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
//      GCFPVString command(string("0"));
//      if (m_propertySBFsubbandSelection.setValue(command) != GCF_NO_ERROR)
      {
        TESTC(true);
        NEXT_TEST(5,"select subbands");
      }
      break;
    }
    
    case F_DISCONNECTED:
    {
      TESTC(false);
      NEXT_TEST(5,"select subbands");
      break;
    }
            
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::test4, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: select subbands
 */
GCFEvent::TResult AVTTestTask::test5(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test5 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      TESTC(true);
      NEXT_TEST(6,"set beam direction");
      break;

      case F_DISCONNECTED:
      {
        TESTC(false);
        NEXT_TEST(6,"set beam direction");
        break;
      }
              
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::test5, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set beam direction
 */
GCFEvent::TResult AVTTestTask::test6(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test6 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status=handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status=GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT:
        break;
  
      case F_ENTRY:
      {
        m_BEAMPOINTTO_received=false;
        GCFPVString directionType(string("AZEL"));
        GCFPVDouble directionAngle1(0.45);
        GCFPVDouble directionAngle2(0.65);
        if(!TESTC(m_extPropsetSBF1.setValue(PROPNAME_DIRECTIONTYPE,directionType) != GCF_NO_ERROR ||
                  m_extPropsetSBF1.setValue(PROPNAME_DIRECTIONANGLE1,directionAngle1) != GCF_NO_ERROR ||
                  m_extPropsetSBF1.setValue(PROPNAME_DIRECTIONANGLE2,directionAngle2) != GCF_NO_ERROR))
        {
          NEXT_TEST(7,"monitor statistics");
        }
        else
        {
          // periodically check if the message has been received by the beamserver
          m_beamserver.setTimer(1.0);
        }
        break;
      }

      case F_TIMER:
      {
        if(m_BEAMPOINTTO_received==true)
        {
          m_BEAMPOINTTO_received=false;
          TESTC(true);
          NEXT_TEST(7,"monitor statistics");
        }
        break;
      }
      
      case F_DISCONNECTED:
      {
        TESTC(false);
        NEXT_TEST(7,"monitor statistics");
        break;
      }
              
      default:
        LOG_DEBUG(formatString("AVTTestTask(%s)::test6, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  return status;
}

/* 
 * Test case: monitor statistics
 */
GCFEvent::TResult AVTTestTask::test7(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test7 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      TESTC(true);
      NEXT_TEST(8,"void");
      break;

    case F_DISCONNECTED:
    {
      TESTC(false);
      NEXT_TEST(8,"void");
      break;
    }
            
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::test7, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: to be done
 */
GCFEvent::TResult AVTTestTask::test8(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test8 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      TESTC(true);
      NEXT_TEST(9,"cleanup");
      break;

    case F_DISCONNECTED:
    {
      TESTC(false);
      NEXT_TEST(9,"cleanup");
      break;
    }
            
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::test8, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: cleanup
 */
GCFEvent::TResult AVTTestTask::test9(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::test9 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status = handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status = GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT:
        break;
  
      case F_ENTRY:
      {
        string cmd("RELEASE ");
        string devices("VT1");
        GCFPVString command(cmd+devices);
        if (!TESTC(m_extPropsetLDS.setValue(PROPNAME_COMMAND,command) != GCF_NO_ERROR))
        {
          FINISH;
        }
        break;
      }
      
      case F_VCHANGEMSG:
      {
        GCFPropValueEvent* pResponse=static_cast<GCFPropValueEvent*>(&event);
        if(strstr(pResponse->pPropName, SCOPE_PAC_LogicalDeviceScheduler) == 0)
        {
          string statusValue=((GCFPVString*)pResponse->pValue)->getValue();
          LOG_DEBUG(formatString("AVTTestTask(%s)::test9 status changed (%s)",getName().c_str(),statusValue.c_str()));
          if(TESTC(statusValue == string("Released")))
          {
            FINISH;
          }
        }
        break;
      }

      case F_DISCONNECTED:
      {
        TESTC(false);
        FINISH;
        break;
      }
              
      default:
        LOG_DEBUG(formatString("AVTTestTask(%s)::test9, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }

  return status;
}

/* 
 * End of all tests
 */
GCFEvent::TResult AVTTestTask::finished(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::finished (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_extPropsetLDS.unsubscribeProp(PROPNAME_STATUS);
      GCFTask::stop();
      break;

    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::finished, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTTestTask::handleBeamServerEvents(GCFEvent& event, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case ABS_BEAMALLOC:
    {
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMALLOC)",getName().c_str(),event.signal));
      ABSBeamallocAckEvent ack;
      ack.handle=0;
      ack.status=ABS_Protocol::SUCCESS;
      p.send(ack);
      m_BEAMALLOC_received=true;
      break;
    }
     
    case ABS_BEAMFREE:
    {
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMFREE)",getName().c_str(),event.signal));
      ABSBeamfreeAckEvent ack;
      ack.handle=0;
      ack.status=ABS_Protocol::SUCCESS;
      p.send(ack);
      m_BEAMFREE_received=true;
      break;
    }
     
    case ABS_BEAMPOINTTO:
    {
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMPOINTTO)",getName().c_str(),event.signal));
      m_BEAMPOINTTO_received=true;
      
      ABSBeampointtoEvent pointToEvent(event);
      m_beamAngle1=pointToEvent.angle[0];
      m_beamAngle2=pointToEvent.angle[1];
      break;
    }
     
    case ABS_WGSETTINGS:
    {
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGSETTINGS)",getName().c_str(),event.signal));
      ABSWgsettingsAckEvent ack;
      ack.status=ABS_Protocol::SUCCESS;
      p.send(ack);
      m_WGSETTINGS_received=true;
      break;
    }
    
    case ABS_WGENABLE:
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGENABLE)",getName().c_str(),event.signal));
      m_WGENABLE_received=true;
      break;
      
    case ABS_WGDISABLE:
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGDISABLE)",getName().c_str(),event.signal));
      m_WGDISABLE_received=true;
      break;

    case F_TIMER:
    {
      GCFTimerEvent *pTimerEvent=static_cast<GCFTimerEvent*>(&event);
      if(pTimerEvent!=0)
      {
      }
      status = GCFEvent::NOT_HANDLED; // pass timer event to other handlers
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::handleBeamServerEvents, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * beamserver state
 */
GCFEvent::TResult AVTTestTask::beamServer(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestTask(%s)::beamServer (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status = handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status = GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT:
        break;

      case F_ENTRY:
        break;
      
      case F_ACCEPT_REQ:
      {
        GCFTCPPort* client = new GCFTCPPort();
        client->init(*this, gBSName, GCFPortInterface::SPP, ABS_PROTOCOL);
        m_beamserver.accept(*client);
        m_client_list.push_back(client);
      }
      break;
  
      case F_CONNECTED:
        break;
              
      case F_DISCONNECTED:
        p.close();
        m_client_list.remove(&p);
        break;
              
      default:
        LOG_DEBUG(formatString("AVTTestTask(%s)::beamServer, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  return status;
}
