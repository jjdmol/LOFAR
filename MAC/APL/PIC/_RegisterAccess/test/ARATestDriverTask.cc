//#  ARATestDriverTask.cc: Implementation of the 
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

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "RSP_Protocol.ph"

#include "ARAConstants.h"
#include "ARAPropertyDefines.h"
#include "../../../APLCommon/src/APL_Defines.h"
#include "ARATestDriverTask.h"

#include <stdio.h>
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>

using namespace LOFAR;
using namespace ARA;
using namespace std;

string ARATestDriverTask::m_taskName("ARATestDriver");

ARATestDriverTask::ARATestDriverTask() :
  GCFTask((State)&ARATestDriverTask::initial, m_taskName),
  m_answer(),
  m_RSPserver(),
  m_propMap(),
  m_bpStatus(0),
  m_ap1Status(0),
  m_ap2Status(0),
  m_ap3Status(0),
  m_ap4Status(0),
  m_ethStatus(0),
  m_rcu1Status(0),
  m_rcu2Status(0),
  m_rcu3Status(0),
  m_rcu4Status(0),
  m_rcu5Status(0),
  m_rcu6Status(0),
  m_rcu7Status(0),
  m_rcu8Status(0)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);

  // fill APCs map
  addPropertySet(SCOPE_PIC);
  addPropertySet(SCOPE_PIC_Rack1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_ETH);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_BP);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics);
  addPropertySet(SCOPE_PIC_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance);
  addPropertySet(SCOPE_PIC_Rack1_Alert);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Alert);
  addPropertySet(SCOPE_PIC_Rack1_SubRack1_Board1_Alert);
  
  m_RSPserver.init(*this, "ARAtestRSPserver", GCFPortInterface::SPP, RSP_PROTOCOL);
  
}

ARATestDriverTask::~ARATestDriverTask()
{
}

void ARATestDriverTask::addPropertySet(string scope)
{
  if(scope == string(SCOPE_PIC))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Station),sizeof(PROPS_Station)/sizeof(PROPS_Station[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Rack),sizeof(PROPS_Rack)/sizeof(PROPS_Rack[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_SubRack),sizeof(PROPS_SubRack)/sizeof(PROPS_SubRack[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1_Board1))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Board),sizeof(PROPS_Board)/sizeof(PROPS_Board[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_ETH))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Ethernet),sizeof(PROPS_Ethernet)/sizeof(PROPS_Ethernet[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_BP) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_FPGA),sizeof(PROPS_FPGA)/sizeof(PROPS_FPGA[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_RCU),sizeof(PROPS_RCU)/sizeof(PROPS_RCU[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_ADCStatistics),sizeof(PROPS_ADCStatistics)/sizeof(PROPS_ADCStatistics[0]));
  }
  else if(scope == string(SCOPE_PIC_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
  }
  else if(scope == string(SCOPE_PIC_Rack1_Alert) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Alert) ||
     scope == string(SCOPE_PIC_Rack1_SubRack1_Board1_Alert))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Alert),sizeof(PROPS_Alert)/sizeof(PROPS_Alert[0]));
  }
}

void ARATestDriverTask::addAllProperties(string scope, TProperty* ptp, int numProperties)
{
  for(unsigned int i=0;i<numProperties;i++)
  {
    string propName = scope+string("_")+string(ptp[i].propName);
    boost::shared_ptr<GCFProperty> propPtr(new GCFProperty(propName));
    propPtr->setAnswer(&m_answer);
    m_propMap[propName]=propPtr;
  }
}

void ARATestDriverTask::subscribeAllProperties()
{
  TPropertyMap::iterator it;
  for(it=m_propMap.begin();it!=m_propMap.end();++it)
  {
    it->second->subscribe();
  }
}

void ARATestDriverTask::updateETHstatus(string& propName,unsigned int& ethStatus,GCFPVUnsigned& pvUnsigned)
{
  LOG_INFO(formatString("updateETHstatus %s", propName.c_str()));
  
  // layout eth status: 
  // 31......24  23.....16  15........8  7........0       
  // #RX[15..8]  #RX[7..0]  #Err[15..8]  #Err[7..0]  
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_PACKETSRECEIVED),0) != string::npos)
  {
    unsigned int tempStatus = ethStatus & 0x0000FFFF; // reset bits
    tempStatus = tempStatus | ((pvUnsigned.getValue()&0xFFFF)<<16);
    updateSystemStatus(ethStatus,tempStatus);
  }
  else if(propName.find(string(PROPNAME_PACKETSERROR),0) != string::npos)
  {
    unsigned int tempStatus = ethStatus & 0xFFFF0000; // reset bits
    tempStatus = tempStatus | (pvUnsigned.getValue()&0xFFFF);
    updateSystemStatus(ethStatus,tempStatus);
  }
}
  
