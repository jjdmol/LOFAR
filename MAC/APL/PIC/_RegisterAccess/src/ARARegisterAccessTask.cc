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
#include <GCF/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include "ARAPropertyDefines.h"
#include "ARAPhysicalModel.h"

using namespace GCF;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;


namespace LOFAR
{

namespace ARA
{

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
      m_RSPclient(*this, m_RSPserverName, GCFPortInterface::SAP, RSP_PROTOCOL),
      m_physicalModel(),
      m_subStatusHandle(0),
      m_subStatsHandleSubbandPower(0),
      m_subStatsHandleBeamletPower(0),
      m_n_racks(1),
      m_n_subracks_per_rack(1),
      m_n_boards_per_subrack(1),
      m_n_aps_per_board(1),
      m_n_rcus_per_ap(2),
      m_n_rcus(2),
      m_status_update_interval(1),
      m_stats_update_interval(1)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  ParameterSet::instance()->adoptFile("RegisterAccess.conf");

  char scopeString[300];
  int rack;
  int subrack;
  int board;
  int ap;
  int rcu;
  
  m_n_racks               = ParameterSet::instance()->getInt(PARAM_N_RACKS);
  m_n_subracks_per_rack   = ParameterSet::instance()->getInt(PARAM_N_SUBRACKS_PER_RACK);
  m_n_boards_per_subrack  = ParameterSet::instance()->getInt(PARAM_N_BOARDS_PER_SUBRACK);
  m_n_aps_per_board       = ParameterSet::instance()->getInt(PARAM_N_APS_PER_BOARD);
  m_n_rcus_per_ap         = ParameterSet::instance()->getInt(PARAM_N_RCUS_PER_AP);
  m_n_rcus                = m_n_rcus_per_ap*
                              m_n_aps_per_board*
                              m_n_boards_per_subrack*
                              m_n_subracks_per_rack*
                              m_n_racks;
  m_status_update_interval = ParameterSet::instance()->getInt(PARAM_STATUS_UPDATE_INTERVAL);
  m_stats_update_interval  = ParameterSet::instance()->getInt(PARAM_STATISTICS_UPDATE_INTERVAL);
  
  // fill MyPropertySets map
  addMyPropertySet(SCOPE_PIC, TYPE_LCU_PIC, PSCAT_LCU_PIC, PROPS_Station);
  addMyPropertySet(SCOPE_PIC_Maintenance, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
  for(rack=1;rack<=m_n_racks;rack++)
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Rack, PSCAT_LCU_PIC_Rack, PROPS_Rack);
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);

