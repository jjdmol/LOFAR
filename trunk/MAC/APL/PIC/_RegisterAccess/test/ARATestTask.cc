//#  ARATestTask.cc: Implementation of the Virtual Telescope test
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

#ifdef GCF4
// datapoint structs are supported in gcf4
#define PROPERTY_BOARD1_STATUS                  "PIC_Rack1_SubRack1_Board1.status"
#define PROPERTY_BOARD1_ALERT_STATUS            "PIC_Rack1_SubRack1_Board1_Alert.status"
#define PROPERTY_AP1_STATUS                     "PIC_Rack1_SubRack1_Board1_AP1.status"
#define PROPERTY_AP1_RCU1                       "PIC_Rack1_SubRack1_Board1_AP1_RCU1"
#define PROPERTY_AP1_RCU1_MAINTENANCE_STATUS    "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance.status"
#define PROPERTY_STATION_PIC                    "PIC"
#define PROPERTY_STATION_PIC_MAINTENANCE_STATUS "PIC_Maintenance.status"
#define PROPERTY_STATION_PAC_LDS_COMMAND        "PAC_LogicalDeviceScheduler_command"
#else
#define PROPERTY_BOARD1_STATUS                  "PIC_Rack1_SubRack1_Board1_status"
#define PROPERTY_BOARD1_ALERT_STATUS            "PIC_Rack1_SubRack1_Board1_Alert_status"
#define PROPERTY_AP1_STATUS                     "PIC_Rack1_SubRack1_Board1_AP1_status"
#define PROPERTY_AP1_RCU1                       "PIC_Rack1_SubRack1_Board1_AP1_RCU1"
#define PROPERTY_AP1_RCU1_MAINTENANCE_STATUS    "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance_status"
#define PROPERTY_STATION_PIC                    "PIC"
#define PROPERTY_STATION_PIC_MAINTENANCE_STATUS "PIC_Maintenance_status"
#define PROPERTY_STATION_PAC_LDS_COMMAND        "PAC_LogicalDeviceScheduler_command"
#endif

#include <math.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "../../../APLCommon/src/APL_Defines.h"
#include "ARATest.h"
#include "ARATestTask.h"

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "RSP_Protocol.ph"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <bitset>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
using namespace ARA;
using namespace std;

string ARATestTask::m_taskName("ARATest");

ARATestTask::ARATestTask(ARATest& tester) :
  GCFTask((State)&ARATestTask::initial, m_taskName),
  m_tester(tester),
  m_answer(),
  m_RSPserver(),
  m_test_passCounter(0),
  m_propAP1status(string(PROPERTY_AP1_STATUS)),
  m_propAP1RCUmaintenanceStatus(string(PROPERTY_AP1_RCU1_MAINTENANCE_STATUS)),
  m_propStationMaintenanceStatus(string(PROPERTY_STATION_PIC_MAINTENANCE_STATUS)),
  m_propLDScommand(string(PROPERTY_STATION_PAC_LDS_COMMAND)),
  m_propBoard1AlertStatus(string(PROPERTY_BOARD1_ALERT_STATUS))
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);

  m_propAP1status.setAnswer(&m_answer);
  m_propAP1RCUmaintenanceStatus.setAnswer(&m_answer);
  m_propStationMaintenanceStatus.setAnswer(&m_answer);
  m_propLDScommand.setAnswer(&m_answer);
  m_propBoard1AlertStatus.setAnswer(&m_answer);
  
  m_RSPserver.init(*this, "ARAtestRSPserver", GCFPortInterface::SPP, RSP_PROTOCOL);
  
}

ARATestTask::~ARATestTask()
{
}

bool ARATestTask::isEnabled()
{
  return (m_RSPserver.isConnected());
}

GCFEvent::TResult ARATestTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      if (!m_RSPserver.isConnected()) 
      {
        m_RSPserver.open();
      }
      break;
    
    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      if (isEnabled())
      {
        TRAN(ARATestTask::test1);
      }
      break;
    }
  
    case F_DISCONNECTED:
    {
      port.setTimer((long)3); // try again in 3 seconds
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
      break;
    }

    case F_TIMER:
    {
      LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
      port.open();
      break;
    }

    case F_EXIT:
    {
      // cancel timers
      m_RSPserver.cancelAllTimers();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 1: Monitor FPGA registers. Goal: load secondary properties
 */
GCFEvent::TResult ARATestTask::test1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test1 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.1.1: Monitor FPGA registers. Goal: load secondary properties");
      bool testOk = (GCF_NO_ERROR==m_propAP1status.requestValue());
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test2);
      }
      break;
    }

    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      bool testOk = (strstr(pPropAnswer->pPropName,PROPERTY_AP1_STATUS)!=0);
      // display the value:
      GCFPVUnsigned status;
      status.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));

      m_tester._avttest(testOk);
      TRAN(ARATestTask::test2);
      break;
    }
     
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test1, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 2: put 1 antenna in maintenance
 */
