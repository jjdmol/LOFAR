//#
//#  ARARegisterAccessTask.cc: implementation of ARARegisterAccessTask class
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "RSP_Protocol.ph"

#include "ARARegisterAccessTask.h"

#include "ARAConstants.h"

#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>
#include "ARAPropertyDefines.h"

using namespace LOFAR;
using namespace ARA;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

string RegisterAccessTask::m_RSPserverName("ARAtestRSPserver");

RegisterAccessTask::RegisterAccessTask(string name)
    : GCFTask((State)&RegisterAccessTask::initial, name),
      m_answer(),
      m_myPropertySetMap(),
      m_APCMap(),
      m_myPropsLoaded(false),
      m_myPropsLoadCounter(0),
      m_APCsLoaded(false),
      m_APCsLoadCounter(0),
      m_RSPclient(*this, m_RSPserverName, GCFPortInterface::SAP, RSP_PROTOCOL)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  // fill MyPropertySets map
  addMyPropertySet(PROPSET_PIC, SCOPE_PIC);
  addMyPropertySet(PROPSET_Rack1, SCOPE_PIC_Rack1);
  addMyPropertySet(PROPSET_SubRack1, SCOPE_PIC_Rack1_SubRack1);
  addMyPropertySet(PROPSET_Board1, SCOPE_PIC_Rack1_SubRack1_Board1);
  addMyPropertySet(PROPSET_ETH, SCOPE_PIC_Rack1_SubRack1_Board1_ETH);
  addMyPropertySet(PROPSET_BP, SCOPE_PIC_Rack1_SubRack1_Board1_BP);
  addMyPropertySet(PROPSET_AP1, SCOPE_PIC_Rack1_SubRack1_Board1_AP1);
  addMyPropertySet(PROPSET_AP2, SCOPE_PIC_Rack1_SubRack1_Board1_AP2);
  addMyPropertySet(PROPSET_AP3, SCOPE_PIC_Rack1_SubRack1_Board1_AP3);
  addMyPropertySet(PROPSET_AP4, SCOPE_PIC_Rack1_SubRack1_Board1_AP4);
  addMyPropertySet(PROPSET_RCU1, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1);
  addMyPropertySet(PROPSET_RCU2, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2);
  addMyPropertySet(PROPSET_RCU1, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1);
  addMyPropertySet(PROPSET_RCU2, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2);
  addMyPropertySet(PROPSET_RCU1, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1);
  addMyPropertySet(PROPSET_RCU2, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2);
  addMyPropertySet(PROPSET_RCU1, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1);
  addMyPropertySet(PROPSET_RCU2, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics);
  addMyPropertySet(PROPSET_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance);
  addMyPropertySet(PROPSET_Alert, SCOPE_PIC_Rack1_Alert);
  addMyPropertySet(PROPSET_Alert, SCOPE_PIC_Rack1_SubRack1_Alert);
  addMyPropertySet(PROPSET_Alert, SCOPE_PIC_Rack1_SubRack1_Board1_Alert);
  
  // fill APCs map
  addAPC(APC_Station, SCOPE_PIC);
  addAPC(APC_Rack, SCOPE_PIC_Rack1);
  addAPC(APC_SubRack, SCOPE_PIC_Rack1_SubRack1);
  addAPC(APC_Board, SCOPE_PIC_Rack1_SubRack1_Board1);
  addAPC(APC_Ethernet, SCOPE_PIC_Rack1_SubRack1_Board1_ETH);
  addAPC(APC_FPGA, SCOPE_PIC_Rack1_SubRack1_Board1_BP);
  addAPC(APC_FPGA, SCOPE_PIC_Rack1_SubRack1_Board1_AP1);
  addAPC(APC_FPGA, SCOPE_PIC_Rack1_SubRack1_Board1_AP2);
  addAPC(APC_FPGA, SCOPE_PIC_Rack1_SubRack1_Board1_AP3);
  addAPC(APC_FPGA, SCOPE_PIC_Rack1_SubRack1_Board1_AP4);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1);
  addAPC(APC_RCU, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_ADCStatistics);
  addAPC(APC_ADCStatistics, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_ADCStatistics);
  addAPC(APC_Maintenance, SCOPE_PIC_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1_Maintenance);
  addAPC(APC_Maintenance, SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2_Maintenance);
  addAPC(APC_Alert, SCOPE_PIC_Rack1_Alert);
  addAPC(APC_Alert, SCOPE_PIC_Rack1_SubRack1_Alert);
  addAPC(APC_Alert, SCOPE_PIC_Rack1_SubRack1_Board1_Alert);
}

