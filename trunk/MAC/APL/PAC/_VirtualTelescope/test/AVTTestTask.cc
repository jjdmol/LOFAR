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

#include <math.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTTest.h"
#include "AVTTestTask.h"
#include "PropertyDefines.h" 

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "../src/LogicalDevice_Protocol.ph"
#include "../src/ABS_Protocol.ph"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace AVT;
using namespace std;

const double pi(asin(1.0)*2.0);

double generateWave(int subband,double angle1,double angle2)
{
  const double amplitude(subband==10?2e4:2e1);
  return (amplitude * pow(1.0+cos(angle1*pi),2.0) * pow(1.0+cos(angle2*pi),2.0) );
}

string AVTTestTask::m_taskName("AVTTest");
bool   AVTTestTask::m_sBeamServerOnly(false);
string gBSName(BSNAME);

AVTTestTask::AVTTestTask(AVTTest<AVTTestTask>& tester) :
  GCFTask((State)&AVTTestTask::initial, m_taskName),
  m_tester(tester),
  m_answer(),
  m_beamserver(*this, gBSName, GCFPortInterface::SPP, ABS_PROTOCOL),
  m_propertyLDScommand(string(PROPERTY_LDS_COMMAND)),
  m_propertyLDSstatus(string(PROPERTY_LDS_STATUS)),
  m_propertyLDSWGFrequency(string(PROPERTY_LDS_WG_FREQUENCY)),
  m_propertyLDSWGAmplitude(string(PROPERTY_LDS_WG_AMPLITUDE)),
  m_propertyLDSWGSamplePeriod(string(PROPERTY_LDS_WG_SAMPLEPERIOD)),
  m_propertySBFdirectionType(string(PROPERTY_SBF_DIRECTIONTYPE)),
  m_propertySBFdirectionAngle1(string(PROPERTY_SBF_DIRECTIONANGLE1)),
  m_propertySBFdirectionAngle2(string(PROPERTY_SBF_DIRECTIONANGLE2)),
  m_propertySBFstatus(string(PROPERTY_SBF_STATUS)),
  m_beamServerProperties(propertySetBeamServer,&m_answer),
  m_BEAMALLOC_received(false),
  m_BEAMFREE_received(false),
  m_BEAMPOINTTO_received(false),
  m_WGSETTINGS_received(false),
  m_WGENABLE_received(false),
  m_WGDISABLE_received(false),
  m_statisticsTimerID(0xffffffff),
  m_beamAngle1(0.0),
  m_beamAngle2(0.0),
  m_seqnr(0)
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  
  m_answer.setTask(this);
  m_propertyLDScommand.setAnswer(&m_answer);
  m_propertyLDSstatus.setAnswer(&m_answer);
  m_propertyLDSWGFrequency.setAnswer(&m_answer);
  m_propertyLDSWGAmplitude.setAnswer(&m_answer);
  m_propertyLDSWGSamplePeriod.setAnswer(&m_answer);
  m_propertySBFdirectionType.setAnswer(&m_answer);
  m_propertySBFdirectionAngle1.setAnswer(&m_answer);
  m_propertySBFdirectionAngle2.setAnswer(&m_answer);
  m_propertySBFstatus.setAnswer(&m_answer);
}

AVTTestTask::~AVTTestTask()
{
}