GCFEvent::TResult ARATestTask::test2(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test2 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.1: put 1 antenna in maintenance");
      m_propAP1RCUmaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_propAP1RCUmaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test3);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned inMaintenance;
      inMaintenance.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTY_AP1_RCU1_MAINTENANCE_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      // display the value:
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test3);
      break;
    }
     
    case F_EXIT:
    {
      m_propAP1RCUmaintenanceStatus.unsubscribe();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test2, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 3: put station in maintenance
 */
GCFEvent::TResult ARATestTask::test3(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test3 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.2: put station in maintenance");
      m_propStationMaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_propStationMaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test4);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned inMaintenance;
      inMaintenance.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTY_STATION_PIC_MAINTENANCE_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      // display the value:

      m_tester._avttest(testOk);
      TRAN(ARATestTask::test4);
      break;
    }
     
    case F_EXIT:
    {
      m_propStationMaintenanceStatus.unsubscribe();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test3, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 4: put station out of maintenance
 */
GCFEvent::TResult ARATestTask::test4(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test4 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.3: put station out of maintenance");
      m_propStationMaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(0);
      bool testOk = (GCF_NO_ERROR==m_propStationMaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test5);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned inMaintenance;
      inMaintenance.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTY_STATION_PIC_MAINTENANCE_STATUS)!=0 &&
        inMaintenance.getValue()==0 );
      m_tester._avttest(testOk);
      
      // check maintenance status of antenna
      testOk = (GCF_NO_ERROR==m_propAP1RCUmaintenanceStatus.requestValue());
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test5);
      }
      break;
    }
     
    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned inMaintenance;
      inMaintenance.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTY_AP1_RCU1_MAINTENANCE_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test5);
      break;
    }

    case F_EXIT:
    {
      m_propStationMaintenanceStatus.unsubscribe();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test4, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 5: put antenna out of maintenance
 */
GCFEvent::TResult ARATestTask::test5(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test5 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.4: put antenna out of maintenance");
      m_propAP1RCUmaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(0);
      bool testOk = (GCF_NO_ERROR==m_propAP1RCUmaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test6);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned inMaintenance;
      inMaintenance.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTY_AP1_RCU1_MAINTENANCE_STATUS)!=0 &&
        inMaintenance.getValue()==0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test6);
      break;
    }
     
    case F_EXIT:
    {
      m_propAP1RCUmaintenanceStatus.unsubscribe();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test5, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 6: schedule maintenance of 1 antenna
 */
GCFEvent::TResult ARATestTask::test6(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test6 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.5: schedule maintenance of 1 antenna");
  
      m_test_passCounter=0;
      // MAINTENANCE <scheduleid>,<resource>,<starttime>,<stoptime>
      string cmd("MAINTENANCE 1,");
      string resource(string(PROPERTY_AP1_RCU1)+string(","));
      
      // create time 10 seconds from now
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(10); // UTC + 10 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(10); // starttime + 10 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      GCFPVString command(cmd+resource+times);

      bool testOk = (GCF_NO_ERROR==m_propLDScommand.setValue(command));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test7);
      }
      printf("the resource will be put in maintenance in 10 seconds... Please wait\n");
      
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_AP1_RCU1_MAINTENANCE_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            bool testOk = (inMaintenance.getValue()!=0 );
            m_tester._avttest(testOk);
            if(!testOk)
            {
              TRAN(ARATestTask::test7);
            }
            else
            {
              printf("the resource will be put out of maintenance in 10 seconds... Please wait\n");
            }
          }
          m_test_passCounter++;
        }
        else if(m_test_passCounter==1) // second pass: out of maintenance
        {
          bool testOk = (inMaintenance.getValue()==0 );
          m_tester._avttest(testOk);
          TRAN(ARATestTask::test7);
        }
      }
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test6, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 7: schedule maintenance of entire station
 */