    for(subrack=1;subrack<=m_n_subracks_per_rack;subrack++)
    {
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_SubRack, PSCAT_LCU_PIC_SubRack, PROPS_SubRack);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);
      
      for(board=1;board<=m_n_boards_per_subrack;board++)
      {
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Board, PSCAT_LCU_PIC_Board, PROPS_Board);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_MEPStatus, PSCAT_LCU_PIC_MEPStatus, PROPS_MEPStatus);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_MEPStatus, PSCAT_LCU_PIC_MEPStatus, PROPS_MEPStatus);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Ethernet, PSCAT_LCU_PIC_Ethernet, PROPS_Ethernet);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_FPGA, PSCAT_LCU_PIC_FPGA, PROPS_FPGA);
    
        for(ap=1;ap<=m_n_aps_per_board;ap++)
        {
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_FPGA, PSCAT_LCU_PIC_FPGA, PROPS_FPGA);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_SYNCStatus, PSCAT_LCU_PIC_SYNCStatus, PROPS_SYNCStatus);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_BoardRCUStatus, PSCAT_LCU_PIC_BoardRCUStatus, PROPS_BoardRCUStatus);
          for(rcu=1;rcu<=m_n_rcus_per_ap;rcu++)
          {
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_RCU, PSCAT_LCU_PIC_RCU, PROPS_RCU);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_ADCStatistics, PSCAT_LCU_PIC_ADCStatistics, PROPS_ADCStatistics);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_LFA, PSCAT_LCU_PIC_LFA, PROPS_LFA);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_HFA, PSCAT_LCU_PIC_HFA, PROPS_HFA);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
          }
        }
      }
    }  
  }
  
  // fill APCs map
  addAPC(APC_Station, SCOPE_PIC);
  addAPC(APC_Maintenance, SCOPE_PIC_Maintenance);
  for(rack=1;rack<=m_n_racks;rack++)
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    addAPC(APC_Rack, scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    addAPC(APC_Maintenance, scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    addAPC(APC_Alert, scopeString);

    for(subrack=1;subrack<=m_n_subracks_per_rack;subrack++)
    {
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      addAPC(APC_SubRack, scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      addAPC(APC_Maintenance, scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
      addAPC(APC_Alert, scopeString);

      for(board=1;board<=m_n_boards_per_subrack;board++)
      {
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        addAPC(APC_Board, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rack,subrack,board);
        addAPC(APC_MEPStatus, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rack,subrack,board);
        addAPC(APC_MEPStatus, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        addAPC(APC_Maintenance, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
        addAPC(APC_Alert, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        addAPC(APC_Ethernet, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        addAPC(APC_FPGA, scopeString);
        for(ap=1;ap<=m_n_aps_per_board;ap++)
        {
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          addAPC(APC_FPGA, scopeString);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rack,subrack,board,ap);
          addAPC(APC_SYNCStatus, scopeString);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rack,subrack,board,ap);
          addAPC(APC_BoardRCUStatus, scopeString);
          for(rcu=1;rcu<=m_n_rcus_per_ap;rcu++)
          {
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
            addAPC(APC_RCU, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rack,subrack,board,ap,rcu);
            addAPC(APC_LFA, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA,rack,subrack,board,ap,rcu);
            addAPC(APC_HFA, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics,rack,subrack,board,ap,rcu);
            addAPC(APC_ADCStatistics, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance,rack,subrack,board,ap,rcu);
            addAPC(APC_Maintenance, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA_Maintenance,rack,subrack,board,ap,rcu);
            addAPC(APC_Maintenance, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA_Maintenance,rack,subrack,board,ap,rcu);
            addAPC(APC_Maintenance, scopeString);
          }
        }
      }
    }
  }
  
  // subscribe to maintenanace properties and alert properties
  
}

RegisterAccessTask::~RegisterAccessTask()
{
}

void RegisterAccessTask::addMyPropertySet(const char* scope,const char* type, TPSCategory category, const TPropertyConfig propconfig[])
{
  boost::shared_ptr<GCFMyPropertySet> propsPtr(new GCFMyPropertySet(scope,type,category,&m_answer));
  m_myPropertySetMap[scope]=propsPtr;
  
  propsPtr->initProperties(propconfig);
}

void RegisterAccessTask::addAPC(string apc,string scope)
{
  m_APCMap[scope]=apc;
}

bool RegisterAccessTask::isConnected()
{
  return m_RSPclient.isConnected();
}

GCFEvent::TResult RegisterAccessTask::initial(GCFEvent& e, GCFPortInterface& /*port*/)
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
      LOG_INFO("Enabling MyPropertySets...");
      m_myPropsLoadCounter=0;
      TMyPropertySetMap::iterator it;
      for(it=m_myPropertySetMap.begin();it!=m_myPropertySetMap.end();++it)
      {
        it->second->enable();
      }
      break;
    }

    case F_MYPS_ENABLED:
    {
      m_myPropsLoadCounter++;
      LOG_INFO(formatString("MyPropset %d enabled", m_myPropsLoadCounter));
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

GCFEvent::TResult RegisterAccessTask::myPropSetsLoaded(GCFEvent& e, GCFPortInterface& /*port*/)
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
      LOG_INFO("configuring propsets using APCs...");
      m_APCsLoadCounter=0;
      TAPCMap::iterator apcIt;
      for(apcIt=m_APCMap.begin();apcIt!=m_APCMap.end();++apcIt)
      {
        TMyPropertySetMap::iterator psIt=m_myPropertySetMap.find(apcIt->first);
        if(psIt != m_myPropertySetMap.end())
        {
          psIt->second->configure(apcIt->second);
        }
      }
      break;
    }

    case F_PS_CONFIGURED:
    {
      m_APCsLoadCounter++;
      LOG_INFO(formatString("Propset %d configured", m_APCsLoadCounter));
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

      if(strstr(pPropAnswer->pPropName,"Maintenance") != 0)
      {
        handleMaintenance(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
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

GCFEvent::TResult RegisterAccessTask::connected(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // get versions
      RSPGetversionEvent getversion;
      getversion.timestamp.setNow();
      getversion.cache = true;
      m_RSPclient.send(getversion);
      
      break;
    }

    case F_TIMER:
    {
      break;
    }

    case RSP_GETVERSIONACK:
    {
      LOG_INFO("RSP_GETVERSIONACK received");
      RSPGetversionackEvent ack(e);

      if(ack.status != SUCCESS)
      {
        LOG_ERROR("RSP_GETVERSION failure");
      }
      else
      {
        char scopeString[300];
        char version[20];
        for (int board = 0; board < ack.versions.rsp().extent(blitz::firstDim); board++)
        {
          int rackNr;
          int subRackNr;
          int relativeBoardNr;
          getBoardRelativeNumbers(board,rackNr,subRackNr,relativeBoardNr);
          sprintf(version,"%d.%d",ack.versions.rsp()(board) >> 4,ack.versions.rsp()(board) & 0xF);
          LOG_INFO(formatString("board[%d].version = 0x%x",board,ack.versions.rsp()(board)));
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
          updateVersion(scopeString,string(version));
          
          sprintf(version,"%d.%d",ack.versions.bp()(board)  >> 4,ack.versions.bp()(board)  & 0xF);
          LOG_INFO(formatString("bp[%d].version = 0x%x",board,ack.versions.bp()(board)));
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rackNr,subRackNr,relativeBoardNr);
          updateVersion(scopeString,string(version));
  
          for (int ap = 0; ap < EPA_Protocol::N_AP; ap++)
          {
            sprintf(version,"%d.%d",ack.versions.ap()(board * EPA_Protocol::N_AP + ap) >> 4,
                                    ack.versions.ap()(board * EPA_Protocol::N_AP + ap) &  0xF);
            LOG_INFO(formatString("ap[%d][%d].version = 0x%x",board,ap,ack.versions.ap()(board * EPA_Protocol::N_AP + ap)));
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,ap+1);
            updateVersion(scopeString,string(version));
          }
        }
      }
      
      // subscribe to status updates
      RSPSubstatusEvent substatus;
      substatus.timestamp.setNow();
      substatus.rcumask = std::bitset<MAX_N_RCUS>((1<<m_n_rcus)-1);
      substatus.period = m_status_update_interval;
      m_RSPclient.send(substatus);
      
      break;
    }
    
    case RSP_SUBSTATUSACK:
    {
      LOG_INFO("RSP_SUBSTATUSACK received");
      RSPSubstatusackEvent ack(e);

      if(ack.status != SUCCESS)
      {
        LOG_ERROR("RSP_SUBSTATUS failure");
      }
      else
      {
        m_subStatusHandle = ack.handle;
      }
      
      TRAN(RegisterAccessTask::subscribingStatsSubbandPower);
      
      break;
    }
    
    case RSP_UPDSTATUS:
    {
    	// handle updstatus events even though we are not subscribed to them yet
    	// this is done to relax the requirements for the ARAtest application
    	// (or you might call it lazyness)
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

GCFEvent::TResult RegisterAccessTask::subscribingStatsSubbandPower(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // subscribe to status updates
      RSPSubstatsEvent substats;
      substats.timestamp.setNow();
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<m_n_rcus)-1);
      substats.period = m_stats_update_interval;
      substats.type = RSP_Protocol::Statistics::SUBBAND_POWER;
      substats.reduction = RSP_Protocol::REPLACE;
      m_RSPclient.send(substats);
      
      break;
    }

    case RSP_SUBSTATSACK:
    {
      LOG_INFO("RSP_SUBSTATSACK received");
      RSPSubstatsackEvent ack(e);

      if(ack.status != SUCCESS)
      {
        LOG_ERROR("RSP_SUBSTATS failure");
      }
      else
      {
        m_subStatsHandleSubbandPower = ack.handle;
      }
      
      TRAN(RegisterAccessTask::subscribingStatsBeamletPower);
      break;
    }
    
    case F_DISCONNECTED:
    {
      LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
      port.close();

      TRAN(RegisterAccessTask::APCsLoaded);
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

GCFEvent::TResult RegisterAccessTask::subscribingStatsBeamletPower(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // subscribe to status updates
      RSPSubstatsEvent substats;
      substats.timestamp.setNow();
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<m_n_rcus)-1);
      substats.period = m_stats_update_interval;
      substats.type = RSP_Protocol::Statistics::BEAMLET_POWER;
      substats.reduction = RSP_Protocol::REPLACE;
      m_RSPclient.send(substats);
      
      break;
    }

    case RSP_SUBSTATSACK:
    {
      LOG_INFO("RSP_SUBSTATSACK received");
      RSPSubstatsackEvent ack(e);

      if(ack.status != SUCCESS)
      {
        LOG_ERROR("RSP_SUBSTATS failure");
      }
      else
      {
        m_subStatsHandleBeamletPower = ack.handle;
      }
      
      TRAN(RegisterAccessTask::operational);
      break;
    }
    
    case F_DISCONNECTED:
    {
      LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
      port.close();

      TRAN(RegisterAccessTask::APCsLoaded);
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

GCFEvent::TResult RegisterAccessTask::operational(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      break;
    }

    case RSP_UPDSTATUS:
    {
      LOG_INFO("RSP_UPDSTATUS received");
      status = handleUpdStatus(e,port);
      break;
    }
    
    case RSP_UPDSTATS:
    {
      LOG_INFO("RSP_UPDSTATS received");
      status = handleUpdStats(e,port);
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

      if(strstr(pPropAnswer->pPropName,"Maintenance") != 0)
      {
        handleMaintenance(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
      }
      break;
    }
    
    case F_EXIT:
    {
      // unsubscribe from status updates
      RSPUnsubstatusEvent unsubStatus;
      unsubStatus.handle = m_subStatusHandle; // remove subscription with this handle
      m_RSPclient.send(unsubStatus);

      // unsubscribe from status updates
      RSPUnsubstatsEvent unsubStats;
      unsubStats.handle = m_subStatsHandleSubbandPower; // remove subscription with this handle
      m_RSPclient.send(unsubStats);
      unsubStats.handle = m_subStatsHandleBeamletPower; // remove subscription with this handle
      m_RSPclient.send(unsubStats);
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

    time_t curTime=(time_t)updStatusEvent.timestamp.sec();
    LOG_INFO(formatString("UpdStatus:time:%s:status:%d:handle:%d", 
        ctime(&curTime),
        updStatusEvent.status,
        updStatusEvent.handle));

    blitz::Array<EPA_Protocol::BoardStatus,  1>& boardStatus = updStatusEvent.sysstatus.board();
    blitz::Array<EPA_Protocol::RCUStatus,  1>& rcuStatus = updStatusEvent.sysstatus.rcu();
    
    int rackNr=1;
    int subRackNr=1;
    int relativeBoardNr=1;
    char scopeString[300];

    int boardNr;
    for(boardNr=boardStatus.lbound(blitz::firstDim); boardNr <= boardStatus.ubound(blitz::firstDim); ++boardNr)
    {
      getBoardRelativeNumbers(boardNr,rackNr,subRackNr,relativeBoardNr);
      rackNr          = boardNr / (m_n_subracks_per_rack*m_n_boards_per_subrack) + 1;
      subRackNr       = boardNr % (m_n_subracks_per_rack*m_n_boards_per_subrack) + 1;
      relativeBoardNr = boardNr % m_n_boards_per_subrack + 1;
      LOG_INFO(formatString("UpdStatus:Rack:%d:SubRack:%d:Board::%d\n",rackNr,subRackNr,relativeBoardNr));
      
      uint8   rspVoltage_15 = boardStatus(boardNr).rsp.voltage_15;
      uint8   rspVoltage_33 = boardStatus(boardNr).rsp.voltage_33;
      uint8   rspFfi0       = boardStatus(boardNr).rsp.ffi0;
      uint8   rspFfi1       = boardStatus(boardNr).rsp.ffi1;
      LOG_INFO(formatString("UpdStatus:RSP voltage_15:%d:voltage_33:%d:ffi0:%d:ffi1:%d",rspVoltage_15,rspVoltage_33,rspFfi0,rspFfi1));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
      updateBoardProperties(scopeString,rspVoltage_15,rspVoltage_33,rspFfi0,rspFfi1);
      
      uint8   bpStatus  = boardStatus(boardNr).fpga.bp_status;
      uint8   bpTemp    = boardStatus(boardNr).fpga.bp_temp;
      uint8   ap1Status = boardStatus(boardNr).fpga.ap1_status;
      uint8   ap1Temp   = boardStatus(boardNr).fpga.ap1_temp;
      uint8   ap2Status = boardStatus(boardNr).fpga.ap2_status;
      uint8   ap2Temp   = boardStatus(boardNr).fpga.ap2_temp;
      uint8   ap3Status = boardStatus(boardNr).fpga.ap3_status;
      uint8   ap3Temp   = boardStatus(boardNr).fpga.ap3_temp;
      uint8   ap4Status = boardStatus(boardNr).fpga.ap4_status;
      uint8   ap4Temp   = boardStatus(boardNr).fpga.ap4_temp;
      uint8   fpgaFfi0  = boardStatus(boardNr).fpga.ffi0;
      uint8   fpgaFfi1  = boardStatus(boardNr).fpga.ffi1;
      LOG_INFO(formatString("UpdStatus:BP status:%d:temp:%d",bpStatus,bpTemp));
      LOG_INFO(formatString("UpdStatus:AP1 status:%d:temp:%d",ap1Status,ap1Temp));
      LOG_INFO(formatString("UpdStatus:AP2 status:%d:temp:%d",ap2Status,ap2Temp));
      LOG_INFO(formatString("UpdStatus:AP3 status:%d:temp:%d",ap3Status,ap3Temp));
      LOG_INFO(formatString("UpdStatus:AP4 status:%d:temp:%d",ap4Status,ap4Temp));
      LOG_INFO(formatString("UpdStatus:ffi0:%d:ffi1:%d",fpgaFfi0,fpgaFfi1));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
      updateFPGAboardProperties(scopeString,fpgaFfi0,fpgaFfi1);
      
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rackNr,subRackNr,relativeBoardNr);
      updateFPGAproperties(scopeString,bpStatus,bpTemp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,1);
      updateFPGAproperties(scopeString,ap1Status,ap1Temp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,2);
      updateFPGAproperties(scopeString,ap2Status,ap2Temp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,3);
      updateFPGAproperties(scopeString,ap3Status,ap3Temp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,4);
      updateFPGAproperties(scopeString,ap4Status,ap4Temp);

      uint32    ethFrames     = boardStatus(boardNr).eth.nof_frames;
      uint32    ethErrors     = boardStatus(boardNr).eth.nof_errors;
      uint8     ethLastError  = boardStatus(boardNr).eth.last_error;
      uint8     ethFfi0       = boardStatus(boardNr).eth.ffi0;
      uint8     ethFfi1       = boardStatus(boardNr).eth.ffi1;
      uint8     ethFfi2       = boardStatus(boardNr).eth.ffi2;
      LOG_INFO(formatString("UpdStatus:ETH frames:%d:errors:%d:last_error:%d:ffi0:%d:ffi1:%d:ffi2:%d",ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rackNr,subRackNr,relativeBoardNr);
      updateETHproperties(scopeString,ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2);  
  
      uint32    mepSeqnr = boardStatus(boardNr).mep.seqnr;
      uint8     mepError = boardStatus(boardNr).mep.error;
      uint8     mepFfi0  = boardStatus(boardNr).mep.ffi0;
      LOG_INFO(formatString("UpdStatus:MEP seqnr:%d:error:%d:ffi:%d",mepSeqnr,mepError,mepFfi0));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rackNr,subRackNr,relativeBoardNr);
      updateMEPStatusProperties(scopeString,mepSeqnr,mepError,mepFfi0);  
      
      uint32    syncSample_count = boardStatus(boardNr).ap1_sync.sample_count;
      uint32    syncSync_count   = boardStatus(boardNr).ap1_sync.sync_count;
      uint32    syncError_count  = boardStatus(boardNr).ap1_sync.error_count;
      LOG_INFO(formatString("SyncStatus ap1:clock_count:%d:count:%d:errors:%d",syncSample_count,syncSync_count,syncError_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,1);
      updateSYNCStatusProperties(scopeString,syncSample_count,syncSync_count,syncError_count);  

      syncSample_count = boardStatus(boardNr).ap2_sync.sample_count;
      syncSync_count   = boardStatus(boardNr).ap2_sync.sync_count;
      syncError_count  = boardStatus(boardNr).ap2_sync.error_count;
      LOG_INFO(formatString("SyncStatus ap2:clock_count:%d:count:%d:errors:%d",syncSample_count,syncSync_count,syncError_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,2);
      updateSYNCStatusProperties(scopeString,syncSample_count,syncSync_count,syncError_count);  

      syncSample_count = boardStatus(boardNr).ap3_sync.sample_count;
      syncSync_count   = boardStatus(boardNr).ap3_sync.sync_count;
      syncError_count  = boardStatus(boardNr).ap3_sync.error_count;
      LOG_INFO(formatString("SyncStatus ap3:clock_count:%d:count:%d:errors:%d",syncSample_count,syncSync_count,syncError_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,3);
      updateSYNCStatusProperties(scopeString,syncSample_count,syncSync_count,syncError_count);  

      syncSample_count = boardStatus(boardNr).ap4_sync.sample_count;
      syncSync_count   = boardStatus(boardNr).ap4_sync.sync_count;
      syncError_count  = boardStatus(boardNr).ap4_sync.error_count;
      LOG_INFO(formatString("SyncStatus ap4:clock_count:%d:count:%d:errors:%d",syncSample_count,syncSync_count,syncError_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,4);
      updateSYNCStatusProperties(scopeString,syncSample_count,syncSync_count,syncError_count);  

      int apNr=1;
      uint8     boardRCUstatusStatusX       = boardStatus(boardNr).ap1_rcu.statusx;
      uint8     boardRCUstatusStatusY       = boardStatus(boardNr).ap1_rcu.statusy;
      uint8     boardRCUstatusFFI0          = boardStatus(boardNr).ap1_rcu.ffi0;
      uint8     boardRCUstatusFFI1          = boardStatus(boardNr).ap1_rcu.ffi1;
      uint32    boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap1_rcu.nof_overflowx;
      uint32    boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap1_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap1:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY);
      int rcuNr=1;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusX);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusY);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap2_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap2_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap2_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap2_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap2_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap2_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap2:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY);
      rcuNr=1;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusX);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusY);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap3_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap3_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap3_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap3_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap3_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap3_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap3:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY);
      rcuNr=1;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusX);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusY);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap4_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap4_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap4_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap4_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap4_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap4_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap4:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY);
      rcuNr=1;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusX);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateRCUproperties(scopeString,boardRCUstatusStatusY);
    }