RegisterAccessTask::~RegisterAccessTask()
{
}

void RegisterAccessTask::addMyPropertySet(const TPropertySet& propset,const char* scope)
{
  boost::shared_ptr<GCFMyPropertySet> propsPtr(new GCFMyPropertySet(propset,scope,&m_answer));
  m_myPropertySetMap[scope]=propsPtr;
}

void RegisterAccessTask::addAPC(string apc,string scope)
{
  boost::shared_ptr<GCFApc> apcPtr(new GCFApc(apc,scope,&m_answer));
  m_APCMap[scope]=apcPtr;
}

bool RegisterAccessTask::isConnected()
{
  return m_RSPclient.isConnected();
}

GCFEvent::TResult RegisterAccessTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      break;
    }

    case F_ENTRY:
    {
      LOG_INFO("Loading MyPropertySets...");
      m_myPropsLoadCounter=0;
      TMyPropertySetMap::iterator it;
      for(it=m_myPropertySetMap.begin();it!=m_myPropertySetMap.end();++it)
      {
        it->second->load();
      }
      break;
    }

    case F_MYPLOADED:
    {
      m_myPropsLoadCounter++;
      LOG_INFO(formatString("MyPropset %d loaded", m_myPropsLoadCounter));
      if(m_myPropsLoadCounter == m_myPropertySetMap.size())
      {
        m_myPropsLoaded=true;
        TRAN(RegisterAccessTask::myPropSetsLoaded);
      }
      break;
    }
    
    case F_EXIT:
    {
      // cancel timers
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::myPropSetsLoaded(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      break;
    }

    case F_ENTRY:
    {
      LOG_INFO("Loading APCs...");
      m_APCsLoadCounter=0;
      TAPCMap::iterator it;
      for(it=m_APCMap.begin();it!=m_APCMap.end();++it)
      {
        it->second->load();
      }
      break;
    }

    case F_APCLOADED:
    {
      m_APCsLoadCounter++;
      LOG_INFO(formatString("APC %d loaded", m_APCsLoadCounter));
      if(m_APCsLoadCounter == m_APCMap.size())
      {
        m_APCsLoaded=true;
        TRAN(RegisterAccessTask::APCsLoaded);
      }
      break;
    }
    
    case F_EXIT:
    {
    	// cancel timers
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::APCsLoaded(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      break;
    }

    case F_ENTRY:
    {
      if (!m_RSPclient.isConnected()) 
      {
        m_RSPclient.open(); // need this otherwise GTM_Sockethandler is not called
      }
      break;
    }

    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      if (isConnected())
      {
        TRAN(RegisterAccessTask::connected);
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

    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&e);
      assert(pPropAnswer);
      LOG_INFO(formatString("property '%s' changed", pPropAnswer->pPropName));
      break;
    }
    
    case F_EXIT:
    {
      // cancel timers
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::connected(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      break;
    }

    case F_TIMER:
    {
      break;
    }

    case RSP_UPDSTATUS:
    {
      LOG_INFO("RSP_UPDSTATUS received");
      status = handleUpdStatus(e,port);
      break;
    }
    
    case F_DISCONNECTED:
    {
    	LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
    	port.close();

    	TRAN(RegisterAccessTask::APCsLoaded);
      break;
    }

    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&e);
      assert(pPropAnswer);
      LOG_INFO(formatString("property '%s' changed", pPropAnswer->pPropName));
      break;
    }
    
    case F_EXIT:
    {
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::handleUpdStatus(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  {
    RSPUpdstatusEvent updStatusEvent(e);
    struct timeval timestamp;
    updStatusEvent.timestamp.get(&timestamp);

    unsigned int eth_status = updStatusEvent.sysstatus.eth_status()(0);
    unsigned int bp_status = updStatusEvent.sysstatus.bp_status()(0);
    unsigned int ap1_status = updStatusEvent.sysstatus.ap_status()(0);
    unsigned int ap2_status = updStatusEvent.sysstatus.ap_status()(1);
    unsigned int ap3_status = updStatusEvent.sysstatus.ap_status()(2);
    unsigned int ap4_status = updStatusEvent.sysstatus.ap_status()(3);
    unsigned int rcu1_status = updStatusEvent.sysstatus.rcu_status()(0);
    unsigned int rcu2_status = updStatusEvent.sysstatus.rcu_status()(1);
    unsigned int rcu3_status = updStatusEvent.sysstatus.rcu_status()(2);
    unsigned int rcu4_status = updStatusEvent.sysstatus.rcu_status()(3);
    unsigned int rcu5_status = updStatusEvent.sysstatus.rcu_status()(4);
    unsigned int rcu6_status = updStatusEvent.sysstatus.rcu_status()(5);
    unsigned int rcu7_status = updStatusEvent.sysstatus.rcu_status()(6);
    unsigned int rcu8_status = updStatusEvent.sysstatus.rcu_status()(7);
  
    LOG_INFO(formatString("UpdStatus:\n\ttime: \t%s\n\tstatus:\t%d\n\thandle:\t%d\n\teth:\t%d\n\tbp:\t%d\n\tap1:\t%d\n\tap2:\t%d\n\tap3:\t%d\n\tap4:\t%d\n\trcu1:\t%d\n\trcu2:\t%d\n\trcu3:\t%d\n\trcu4:\t%d\n\trcu5:\t%d\n\trcu6:\t%d\n\trcu7:\t%d\n\trcu8:\t%d\n", 
        ctime(&timestamp.tv_sec),
        updStatusEvent.status,
        updStatusEvent.handle,
        eth_status,
        bp_status,
        ap1_status,
        ap2_status,
        ap3_status,
        ap4_status,
        rcu1_status,
        rcu2_status,
        rcu3_status,
        rcu4_status,
        rcu5_status,
        rcu6_status,
        rcu7_status,
        rcu8_status
        ));
  
    updateETHproperties(SCOPE_PIC_Rack1_SubRack1_Board1_ETH,eth_status);  
    updateFPGAproperties(SCOPE_PIC_Rack1_SubRack1_Board1_BP,bp_status);
    updateFPGAproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP1,ap1_status);
    updateFPGAproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP2,ap2_status);
    updateFPGAproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP3,ap3_status);
    updateFPGAproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP4,ap4_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU1,rcu1_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP1_RCU2,rcu2_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU1,rcu3_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP2_RCU2,rcu4_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU1,rcu5_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP3_RCU2,rcu6_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU1,rcu7_status);
    updateRCUproperties(SCOPE_PIC_Rack1_SubRack1_Board1_AP4_RCU2,rcu8_status);
    
  }
  LOG_INFO("Komtiehier???");
  
  return status;
}