void ARATestDriverTask::updateFPGAstatus(string& propName,unsigned int& fpgaStatus,GCFPVBool& pvBool, GCFPVDouble& pvDouble)
{
  LOG_INFO(formatString("updateFPGAstatus %s", propName.c_str()));
  
  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_ALIVE),0) != string::npos)
  {
    unsigned int tempStatus = fpgaStatus & 0x000000FF; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvBool.getValue())<<8);
    updateSystemStatus(fpgaStatus,tempStatus);
  }
  else if(propName.find(string(PROPNAME_TEMPERATURE),0) != string::npos)
  {
    unsigned int tempStatus = fpgaStatus & 0x00000100; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvDouble.getValue()*100)&0xFF); // mult with 100 to get 2 significant decimals
    updateSystemStatus(fpgaStatus,tempStatus);
  }
  if(propName.find(string(PROPNAME_VERSION),0) != string::npos)
  {
    // nothing to be done here
  }
}

void ARATestDriverTask::updateRCUstatus(string& propName,unsigned int& rcuStatus,GCFPVBool& pvBool)
{
  LOG_INFO(formatString("updateRCUstatus %s", propName.c_str()));
  
  // layout rcu status: 
  // 7 6       5       4       3 2 1 0
  // - ringerr rcuHerr rcuVerr - - - statsReady
  if(propName.find(string(PROPNAME_RINGERR),0) != string::npos)
  {
    unsigned int tempStatus = rcuStatus & 0x00000031; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvBool.getValue())<<6);
    updateSystemStatus(rcuStatus,tempStatus);
  }
  else if(propName.find(string(PROPNAME_RCUHERR),0) != string::npos)
  {
    unsigned int tempStatus = rcuStatus & 0x00000051; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvBool.getValue())<<5);
    updateSystemStatus(rcuStatus,tempStatus);
  }
  else if(propName.find(string(PROPNAME_RCUVERR),0) != string::npos)
  {
    unsigned int tempStatus = rcuStatus & 0x00000061; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvBool.getValue())<<4);
    updateSystemStatus(rcuStatus,tempStatus);
  }
  if(propName.find(string(PROPNAME_STATSREADY),0) != string::npos)
  {
    unsigned int tempStatus = rcuStatus & 0x00000070; // reset bits
    tempStatus = tempStatus | ((unsigned int)(pvBool.getValue())<<0);
    updateSystemStatus(rcuStatus,tempStatus);
  }
}

void ARATestDriverTask::updateSystemStatus(unsigned int& statusItem,unsigned int newStatus)
{
  LOG_INFO(formatString("updateSystemStatus %d -> %d", statusItem,newStatus));
  
  // statusItem is a reference to the member variable that may have to be changed.
  // 
  if(statusItem != newStatus)
  {
    LOG_INFO(formatString("updateSystemStatus: status changed (%d -> %d), sending UPDSTATUS", statusItem,newStatus));
    statusItem = newStatus;
    
    // send new status to RA application
    RSPUpdstatusEvent updStatusEvent;
    struct timeval timeValNow;
    time(&timeValNow.tv_sec);
    timeValNow.tv_usec=0;
    updStatusEvent.timestamp.set(timeValNow);
    updStatusEvent.status=0; // ignore ??
    updStatusEvent.handle=1; // ignore
    blitz::Array<uint16, 1>& ap_status = updStatusEvent.sysstatus.ap_status();
    blitz::Array<uint16, 1>& bp_status = updStatusEvent.sysstatus.bp_status();
    blitz::Array<uint32, 1>& eth_status = updStatusEvent.sysstatus.eth_status();
    blitz::Array<uint16, 1>& rcu_status = updStatusEvent.sysstatus.rcu_status();
    
    LOG_ERROR("bp_status(0) = m_bpStatus; crashes!!");
/*
    bp_status(0) = m_bpStatus; // crash!!
    ap_status(0) = m_ap1Status;
    ap_status(1) = m_ap2Status;
    ap_status(2) = m_ap3Status;
    ap_status(3) = m_ap4Status;
    eth_status(0) = m_ethStatus;
    rcu_status(0) = m_rcu1Status;
    rcu_status(1) = m_rcu2Status;
    rcu_status(2) = m_rcu3Status;
    rcu_status(3) = m_rcu4Status;
    rcu_status(4) = m_rcu5Status;
    rcu_status(5) = m_rcu6Status;
    rcu_status(6) = m_rcu7Status;
    rcu_status(7) = m_rcu8Status;
*/
    m_RSPserver.send(updStatusEvent);
  }  
}

