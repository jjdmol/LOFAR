//#  AVTTestMAC2Task.cc: Implementation of the Virtual Telescope test
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

#define SCOPE_PIC_Rack1_Subrack1_Board1_Maintenance           "PIC_Rack1_SubRack1_Board1_Maintenance"
#define SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance  "PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance"
#define SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance  "PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance"
#define SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1              "PIC_Rack1_SubRack1_Board1_AP1_RCU1"
#define SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1              "PIC_Rack1_SubRack1_Board1_AP2_RCU1"
#define SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1              "PIC_Rack1_SubRack1_Board1_AP3_RCU1"
#define SCOPE_PAC_VT1                                         "PAC_VT1"
#define SCOPE_PAC_VT2                                         "PAC_VT2"
#define SCOPE_PAC_VT3                                         "PAC_VT3"

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>

#include <APLCommon/APL_Defines.h>
#include "AVTTestMAC2Task.h"
#include "../src/AVTPropertyDefines.h" 

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "../src/LogicalDevice_Protocol.ph"

#include <stdio.h>
#include <time.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
using namespace AVT;
using namespace std;

#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(AVTTestMAC2Task::test##_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
    TRAN(AVTTestMAC2Task::finished); \
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

#define ADDTRANTARGET(_targetvector_,_target_,_num_,_descr_) \
  { \
    TTranTarget tt={static_cast<State>(&_target_), #_target_,#_num_,_descr_}; \
    _targetvector_.push_back(tt); \
  }
  
#define TESTTRAN(_trantarget_) \
  { \
    setCurSubTest(_trantarget_->testNum,_trantarget_->description); \
    tran(static_cast<State>(_trantarget_->target), __func__, _trantarget_->targetName); \
  }

#define NEXTTEST(_iterator_) \
  { \
    std::vector<TTranTarget>::iterator tempIt = _iterator_; \
    _iterator_++; \
    TESTTRAN(tempIt); \
  }
  

string AVTTestMAC2Task::m_taskName("AVTTestMAC2");
string g_timerPortName("timerPort");

AVTTestMAC2Task::AVTTestMAC2Task() :
  GCFTask((State)&AVTTestMAC2Task::initial, m_taskName),
  Test(m_taskName),
  m_testSequence(),
  m_testSequenceIt(),
  m_answer(),
  m_timerPort(*this, g_timerPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_propertysetLDS(SCOPE_PAC_LogicalDeviceScheduler,TYPE_LCU_PAC_LogicalDeviceScheduler.c_str(),&m_answer),
  m_propertysetBoard1Maintenance(SCOPE_PIC_Rack1_Subrack1_Board1_Maintenance,TYPE_LCU_PIC_Maintenance.c_str(),&m_answer),
  m_propertysetAP1RCU1Maintenance(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance,TYPE_LCU_PIC_Maintenance.c_str(),&m_answer),
  m_propertysetAP1RCU2Maintenance(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance,TYPE_LCU_PIC_Maintenance.c_str(),&m_answer),
  m_propertysetAP1RCU1(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1,TYPE_LCU_PIC_RCU.c_str(),&m_answer),
  m_propertysetAP2RCU1(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1,TYPE_LCU_PIC_RCU.c_str(),&m_answer),
  m_propertysetAP3RCU1(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1,TYPE_LCU_PIC_RCU.c_str(),&m_answer),
  m_propertysetVT1(SCOPE_PAC_VT1,TYPE_LCU_PAC_VT.c_str(),&m_answer),
  m_propertysetVT2(SCOPE_PAC_VT2,TYPE_LCU_PAC_VT.c_str(),&m_answer),
  m_propertysetVT3(SCOPE_PAC_VT3,TYPE_LCU_PAC_VT.c_str(),&m_answer),
  m_maintenanceChangedCounter(0),
  m_suspendedCounter(0)
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_1,1,"3_2_4_1: Start VT, all antennas in maintenance");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_2,2,"3_2_4_2: Start VT, too many antennas in maintenance");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_3,3,"3_2_4_3: Start VT, no antennas in maintenance");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_4,4,"3_2_4_3: Start VT, no antennas in maintenance");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_5,5,"3_2_4_5: Change antenna parameter using the first VT");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_6,6,"3_2_4_5: Change antenna parameter using the first VT");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_7,7,"3_2_4_7: Abort the first VT");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_8,8,"3_2_4_8: Change antenna parameter using the second VT");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_1,9,"3_2_5_2: Schedule the same VT at non-overlapping timespans");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_2,10,"3_2_5_3: Schedule the same VT at overlapping timespans");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_3,11,"3_2_5_4: Cancel a VT Schedule");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_4,12,"3_2_5_5: Schedule two VT's at the same time");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_5,13,"3_2_6_1: Display subband statistics of a running VT");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_6_1,14,"3_2_7_1: Antenna defect, no VT uses this antenna");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_1,15,"3_2_7_2: 1 antenna defect, VT keeps on running");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_2,16,"3_2_7_3: more antennas defect, VT stops");
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_3,17,"3_2_7_3: more antennas defect, VT stops");
  m_testSequenceIt = m_testSequence.begin();

}