GCFEvent::TResult ARATestTask::test7(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test7 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.6: schedule maintenance of entire station");

      m_test_passCounter=0;
      // MAINTENANCE <scheduleid>,<resource>,<starttime>,<stoptime>
      string cmd("MAINTENANCE 2,");
      string resource(string(PROPERTY_STATION_PIC)+string(","));
      // create time 10 seconds from now
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(10); // UTC + 10 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(10); // starttime + 10 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      GCFPVString command(cmd+resource+times);

      bool testOk = (GCF_NO_ERROR==m_propLDScommand.setValue(command));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        TRAN(ARATestTask::test8);
      }
      printf("the resource will be put in maintenance in 10 seconds... Please wait\n");
      
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_STATION_PIC_MAINTENANCE_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            bool testOk = (inMaintenance.getValue()!=0 );
            m_tester._avttest(testOk);
            if(!testOk)
            {
              TRAN(ARATestTask::test8);
            }
            else
            {
              printf("the resource will be put out of maintenance in 10 seconds... Please wait\n");
            }
          }
          m_test_passCounter++;
        }
        else if(m_test_passCounter==1) // second pass: out of maintenance
        {
          bool testOk = (inMaintenance.getValue()==0 );
          m_tester._avttest(testOk);
          TRAN(ARATestTask::test8);
        }
      }
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test7, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 8: simulate board defect
 */
GCFEvent::TResult ARATestTask::test8(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test8 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.3.1: simulate board defect");
      m_propBoard1AlertStatus.subscribe();
      
      // send write register message to RA test port
      RSPUpdstatusEvent updStatusEvent;
      struct timeval timeValNow;
      time(&timeValNow.tv_sec);
      timeValNow.tv_usec=0;
      updStatusEvent.timestamp.set(timeValNow);
      updStatusEvent.status=1;
      updStatusEvent.handle=1;
      
      EPA_Protocol::BoardStatus boardStatus;
      memset(&boardStatus,0,sizeof(boardStatus));
      boardStatus.ap[0].temp = 28;
      boardStatus.ap[1].temp = 29;
      boardStatus.ap[2].temp = 30;
      boardStatus.ap[3].temp = 31;

      EPA_Protocol::RCUStatus rcuStatus;
      std::bitset<8> rcuBitStatus;
      rcuBitStatus[7] = 1; // overflow
      rcuStatus.status = rcuBitStatus.to_ulong();
      updStatusEvent.sysstatus.board().resize(1);
      updStatusEvent.sysstatus.board()(0) = boardStatus;
      updStatusEvent.sysstatus.rcu().resize(1);
      updStatusEvent.sysstatus.rcu()(0) = rcuStatus;

      m_RSPserver.send(updStatusEvent);

      // now wait for the alert status to change    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_BOARD1_STATUS)!=0)
      {
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        bool testOk = ( status.getValue()!=0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          TRAN(ARATestTask::test9);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_BOARD1_ALERT_STATUS)!=0)
      {
        // check alert status
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        bool testOk = ( status.getValue()!=0 );
        m_tester._avttest(testOk);
        TRAN(ARATestTask::test9);
      }
      break;
    }
     
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test8, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 9: simulate board defect fixed
 */
GCFEvent::TResult ARATestTask::test9(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test9 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.3.2: simulate board defect fixed");
      // test8 already subscribed us to the alert status property
      
      // send write register message to RA test port
      RSPUpdstatusEvent updStatusEvent;
      struct timeval timeValNow;
      time(&timeValNow.tv_sec);
      timeValNow.tv_usec=0;
      updStatusEvent.timestamp.set(timeValNow);
      updStatusEvent.status=0;
      updStatusEvent.handle=1;

      EPA_Protocol::BoardStatus boardStatus;
      memset(&boardStatus,0,sizeof(boardStatus));
      boardStatus.ap[0].temp = 28;
      boardStatus.ap[1].temp = 29;
      boardStatus.ap[2].temp = 30;
      boardStatus.ap[3].temp = 31;

      EPA_Protocol::RCUStatus rcuStatus;
      std::bitset<8> rcuBitStatus;
      rcuBitStatus[7] = 0; // no overflow
      rcuStatus.status = rcuBitStatus.to_ulong();
      updStatusEvent.sysstatus.board().resize(1);
      updStatusEvent.sysstatus.board()(0) = boardStatus;
      updStatusEvent.sysstatus.rcu().resize(1);
      updStatusEvent.sysstatus.rcu()(0) = rcuStatus;

      m_RSPserver.send(updStatusEvent);

      // now wait for the alert status to change    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_BOARD1_STATUS)!=0)
      {
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        bool testOk = ( status.getValue()==0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          TRAN(ARATestTask::finished);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_BOARD1_ALERT_STATUS)!=0)
      {
        // check alert status
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        bool testOk = ( status.getValue()==0 );
        m_tester._avttest(testOk);
        TRAN(ARATestTask::finished);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propBoard1AlertStatus.unsubscribe();
      break;
    }
     
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::test9, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}




/* 
 * End of all tests
 */
GCFEvent::TResult ARATestTask::finished(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::finished (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      GCFTask::stop();
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestTask(%s)::finished, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