bool ARATestDriverTask::isEnabled()
{
  return (m_RSPserver.isConnected());
}

GCFEvent::TResult ARATestDriverTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestDriverTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
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
        TRAN(ARATestDriverTask::enabled);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestDriverTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 1: Monitor FPGA registers. Goal: load secondary properties
 */
GCFEvent::TResult ARATestDriverTask::enabled(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestDriverTask(%s)::enabled (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      // subscribe to all properties of all property sets.
      subscribeAllProperties();
      break;
    }

    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      LOG_INFO(formatString("property changed: %s", pPropAnswer->pPropName));
      GCFPVUnsigned pvUnsigned;
      GCFPVDouble   pvDouble;
      GCFPVBool     pvBool;
      GCFPVString   pvString;
      switch(pPropAnswer->pValue->getType())
      {
        case GCFPValue::LPT_BOOL:
          pvBool.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property value: %d", pvBool.getValue()));
          break;
        case GCFPValue::LPT_UNSIGNED:
          pvUnsigned.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %d", pvUnsigned.getValue()));
          break;
        case GCFPValue::LPT_DOUBLE:
          pvDouble.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %f", pvDouble.getValue()));
          break;
        case GCFPValue::LPT_STRING:
          pvString.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s", pvString.getValue().c_str()));
          break;
        case GCFPValue::NO_LPT:
        case GCFPValue::LPT_CHAR:
        case GCFPValue::LPT_INTEGER:
        case GCFPValue::LPT_BIT32:
        case GCFPValue::LPT_BLOB:
        case GCFPValue::LPT_REF:
        case GCFPValue::LPT_DATETIME:
        case GCFPValue::LPT_DYNARR:
        case GCFPValue::LPT_DYNBOOL:
        case GCFPValue::LPT_DYNCHAR:
        case GCFPValue::LPT_DYNUNSIGNED:
        case GCFPValue::LPT_DYNINTEGER:
        case GCFPValue::LPT_DYNBIT32:
        case GCFPValue::LPT_DYNBLOB:
        case GCFPValue::LPT_DYNREF:
        case GCFPValue::LPT_DYNDOUBLE:
        case GCFPValue::LPT_DYNDATETIME:
        case GCFPValue::LPT_DYNSTRING:
        default:
          break;
      }
      // for now, we are only interested in changes of the BP status, AP status,
      // ETH status or RCU status;
      string propName(pPropAnswer->pPropName);
      if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_ETH),0) != string::npos)
      {
        updateETHstatus(propName,m_ethStatus,pvUnsigned);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_BP),0) != string::npos)
      {
        updateFPGAstatus(propName,m_bpStatus,pvBool,pvDouble);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1),0) != string::npos)
      {
        updateFPGAstatus(propName,m_ap1Status,pvBool,pvDouble);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2),0) != string::npos)
      {
        updateFPGAstatus(propName,m_ap2Status,pvBool,pvDouble);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3),0) != string::npos)
      {
        updateFPGAstatus(propName,m_ap3Status,pvBool,pvDouble);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4),0) != string::npos)
      {
        updateFPGAstatus(propName,m_ap4Status,pvBool,pvDouble);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu1Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu2Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu3Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu4Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu5Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu6Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu7Status,pvBool);
      }
      else if(propName.find(string(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2),0) != string::npos)
      {
        updateRCUstatus(propName,m_rcu8Status,pvBool);
      }
      break;
    }
     
    case F_DISCONNECTED:
    {
     LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
     port.close();

     TRAN(ARATestDriverTask::initial);
      break;
    }

    case F_EXIT:
    {
      break;
    }

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestDriverTask(%s)::test1, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

