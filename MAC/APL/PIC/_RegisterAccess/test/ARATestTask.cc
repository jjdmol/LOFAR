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

#define PROPERTY_STATUS                     "status"
#define PROPERTY_COMMAND                    "command"
#define PROPERTIES_BOARD1                   "PIC_Rack1_SubRack1_Board1"
#define PROPERTIES_BOARD1_ALERT             "PIC_Rack1_SubRack1_Board1_Alert"
#define PROPERTIES_AP1                      "PIC_Rack1_SubRack1_Board1_AP1"
#define PROPERTIES_AP1_RCU1                 "PIC_Rack1_SubRack1_Board1_AP1_RCU1"
#define PROPERTIES_AP1_RCU1_MAINTENANCE     "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance"
#define PROPERTIES_STATION_PIC              "PIC"
#define PROPERTIES_STATION_PIC_MAINTENANCE  "PIC_Maintenance"
#define PROPERTIES_STATION_PAC              "PAC"

#include <math.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "../../../APLCommon/src/APL_Defines.h"
#include "ARATest.h"
#include "ARATestTask.h"
#include "PropertyDefines.h" 

#define DECLARE_SIGNAL_NAMES
#include "ARATest_Protocol.ph"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace ARA;
using namespace std;

string ARATestTask::m_taskName("ARATest");
string ARATestTask::m_RATestServerName("RAtest");

ARATestTask::ARATestTask(ARATest& tester) :
  GCFTask((State)&ARATestTask::initial, m_taskName),
  m_tester(tester),
  m_answer(),
  m_RAtestPort(*this, m_RATestServerName, GCFPortInterface::SAP, ARATEST_PROTOCOL),
  m_test_passCounter(0),
  m_psBP(string("ApcFPGAType"),string(PROPERTIES_AP1)),
  m_psRCUmaintenance(string("ApcMaintenanceType"),string(PROPERTIES_AP1_RCU1_MAINTENANCE)),
  m_psStationMaintenance(string("ApcMaintenanceType"),string(PROPERTIES_STATION_PIC_MAINTENANCE)),
  m_psLDScommand(string("ApcLogicalDeviceScheduler"),string(PROPERTIES_STATION_PAC)),
  m_psBoard1Alert(string("ApcAlertType"),string(PROPERTIES_BOARD1_ALERT)),
  m_psBoard1(string("ApcBoardType"),string(PROPERTIES_BOARD1))
{
  m_answer.setTask(this);
}

ARATestTask::~ARATestTask()
{
}

