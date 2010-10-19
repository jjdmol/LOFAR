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

const char SCOPE_PAC_LDS[] = "PAC_LogicalDeviceScheduler";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1[] = "PIC_Rack1_SubRack1_Board1_AP1";
const char SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance[] = "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance";
const char SCOPE_PIC_Rack1_SubRack1_Board1_Alert[] = "PIC_Rack1_SubRack1_Board1_Alert";

#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(ARATestTask::test##_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
    TRAN(ARATestTask::finished); \
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

#include <math.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "APLCommon/APL_Defines.h"
#include "ARATestTask.h"
#include "../src/ARAPropertyDefines.h"

#undef PACKAGE
#undef VERSION
#include "RSP_Protocol.ph"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <bitset>
#include <time.h>

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR
{

namespace ARA
{

string ARATestTask::m_taskName("ARATest");

ARATestTask::ARATestTask() :
  GCFTask((State)&ARATestTask::initial, m_taskName),
  Test("RegisterAccessTest"),
  m_answer(),
  m_RSPserver(),
  m_test_passCounter(0),
  m_propsetLoadedCounter(0),
  m_extPropSetAP1(SCOPE_PIC_Rack1_SubRack1_Board1_AP1,TYPE_LCU_PIC_FPGA,&m_answer),
  m_extPropSetAP1RCUmaintenance(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance,TYPE_LCU_PIC_Maintenance,&m_answer),
  m_extPropSetBoardAlert(SCOPE_PIC_Rack1_SubRack1_Board1_Alert,TYPE_LCU_PIC_Alert,&m_answer),
  m_extPropSetStationMaintenance(SCOPE_PIC_Maintenance,TYPE_LCU_PIC_Maintenance,&m_answer),
  m_extPropSetLDS(SCOPE_PAC_LDS,TYPE_LCU_PAC_LogicalDeviceScheduler,&m_answer)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);

  m_RSPserver.init(*this, "ARAtestRSPserver", GCFPortInterface::SPP, RSP_PROTOCOL);
}

ARATestTask::~ARATestTask()
{
}

void ARATestTask::run()
{
  start(); // make initial transition
  GCFTask::run();
}

bool ARATestTask::isEnabled()
{
  return (m_RSPserver.isConnected());
}

