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

#ifdef GCF4
// datapoint structs are supported in gcf4
#define PROPERTY_BOARD1_MAINTENANCE_STATUS      "PIC_Rack1_SubRack1_Board1_Maintenance.status"
#define PROPERTY_AP1_MAINTENANCE_STATUS         "PIC_Rack1_SubRack1_Board1_AP1_Maintenance.status"
#define PROPERTY_SRG1_FREQUENCY                 "PAC_SRG1.frequency"
#define PROPERTY_AP1_RCU1_STATUS                "PIC_Rack1_SubRack1_Board1_AP1_RCU1.status"
#define PROPERTY_AP2_RCU1_STATUS                "PIC_Rack1_SubRack1_Board1_AP2_RCU1.status"
#define PROPERTY_AP2_RCU2_STATUS                "PIC_Rack1_SubRack1_Board1_AP2_RCU2.status"
#define PROPERTY_VT1_STATUS                     "PAC_VT1.status"
#define PROPERTY_VT2_STATUS                     "PAC_VT2.status"
#else
#define PROPERTY_BOARD1_MAINTENANCE_STATUS      "PIC_Rack1_SubRack1_Board1_Maintenance_status"
#define PROPERTY_AP1_MAINTENANCE_STATUS         "PIC_Rack1_SubRack1_Board1_AP1_Maintenance_status"
#define PROPERTY_SRG1_FREQUENCY                 "PAC_SRG1_frequency"
#define PROPERTY_AP1_RCU1_STATUS                "PIC_Rack1_SubRack1_Board1_AP1_RCU1_status"
#define PROPERTY_AP2_RCU1_STATUS                "PIC_Rack1_SubRack1_Board1_AP2_RCU1_status"
#define PROPERTY_AP2_RCU2_STATUS                "PIC_Rack1_SubRack1_Board1_AP2_RCU2_status"
#define PROPERTY_VT1_STATUS                     "PAC_VT1_status"
#define PROPERTY_VT2_STATUS                     "PAC_VT2_status"
#endif

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTTestMAC2Task.h"
#include "PropertyDefines.h" 

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "../src/LogicalDevice_Protocol.ph"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
using namespace AVT;
using namespace std;

string AVTTestMAC2Task::m_taskName("AVTTestMAC2");

AVTTestMAC2Task::AVTTestMAC2Task(AVTTest<AVTTestMAC2Task>& tester) :
  GCFTask((State)&AVTTestMAC2Task::initial, m_taskName),
  m_testSequence(),
  m_testSequenceIt(),
  m_tester(tester),
  m_answer(),
  m_propertyLDScommand(string(PROPERTY_LDS_COMMAND)),
  m_propertyLDSstatus(string(PROPERTY_LDS_STATUS)),
  m_propBoard1MaintenanceStatus(string(PROPERTY_BOARD1_MAINTENANCE_STATUS)),
  m_propAP1MaintenanceStatus(string(PROPERTY_AP1_MAINTENANCE_STATUS)),
  m_propSRG1Frequency(string(PROPERTY_SRG1_FREQUENCY)),
  m_propAP1RCU1Status(string(PROPERTY_AP1_RCU1_STATUS)),
  m_propAP2RCU1Status(string(PROPERTY_AP2_RCU1_STATUS)),
  m_propAP2RCU2Status(string(PROPERTY_AP2_RCU2_STATUS)),
  m_propVT1Status(string(PROPERTY_VT1_STATUS)),
  m_propVT2Status(string(PROPERTY_VT2_STATUS))

{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_1);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_2);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_3);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_4);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_5);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_6);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_7);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_4_8);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_1);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_2);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_3);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_4);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_5_5);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_6_1);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_1);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_2);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::test_3_2_7_3);
  ADDTRANTARGET(m_testSequence,AVTTestMAC2Task::finished);
  m_testSequenceIt = m_testSequence.begin();

  m_propertyLDScommand.setAnswer(&m_answer);
  m_propertyLDSstatus.setAnswer(&m_answer);
  m_propBoard1MaintenanceStatus.setAnswer(&m_answer);
  m_propAP1MaintenanceStatus.setAnswer(&m_answer);
  m_propAP1RCU1Status.setAnswer(&m_answer);
  m_propAP2RCU1Status.setAnswer(&m_answer);
  m_propAP2RCU2Status.setAnswer(&m_answer);
  m_propVT1Status.setAnswer(&m_answer);
  m_propVT2Status.setAnswer(&m_answer);
}

