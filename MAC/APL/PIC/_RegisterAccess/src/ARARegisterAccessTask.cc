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
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include "ARAPropertyDefines.h"
#include "ARAPhysicalModel.h"

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
      m_RSPclient(*this, m_RSPserverName, GCFPortInterface::SAP, RSP_PROTOCOL),
      m_physicalModel(),
      m_subStatusHandle(0),
      m_subStatsHandleSubbandPower(0),
      m_subStatsHandleSubbandMean(0),
      m_subStatsHandleBeamletPower(0),
      m_subStatsHandleBeamletMean(0)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  char scopeString[300];
  int rack;
  int subrack;
  int board;
  int ap;
  int rcu;
  
  // fill MyPropertySets map
  addMyPropertySet(PROPSET_PIC, SCOPE_PIC);
  addMyPropertySet(PROPSET_Maintenance, SCOPE_PIC_Maintenance);
  for(rack=1;rack<=N_RACKS;rack++)
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    addMyPropertySet(PROPSET_Racks[rack-1], scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    addMyPropertySet(PROPSET_Maintenance, scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    addMyPropertySet(PROPSET_Alert, scopeString);

    for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
    {
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      addMyPropertySet(PROPSET_SubRacks[subrack-1], scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      addMyPropertySet(PROPSET_Maintenance, scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
      addMyPropertySet(PROPSET_Alert, scopeString);
      
      for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
      {
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        addMyPropertySet(PROPSET_Boards[board-1], scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        addMyPropertySet(PROPSET_Maintenance, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
        addMyPropertySet(PROPSET_Alert, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        addMyPropertySet(PROPSET_ETH, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        addMyPropertySet(PROPSET_BP, scopeString);
    
        for(ap=1;ap<=N_APS_PER_BOARD;ap++)
        {
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          addMyPropertySet(PROPSET_APs[ap-1], scopeString);
          for(rcu=1;rcu<=N_RCUS_PER_AP;rcu++)
          {
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_RCUs[rcu-1], scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_ADCStatistics, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_Maintenance, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_LFA, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_HFA, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_Maintenance, scopeString);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA_Maintenance,rack,subrack,board,ap,rcu);
            addMyPropertySet(PROPSET_Maintenance, scopeString);
          }
        }
      }
    }  
  }
  
  // fill APCs map
  addAPC(APC_Station, SCOPE_PIC);
  addAPC(APC_Maintenance, SCOPE_PIC_Maintenance);
  for(rack=1;rack<=N_RACKS;rack++)
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    addAPC(APC_Rack, scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    addAPC(APC_Maintenance, scopeString);
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    addAPC(APC_Alert, scopeString);

    for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
    {
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      addAPC(APC_SubRack, scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      addAPC(APC_Maintenance, scopeString);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
      addAPC(APC_Alert, scopeString);

      for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
      {
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        addAPC(APC_Board, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        addAPC(APC_Maintenance, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
        addAPC(APC_Alert, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        addAPC(APC_Ethernet, scopeString);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        addAPC(APC_FPGA, scopeString);
        for(ap=1;ap<=N_APS_PER_BOARD;ap++)
        {
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          addAPC(APC_FPGA, scopeString);
          for(rcu=1;rcu<=N_RCUS_PER_AP;rcu++)
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
          int rackNr          = board / (N_SUBRACKS_PER_RACK*N_BOARDS_PER_SUBRACK) + 1;
          int subRackNr       = board % (N_SUBRACKS_PER_RACK*N_BOARDS_PER_SUBRACK) + 1;
          int relativeBoardNr = board % N_BOARDS_PER_SUBRACK + 1;
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
      substatus.rcumask = std::bitset<MAX_N_RCUS>((1<<N_RCUS)-1);
      substatus.period = STATUS_UPDATE_INTERVAL;
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
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<N_RCUS)-1);
      substats.period = STATISTICS_UPDATE_INTERVAL;
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
      
      TRAN(RegisterAccessTask::subscribingStatsSubbandMean);
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

GCFEvent::TResult RegisterAccessTask::subscribingStatsSubbandMean(GCFEvent& e, GCFPortInterface &port)
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
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<N_RCUS)-1);
      substats.period = STATISTICS_UPDATE_INTERVAL;
      substats.type = RSP_Protocol::Statistics::SUBBAND_MEAN;
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
        m_subStatsHandleSubbandMean = ack.handle;
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
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<N_RCUS)-1);
      substats.period = STATISTICS_UPDATE_INTERVAL;
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
      
      TRAN(RegisterAccessTask::subscribingStatsBeamletMean);
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

GCFEvent::TResult RegisterAccessTask::subscribingStatsBeamletMean(GCFEvent& e, GCFPortInterface &port)
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
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<N_RCUS)-1);
      substats.period = STATISTICS_UPDATE_INTERVAL;
      substats.type = RSP_Protocol::Statistics::BEAMLET_MEAN;
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
        m_subStatsHandleBeamletMean = ack.handle;
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
      LOG_INFO(formatString("property '%s' changed", pPropAnswer->pPropName));

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
      unsubStats.handle = m_subStatsHandleSubbandMean; // remove subscription with this handle
      m_RSPclient.send(unsubStats);
      unsubStats.handle = m_subStatsHandleBeamletPower; // remove subscription with this handle
      m_RSPclient.send(unsubStats);
      unsubStats.handle = m_subStatsHandleBeamletMean; // remove subscription with this handle
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
    LOG_INFO(formatString("UpdStatus:\n\ttime: \t%s\n\tstatus:\t%d\n\thandle:\t%d", 
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
      rackNr          = boardNr / (N_SUBRACKS_PER_RACK*N_BOARDS_PER_SUBRACK) + 1;
      subRackNr       = boardNr % (N_SUBRACKS_PER_RACK*N_BOARDS_PER_SUBRACK) + 1;
      relativeBoardNr = boardNr % N_BOARDS_PER_SUBRACK + 1;
      LOG_INFO(formatString("UpdStatus:\n\tRack:\t%d\n\tSubRack:\t%d\n\tBoard:\t%d\n",rackNr,subRackNr,relativeBoardNr));
      
      uint8   rspVoltage_15 = boardStatus(boardNr).rsp.voltage_15;
      uint8   rspVoltage_22 = boardStatus(boardNr).rsp.voltage_22;
      uint16  rspFfi        = boardStatus(boardNr).rsp.ffi;
      LOG_INFO(formatString("UpdStatus:\n\tRSP voltage_15:\t%d\n\tRSP voltage_22:\t%d\n\tRSP ffi\t%d",rspVoltage_15,rspVoltage_22,rspFfi));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
      updateBoardProperties(scopeString,rspVoltage_15,rspVoltage_22,rspFfi);
      
      uint8   bpStatus  = boardStatus(boardNr).bp.status;
      uint8   bpTemp    = boardStatus(boardNr).bp.temp;
      LOG_INFO(formatString("UpdStatus:\n\tBP status:\t%d\n\tBP temp:\t%d",bpStatus,bpTemp));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rackNr,subRackNr,relativeBoardNr);
      updateFPGAproperties(scopeString,bpStatus,bpTemp);

      for(int apNr=0; apNr < EPA_Protocol::N_AP; ++apNr)
      {
        uint8   apStatus  = boardStatus(boardNr).ap[apNr].status;
        uint8   apTemp    = boardStatus(boardNr).ap[apNr].temp;
        LOG_INFO(formatString("UpdStatus:\n\tAP[%d] status:\t%d\n\tAP[%d] temp:\t%d",apNr,apStatus,apNr,apTemp));
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,apNr+1);
        updateFPGAproperties(scopeString,apStatus,apTemp);
      }      
  
      uint32    ethFrames     = boardStatus(boardNr).eth.nof_frames;
      uint32    ethErrors     = boardStatus(boardNr).eth.nof_errors;
      uint8     ethLastError  = boardStatus(boardNr).eth.last_error;
      uint8     ethFfi0       = boardStatus(boardNr).eth.ffi0;
      uint8     ethFfi1       = boardStatus(boardNr).eth.ffi1;
      uint8     ethFfi2       = boardStatus(boardNr).eth.ffi2;
      LOG_INFO(formatString("UpdStatus:\n\tETH frames:\t%d\n\tETH errors:\t%d\n\tETH last_error:\t%d\n\tETH ffi0:\t%d\n\tETH ffi1:\t%d\n\tETH ffi2:\t%d",ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rackNr,subRackNr,relativeBoardNr);
      updateETHproperties(scopeString,ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2);  
  
      uint16    readSeqnr = boardStatus(boardNr).read.seqnr;
      uint8     readError = boardStatus(boardNr).read.error;
      uint8     readFfi   = boardStatus(boardNr).read.ffi;
      LOG_INFO(formatString("UpdStatus:\n\tREAD seqnr:\t%d\n\tREAD error:\t%d\n\tREAD ffr:\t%d",readSeqnr,readError,readFfi));
      
      uint16    writeSeqnr = boardStatus(boardNr).write.seqnr;
      uint8     writeError = boardStatus(boardNr).write.error;
      uint8     writeFfi   = boardStatus(boardNr).write.ffi;
      LOG_INFO(formatString("UpdStatus:\n\tWRITE seqnr:\t%d\n\tWRITE error:\t%d\n\tWRITE ffr:\t%d",writeSeqnr,writeError,writeFfi));

    }

    for(int rcuNr=rcuStatus.lbound(blitz::firstDim); rcuNr <= rcuStatus.ubound(blitz::firstDim); ++rcuNr)
    {
      uint8   rcuStatusBits = rcuStatus(rcuNr).status;
      LOG_INFO(formatString("UpdStatus:\n\tRCU[%d] status:\t0x%x",rcuNr,rcuStatusBits));
      
      int rackRelativeNr    = rcuNr / (N_RCUS_PER_AP*N_APS_PER_BOARD*N_BOARDS_PER_SUBRACK*N_SUBRACKS_PER_RACK) + 1;
      int subRackRelativeNr = rcuNr / (N_RCUS_PER_AP*N_APS_PER_BOARD*N_BOARDS_PER_SUBRACK) + 1;
      int boardRelativeNr   = rcuNr / (N_RCUS_PER_AP*N_APS_PER_BOARD) + 1;
      int apRelativeNr      = ( ( rcuNr % (N_RCUS_PER_AP*N_APS_PER_BOARD) ) / N_RCUS_PER_AP) + 1;
      int rcuRelativeNr     = ( rcuNr % N_RCUS_PER_AP) + 1;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      updateRCUproperties(scopeString,rcuStatusBits);
    }
  }
  
  return status;
}

GCFEvent::TResult RegisterAccessTask::handleUpdStats(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  {
    RSPUpdstatsEvent updStatsEvent(e);

    time_t curTime=(time_t)updStatsEvent.timestamp.sec();
    LOG_INFO(formatString("UpdStats:\n\ttime: \t%s\n\tstatus:\t%d\n\thandle:\t%d", 
        ctime(&curTime),
        updStatsEvent.status,
        updStatsEvent.handle));

    blitz::Array<std::complex<double>, 3>& statistics = updStatsEvent.stats();
    int maxRCUs     = statistics.ubound(blitz::secondDim) - statistics.lbound(blitz::secondDim) + 1;
    int maxBeamlets = statistics.ubound(blitz::thirdDim) - statistics.lbound(blitz::thirdDim) + 1;

    int type,rcu,beamlet;
    // type will always be 0. The subscription determines if the received array contains
    // power or mean statistics
    for(type=statistics.lbound(blitz::firstDim);type<=statistics.ubound(blitz::firstDim);type++)
    {
      GCFPValueArray valuePointerVector;
      
      // first elements indicate the length of the array
      valuePointerVector.push_back(new GCFPVDouble(maxRCUs));
      valuePointerVector.push_back(new GCFPVDouble(maxBeamlets));
      
      // then for each rcu, the statistics of the beamlets or subbands
      for(rcu=statistics.lbound(blitz::secondDim);rcu<=statistics.ubound(blitz::secondDim);rcu++)
      {
        for(beamlet=statistics.lbound(blitz::thirdDim);beamlet<=statistics.ubound(blitz::thirdDim);beamlet++)
        {
          std::complex<double> stat = statistics(type,rcu,beamlet);
          double newStat;
          if(updStatsEvent.handle == m_subStatsHandleSubbandPower ||
             updStatsEvent.handle == m_subStatsHandleBeamletPower)
          {
            // power = 10 * log( (real + imag + 0.0001) / (1<<16))
            newStat = real(stat) + imag(stat);
            newStat += 0.00001; // prevent infinite values
            newStat = log10( newStat / (double)(1<<16)) * 10.0;
          }
          else if(updStatsEvent.handle == m_subStatsHandleSubbandMean ||
                  updStatsEvent.handle == m_subStatsHandleBeamletMean)
          {
            // mean = abs( z ) ^2 /1000000
            newStat = real(stat) * real(stat) + imag(stat) * imag(stat);
            newStat = newStat / 1000000.0;
          }
          else
          {
            newStat = real(stat) * real(stat) + imag(stat) * imag(stat);
            newStat = newStat / 1000000.0;
          }
          
          valuePointerVector.push_back(new GCFPVDouble(newStat));
        }
      }

      // convert the vector of unsigned values to a dynamic array
      GCFPVDynArr dynamicArray(GCFPValue::LPT_DOUBLE,valuePointerVector);
      
      // set the property
      TMyPropertySetMap::iterator propSetIt=m_myPropertySetMap.find(string(SCOPE_PIC));
      if(propSetIt != m_myPropertySetMap.end())
      {
        if(updStatsEvent.handle == m_subStatsHandleSubbandPower)
        {
          propSetIt->second->setValue(string(PROPNAME_STATISTICSSUBBANDPOWER),dynamicArray);
        }
        else if(updStatsEvent.handle == m_subStatsHandleSubbandMean)
        {
          propSetIt->second->setValue(string(PROPNAME_STATISTICSSUBBANDMEAN),dynamicArray);
        }
        else if(updStatsEvent.handle == m_subStatsHandleBeamletPower)
        {
          propSetIt->second->setValue(string(PROPNAME_STATISTICSBEAMLETPOWER),dynamicArray);
        }
        else if(updStatsEvent.handle == m_subStatsHandleBeamletMean)
        {
          propSetIt->second->setValue(string(PROPNAME_STATISTICSBEAMLETMEAN),dynamicArray);
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
  }
  
  return status;
}
void RegisterAccessTask::updateBoardProperties(string scope,
                                               uint8  voltage_15,
                                               uint8  voltage_22,
                                               uint8  ffi)
{
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    GCFPVUnsigned pvTemp(voltage_15);
    it->second->setValue(string(PROPNAME_VOLTAGE15),pvTemp);
    
    pvTemp.setValue(voltage_22);
    it->second->setValue(string(PROPNAME_VOLTAGE22),pvTemp);

    pvTemp.setValue(ffi);
    it->second->setValue(string(PROPNAME_FFI),pvTemp);
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
  if(it != m_myPropertySetMap.end())
  {
    GCFPVUnsigned pvTemp(frames);
    it->second->setValue(string(PROPNAME_PACKETSRECEIVED),pvTemp);
    
    pvTemp.setValue(errors);
    it->second->setValue(string(PROPNAME_PACKETSERROR),pvTemp);

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

void RegisterAccessTask::updateFPGAproperties(string scope,uint8 status, uint8 temp)
{
  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    GCFPVBool pvBool(status & 0x01);
    it->second->setValue(string(PROPNAME_ALIVE),pvBool);
    
    GCFPVDouble pvDouble(static_cast<double>(temp)/100.0);
    it->second->setValue(string(PROPNAME_TEMPERATURE),pvDouble);
  }
}

void RegisterAccessTask::updateRCUproperties(string scope,uint8 status)
{
  // layout rcu status: 
  // 7        6       5       4       3 2 1 0
  // overflow rcu_pwr hba_pwr lba_pwr filter
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it != m_myPropertySetMap.end())
  {
    unsigned int tempStatus = (status >> 7 ) & 0x01;
    GCFPVBool pvBoolOverflow(tempStatus);
    it->second->setValue(string(PROPNAME_OVERFLOW),pvBoolOverflow);
    
    tempStatus = (status >> 6) & 0x01;
    GCFPVBool pvBoolRcuPwr(tempStatus);
    it->second->setValue(string(PROPNAME_RCUPWR),pvBoolRcuPwr);

    tempStatus = (status >> 5) & 0x01;
    GCFPVBool pvBoolHbaPwr(tempStatus);
    it->second->setValue(string(PROPNAME_HBAPWR),pvBoolHbaPwr);

    tempStatus = (status >> 4) & 0x01;
    GCFPVBool pvBoolLbaPwr(tempStatus);
    it->second->setValue(string(PROPNAME_LBAPWR),pvBoolLbaPwr);

    tempStatus = (status >> 0) & 0x0F;
    GCFPVUnsigned pvFilter(tempStatus);
    it->second->setValue(string(PROPNAME_FILTER),pvFilter);
    
    if(pvBoolOverflow.getStatus() ||
       pvBoolRcuPwr.getStatus() ||
       pvBoolHbaPwr.getStatus() ||
       pvBoolLbaPwr.getStatus())
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
  if(it != m_myPropertySetMap.end())
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