/*
    for(int rcuNr=rcuStatus.lbound(blitz::firstDim); rcuNr <= rcuStatus.ubound(blitz::firstDim); ++rcuNr)
    {
      uint8   rcuStatusBits = rcuStatus(rcuNr).status;
      LOG_INFO(formatString("UpdStatus:RCU[%d] status:0x%x",rcuNr,rcuStatusBits));
      
      int rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr;
      getRCURelativeNumbers(rcuNr,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);

      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      LOG_DEBUG(formatString("RCU[%d]= %s",rcuNr,scopeString));
      updateRCUproperties(scopeString,rcuStatusBits);
    }
*/
  }
  
  return status;
}

GCFEvent::TResult RegisterAccessTask::handleUpdStats(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  {
    RSPUpdstatsEvent updStatsEvent(e);

    time_t curTime=(time_t)updStatsEvent.timestamp.sec();
    LOG_INFO(formatString("UpdStats:time:%s:status:%d:handle:%d", 
        ctime(&curTime),
        updStatsEvent.status,
        updStatsEvent.handle));

    blitz::Array<double, 2>& statistics = updStatsEvent.stats();
    int maxRCUs     = statistics.ubound(blitz::firstDim) - statistics.lbound(blitz::firstDim) + 1;
    int maxSubbands = statistics.ubound(blitz::secondDim) - statistics.lbound(blitz::secondDim) + 1;

    int rcu,subband;
    GCFPValueArray valuePointerVector;
    
    // first elements indicate the length of the array
    valuePointerVector.push_back(new GCFPVDouble(maxRCUs));
    valuePointerVector.push_back(new GCFPVDouble(maxSubbands));
    
    // then for each rcu, the statistics of the beamlets or subbands
    for(rcu=statistics.lbound(blitz::firstDim);rcu<=statistics.ubound(blitz::firstDim);rcu++)
    {
      for(subband=statistics.lbound(blitz::secondDim);subband<=statistics.ubound(blitz::secondDim);subband++)
      {
        double stat = statistics(rcu,subband);
        valuePointerVector.push_back(new GCFPVDouble(stat));
      }
    }

    // convert the vector of unsigned values to a dynamic array
    GCFPVDynArr dynamicArray(LPT_DOUBLE,valuePointerVector);
    
    // set the property
    TMyPropertySetMap::iterator propSetIt=m_myPropertySetMap.find(string(SCOPE_PIC));
    if(propSetIt != m_myPropertySetMap.end())
    {
      if(updStatsEvent.handle == m_subStatsHandleSubbandPower)
      {
        propSetIt->second->setValue(string(PROPNAME_STATISTICSSUBBANDPOWER),dynamicArray);
      }
      else if(updStatsEvent.handle == m_subStatsHandleBeamletPower)
      {
        propSetIt->second->setValue(string(PROPNAME_STATISTICSBEAMLETPOWER),dynamicArray);
      }
    }
    
    // cleanup
    GCFPValueArray::iterator it=valuePointerVector.begin();
    while(it!=valuePointerVector.end())
    {
      delete *it;
      valuePointerVector.erase(it);
      it=valuePointerVector.begin();
    }
  }
  
  return status;
}
void RegisterAccessTask::updateBoardProperties(string scope,
                                               uint8  voltage_15,
                                               uint8  voltage_33,
                                               uint8  ffi0,
                                               uint8  ffi1)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(voltage_15);
    it->second->setValue(string(PROPNAME_VOLTAGE15),pvTemp);
    
    pvTemp.setValue(voltage_33);
    it->second->setValue(string(PROPNAME_VOLTAGE33),pvTemp);

    pvTemp.setValue(ffi0);
    it->second->setValue(string(PROPNAME_FFI0),pvTemp);

    pvTemp.setValue(ffi1);
    it->second->setValue(string(PROPNAME_FFI1),pvTemp);
  }
}