AVTTestMAC2Task::~AVTTestMAC2Task()
{
}

GCFEvent::TResult AVTTestMAC2Task::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      NEXTTEST(m_testSequenceIt);
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 3_2_4_1: Start VT, all antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_1: Start VT, all antennas in maintenance");
      
      // put board in maintenance.
      m_propBoard1MaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_propBoard1MaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
         
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_BOARD1_MAINTENANCE_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        bool testOk = ( inMaintenance.getValue()!=0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          // the board is in maintenance, now start the VT
          m_propertyLDSstatus.subscribe();
          
          // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
          //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
          string cmd("SCHEDULE 1,VT,");
          string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
          string times("0,0,");
          string freq("110.0,");
          string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
          string direction("AZEL,0.0,0.0");
          GCFPVString command(cmd+devices+times+freq+subbands+direction);
          if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
          {
            m_tester._avttest(false);
            NEXTTEST(m_testSequenceIt);
          }
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = ( status.getValue() == string("Error") );
        m_tester._avttest(testOk);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propBoard1MaintenanceStatus.unsubscribe();
      m_propertyLDSstatus.unsubscribe();
      break;
    }
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_2: Start VT, too many antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_2(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_2: 7 antennas required, 2 in maintenance, so only 6 available");
      
      // put AP1 in maintenance.
      m_propAP1MaintenanceStatus.subscribe();
      GCFPVUnsigned inMaintenance(1);
      bool testOk = (GCF_NO_ERROR==m_propAP1MaintenanceStatus.setValue(inMaintenance));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_AP1_MAINTENANCE_STATUS)!=0)
      {
        GCFPVUnsigned inMaintenance;
        inMaintenance.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %d",pPropAnswer->pPropName,inMaintenance.getValue()));
        bool testOk = ( inMaintenance.getValue()!=0 );
        m_tester._avttest(testOk);
        if(!testOk)
        {
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          // the board is in maintenance, now start the VT
          m_propertyLDSstatus.subscribe();
          
          // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
          //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
          string cmd("SCHEDULE 2,VT,");
          string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
          string times("0,0,");
          string freq("110.0,");
          string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
          string direction("AZEL,0.0,0.0");
          GCFPVString command(cmd+devices+times+freq+subbands+direction);
          if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
          {
            m_tester._avttest(false);
            NEXTTEST(m_testSequenceIt);
          }
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = ( status.getValue() == string("Error") );
        m_tester._avttest(testOk);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propAP1MaintenanceStatus.unsubscribe();
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_3: Start VT, no antennas in maintenance
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_3(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_3: Start VT, no antennas in maintenance");
      // put Board1 and AP1 out of maintenance.
      GCFPVUnsigned outMaintenance(0);
      bool testOk1 = (GCF_NO_ERROR==m_propBoard1MaintenanceStatus.setValue(outMaintenance));
      m_tester._avttest(testOk1);
      bool testOk2 = (GCF_NO_ERROR==m_propAP1MaintenanceStatus.setValue(outMaintenance));
      m_tester._avttest(testOk2);
      if(!testOk1 || !testOk2)
      {
        NEXTTEST(m_testSequenceIt);
      }
      else
      {
        // the resources are out of maintenance, now start the VT
        m_propertyLDSstatus.subscribe();
        m_propVT1Status.subscribe();

        // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
        //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
        string cmd("SCHEDULE 3,VT,");
        string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
        string times("0,0,");
        string freq("110.0,");
        string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
        string direction("AZEL,0.0,0.0");
        GCFPVString command(cmd+devices+times+freq+subbands+direction);
        if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Resumed"))
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
        else if(status.getValue() != string("Claimed") &&
                status.getValue() != string("Prepared"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      m_propVT1Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_4_4: Start another VT, sharing resources with the first
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_4_4(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_4: Start another VT, sharing resources with the first");
      // start the second VT
      m_propertyLDSstatus.subscribe();
      m_propVT2Status.subscribe();

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 5,VT,");
      string devices(string("VT2")+string(",")+string("SBF2")+string(",")+string("SRG1")+string(","));
      string times("0,0,");
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_VT2_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Resumed"))
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
        else if(status.getValue() != string("Claimed") &&
                status.getValue() != string("Prepared"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      m_propVT2Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_5: Change antenna parameter using the first VT");
      m_propertyLDSstatus.subscribe();
      m_propSRG1Frequency.subscribe();

      // SET FREQUENCY <vt_name>,<starttime>,<stoptime>,<frequency>
      string cmd("SET FREQUENCY ");
      string devices(string("VT1")+string(","));
      string times("0,0,");
      string freq("55.0,");
      GCFPVString command(cmd+devices+times+freq);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_SRG1_FREQUENCY)!=0)
      {
        // the frequency of SRG1 has changed
        GCFPVDouble frequency;
        frequency.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %f",pPropAnswer->pPropName,frequency.getValue()));
        m_tester._avttest(true);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propSRG1Frequency.unsubscribe();
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_6: Change antenna parameter using the second VT");
      m_propertyLDSstatus.subscribe();
      m_propSRG1Frequency.subscribe();

      // SET FREQUENCY <vt_name>,<starttime>,<stoptime>,<frequency>
      string cmd("SET FREQUENCY ");
      string devices(string("VT2")+string(","));
      string times("0,0,");
      string freq("110.0,");
      GCFPVString command(cmd+devices+times+freq);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error")) // expected, because the other VT is the owner
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_SRG1_FREQUENCY)!=0)
      {
        // the frequency of SRG1 has changed. this is not supposed to happen
        GCFPVDouble frequency;
        frequency.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %f",pPropAnswer->pPropName,frequency.getValue()));
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propSRG1Frequency.unsubscribe();
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_7: Abort the first VT");
      m_propVT1Status.subscribe();

      // SUSPEND <vt_name>
      string cmd("SUSPEND ");
      string devices(string("VT1"));
      GCFPVString command(cmd+devices);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_tester._avttest(true);
        }
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propVT1Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_4_8: Change antenna parameter using the second VT");
      m_propertyLDSstatus.subscribe();
      m_propSRG1Frequency.subscribe();

      // SET FREQUENCY <vt_name>,<starttime>,<stoptime>,<frequency>
      string cmd("SET FREQUENCY ");
      string devices(string("VT2")+string(","));
      string times("0,0,");
      string freq("110.0,");
      GCFPVString command(cmd+devices+times+freq);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_SRG1_FREQUENCY)!=0)
      {
        // the frequency of SRG1 has changed. this is not supposed to happen
        GCFPVDouble frequency;
        frequency.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %f",pPropAnswer->pPropName,frequency.getValue()));
        m_tester._avttest(true);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propSRG1Frequency.unsubscribe();
      m_propertyLDSstatus.unsubscribe();
      // stop the last running VT
      // SUSPEND <vt_name>
      string cmd("SUSPEND ");
      string devices(string("VT2"));
      GCFPVString command(cmd+devices);
      m_propertyLDScommand.setValue(command);
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_1: Schedule VT");
      m_propertyLDSstatus.subscribe();

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 6,VT,");
      string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
      
      // create time 10 seconds from now
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(10); // UTC + 10 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(30); // starttime + 30 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_2: Schedule the same VT at non-overlapping timespans");
      m_propertyLDSstatus.subscribe();

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 7,VT,");
      string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
      
      // create time 10 seconds from now
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(60); // UTC + 60 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(30); // starttime + 30 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
        else
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_3: Schedule the same VT at overlapping timespans");
      m_propertyLDSstatus.subscribe();

      // SCHEDULE <scheduleid>,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd("SCHEDULE 8,VT,");
      string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
      
      // create time 10 seconds from now
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(30); // UTC + 30 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(60); // starttime + 60 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(false);
        }
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_4: Cancel a VT Schedule");
      m_propertyLDSstatus.subscribe();

      // CANCEL <scheduleid>
      string cmd("CANCEL 7");
      GCFPVString command(cmd);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
        }
        else
        {
          m_tester._avttest(true);
        }
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_5_5: Schedule two VT's at the same time");
      m_propertyLDSstatus.subscribe();

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      string cmd1("SCHEDULE 9,VT,");
      string cmd2("SCHEDULE 10,VT,");
      string devices1(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG1")+string(","));
      string devices2(string("VT2")+string(",")+string("SBF2")+string(",")+string("SRG2")+string(","));
      
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(10); // UTC + 10 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(20); // starttime + 20 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command1(cmd1+devices1+times+freq+subbands+direction);
      GCFPVString command2(cmd2+devices2+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command1) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      if (m_propertyLDScommand.setValue(command2) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(false);
        }
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_6_1: Display subband statistics of a running VT");
      m_tester._avttest(false);
      NEXTTEST(m_testSequenceIt);
      break;
    }
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

/* 
 * Test case 3_2_7_1: Antenna defect, no VT uses this antenna
 */
GCFEvent::TResult AVTTestMAC2Task::test_3_2_7_1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_1: Antenna defect, no VT uses this antenna");
      // start vt
      m_propertyLDSstatus.subscribe();
      m_propVT1Status.subscribe();

      // SCHEDULE <scheduleid>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
      
      // NOTE: SRG3 has to be configured to use RCU2..8, NOT RCU1
      string cmd("SCHEDULE 11,VT,");
      string devices(string("VT1")+string(",")+string("SBF1")+string(",")+string("SRG3")+string(","));
      
      boost::posix_time::ptime startTime(boost::posix_time::second_clock::universal_time());
      startTime += boost::posix_time::seconds(0); // UTC + 0 seconds
      boost::posix_time::ptime stopTime(startTime);
      stopTime += boost::posix_time::seconds(10); // starttime + 10 seconds
      char timesString[100];
      sprintf(timesString,"%d,%d,",startTime.time_of_day().seconds(),stopTime.time_of_day().seconds());
      
      string times(timesString);
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.2,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_LDS_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Error"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Resumed"))
        {
          // VT is active
          m_tester._avttest(true);
          GCFPVUnsigned inError(1);
          bool testOk = (GCF_NO_ERROR==m_propAP1RCU1Status.setValue(inError));
          m_tester._avttest(testOk);
          if(!testOk)
          {
            NEXTTEST(m_testSequenceIt);
          }
        }
        else if(status.getValue() != string("Claimed") &&
                status.getValue() != string("Prepared"))
        {
          m_tester._avttest(false);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propertyLDSstatus.unsubscribe();
      m_propVT1Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_2: 1 antenna defect, VT keeps on running");
      m_propVT1Status.subscribe();

      GCFPVUnsigned inError(1);
      bool testOk = (GCF_NO_ERROR==m_propAP2RCU1Status.setValue(inError));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        NEXTTEST(m_testSequenceIt);
      }
      else
      {
        // get the status of VT1. it should be Resumed
        testOk = (GCF_NO_ERROR==m_propVT1Status.requestValue());
        m_tester._avttest(testOk);
        if(!testOk)
        {
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_tester._avttest(false);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_VGETRESP:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // display the value:
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        bool testOk = (status.getValue() == string("Resumed"));
        m_tester._avttest(testOk);
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propVT1Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Test case 3_2_7_3: more antennas defect, VT stops");
      m_propVT1Status.subscribe();

      GCFPVUnsigned inError(1);
      bool testOk = (GCF_NO_ERROR==m_propAP2RCU2Status.setValue(inError));
      m_tester._avttest(testOk);
      if(!testOk)
      {
        NEXTTEST(m_testSequenceIt);
      }
      break;
    }      
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      if(strstr(pPropAnswer->pPropName,PROPERTY_VT1_STATUS)!=0)
      {
        // the status of the Logical device scheduler has changed
        GCFPVString status;
        status.copy(*pPropAnswer->pValue);
        LOG_INFO(formatString("Value of '%s': %s",pPropAnswer->pPropName,status.getValue().c_str()));
        if(status.getValue() == string("Suspended"))
        {
          m_tester._avttest(true);
          NEXTTEST(m_testSequenceIt);
        }
      }
      break;
    }
    
    case F_EXIT:
    {
      m_propVT1Status.unsubscribe();
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestMAC2Task(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
      GCFTask::stop();
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTTestTask(%s)::%s, default",getName().c_str(),__func__));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