GCFEvent::TResult ARATestTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("ARATestTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_extPropSetAP1.load();
      m_extPropSetAP1RCUmaintenance.load();
      m_extPropSetBoardAlert.load();
      m_extPropSetStationMaintenance.load();
      m_extPropSetLDS.load();
      break;
      
    case F_EXTPS_LOADED:
      m_propsetLoadedCounter++;
      if(m_propsetLoadedCounter==5)
      {
        if (!m_RSPserver.isConnected()) 
        {
          m_RSPserver.open();
        }
      }
      break;
    
    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      if (isEnabled())
      {
        NEXT_TEST(1,"Monitor FPGA registers. Goal: load secondary properties");
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
      LOG_DEBUG(formatString("ARATestTask(%s)::initial, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test1 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.1.1: Monitor FPGA registers. Goal: load secondary properties");
      if(!TESTC(GCF_NO_ERROR==m_extPropSetAP1.requestValue(PROPNAME_STATUS)));
      {
        NEXT_TEST(2,"put 1 antenna in maintenance");
      }
      break;
    }

    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      TESTC(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1)!=0 &&
            strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0);
      // display the value:
      GCFPVUnsigned status;
      status.copy(*pPropAnswer->pValue);
      LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));

      NEXT_TEST(2,"put 1 antenna in maintenance");
      break;
    }
     
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test1, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test2 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.1: put 1 antenna in maintenance");
      m_extPropSetAP1RCUmaintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(1);
      if(!TESTC(GCF_NO_ERROR==m_extPropSetAP1RCUmaintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXT_TEST(3,"put station in maintenance");
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
      TESTC(
        strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      // display the value:
      NEXT_TEST(3,"put station in maintenance");
      break;
    }
     
    case F_EXIT:
    {
      m_extPropSetAP1RCUmaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test2, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test3 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.2: put station in maintenance");
      m_extPropSetStationMaintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(1);
      if(!TESTC(GCF_NO_ERROR==m_extPropSetStationMaintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXT_TEST(4,"put station out of maintenance");
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
      TESTC(
        strstr(pPropAnswer->pPropName,SCOPE_PIC_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      // display the value:

      NEXT_TEST(4,"put station out of maintenance");
      break;
    }
     
    case F_EXIT:
    {
      m_extPropSetStationMaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test3, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test4 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.3: put station out of maintenance");
      m_extPropSetStationMaintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(0);
      if(!TESTC(GCF_NO_ERROR==m_extPropSetStationMaintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXT_TEST(5,"put antenna out of maintenance");
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
      TESTC(
        strstr(pPropAnswer->pPropName,SCOPE_PIC_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0 &&
        inMaintenance.getValue()==0 );
      
      // check maintenance status of antenna
      if(!TESTC(GCF_NO_ERROR==m_extPropSetAP1RCUmaintenance.requestValue(PROPNAME_STATUS)))
      {
        NEXT_TEST(5,"put antenna out of maintenance");
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
      TESTC(
        strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0 &&
        inMaintenance.getValue()!=0 );
      NEXT_TEST(5,"put antenna out of maintenance");
      break;
    }

    case F_EXIT:
    {
      m_extPropSetStationMaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test4, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test5 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.4: put antenna out of maintenance");
      m_extPropSetAP1RCUmaintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(0);
      if(!TESTC(GCF_NO_ERROR==m_extPropSetAP1RCUmaintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXT_TEST(6,"schedule maintenance of 1 antenna");
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
      TESTC(
        strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0 &&
        inMaintenance.getValue()==0 );
      NEXT_TEST(6,"schedule maintenance of 1 antenna");
      break;
    }
     
    case F_EXIT:
    {
      m_extPropSetAP1RCUmaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test5, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test6 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.5: schedule maintenance of 1 antenna");
  
      m_extPropSetAP1RCUmaintenance.subscribeProp(PROPNAME_STATUS);
      m_test_passCounter=0;
      // MAINTENANCE <scheduleid>,<resource>,<starttime>,<stoptime>
      string cmd("MAINTENANCE 1,");
      string resource(string("PIC_Rack1_SubRack1_Board1_AP1_RCU1")+string(","));
      
      // create time 10 seconds from now
      time_t timeNow = time(0);
      struct tm* utcTimeStruct = gmtime(&timeNow);
      time_t utcTime = mktime(utcTimeStruct);
      time_t startTime = utcTime + 10; // UTC + 10 seconds
      time_t stopTime = startTime + 10; // starttime + 10 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d",(int)startTime,(int)stopTime);
      
      string times(timesString);
      GCFPVString command(cmd+resource+times);

      if(!TESTC(GCF_NO_ERROR==m_extPropSetLDS.setValue("command",command)))
      {
        NEXT_TEST(7,"schedule maintenance of entire station");
      }
      else
      {
	      printf("the resource will be put in maintenance in 10 seconds... Please wait\n");
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance)!=0 &&
         strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            if(!TESTC(inMaintenance.getValue()!=0))
            {
              NEXT_TEST(7,"schedule maintenance of entire station");
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
          TESTC(inMaintenance.getValue()==0);
          NEXT_TEST(7,"schedule maintenance of entire station");
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_extPropSetAP1RCUmaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test6, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test7 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.2.6: schedule maintenance of entire station");
      
      m_extPropSetStationMaintenance.subscribeProp(PROPNAME_STATUS);
      m_test_passCounter=0;
      // MAINTENANCE <scheduleid>,<resource>,<starttime>,<stoptime>
      string cmd("MAINTENANCE 2,");
      string resource(string("PIC")+string(","));
      // create time 10 seconds from now
      time_t timeNow = time(0);
      struct tm* utcTimeStruct = gmtime(&timeNow);
      time_t utcTime = mktime(utcTimeStruct);
      time_t startTime = utcTime + 10; // UTC + 10 seconds
      time_t stopTime = startTime + 10; // starttime + 10 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d",(int)startTime,(int)stopTime);
      
      string times(timesString);
      GCFPVString command(cmd+resource+times);

      if(!TESTC(GCF_NO_ERROR==m_extPropSetLDS.setValue("command",command)))
      {
        NEXT_TEST(8,"simulate board defect");
      }
      else
      {
	      printf("the resource will be put in maintenance in 10 seconds... Please wait\n");
      }    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Maintenance)!=0 &&
        strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        if(m_test_passCounter==0) // first pass: in maintenance
        {
          {
            if(!TESTC(inMaintenance.getValue()!=0))
            {
              NEXT_TEST(8,"simulate board defect");
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
          TESTC(inMaintenance.getValue()==0);
          NEXT_TEST(8,"simulate board defect");
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_extPropSetStationMaintenance.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test7, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test8 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.3.1: simulate board defect");
      m_extPropSetBoardAlert.subscribeProp(PROPNAME_STATUS);
      
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
      boardStatus.fpga.ap1_temp = 2850;
      boardStatus.fpga.ap2_temp = 2902;
      boardStatus.fpga.ap3_temp = 2953;
      boardStatus.fpga.ap4_temp = 3005;

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
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_Alert)!=0 &&
         strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0)
      {
        // check alert status
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        TESTC(status.getValue()!=0);
        NEXT_TEST(9,"simulate board defect fixed");
      }
      break;
    }
     
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test8, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::test9 (%d)",getName().c_str(),event.signal));
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
      boardStatus.fpga.ap1_temp = 28;
      boardStatus.fpga.ap2_temp = 29;
      boardStatus.fpga.ap3_temp = 30;
      boardStatus.fpga.ap4_temp = 31;

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
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_Alert)!=0 &&
         strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0)
      {
        // check alert status
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        TESTC(status.getValue()==0);
        NEXT_TEST(10,"put all rcu's in overflow");
      }
      break;
    }
    
    case F_EXIT:
    {
      break;
    }
     
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test9, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


/* 
 * Test case 10: put all rcu's in overflow
 */
GCFEvent::TResult ARATestTask::test10(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("ARATestTask(%s)::test10 (%d)",getName().c_str(),event.signal));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("3.2.3.1a: simulate all rcu's overflow");
      
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
      boardStatus.fpga.ap1_temp = 28;
      boardStatus.fpga.ap2_temp = 29;
      boardStatus.fpga.ap3_temp = 30;
      boardStatus.fpga.ap4_temp = 31;

      EPA_Protocol::RCUStatus rcuStatus;
      std::bitset<8> rcuBitStatus;
      rcuBitStatus[7] = 1; // overflow
      rcuStatus.status = rcuBitStatus.to_ulong();
      updStatusEvent.sysstatus.board().resize(1);
      updStatusEvent.sysstatus.board()(0) = boardStatus;
      updStatusEvent.sysstatus.rcu().resize(6);
      updStatusEvent.sysstatus.rcu()(0) = rcuStatus;
      updStatusEvent.sysstatus.rcu()(1) = rcuStatus;
      updStatusEvent.sysstatus.rcu()(2) = rcuStatus;
      updStatusEvent.sysstatus.rcu()(3) = rcuStatus;
      updStatusEvent.sysstatus.rcu()(4) = rcuStatus;
      updStatusEvent.sysstatus.rcu()(5) = rcuStatus;

      m_RSPserver.send(updStatusEvent);

      // now wait for the alert status to change    
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_Alert)!=0 &&
         strstr(pPropAnswer->pPropName,PROPNAME_STATUS)!=0)
      {
        // check alert status
        GCFPVUnsigned status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,status.getValue()));
        TESTC(status.getValue()!=0);
        FINISH
      }
      break;
    }
    
    case F_EXIT:
    {
      m_extPropSetBoardAlert.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
     
    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::test10, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestTask(%s)::finished (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      GCFTask::stop();
      break;

    default:
      LOG_DEBUG(formatString("ARATestTask(%s)::finished, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


} // namespace ARA


} // namespace LOFAR