void RegisterAccessTask::updateETHproperties(string scope,
                                             uint32 frames,
                                             uint32 errors,
                                             uint8  lastError,
                                             uint8  ffi0,
                                             uint8  ffi1,
                                             uint8  ffi2)
{
  // layout eth status: 
  // 31......24  23.....16  15........8  7........0       
  // #RX[15..8]  #RX[7..0]  #Err[15..8]  #Err[7..0]  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(frames);
    it->second->setValue(string(PROPNAME_FRAMESRECEIVED),pvTemp);
    
    pvTemp.setValue(errors);
    it->second->setValue(string(PROPNAME_FRAMESERROR),pvTemp);

    pvTemp.setValue(lastError);
    it->second->setValue(string(PROPNAME_LASTERROR),pvTemp);

    pvTemp.setValue(ffi0);
    it->second->setValue(string(PROPNAME_FFI0),pvTemp);

    pvTemp.setValue(ffi1);
    it->second->setValue(string(PROPNAME_FFI1),pvTemp);

    pvTemp.setValue(ffi2);
    it->second->setValue(string(PROPNAME_FFI2),pvTemp);
  }
}

/**
 * update MEP status properties 
 */
void RegisterAccessTask::updateMEPStatusProperties(string scope,uint32 seqnr,
                                                                uint8  error,
                                                                uint8  ffi0)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(seqnr);
    it->second->setValue(string(PROPNAME_SEQNR),pvTemp);
    
    pvTemp.setValue(error);
    it->second->setValue(string(PROPNAME_ERROR),pvTemp);

    pvTemp.setValue(ffi0);
    it->second->setValue(string(PROPNAME_FFI0),pvTemp);
  }
}

