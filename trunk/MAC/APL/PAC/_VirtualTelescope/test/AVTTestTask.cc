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

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTTest.h"
#include "AVTTestTask.h"
#include "PropertyDefines.h" 

#define DECLARE_SIGNAL_NAMES
#include "../src/LogicalDevice_Protocol.ph"
#include "../src/ABS_Protocol.ph"

#include <stdio.h>

string AVTTestTask::m_taskName("AVTTest");
bool   AVTTestTask::m_sBeamServerOnly(false);
string gBSName(BSNAME);

AVTTestTask::AVTTestTask(AVTTest& tester) :
  GCFTask((State)&AVTTestTask::initial, m_taskName),
  m_tester(tester),
  m_answer(),
  m_beamserver(*this, gBSName, GCFPortInterface::SPP, ABS_PROTOCOL),
  m_propertyLDScommand(string(PROPERTY_LDS_COMMAND)),
  m_propertyLDSstatus(string(PROPERTY_LDS_STATUS)),
  m_propertySBFdirectionType(string(PROPERTY_SBF_DIRECTIONTYPE)),
  m_propertySBFdirectionAngle1(string(PROPERTY_SBF_DIRECTIONANGLE1)),
  m_propertySBFdirectionAngle2(string(PROPERTY_SBF_DIRECTIONANGLE2)),
  m_propertySBFstatus(string(PROPERTY_SBF_STATUS)),
  m_beamServerProperties(propertySetBeamServer,&m_answer)
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  
  m_answer.setTask(this);
  m_propertyLDScommand.setAnswer(&m_answer);
  m_propertyLDSstatus.setAnswer(&m_answer);
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_beamServerProperties.load();
      m_beamserver.open(); // start listening
      break;
      
    case F_MYPLOADED_SIG:
      if(m_sBeamServerOnly)
      {
        TRAN(AVTTestTask::beamServer);
      }
      else
      {
        TRAN(AVTTestTask::connected);
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTTestTask::connected(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::connected (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;
      
    case F_ENTRY_SIG:
      m_propertyLDSstatus.subscribe();
      m_propertySBFstatus.subscribe();
      TRAN(AVTTestTask::test1);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::connected, default",getName().c_str()));
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
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
 * Test case: initialize EPA waveform generator
 */
GCFEvent::TResult AVTTestTask::test2(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test2 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test3);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test2, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: create virtual telescope
 */
GCFEvent::TResult AVTTestTask::test3(GCFEvent& event, GCFPortInterface& p)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test3 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  status = handleBeamServerEvents(event,p);
  if(status!=GCFEvent::HANDLED)
  {
    status = GCFEvent::HANDLED;
    switch (event.signal)
    {
      case F_INIT_SIG:
        break;

      case F_ENTRY_SIG:
      {
        // SCHEDULE <vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
        //          <frequency>,<subbands>,<direction>
        string cmd("SCHEDULE ");
        string devices(string(VTNAME)+string(",")+string(SBFNAME)+string(",")+string(SRGNAME)+string(","));
        string times("0,0,");
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
      
      case F_VCHANGEMSG_SIG:
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
            TRAN(AVTTestTask::test4);
          }
          else if(statusValue != string("Claimed") &&
                  statusValue != string("Prepared"))
          {
            m_tester._avttest(false);
            TRAN(AVTTestTask::test4);
          }
        }
        break;
      }
      
      case F_DISCONNECTED_SIG:
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
 * Test case: select beamlets
 */
GCFEvent::TResult AVTTestTask::test4(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test4 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    {
//      GCFPVString command(string("0"));
//      if (m_propertySBFsubbandSelection.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test5);
      }
      break;
    }
    
    case F_VCHANGEMSG_SIG:
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test6);
      break;

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
GCFEvent::TResult AVTTestTask::test6(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test6 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    {
      GCFPVString directionType(string("AZEL"));
      GCFPVDouble directionAngle1(45.0);
      GCFPVDouble directionAngle2(45.0);
      if(m_propertySBFdirectionType.setValue(directionType) != GCF_NO_ERROR ||
         m_propertySBFdirectionAngle1.setValue(directionAngle1) != GCF_NO_ERROR ||
         m_propertySBFdirectionAngle2.setValue(directionAngle2) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test7);
      }
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&event);
      assert(pResponse);
      string expectedProperty=m_propertySBFstatus.getFullName();
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
        TRAN(AVTTestTask::test7);
      }
      break;
    }

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test6, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test8);
      break;

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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test9);
      break;

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
GCFEvent::TResult AVTTestTask::test9(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test9 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::finished);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::test9, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      m_propertyLDSstatus.unsubscribe();
      m_propertySBFstatus.unsubscribe();
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::handleBeamServerEvents (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case ABS_BEAMALLOC:
    {
      ABSBeamalloc_AckEvent ack(0, SUCCESS);
      p.send(ack);
      break;
    }
     
    case ABS_BEAMFREE:
    {
      ABSBeamfree_AckEvent ack(0, SUCCESS);
      p.send(ack);
      break;
    }
     
    case ABS_BEAMPOINTTO:
      break;
      
    case ABS_WGSETTINGS:
    {
      ABSWgsettings_AckEvent ack(SUCCESS);
      p.send(ack);
      break;
    }
    
    case ABS_WGENABLE:
      break;
      
    case ABS_WGDISABLE:
      break;

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
      case F_INIT_SIG:
        break;

      case F_ENTRY_SIG:
        break;
      
      case F_CONNECTED_SIG:
        break;
              
      case F_DISCONNECTED_SIG:
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