AVTTestMAC2Task::~AVTTestMAC2Task()
{
}

void AVTTestMAC2Task::run()
{
  start(); // make initial transition
  GCFTask::run();
}

GCFEvent::TResult AVTTestMAC2Task::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_timerPort.open();
      NEXTTEST(m_testSequenceIt);
      break;
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 3_2_4_1: Start VT, all antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_1(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_1: Start VT, all antennas in maintenance");
      
      // put board in maintenance.
      m_propertysetBoard1Maintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(1);
      if(!TESTC(GCF_NO_ERROR==m_propertysetBoard1Maintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
         
    case F_TIMER:
    {
      if(&p == &m_timerPort)
      {
        if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
        {
          m_timerPort.setTimer(1.0);
        }
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_Subrack1_Board1_Maintenance)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        if(!TESTC( inMaintenance.getValue()!=0 ))
        {
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
          //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
          string cmd("SCHEDULE 1,VT,");
          string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
          string times("0,0,");
          string freq("110.0,");
          string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
          string direction("AZEL,0.0,0.0");
          GCFPVString command(cmd+devices+times+freq+subbands+direction);

          TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR);

          if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
          {
            m_timerPort.setTimer(1.0);
          }
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(TESTC( status.getValue() == string("Releasing") ))
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // display the value:
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if( status.getValue() == string("Releasing") )
        {
          TESTC(true);
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          m_propertysetVT1.requestValue(PROPNAME_STATUS);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      GCFPVUnsigned outMaintenance(0);
      m_propertysetBoard1Maintenance.setValue(PROPNAME_STATUS,outMaintenance);
      m_propertysetBoard1Maintenance.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT1.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
      
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_2: Start VT, too many antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_2(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_2: 7 antennas required, 2 in maintenance, so only 6 available");
      
      // put AP1 in maintenance.
      m_maintenanceChangedCounter=0;
      m_propertysetAP1RCU1Maintenance.subscribeProp(PROPNAME_STATUS);
      m_propertysetAP1RCU2Maintenance.subscribeProp(PROPNAME_STATUS);
      GCFPVUnsigned inMaintenance(1);
      if(!TESTC(GCF_NO_ERROR==m_propertysetAP1RCU1Maintenance.setValue(PROPNAME_STATUS,inMaintenance) &&
                GCF_NO_ERROR==m_propertysetAP1RCU2Maintenance.setValue(PROPNAME_STATUS,inMaintenance)))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_TIMER:
    {
      if(&p == &m_timerPort)
      {
        if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
        {
          m_timerPort.setTimer(1.0);
        }
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance)!=0 ||
         strstr(pPropAnswer->pPropName,SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance)!=0)
      {
        m_maintenanceChangedCounter++;
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));

        if(!TESTC( inMaintenance.getValue()!=0 ))
        {
          NEXTTEST(m_testSequenceIt);
        }
        else if(m_maintenanceChangedCounter==2)
        {
          // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
          //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
          string cmd("SCHEDULE 2,VT,");
          string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
          string times("0,0,");
          string freq("110.0,");
          string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
          string direction("AZEL,0.0,0.0");
          GCFPVString command(cmd+devices+times+freq+subbands+direction);
          if(!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
          {
            NEXTTEST(m_testSequenceIt);
          }
          if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
          {
            m_timerPort.setTimer(1.0);
          }
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = ( status.getValue() == string("Releasing") );
        if(testOk)
        {
          TESTC(testOk);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      GCFPVUnsigned outMaintenance(0);
      m_propertysetAP1RCU1Maintenance.setValue(PROPNAME_STATUS,outMaintenance);
      m_propertysetAP1RCU2Maintenance.setValue(PROPNAME_STATUS,outMaintenance);
      m_propertysetAP1RCU1Maintenance.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetAP1RCU2Maintenance.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT1.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_3: Start VT, no antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_3(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_TIMER:
    {
      if(&p == &m_timerPort)
      {
        if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
        {
          m_timerPort.setTimer(1.0);
        }
      }
      break;
    }
    
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_3: Start VT, no antennas in maintenance");
      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 3,VT,");
      string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
      string times("0,0,");
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.0,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      if(GCF_NO_ERROR != m_propertysetVT1.subscribeProp(PROPNAME_STATUS))
      {
          m_timerPort.setTimer(1.0);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = ( status.getValue() == string("Active") );
        if(testOk)
        {
          TESTC(testOk);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetVT1.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_4: Start another VT, sharing resources with the first
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_4(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_TIMER:
    {
      if(&p == &m_timerPort)
      {
        if(GCF_NO_ERROR != m_propertysetVT2.subscribeProp(PROPNAME_STATUS))
        {
          m_timerPort.setTimer(1.0);
        }
      }
      break;
    }
    
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_4: Start another VT, sharing resources with the first");
      // start the second VT
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 5,VT,");
      string devices(string("VT2")+string(",")+string("BF2")+string(",")+string("SRG1")+string(","));
      string times("0,0,");
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      if(GCF_NO_ERROR != m_propertysetVT2.subscribeProp(PROPNAME_STATUS))
      {
        m_timerPort.setTimer(1.0);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          TESTC(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT2)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = ( status.getValue() == string("Active") );
        if(testOk)
        {
          TESTC(testOk);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT2.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_5: Change antenna parameter using the first VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_5(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_5: Change antenna parameter using the first VT");
      m_propertysetVT1.subscribeProp(PROPNAME_STATUS);
      m_suspendedCounter=0;

      // SUSPEND VT1
      string cmd("SUSPEND");
      GCFPVString command(cmd);
      if (!TESTC(m_propertysetVT1.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // the status of the VT has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_suspendedCounter++;
          if(m_suspendedCounter==1)
          {
            // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
            string cmd = "PREPARE ";
            string times("0,0,");
            string freq("100.0,");
            string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
            string direction("AZEL,0.0,0.0");
            GCFPVString command(cmd+times+freq+subbands+direction);
            if (!TESTC(m_propertysetVT1.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
            {
              NEXTTEST(m_testSequenceIt);
            }
          }
          else
          {
            // RESUME VT1
            string cmd = "RESUME";
            GCFPVString command(cmd);
            if (m_propertysetVT1.setValue(PROPNAME_COMMAND,command) != GCF_NO_ERROR)
            {
              TESTC(false);
              NEXTTEST(m_testSequenceIt);
            }
          }
        }
        else if(status.getValue() == string("Active"))
        {
          TESTC(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetVT1.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_6: Change antenna parameter using the second VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_6(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_6: Change antenna parameter using the second VT");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);
      m_propertysetVT2.subscribeProp(PROPNAME_STATUS);
      m_suspendedCounter=0;
      
      // SUSPEND VT2
      string cmd("SUSPEND");
      GCFPVString command(cmd);
      if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(!TESTC(status.getValue() != string("Error")))
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT2)!=0)
      {
        // the status of the VT has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_suspendedCounter++;
          if(m_suspendedCounter==1)
          {
            // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
            string cmd = "PREPARE ";
            string times("0,0,");
            string freq("120.0,");
            string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
            string direction("AZEL,0.2,0.0");
            GCFPVString command(cmd+times+freq+subbands+direction);
            if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
            {
              NEXTTEST(m_testSequenceIt);
            }
          }
          else
          {
            // RESUME VT2
            string cmd = "RESUME";
            GCFPVString command(cmd);
            if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
            {
              NEXTTEST(m_testSequenceIt);
            }
          }
        }
        else if(status.getValue() == string("Active"))
        {
          TESTC(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT2.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_7: Abort the first VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_7(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_7: Abort the first VT");
      m_propertysetVT1.subscribeProp(PROPNAME_STATUS);

      // SUSPEND VT1
      string cmd("SUSPEND");
      GCFPVString command(cmd);
      if (!TESTC(m_propertysetVT1.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT1)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          // RELEASE VT1
          string cmd("RELEASE");
          GCFPVString command(cmd);
          if (!TESTC(m_propertysetVT1.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
          {
            NEXTTEST(m_testSequenceIt);
          }
        }
        else if(status.getValue() == string("Releasing"))
        {
          TESTC(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetVT1.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_8: Change antenna parameter using the second VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_8(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_8: Change antenna parameter using the second VT");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);
      m_propertysetVT2.subscribeProp(PROPNAME_STATUS);
      m_suspendedCounter=0;
      
      // SUSPEND VT2
      string cmd("SUSPEND");
      GCFPVString command(cmd);
      if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(!TESTC(status.getValue() != string("Error")))
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT2)!=0)
      {
        // the status of the VT has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_suspendedCounter++;
          if(m_suspendedCounter==1)
          {
            // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
            string cmd = "PREPARE ";
            string times("0,0,");
            string freq("120.0,");
            string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
            string direction("AZEL,0.2,0.0");
            GCFPVString command(cmd+times+freq+subbands+direction);
            if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
            {
              NEXTTEST(m_testSequenceIt);
            }
          }
          else
          {
            // RESUME VT2
            string cmd = "RESUME";
            GCFPVString command(cmd);
            if (!TESTC(m_propertysetVT2.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
            {
              NEXTTEST(m_testSequenceIt);
            }
          }
        }
        else if(status.getValue() == string("Active"))
        {
          TESTC(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT2.unsubscribeProp(PROPNAME_STATUS);
      // stop the last running VT
      {
        // SUSPEND VT2
        string cmd("SUSPEND");
        GCFPVString command(cmd);
        m_propertysetVT2.setValue(PROPNAME_COMMAND,command);
      }
      {
        string cmd("RELEASE");
        GCFPVString command(cmd);
        m_propertysetVT2.setValue(PROPNAME_COMMAND,command);
      }
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_5_1: Schedule VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_5_1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_1: Schedule VT");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 6,VT,");
      string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
      
      // create time 1-1-2010 12:00:00
      struct tm utcTimeStruct;
      memset(&utcTimeStruct,0,sizeof(utcTimeStruct));
      utcTimeStruct.tm_year = 2010-1900;
      utcTimeStruct.tm_mon  = 0;
      utcTimeStruct.tm_mday = 1;
      utcTimeStruct.tm_hour = 12;
      utcTimeStruct.tm_min  = 0;
      utcTimeStruct.tm_sec  = 0;
      
      time_t startTime = mktime(&utcTimeStruct);
      time_t stopTime = startTime + 30; // starttime + 30 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",(int)startTime,(int)stopTime);
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() != string("Error"));
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_5_2: Schedule the same VT at non-overlapping timespans
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_5_2(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_2: Schedule the same VT at non-overlapping timespans");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 7,VT,");
      string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
      
      // create time 1-1-2010 12:00:00
      struct tm utcTimeStruct;
      memset(&utcTimeStruct,0,sizeof(utcTimeStruct));
      utcTimeStruct.tm_year = 2010-1900;
      utcTimeStruct.tm_mon  = 0;
      utcTimeStruct.tm_mday = 1;
      utcTimeStruct.tm_hour = 12;
      utcTimeStruct.tm_min  = 0;
      utcTimeStruct.tm_sec  = 0;
      
      time_t startTime = mktime(&utcTimeStruct) + 60;
      time_t stopTime = startTime + 30; // starttime + 30 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",(int)startTime,(int)stopTime);
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() != string("Error"));
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_5_3: Schedule the same VT at overlapping timespans
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_5_3(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_3: Schedule the same VT at overlapping timespans");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 8,VT,");
      string devices(string("VT1")+string(",")+string("BF1")+string(",")+string("SRG1")+string(","));
      
      // create time 1-1-2010 12:00:00
      struct tm utcTimeStruct;
      memset(&utcTimeStruct,0,sizeof(utcTimeStruct));
      utcTimeStruct.tm_year = 2010-1900;
      utcTimeStruct.tm_mon  = 0;
      utcTimeStruct.tm_mday = 1;
      utcTimeStruct.tm_hour = 12;
      utcTimeStruct.tm_min  = 0;
      utcTimeStruct.tm_sec  = 0;
      
      time_t startTime = mktime(&utcTimeStruct) + 10;
      time_t stopTime = startTime + 60; // starttime + 60 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",(int)startTime,(int)stopTime);

      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() == string("Error")); // Error is good
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_5_4: Cancel a VT Schedule
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_5_4(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_4: Cancel a VT Schedule");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // CANCEL <scheduleid>
      string cmd("CANCEL 7");
      GCFPVString command(cmd);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() != string("Error"));
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_5_5: Schedule two VT's at the same time
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_5_5(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_5: Schedule two VT's at the same time");
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 9,VT,");
      string devices(string("VT2")+string(",")+string("BF2")+string(",")+string("SRG2")+string(","));
      
      // create time 1-1-2010 12:00:00
      struct tm utcTimeStruct;
      memset(&utcTimeStruct,0,sizeof(utcTimeStruct));
      utcTimeStruct.tm_year = 2010-1900;
      utcTimeStruct.tm_mon  = 0;
      utcTimeStruct.tm_mday = 1;
      utcTimeStruct.tm_hour = 12;
      utcTimeStruct.tm_min  = 0;
      utcTimeStruct.tm_sec  = 0;
      
      time_t startTime = mktime(&utcTimeStruct);
      time_t stopTime = startTime + 30; // starttime + 30 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",(int)startTime,(int)stopTime);
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() != string("Error"));
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      // cancel all valid schedules
      // CANCEL <scheduleid>
      string cmd("CANCEL 6");
      GCFPVString command(cmd);
      m_propertysetLDS.setValue(PROPNAME_COMMAND,command);
      cmd = string("CANCEL 9");
      command.setValue(cmd);
      m_propertysetLDS.setValue(PROPNAME_COMMAND,command);

      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_6_1: Display subband statistics of a running VT
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_6_1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_6_1: Display subband statistics of a running VT");
      TESTC(false || "uh oh" );
      NEXTTEST(m_testSequenceIt);
      break;
    }
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_7_1: Antenna defect, no VT uses this antenna
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_7_1(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_TIMER:
    {
      if(&p == &m_timerPort)
      {
        if(GCF_NO_ERROR != m_propertysetVT3.subscribeProp(PROPNAME_STATUS))
        {
          m_timerPort.setTimer(1.0);
        }
      }
      break;
    }
    
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_1: Antenna defect, no VT uses this antenna");
      // start vt
      m_propertysetLDS.subscribeProp(PROPNAME_STATUS);

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      
      // NOTE: SRG2 uses RCU1 .. RCU4 (= AP1_RCU1,AP1_RCU2,AP2_RCU1,AP2_RCU2)
      string cmd("SCHEDULE 11,VT,");
      string devices(string("VT3")+string(",")+string("BF3")+string(",")+string("SRG2")+string(","));
      
      string times("0,0,");
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (!TESTC(m_propertysetLDS.setValue(PROPNAME_COMMAND,command) == GCF_NO_ERROR))
      {
        NEXTTEST(m_testSequenceIt);
      }
      if(GCF_NO_ERROR != m_propertysetVT3.subscribeProp(PROPNAME_STATUS))
      {
        m_timerPort.setTimer(1.0);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_LogicalDeviceScheduler)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(!TESTC(status.getValue() != string("Error")))
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT3)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Active"))
        {
          // VT is active
          TESTC(true);
          GCFPVUnsigned inError(1);
          TESTC(GCF_NO_ERROR==m_propertysetAP3RCU1.setValue(PROPNAME_STATUS,inError));
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetLDS.unsubscribeProp(PROPNAME_STATUS);
      m_propertysetVT3.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_7_2: 1 antenna defect, VT keeps on running
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_7_2(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_2: 1 antenna defect, VT keeps on running");

      GCFPVUnsigned inError(1);
      if(!TESTC(GCF_NO_ERROR==m_propertysetAP1RCU1.setValue(PROPNAME_STATUS,inError)))
      {
        NEXTTEST(m_testSequenceIt);
      }
      else
      {
        // get the status of VT3. it should be Active
        if(!TESTC(GCF_NO_ERROR==m_propertysetVT3.requestValue(PROPNAME_STATUS)))
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }      
    
    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT3)!=0)
      {
        // display the value:
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        TESTC(status.getValue() == string("Active"));
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_7_3: more antennas defect, VT stops
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_7_3(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_3: more antennas defect, VT stops");
      m_propertysetVT3.subscribeProp(PROPNAME_STATUS);

      GCFPVUnsigned inError(1);
      if(!TESTC(GCF_NO_ERROR==m_propertysetAP2RCU1.setValue(PROPNAME_STATUS,inError)))
      {
        FINISH;
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&event);
      
      if(strstr(pPropAnswer->pPropName,SCOPE_PAC_VT3)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          TESTC(true);
          FINISH;
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertysetVT3.unsubscribeProp(PROPNAME_STATUS);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * End of all tests
 */
GCFEvent::TResult AVTTestMAC2Task::finished(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      GCFTask::stop();
      break;

    default:
      LOG_DEBUG(formatString("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