void RegisterAccessTask::updateSYNCStatusProperties(string scope,uint32 sample_count,
                                                                 uint32 sync_count,
                                                                 uint32 error_count)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(sample_count);
    it->second->setValue(string(PROPNAME_SAMPLECOUNT),pvTemp);
    
    pvTemp.setValue(sync_count);
    it->second->setValue(string(PROPNAME_SYNCCOUNT),pvTemp);

    pvTemp.setValue(error_count);
    it->second->setValue(string(PROPNAME_ERRORCOUNT),pvTemp);
  }
}
                                             
void RegisterAccessTask::updateFPGAboardProperties(string scope, uint8 ffi0, 
                                                                 uint8 ffi1)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvUns(ffi0);
    it->second->setValue(string(PROPNAME_FPGAFFI0),pvUns);

    pvUns.setValue(ffi1);
    it->second->setValue(string(PROPNAME_FPGAFFI1),pvUns);
  }
}

void RegisterAccessTask::updateFPGAproperties(string scope, uint8 status, 
                                                            uint8 temp)
{
  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvUns(status);
    it->second->setValue(string(PROPNAME_STATUS),pvUns);
    
    GCFPVDouble pvDouble(static_cast<double>(temp)/100.0);
    it->second->setValue(string(PROPNAME_TEMPERATURE),pvDouble);
  }
}