void RegisterAccessTask::updateETHproperties(string scope,unsigned int status)
{
  // layout eth status: 
  // 31......24  23.....16  15........8  7........0       
  // #RX[15..8]  #RX[7..0]  #Err[15..8]  #Err[7..0]  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    unsigned int tempStatus = (status >> 16) & 0xFFFF;
    GCFPVUnsigned pvTemp(tempStatus);
    it->second->setValue(string(PROPNAME_PACKETSRECEIVED),pvTemp);
    
    tempStatus = status & 0xFFFF;
    pvTemp.setValue(tempStatus);
    it->second->setValue(string(PROPNAME_PACKETSERROR),pvTemp);
  }
}

void RegisterAccessTask::updateFPGAproperties(string scope,unsigned int status)
{
  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    unsigned int tempStatus = (status >> 8) & 0x01;
    GCFPVBool pvBool(tempStatus);
    it->second->setValue(string(PROPNAME_ALIVE),pvBool);
    
    tempStatus = status & 0xFF;
    GCFPVDouble pvDouble(static_cast<double>(tempStatus)/100.0);
    it->second->setValue(string(PROPNAME_TEMPERATURE),pvDouble);
  }
}

void RegisterAccessTask::updateRCUproperties(string scope,unsigned int status)
{
  // layout rcu status: 
  // 7 6       5       4       3 2 1 0
  // - ringerr rcuHerr rcuVerr - - - statsReady
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    unsigned int tempStatus = (status >> 6 ) & 0x01;
    GCFPVBool pvBool(tempStatus);
    it->second->setValue(string(PROPNAME_RINGERR),pvBool);
    
    tempStatus = (status >> 5) & 0x01;
    pvBool.setValue(tempStatus);
    it->second->setValue(string(PROPNAME_RCUHERR),pvBool);

    tempStatus = (status >> 4) & 0x01;
    pvBool.setValue(tempStatus);
    it->second->setValue(string(PROPNAME_RCUVERR),pvBool);

    tempStatus = (status >> 0) & 0x01;
    pvBool.setValue(tempStatus);
    it->second->setValue(string(PROPNAME_STATSREADY),pvBool);
  }
}