bool ARATestTask::isEnabled()
{
  return (m_RAtestPort.isConnected());
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
      if (!m_RAtestPort.isConnected()) 
      {
        m_RAtestPort.open();
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
      m_RAtestPort.cancelAllTimers();
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
      bool testOk = (GCF_NO_ERROR==m_psBP.requestValue(string(PROPERTY_STATUS)));
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
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_AP1)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 );
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
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_psRCUmaintenance.setValue(string(PROPERTY_STATUS),inMaintenance));
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
      const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_AP1_RCU1_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 &&
        pInMaintenance->getValue()!=0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test3);
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
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_psStationMaintenance.setValue(string(PROPERTY_STATUS),inMaintenance));
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
      const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_STATION_PIC_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 &&
        pInMaintenance->getValue()!=0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test4);
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
      GCFPVUnsigned inMaintenance(0);
      bool testOk = (GCF_NO_ERROR==m_psStationMaintenance.setValue(string(PROPERTY_STATUS),inMaintenance));
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
      const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_STATION_PIC_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 &&
        pInMaintenance->getValue()==0 );
      m_tester._avttest(testOk);
      
      // check maintenance status of antenna
      testOk = (GCF_NO_ERROR==m_psRCUmaintenance.requestValue(string(PROPERTY_STATUS)));
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
      const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_AP1_RCU1_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 &&
        pInMaintenance->getValue()!=0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test5);
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
      GCFPVUnsigned inMaintenance(0);
      bool testOk = (GCF_NO_ERROR==m_psRCUmaintenance.setValue(string(PROPERTY_STATUS),inMaintenance));
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
      const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
      bool testOk = (
        strstr(pPropAnswer->pPropName,PROPERTIES_AP1_RCU1_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0 &&
        pInMaintenance->getValue()==0 );
      m_tester._avttest(testOk);
      TRAN(ARATestTask::test6);
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
      m_test_passCounter=0;
      // SCHEDULE MAINTENANCE <resource>,<starttime>,<stoptime>
      string cmd("SCHEDULE MAINTENANCE");
      string resource(string(PROPERTIES_AP1_RCU1)+string(","));
      time_t rawtime;
      tm * starttm;
      tm * stoptm;
      time ( &rawtime );
      starttm = gmtime(&rawtime);
      starttm = gmtime(&rawtime);
      starttm->tm_sec+=10; // 10 seconds in the future
      stoptm->tm_sec+=20; // 20 seconds in the future
      char startTime[100];
      char stopTime[100];
      sprintf(startTime,"%02d-%02d-%04d %02d:%02d:%02d",starttm->tm_mday,starttm->tm_mon+1,starttm->tm_year+1900,starttm->tm_hour,starttm->tm_min,starttm->tm_sec);
      sprintf(stopTime,"%02d-%02d-%04d %02d:%02d:%02d",stoptm->tm_mday,stoptm->tm_mon+1,stoptm->tm_year+1900,stoptm->tm_hour,stoptm->tm_min,stoptm->tm_sec);
      GCFPVString command(cmd+resource+string(startTime)+","+string(stopTime));

      bool testOk = (GCF_NO_ERROR==m_psLDScommand.setValue(string(PROPERTY_COMMAND),command));
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
      if(strstr(pPropAnswer->pPropName,PROPERTIES_AP1_RCU1_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            bool testOk = (pInMaintenance->getValue()!=0 );
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
          bool testOk = (pInMaintenance->getValue()==0 );
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
      m_test_passCounter=0;
      // SCHEDULE MAINTENANCE <resource>,<starttime>,<stoptime>
      string cmd("SCHEDULE MAINTENANCE");
      string resource(string(PROPERTIES_STATION_PIC)+string(","));
      time_t rawtime;
      tm * starttm;
      tm * stoptm;
      time ( &rawtime );
      starttm = gmtime(&rawtime);
      starttm = gmtime(&rawtime);
      starttm->tm_sec+=10; // 10 seconds in the future
      stoptm->tm_sec+=20; // 20 seconds in the future
      char startTime[100];
      char stopTime[100];
      sprintf(startTime,"%02d-%02d-%04d %02d:%02d:%02d",starttm->tm_mday,starttm->tm_mon+1,starttm->tm_year+1900,starttm->tm_hour,starttm->tm_min,starttm->tm_sec);
      sprintf(stopTime,"%02d-%02d-%04d %02d:%02d:%02d",stoptm->tm_mday,stoptm->tm_mon+1,stoptm->tm_year+1900,stoptm->tm_hour,stoptm->tm_min,stoptm->tm_sec);
      GCFPVString command(cmd+resource+string(startTime)+","+string(stopTime));

      bool testOk = (GCF_NO_ERROR==m_psLDScommand.setValue(string(PROPERTY_COMMAND),command));
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
      if(strstr(pPropAnswer->pPropName,PROPERTIES_STATION_PIC_MAINTENANCE)!=0 &&
        strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        const GCFPVUnsigned* pInMaintenance = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            bool testOk = (pInMaintenance->getValue()!=0 );
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
          bool testOk = (pInMaintenance->getValue()==0 );
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
      m_psBoard1Alert.subscribe(string(PROPERTY_STATUS));
      
      // send write register message to RA test port
      ARATESTWriteRegisterEvent wrEvent;
      wrEvent.board = 1;
      wrEvent.BP = 0; // not used
      wrEvent.AP = 0; // not used
      wrEvent.ETH = 0; // not used
      wrEvent.RCU = 0; // not used
      wrEvent.value = 1; // what is the status for erreur?
      m_RAtestPort.send(wrEvent);

      // now wait for the alert status to change    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTIES_BOARD1)!=0 &&
         strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        const GCFPVUnsigned* pStatus = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        bool testOk = ( pStatus->getValue()!=0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          TRAN(ARATestTask::test9);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTIES_BOARD1_ALERT)!=0 &&
         strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        // check alert status
        const GCFPVUnsigned* pStatus = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        bool testOk = ( pStatus->getValue()!=0 );
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
      // test8 already subscribed us to the alert status property
      
      // send write register message to RA test port
      ARATESTWriteRegisterEvent wrEvent;
      wrEvent.board = 1;
      wrEvent.BP = 0; // not used
      wrEvent.AP = 0; // not used
      wrEvent.ETH = 0; // not used
      wrEvent.RCU = 0; // not used
      wrEvent.value = 0; // what is the status for ok?
      m_RAtestPort.send(wrEvent);

      // now wait for the alert status to change    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTIES_BOARD1)!=0 &&
         strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        const GCFPVUnsigned* pStatus = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        bool testOk = ( pStatus->getValue()==0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          m_psBoard1Alert.unsubscribe(string(PROPERTY_STATUS));
          TRAN(ARATestTask::finished);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTIES_BOARD1_ALERT)!=0 &&
         strstr(pPropAnswer->pPropName,PROPERTY_STATUS)!=0)
      {
        // check alert status
        const GCFPVUnsigned* pStatus = static_cast<const GCFPVUnsigned*>(pPropAnswer->pValue);
        bool testOk = ( pStatus->getValue()==0 );
        m_tester._avttest(testOk);
        m_psBoard1Alert.unsubscribe(string(PROPERTY_STATUS));
        TRAN(ARATestTask::finished);
      }
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