void RegisterAccessTask::updateBoardRCUproperties(string scope,uint8  statusX,
                                                               uint8  statusY,
                                                               uint8  ffi0,
                                                               uint8  ffi1,
                                                               uint32 nof_overflowX,
                                                               uint32 nof_overflowY)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvUns(statusX);
    it->second->setValue(string(PROPNAME_STATUSX),pvUns);
    pvUns.setValue(statusY);
    it->second->setValue(string(PROPNAME_STATUSY),pvUns);
    pvUns.setValue(ffi0);
    it->second->setValue(string(PROPNAME_FFI0),pvUns);
    pvUns.setValue(ffi1);
    it->second->setValue(string(PROPNAME_FFI1),pvUns);
    pvUns.setValue(nof_overflowX);
    it->second->setValue(string(PROPNAME_NOFOVERFLOWX),pvUns);
    pvUns.setValue(nof_overflowY);
    it->second->setValue(string(PROPNAME_NOFOVERFLOWY),pvUns);
  }
}

void RegisterAccessTask::updateRCUproperties(string scope,uint8 status)
{
  // layout rcu status (for both statusX and statusY): 
  // 7         6         5         4        3        2       1          0
  // VDDVCCEN VHENABLE VLENABLE FILSELB FILSELA BANDSEL HBAENABLE LBAENABLE
    
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    unsigned int tempStatus = (status >> 7 ) & 0x01;
    GCFPVBool pvBoolVddVccEn(tempStatus);
    it->second->setValue(string(PROPNAME_VDDVCCEN),pvBoolVddVccEn);
    
    tempStatus = (status >> 6) & 0x01;
    GCFPVBool pvBoolVhEnable(tempStatus);
    it->second->setValue(string(PROPNAME_VHENABLE),pvBoolVhEnable);

    tempStatus = (status >> 5) & 0x01;
    GCFPVBool pvBoolVlEnable(tempStatus);
    it->second->setValue(string(PROPNAME_VLENABLE),pvBoolVlEnable);

    tempStatus = (status >> 4) & 0x01;
    GCFPVBool pvBoolFilSelB(tempStatus);
    it->second->setValue(string(PROPNAME_FILSELB),pvBoolFilSelB);

    tempStatus = (status >> 3) & 0x01;
    GCFPVBool pvBoolFilSelA(tempStatus);
    it->second->setValue(string(PROPNAME_FILSELA),pvBoolFilSelA);

    tempStatus = (status >> 2) & 0x01;
    GCFPVBool pvBoolBandSel(tempStatus);
    it->second->setValue(string(PROPNAME_BANDSEL),pvBoolBandSel);

    tempStatus = (status >> 1) & 0x01;
    GCFPVBool pvBoolHBAEnable(tempStatus);
    it->second->setValue(string(PROPNAME_HBAENABLE),pvBoolHBAEnable);

    tempStatus = (status >> 0) & 0x01;
    GCFPVBool pvBoolLBAEnable(tempStatus);
    it->second->setValue(string(PROPNAME_LBAENABLE),pvBoolLBAEnable);
    
    if(pvBoolVddVccEn.getValue() ||
       pvBoolVhEnable.getValue() ||
       pvBoolVlEnable.getValue() ||
       pvBoolFilSelB.getValue() ||
       pvBoolFilSelA.getValue() ||
       pvBoolBandSel.getValue() ||
       pvBoolHBAEnable.getValue() ||
       pvBoolLBAEnable.getValue())
    {
      GCFPVUnsigned pvUnsignedStatus(STATUS_ERROR);
      it->second->setValue(string(PROPNAME_STATUS),pvUnsignedStatus);
    }       
    else
    {
      GCFPVUnsigned pvUnsignedStatus(STATUS_OK);
      it->second->setValue(string(PROPNAME_STATUS),pvUnsignedStatus);
    }       
  }
}