GCFEvent::TResult AVTTestTask::initial(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_beamServerProperties.load();
      m_beamserver.open(); // start listening
      break;
    
    case F_MYPLOADED:
      if(m_sBeamServerOnly)
      {
        TRAN(AVTTestTask::beamServer);
      }
      else
      {
        TRAN(AVTTestTask::propertiesLoaded);
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTTestTask::propertiesLoaded(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::propertiesLoaded (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_TIMER: // intentional fall through
    case F_ENTRY:
      if(GCF_PML_ERROR==m_propertyLDSstatus.subscribe())
      {
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s): unable to subscribe to LogicalDeviceProperties",getName().c_str()));
        m_beamserver.setTimer(1.0); // abuse the beamserver to set a timert
      }
      else
      {
        TRAN(AVTTestTask::test1);
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::propertiesLoaded, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test1 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      // ignore during first increments
      m_tester._avttest(true);
      TRAN(AVTTestTask::test2);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test1, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test3 (%d)",getName().c_str(),event.signal));
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
        string devices(string(VTNAME)+string(",")+string(SBFNAME)+string(",")+string(SRGNAME)+string(","));
        string times("0,0,"); // from now until forever
        string freq("110.0,");
        string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
        string direction("AZEL,0.0,0.0");
        GCFPVString command(cmd+devices+times+freq+subbands+direction);
        if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
        {
          m_tester._avttest(false);
          TRAN(AVTTestTask::test4);
        }
        break;
      }
      
      case F_VCHANGEMSG:
      {
        GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&event);
        assert(pResponse);
        string expectedProperty=m_propertyLDSstatus.getFullName();
        if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
        {
          string statusValue=((GCFPVString*)pResponse->pValue)->getValue();
          LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test3 status changed (%s)",getName().c_str(),statusValue.c_str()));
          if(statusValue == string("Resumed"))
          {
            m_tester._avttest(true);
            TRAN(AVTTestTask::test3);
          }
          else if(statusValue != string("Claimed") &&
                  statusValue != string("Prepared"))
          {
            m_tester._avttest(false);
            TRAN(AVTTestTask::test3);
          }
        }
        break;
      }
      
      case F_DISCONNECTED:
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test4);
        break;
      }
              
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test3, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test2 (%s)",getName().c_str(),evtstr(event)));
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
        GCFPVUnsigned samplePeriod(2);
        if (m_propertyLDSWGFrequency.setValue(frequency) != GCF_NO_ERROR ||
            m_propertyLDSWGAmplitude.setValue(amplitude) != GCF_NO_ERROR ||
            m_propertyLDSWGSamplePeriod.setValue(samplePeriod) != GCF_NO_ERROR)
        {
          m_tester._avttest(false);
          TRAN(AVTTestTask::test4);
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
          m_tester._avttest(true);
          TRAN(AVTTestTask::test4);
        }
        else
        {
          m_beamserver.setTimer(1.0);
        }
        break;
      }
      
      case F_DISCONNECTED:
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test4);
        break;
      }
              
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test2, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test4 (%s)",getName().c_str(),evtstr(event)));
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
        m_tester._avttest(false);
        TRAN(AVTTestTask::test5);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&event);
      assert(pResponse);
      string expectedProperty=m_propertyLDSstatus.getFullName();
      if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
      {
        if(((GCFPVString*)pResponse->pValue)->getValue() == string("OK"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(true);
        }
        TRAN(AVTTestTask::test5);
      }
      break;
    }

    case F_DISCONNECTED:
    {
      m_tester._avttest(false);
      TRAN(AVTTestTask::test5);
      break;
    }
            
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test4, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test5 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test6);
      break;

      case F_DISCONNECTED:
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test6);
        break;
      }
              
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test5, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test6 (%s)",getName().c_str(),evtstr(event)));
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
        if(m_propertySBFdirectionType.setValue(directionType) != GCF_NO_ERROR ||
           m_propertySBFdirectionAngle1.setValue(directionAngle1) != GCF_NO_ERROR ||
           m_propertySBFdirectionAngle2.setValue(directionAngle2) != GCF_NO_ERROR)
        {
          m_tester._avttest(false);
          TRAN(AVTTestTask::test7);
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
          m_tester._avttest(true);
          TRAN(AVTTestTask::test7);
        }
        break;
      }
      
      case F_DISCONNECTED:
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test7);
        break;
      }
              
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test6, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test7 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test8);
      break;

    case F_DISCONNECTED:
    {
      m_tester._avttest(false);
      TRAN(AVTTestTask::test8);
      break;
    }
            
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test7, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test8 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test9);
      break;

    case F_DISCONNECTED:
    {
      m_tester._avttest(false);
      TRAN(AVTTestTask::test9);
      break;
    }
            
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test8, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test9 (%s)",getName().c_str(),evtstr(event)));
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
        string devices(string(VTNAME));
        GCFPVString command(cmd+devices);
        if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
        {
          m_tester._avttest(false);
          TRAN(AVTTestTask::finished);
        }
        break;
      }
      
      case F_VCHANGEMSG:
      {
        GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&event);
        assert(pResponse);
        string expectedProperty=m_propertyLDSstatus.getFullName();
        if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
        {
          string statusValue=((GCFPVString*)pResponse->pValue)->getValue();
          LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test9 status changed (%s)",getName().c_str(),statusValue.c_str()));
          if(statusValue == string("Released"))
          {
            m_tester._avttest(true);
            TRAN(AVTTestTask::finished);
          }
        }
        break;
      }

      case F_DISCONNECTED:
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::finished);
        break;
      }
              
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test9, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::finished (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_propertyLDSstatus.unsubscribe();
      GCFTask::stop();
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::finished, default",getName().c_str()));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMALLOC)",getName().c_str(),event.signal));
      ABSBeamallocAckEvent ack;
      ack.handle=0;
      ack.status=SUCCESS;
      p.send(ack);
      m_BEAMALLOC_received=true;
      break;
    }
     
    case ABS_BEAMFREE:
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMFREE)",getName().c_str(),event.signal));
      ABSBeamfreeAckEvent ack;
      ack.handle=0;
      ack.status=SUCCESS;
      p.send(ack);
      m_BEAMFREE_received=true;
      break;
    }
     
    case ABS_BEAMPOINTTO:
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_BEAMPOINTTO)",getName().c_str(),event.signal));
      m_BEAMPOINTTO_received=true;
      
      ABSBeampointtoEvent* pPointToEvent=static_cast<ABSBeampointtoEvent*>(&event);
      if(pPointToEvent!=0)
      {
        m_beamAngle1=pPointToEvent->angle[0];
        m_beamAngle2=pPointToEvent->angle[1];
      }
      break;
    }
     
    case ABS_WGSETTINGS:
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGSETTINGS)",getName().c_str(),event.signal));
      ABSWgsettingsAckEvent ack;
      ack.status=SUCCESS;
      p.send(ack);
      m_WGSETTINGS_received=true;
      break;
    }
    
    case ABS_WGENABLE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGENABLE)",getName().c_str(),event.signal));
      m_WGENABLE_received=true;
      break;
      
    case ABS_WGDISABLE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (ABS_WGDISABLE)",getName().c_str(),event.signal));
      m_WGDISABLE_received=true;
      break;

    case F_TIMER:
    {
      GCFTimerEvent *pTimerEvent=static_cast<GCFTimerEvent*>(&event);
      if(pTimerEvent!=0)
      {
        if(m_statisticsTimerID==pTimerEvent->id)
        {
          for(int i=0;i<127;i++)
          {          
            GCFPVDouble wavePower(generateWave(i,m_beamAngle1,m_beamAngle2));
            char strPropertyPowerX[100];
            char strPropertyPowerY[100];
            sprintf(strPropertyPowerX,"PAC_BeamServer_power%03d_x",i);
            sprintf(strPropertyPowerY,"PAC_BeamServer_power%03d_y",i);
            m_beamServerProperties.setValue(string(strPropertyPowerX),wavePower);
            m_beamServerProperties.setValue(string(strPropertyPowerY),wavePower);
          }
          GCFPVUnsigned seqnr(m_seqnr++);
          m_beamServerProperties.setValue(string("PAC_BeamServer_seqnr"),seqnr);
          m_statisticsTimerID=m_beamserver.setTimer(3.0); // statistics timer
        }
      }
      status = GCFEvent::NOT_HANDLED; // pass timer event to other handlers
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::beamServer (%d)",getName().c_str(),event.signal));
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
        m_statisticsTimerID=m_beamserver.setTimer(3.0); // statistics update
        break;
      
      case F_CONNECTED:
        break;
              
      case F_DISCONNECTED:
        p.close();
        break;
              
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::beamServer, default",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  return status;
}