void RegisterAccessTask::updateVersion(string scope, string version)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVString pvString(version);
    it->second->setValue(string(PROPNAME_VERSION),pvString);
  }    
}

void RegisterAccessTask::handleMaintenance(string propName, const GCFPValue& value)
{
  GCFPVBool pvBool;
  pvBool.copy(value);
  bool maintenanceFlag(pvBool.getValue());
  
  // strip last part of the property name to get the resource name.
  int pos=propName.find_last_of("_.");
  string resource = propName.substr(0,pos);
  m_physicalModel.inMaintenance(maintenanceFlag,resource);
}

void RegisterAccessTask::getBoardRelativeNumbers(int boardNr,int& rackNr,int& subRackNr,int& relativeBoardNr)
{
  rackNr          = boardNr / (m_n_subracks_per_rack*m_n_boards_per_subrack) + 1;
  subRackNr       = (boardNr / m_n_boards_per_subrack) % m_n_subracks_per_rack + 1;
  relativeBoardNr = boardNr % m_n_boards_per_subrack + 1;
}

void RegisterAccessTask::getRCURelativeNumbers(int rcuNr,int& rackRelativeNr,int& subRackRelativeNr,int& boardRelativeNr,int& apRelativeNr,int& rcuRelativeNr)
{
  rackRelativeNr    = rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack*m_n_subracks_per_rack) + 1;
  subRackRelativeNr = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack) ) % m_n_subracks_per_rack + 1;
  boardRelativeNr   = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board) ) % m_n_boards_per_subrack + 1;
  apRelativeNr      = ( rcuNr / (m_n_rcus_per_ap) ) % m_n_aps_per_board + 1;
  rcuRelativeNr     = ( rcuNr % m_n_rcus_per_ap) + 1;
}

} // namespace ARA


} // namespace LOFAR

