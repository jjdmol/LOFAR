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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

// this include needs to be first!
#include <RSP_Protocol/RSP_Protocol.ph>

#include "ARARegisterAccessTask.h"
#include "ARAConstants.h"

#include <Common/lofar_iostream.h>
#include <Common/lofar_strstream.h>
#include <time.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <GCF/ParameterSet.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <APL/APLCommon/APL_Defines.h>
#include "ARAPropertyDefines.h"
#include "ARAPhysicalModel.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;


namespace LOFAR
{

namespace ARA
{
INIT_TRACER_CONTEXT(RegisterAccessTask,LOFARLOGGER_PACKAGE);

string RegisterAccessTask::m_RSPserverName("RSPserver");

RegisterAccessTask::RegisterAccessTask(string name)
    : GCFTask((State)&RegisterAccessTask::initial, name),
      m_answer(),
      m_myPropertySetMap(),
      m_myPropsLoaded(false),
      m_myPropsLoadCounter(0),
      m_RSPclient(*this, m_RSPserverName, GCFPortInterface::SAP, RSP_PROTOCOL),
      m_physicalModel(),
      m_propertySet2RCUMap(),
      m_subStatusHandle(0),
      m_subStatsHandleSubbandPower(0),
      m_subStatsHandleBeamletPower(0),
      m_subXcStatsHandle(0),
      m_n_racks(1),
      m_n_subracks_per_rack(1),
      m_n_boards_per_subrack(1),
      m_n_aps_per_board(1),
      m_n_rcus_per_ap(2),
      m_n_rcus(2),
      m_status_update_interval(1),
      m_stats_update_interval(1),
      m_centralized_stats(false),
      m_integrationTime(0),
      m_integrationMethod(0),
      m_integratingStatisticsSubband(),
      m_numStatisticsSubband(0),
      m_integratingStatisticsBeamlet(),
      m_numStatisticsBeamlet(0),
      m_integratingXcStatistics(),
      m_numXcStatistics(0),
      m_integrationTimerID(0),
      m_commandHandle(0),
      m_pendingCommands()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);
  
  ParameterSet::instance()->adoptFile(GCF::ParameterSet::instance()->getSearchPath() + string("RegisterAccess.conf"));

  char scopeString[300];
  int rack;
  int subrack;
  int board;
  int ap;
  int rcu;
  int globalRcuNr(0);
  
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
  m_centralized_stats      = (0!=ParameterSet::instance()->getInt(PARAM_STATISTICS_CENTRALIZED));
  
  // fill MyPropertySets map
  addMyPropertySet(SCOPE_PIC, TYPE_LCU_PIC, PSCAT_LCU_PIC, PROPS_Station, GCFMyPropertySet::USE_DB_DEFAULTS);
  addMyPropertySet(SCOPE_PIC_Maintenance, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
  addMyPropertySet(SCOPE_PIC_Command, TYPE_LCU_PIC_Command, PSCAT_LCU_PIC_Command, PROPS_Command);
  for(rack=0;rack<m_n_racks;rack++)
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Rack, PSCAT_LCU_PIC_Rack, PROPS_Rack);
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    addMyPropertySet(scopeString,TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);

    for(subrack=0;subrack<m_n_subracks_per_rack;subrack++)
    {
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_SubRack, PSCAT_LCU_PIC_SubRack, PROPS_SubRack);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Command,rack,subrack);
      addMyPropertySet(scopeString, TYPE_LCU_PIC_Command, PSCAT_LCU_PIC_Command, PROPS_Command);
      
      for(board=0;board<m_n_boards_per_subrack;board++)
      {
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Board, PSCAT_LCU_PIC_Board, PROPS_Board);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_MEPStatus, PSCAT_LCU_PIC_MEPStatus, PROPS_MEPStatus);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Maintenance, PSCAT_LCU_PIC_Maintenance, PROPS_Maintenance);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Alert, PSCAT_LCU_PIC_Alert, PROPS_Alert);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Command,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Command, PSCAT_LCU_PIC_Command, PROPS_Command);
        
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_Ethernet, PSCAT_LCU_PIC_Ethernet, PROPS_Ethernet);
        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        addMyPropertySet(scopeString, TYPE_LCU_PIC_FPGA, PSCAT_LCU_PIC_FPGA, PROPS_FPGA);
    
        for(ap=0;ap<m_n_aps_per_board;ap++)
        {
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_FPGA, PSCAT_LCU_PIC_FPGA, PROPS_FPGA);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_SYNCStatus, PSCAT_LCU_PIC_SYNCStatus, PROPS_SYNCStatus);
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rack,subrack,board,ap);
          addMyPropertySet(scopeString, TYPE_LCU_PIC_BoardRCUStatus, PSCAT_LCU_PIC_BoardRCUStatus, PROPS_BoardRCUStatus);
          for(rcu=0;rcu<m_n_rcus_per_ap;rcu++)
          {
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_RCU, PSCAT_LCU_PIC_RCU, PROPS_RCU);
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Command,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_Command, PSCAT_LCU_PIC_Command, PROPS_Command);

            m_propertySet2RCUMap[string(scopeString)] = globalRcuNr++;

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
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA_Command,rack,subrack,board,ap,rcu);
            addMyPropertySet(scopeString, TYPE_LCU_PIC_Command, PSCAT_LCU_PIC_Command, PROPS_Command);
          }
        }
      }
    }  
  }
  
  // subscribe to maintenanace properties and alert properties
  
}

RegisterAccessTask::~RegisterAccessTask()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void RegisterAccessTask::addMyPropertySet(const char* scope,
    const char* type, 
    TPSCategory category, 
    const TPropertyConfig propconfig[],
    GCFMyPropertySet::TDefaultUse defaultUse)
{
  boost::shared_ptr<GCFMyPropertySet> propsPtr(new GCFMyPropertySet(scope,type,category,&m_answer, defaultUse));
  m_myPropertySetMap[scope]=propsPtr;
  
  propsPtr->initProperties(propconfig);
}

bool RegisterAccessTask::isConnected()
{
  return m_RSPclient.isConnected();
}

GCFEvent::TResult RegisterAccessTask::initial(GCFEvent& e, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
      GCFPropSetAnswerEvent* pPropAnswer = static_cast<GCFPropSetAnswerEvent*>(&e);
      assert(pPropAnswer);
      if(pPropAnswer->result != 0)
      {
        LOG_WARN(formatString("MyPropset %s could not be enabled: %d",pPropAnswer->pScope,pPropAnswer->result));
      }
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      break;
    }

    case F_ENTRY:
    {
      LOG_INFO("NOT configuring propsets using APCs...");
      TRAN(RegisterAccessTask::APCsLoaded);
      break;
    }

    case F_PS_CONFIGURED:
    {
/*
      m_APCsLoadCounter++;
      LOG_INFO(formatString("Propset %d configured", m_APCsLoadCounter));
      if(m_APCsLoadCounter == m_APCMap.size())
      {
        m_APCsLoaded=true;
        TRAN(RegisterAccessTask::APCsLoaded);
      }
*/
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
        bool res=m_RSPclient.open(); // need this otherwise GTM_Sockethandler is not called
        LOG_DEBUG(formatString("m_RSPclient.open() returned %s",(res?"true":"false")));
        if(!res)
        {
          m_RSPclient.setTimer((long)3);
        }  
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

    case F_VGETRESP:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&e);
      
      if(strstr(pPropAnswer->pPropName,"Maintenance.status") != 0)
      {
        handleMaintenance(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName, "status") != 0)
      {
        LOG_DEBUG("status property changed");
        
        _refreshFunctionality();
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&e);
      
      if(strstr(pPropAnswer->pPropName,"Maintenance.status") != 0)
      {
        handleMaintenance(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName, "status") != 0)
      {
        LOG_DEBUG("status property changed");
        
        _refreshFunctionality();
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
          updateVersion(scopeString,string(version),double(ack.timestamp));
          
          sprintf(version,"%d.%d",ack.versions.bp()(board)  >> 4,ack.versions.bp()(board)  & 0xF);
          LOG_INFO(formatString("bp[%d].version = 0x%x",board,ack.versions.bp()(board)));
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rackNr,subRackNr,relativeBoardNr);
          updateVersion(scopeString,string(version),double(ack.timestamp));
  
          for (int ap = 0; ap < EPA_Protocol::N_AP; ap++)
          {
            sprintf(version,"%d.%d",ack.versions.ap()(board * EPA_Protocol::N_AP + ap) >> 4,
                                    ack.versions.ap()(board * EPA_Protocol::N_AP + ap) &  0xF);
            LOG_INFO(formatString("ap[%d][%d].version = 0x%x",board,ap,ack.versions.ap()(board * EPA_Protocol::N_AP + ap)));
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,ap);
            updateVersion(scopeString,string(version),double(ack.timestamp));
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
      
      TRAN(RegisterAccessTask::subscribingXcStats);
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

GCFEvent::TResult RegisterAccessTask::subscribingXcStats(GCFEvent& e, GCFPortInterface &port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // subscribe to xc stats updates
      RSPSubxcstatsEvent substats;
      substats.timestamp.setNow();
      substats.rcumask = std::bitset<MAX_N_RCUS>((1<<m_n_rcus)-1);
      substats.period = m_stats_update_interval;
      m_RSPclient.send(substats);
      
      break;
    }

    case RSP_SUBXCSTATSACK:
    {
      LOG_INFO("RSP_SUBXCSTATSACK received");
      RSPSubxcstatsackEvent ack(e);

      if(ack.status != SUCCESS)
      {
        LOG_ERROR("RSP_SUBXCSTATS failure");
      }
      else
      {
        m_subXcStatsHandle = ack.handle;
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      TMyPropertySetMap::iterator propsetIt = m_myPropertySetMap.find(SCOPE_PIC);
      if(propsetIt != m_myPropertySetMap.end())
      {
        boost::shared_ptr<GCFPVInteger> pvIntegrationTime(static_cast<GCFPVInteger*>(propsetIt->second->getValue(PROPNAME_INTEGRATIONTIME)));
        if(pvIntegrationTime != 0)
        {
          m_integrationTime = pvIntegrationTime->getValue();
          if(m_integrationTime > 0)
	  {
            m_integrationTimerID = m_RSPclient.setTimer(static_cast<double>(m_integrationTime));
	  }
        }
        boost::shared_ptr<GCFPVInteger> pvIntegrationMethod(static_cast<GCFPVInteger*>(propsetIt->second->getValue(PROPNAME_INTEGRATIONMETHOD)));
        if(pvIntegrationMethod != 0)
        {
          m_integrationMethod = pvIntegrationMethod->getValue();
        }
      }
      
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
    
    case RSP_UPDXCSTATS:
    {
      LOG_INFO("RSP_UPDXCSTATS received");
      status = handleUpdXcStats(e,port);
      break;
    }
    
    case RSP_SETRCUACK:
    {
      LOG_INFO("RSP_SETRCUACK received");
      break;
    }
    
//TODO    case RSP_COMMANDRESULT:
//TODO    {
//TODO      LOG_INFO("RSP_COMMANDRESULT received");
//TODO      handleCommandResult(e);
//TODO      break;
//TODO    }
    
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

      if(strstr(pPropAnswer->pPropName,"Maintenance."PROPNAME_STATUS) != 0)
      {
        handleMaintenance(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,"Command."PROPNAME_COMMAND) != 0)
      {
        handleCommand(string(pPropAnswer->pPropName),*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName, PROPNAME_STATUS) != 0)
      {
        _refreshFunctionality();
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_LBAENABLE) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_LBAENABLE,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_HBAENABLE) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_HBAENABLE,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_BANDSEL) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_BANDSEL,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_FILSEL0) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_FILSEL0,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_FILSEL1) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_FILSEL1,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_VLENABLE) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_VLENABLE,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_VHENABLE) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_VHENABLE,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_VDDVCCEN) != 0)
      {
        handleRCUSettings(string(pPropAnswer->pPropName),BIT_VDDVCCEN,*pPropAnswer->pValue);
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_INTEGRATIONTIME) != 0)
      {
        GCFPVInteger pvInt;
        pvInt.copy(*pPropAnswer->pValue);
        m_integrationTime = pvInt.getValue();
        LOG_INFO(formatString("integration time changed to %d",m_integrationTime));

        m_RSPclient.cancelTimer(m_integrationTimerID);
        
        if(m_integrationTime == 0)
        {
          m_integratingStatisticsSubband.free();
          m_numStatisticsSubband=0;
          m_integratingStatisticsBeamlet.free();
          m_numStatisticsBeamlet=0;
        }
        else
        {
          m_integratingStatisticsSubband.free();
          m_numStatisticsSubband=0;
          m_integratingStatisticsBeamlet.free();
          m_numStatisticsBeamlet=0;
          m_integrationTimerID = m_RSPclient.setTimer(static_cast<double>(m_integrationTime));
        }
      }
      else if(strstr(pPropAnswer->pPropName,PROPNAME_INTEGRATIONMETHOD) != 0)
      {
        GCFPVInteger pvInt;
        pvInt.copy(*pPropAnswer->pValue);
        m_integrationMethod = pvInt.getValue();
        LOG_INFO(formatString("integration method changed to %d",m_integrationMethod));
      }
      break;
    }
    
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(e);
      if(timerEvent.id == m_integrationTimerID)
      {
        _integrateStatistics();
        m_integrationTimerID = m_RSPclient.setTimer(static_cast<double>(m_integrationTime));
      }
      break;
    }
    
    case F_EXIT:
    {
      m_RSPclient.cancelTimer(m_integrationTimerID);
      
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
      RSPUnsubxcstatsEvent unsubXcStats;
      unsubXcStats.handle = m_subXcStatsHandle; // remove subscription with this handle
      m_RSPclient.send(unsubXcStats);
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
    
    int rackNr;
    int subRackNr;
    int relativeBoardNr;
    char scopeString[300];
    double timestamp = double(updStatusEvent.timestamp);

    int boardNr;
    for(boardNr=boardStatus.lbound(blitz::firstDim); boardNr <= boardStatus.ubound(blitz::firstDim); ++boardNr)
    {
      getBoardRelativeNumbers(boardNr,rackNr,subRackNr,relativeBoardNr);
      LOG_INFO(formatString("UpdStatus:Rack:%d:SubRack:%d:Board::%d\n",rackNr,subRackNr,relativeBoardNr));
      
      uint8   rspVoltage_1_5 = boardStatus(boardNr).rsp.voltage_1_5;
      uint8   rspVoltage_2_5 = boardStatus(boardNr).rsp.voltage_2_5;
      uint8   rspVoltage_3_3 = boardStatus(boardNr).rsp.voltage_3_3;
      uint8   rspVoltage_12 = boardStatus(boardNr).rsp.voltage_12;
      LOG_INFO(formatString("UpdStatus:RSP voltage_1_5:%d:voltage_2_5:%d:voltage_3_3:%d:voltage_12:%d",rspVoltage_1_5,rspVoltage_2_5,rspVoltage_3_3,rspVoltage_12));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
      updateBoardProperties(scopeString,rspVoltage_1_5,rspVoltage_2_5,rspVoltage_3_3,rspVoltage_12,timestamp);
      
      uint8   bpTemp    = boardStatus(boardNr).fpga.bp_temp;
      uint8   ap0Temp   = boardStatus(boardNr).fpga.ap0_temp;
      uint8   ap1Temp   = boardStatus(boardNr).fpga.ap1_temp;
      uint8   ap2Temp   = boardStatus(boardNr).fpga.ap2_temp;
      uint8   ap3Temp   = boardStatus(boardNr).fpga.ap3_temp;
      LOG_INFO(formatString("UpdStatus:BP temp:%d",bpTemp));
      LOG_INFO(formatString("UpdStatus:AP0 temp:%d",ap0Temp));
      LOG_INFO(formatString("UpdStatus:AP1 temp:%d",ap1Temp));
      LOG_INFO(formatString("UpdStatus:AP2 temp:%d",ap2Temp));
      LOG_INFO(formatString("UpdStatus:AP3 temp:%d",ap3Temp));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rackNr,subRackNr,relativeBoardNr);
      updateFPGAboardProperties(scopeString,timestamp);
      
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rackNr,subRackNr,relativeBoardNr);
      updateFPGAproperties(scopeString,bpTemp,timestamp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,0);
      updateFPGAproperties(scopeString,ap0Temp,timestamp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,1);
      updateFPGAproperties(scopeString,ap1Temp,timestamp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,2);
      updateFPGAproperties(scopeString,ap2Temp,timestamp);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rackNr,subRackNr,relativeBoardNr,3);
      updateFPGAproperties(scopeString,ap3Temp,timestamp);

      uint32    ethFrames     = boardStatus(boardNr).eth.nof_frames;
      uint32    ethErrors     = boardStatus(boardNr).eth.nof_errors;
      uint8     ethLastError  = boardStatus(boardNr).eth.last_error;
      uint8     ethFfi0       = boardStatus(boardNr).eth.ffi0;
      uint8     ethFfi1       = boardStatus(boardNr).eth.ffi1;
      uint8     ethFfi2       = boardStatus(boardNr).eth.ffi2;
      LOG_INFO(formatString("UpdStatus:ETH frames:%d:errors:%d:last_error:%d:ffi0:%d:ffi1:%d:ffi2:%d",ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rackNr,subRackNr,relativeBoardNr);
      updateETHproperties(scopeString,ethFrames,ethErrors,ethLastError,ethFfi0,ethFfi1,ethFfi2,timestamp);  
  
      uint32    mepSeqnr = boardStatus(boardNr).mep.seqnr;
      uint8     mepError = boardStatus(boardNr).mep.error;
      uint8     mepFfi0  = boardStatus(boardNr).mep.ffi0;
      LOG_INFO(formatString("UpdStatus:MEP seqnr:%d:error:%d:ffi:%d",mepSeqnr,mepError,mepFfi0));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_MEPStatus,rackNr,subRackNr,relativeBoardNr);
      updateMEPStatusProperties(scopeString,mepSeqnr,mepError,mepFfi0,timestamp);  
      
      uint32    syncSample_offset = boardStatus(boardNr).ap0_sync.sample_offset;
      uint32    syncSync_count    = boardStatus(boardNr).ap0_sync.sync_count;
      uint32    syncSlice_count   = boardStatus(boardNr).ap0_sync.slice_count;
      LOG_INFO(formatString("SyncStatus ap0:sample_offset:%d:sync_count:%d:slice_count:%d",syncSample_offset,syncSync_count,syncSlice_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,0);
      updateSYNCStatusProperties(scopeString,syncSample_offset,syncSync_count,syncSlice_count,timestamp);

      syncSample_offset = boardStatus(boardNr).ap1_sync.sample_offset;
      syncSync_count    = boardStatus(boardNr).ap1_sync.sync_count;
      syncSlice_count   = boardStatus(boardNr).ap1_sync.slice_count;
      LOG_INFO(formatString("SyncStatus ap1:sample_offset:%d:sync_count:%d:slice_count:%d",syncSample_offset,syncSync_count,syncSlice_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,1);
      updateSYNCStatusProperties(scopeString,syncSample_offset,syncSync_count,syncSlice_count,timestamp);

      syncSample_offset = boardStatus(boardNr).ap2_sync.sample_offset;
      syncSync_count    = boardStatus(boardNr).ap2_sync.sync_count;
      syncSlice_count   = boardStatus(boardNr).ap2_sync.slice_count;
      LOG_INFO(formatString("SyncStatus ap2:sample_offset:%d:sync_count:%d:slice_count:%d",syncSample_offset,syncSync_count,syncSlice_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,2);
      updateSYNCStatusProperties(scopeString,syncSample_offset,syncSync_count,syncSlice_count,timestamp);

      syncSample_offset = boardStatus(boardNr).ap3_sync.sample_offset;
      syncSync_count    = boardStatus(boardNr).ap3_sync.sync_count;
      syncSlice_count   = boardStatus(boardNr).ap3_sync.slice_count;
      LOG_INFO(formatString("SyncStatus ap3:sample_offset:%d:sync_count:%d:slice_count:%d",syncSample_offset,syncSync_count,syncSlice_count));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_SYNCStatus,rackNr,subRackNr,relativeBoardNr,3);
      updateSYNCStatusProperties(scopeString,syncSample_offset,syncSync_count,syncSlice_count,timestamp);

      int apNr=0;
      uint8     boardRCUstatusStatusX       = boardStatus(boardNr).ap0_rcu.statusx;
      uint8     boardRCUstatusStatusY       = boardStatus(boardNr).ap0_rcu.statusy;
      uint8     boardRCUstatusFFI0          = boardStatus(boardNr).ap0_rcu.ffi0;
      uint8     boardRCUstatusFFI1          = boardStatus(boardNr).ap0_rcu.ffi1;
      uint32    boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap0_rcu.nof_overflowx;
      uint32    boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap0_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap0:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusFFI0,boardRCUstatusFFI1,timestamp);
      int rcuNr=0;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusNofOverflowX,timestamp);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusY,boardRCUstatusNofOverflowY,timestamp);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap1_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap1_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap1_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap1_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap1_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap1_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap1:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusFFI0,boardRCUstatusFFI1,timestamp);
      rcuNr=0;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusNofOverflowX,timestamp);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusY,boardRCUstatusNofOverflowY,timestamp);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap2_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap2_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap2_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap2_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap2_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap2_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap2:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusFFI0,boardRCUstatusFFI1,timestamp);
      rcuNr=0;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusNofOverflowX,timestamp);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusY,boardRCUstatusNofOverflowY,timestamp);

      apNr++;
      boardRCUstatusStatusX       = boardStatus(boardNr).ap3_rcu.statusx;
      boardRCUstatusStatusY       = boardStatus(boardNr).ap3_rcu.statusy;
      boardRCUstatusFFI0          = boardStatus(boardNr).ap3_rcu.ffi0;
      boardRCUstatusFFI1          = boardStatus(boardNr).ap3_rcu.ffi1;
      boardRCUstatusNofOverflowX  = boardStatus(boardNr).ap3_rcu.nof_overflowx;
      boardRCUstatusNofOverflowY  = boardStatus(boardNr).ap3_rcu.nof_overflowy;
      LOG_INFO(formatString("BoardRCUStatus ap3:statusX:%d:statusY:%d:ffi0:%d:ffi1:%d:nofOverflowX:%d:nofOverflowY:%d",boardRCUstatusStatusX,boardRCUstatusStatusY,boardRCUstatusFFI0,boardRCUstatusFFI1,boardRCUstatusNofOverflowX,boardRCUstatusNofOverflowY));
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_BoardRCUStatus,rackNr,subRackNr,relativeBoardNr,apNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusFFI0,boardRCUstatusFFI1,timestamp);
      rcuNr=0;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusX,boardRCUstatusNofOverflowX,timestamp);
      rcuNr++;
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackNr,subRackNr,relativeBoardNr,apNr,rcuNr);
      updateBoardRCUproperties(scopeString,boardRCUstatusStatusY,boardRCUstatusNofOverflowY,timestamp);
    }

    for(int rcuNr=rcuStatus.lbound(blitz::firstDim); rcuNr <= rcuStatus.ubound(blitz::firstDim); ++rcuNr)
    {
      uint8   rcuStatusBits = rcuStatus(rcuNr).status;
      LOG_INFO(formatString("UpdStatus:RCU[%d] status:0x%x",rcuNr,rcuStatusBits));
      
      int rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr;
      getRCURelativeNumbers(rcuNr,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);

      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      LOG_DEBUG(formatString("RCU[%d]= %s",rcuNr,scopeString));
      updateRCUproperties(scopeString,rcuStatusBits,timestamp);
    }
  }
  
  return status;
}

GCFEvent::TResult RegisterAccessTask::handleUpdStats(GCFEvent& e, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  {
    RSPUpdstatsEvent updStatsEvent(e);

    time_t curTime=(time_t)updStatsEvent.timestamp.sec();
    LOG_INFO(formatString("UpdStats:time:%s:status:%d:handle:%d", 
        ctime(&curTime),
        updStatsEvent.status,
        updStatsEvent.handle));

    _addStatistics(updStatsEvent.stats(), updStatsEvent.handle);
  }
  return status;
}

GCFEvent::TResult RegisterAccessTask::handleUpdXcStats(GCFEvent& e, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  {
    RSPUpdxcstatsEvent updXcStatsEvent(e);

    time_t curTime=(time_t)updXcStatsEvent.timestamp.sec();
    LOG_INFO(formatString("UpdXcStats:time:%s:status:%d:handle:%d", 
        ctime(&curTime),
        updXcStatsEvent.status,
        updXcStatsEvent.handle));

    _addXcStatistics(updXcStatsEvent.stats(), updXcStatsEvent.handle);
  }
  return status;
}

void RegisterAccessTask::updateBoardProperties(string scope,
                                               uint8  voltage_15,
                                               uint8  voltage_33,
                                               uint8  ffi0,
                                               uint8  ffi1,
                                               double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    double v15 = static_cast<double>(voltage_15) * (2.5/192.0);
    GCFPVDouble pvDouble15(v15);
    it->second->setValueTimed(string(PROPNAME_VOLTAGE15),pvDouble15,timestamp);
    
    double v33 = static_cast<double>(voltage_33) * (5.0/192.0);
    GCFPVDouble pvDouble33(v33);
    it->second->setValueTimed(string(PROPNAME_VOLTAGE33),pvDouble33, timestamp);

    GCFPVUnsigned pvTemp;
    pvTemp.setValue(ffi0);
    it->second->setValueTimed(string(PROPNAME_FFI0),pvTemp, timestamp);

    pvTemp.setValue(ffi1);
    it->second->setValueTimed(string(PROPNAME_FFI1),pvTemp, timestamp);
  }
}

void RegisterAccessTask::updateETHproperties(string scope,
                                             uint32 frames,
                                             uint32 errors,
                                             uint8  lastError,
                                             uint8  ffi0,
                                             uint8  ffi1,
                                             uint8  ffi2,
                                             double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
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
    it->second->setValueTimed(string(PROPNAME_FRAMESRECEIVED),pvTemp, timestamp);
    
    pvTemp.setValue(errors);
    it->second->setValueTimed(string(PROPNAME_FRAMESERROR),pvTemp, timestamp);

    pvTemp.setValue(lastError);
    it->second->setValueTimed(string(PROPNAME_LASTERROR),pvTemp, timestamp);

    pvTemp.setValue(ffi0);
    it->second->setValueTimed(string(PROPNAME_FFI0),pvTemp, timestamp);

    pvTemp.setValue(ffi1);
    it->second->setValueTimed(string(PROPNAME_FFI1),pvTemp, timestamp);

    pvTemp.setValue(ffi2);
    it->second->setValueTimed(string(PROPNAME_FFI2),pvTemp, timestamp);
  }
}

/**
 * update MEP status properties 
 */
void RegisterAccessTask::updateMEPStatusProperties(string scope,uint32 seqnr,
                                                                uint8  error,
                                                                uint8  ffi0,
                                                                double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(seqnr);
    it->second->setValueTimed(string(PROPNAME_SEQNR),pvTemp, timestamp);
    
    pvTemp.setValue(error);
    it->second->setValueTimed(string(PROPNAME_ERROR),pvTemp, timestamp);

    pvTemp.setValue(ffi0);
    it->second->setValueTimed(string(PROPNAME_FFI0),pvTemp, timestamp);
  }
}

void RegisterAccessTask::updateSYNCStatusProperties(string scope,uint32 sample_count,
                                                                 uint32 sync_count,
                                                                 uint32 error_count,
                                                                 double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvTemp(sample_count);
    it->second->setValueTimed(string(PROPNAME_SAMPLECOUNT),pvTemp, timestamp);
    
    pvTemp.setValue(sync_count);
    it->second->setValueTimed(string(PROPNAME_SYNCCOUNT),pvTemp, timestamp);

    pvTemp.setValue(error_count);
    it->second->setValueTimed(string(PROPNAME_ERRORCOUNT),pvTemp, timestamp);
  }
}
                                             
void RegisterAccessTask::updateFPGAboardProperties(string scope, double /*timestamp*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
  }
}

void RegisterAccessTask::updateFPGAproperties(string scope, uint8 temp,
                                                            double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVDouble pvDouble(static_cast<double>(temp));
    it->second->setValueTimed(string(PROPNAME_TEMPERATURE),pvDouble, timestamp);
  }
}

void RegisterAccessTask::updateBoardRCUproperties(string scope,uint8  ffi0,
                                                               uint8  ffi1,
                                                               double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVUnsigned pvUns(ffi0);
    it->second->setValueTimed(string(PROPNAME_FFI0),pvUns, timestamp);
    pvUns.setValue(ffi1);
    it->second->setValueTimed(string(PROPNAME_FFI1),pvUns, timestamp);
  }
}

void RegisterAccessTask::updateRCUproperties(string scope,uint8 status, double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // layout rcu status (for both statusX and statusY): 
  // 7         6         5         4        3        2       1          0
  // VDDVCCEN VHENABLE VLENABLE FILSEL1 FILSEL0 BANDSEL HBAENABLE LBAENABLE
    
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    unsigned int tempStatus = (status >> 7 ) & 0x01;
    GCFPVBool pvBoolVddVccEn(tempStatus);
    if(pvBoolVddVccEn.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_VDDVCCEN)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_VDDVCCEN),pvBoolVddVccEn, timestamp);
    }
    
    tempStatus = (status >> 6) & 0x01;
    GCFPVBool pvBoolVhEnable(tempStatus);
    if(pvBoolVhEnable.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_VHENABLE)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_VHENABLE),pvBoolVhEnable, timestamp);
    }
    
    tempStatus = (status >> 5) & 0x01;
    GCFPVBool pvBoolVlEnable(tempStatus);
    if(pvBoolVlEnable.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_VLENABLE)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_VLENABLE),pvBoolVlEnable, timestamp);
    }
    
    tempStatus = (status >> 4) & 0x01;
    GCFPVBool pvBoolFilSel1(tempStatus);
    if(pvBoolFilSel1.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_FILSEL1)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_FILSEL1),pvBoolFilSel1, timestamp);
    }
    
    tempStatus = (status >> 3) & 0x01;
    GCFPVBool pvBoolFilSel0(tempStatus);
    if(pvBoolFilSel0.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_FILSEL0)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_FILSEL0),pvBoolFilSel0, timestamp);
    }
    
    tempStatus = (status >> 2) & 0x01;
    GCFPVBool pvBoolBandSel(tempStatus);
    if(pvBoolBandSel.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_BANDSEL)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_BANDSEL),pvBoolBandSel, timestamp);
    }
    
    tempStatus = (status >> 1) & 0x01;
    GCFPVBool pvBoolHBAEnable(tempStatus);
    if(pvBoolHBAEnable.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_HBAENABLE)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_HBAENABLE),pvBoolHBAEnable, timestamp);
    }
    
    tempStatus = (status >> 0) & 0x01;
    GCFPVBool pvBoolLBAEnable(tempStatus);
    if(pvBoolLBAEnable.getValue() != boost::shared_ptr<GCFPVBool>(static_cast<GCFPVBool*>(it->second->getOldValue(PROPNAME_LBAENABLE)))->getValue())
    {
      it->second->setValueTimed(string(PROPNAME_LBAENABLE),pvBoolLBAEnable, timestamp);
    }
  }
}

void RegisterAccessTask::updateBoardRCUproperties(string scope,uint8 /*status*/, uint32 nof_overflow, double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    LOG_WARN("ignoring status field in BoardRCUStatus");
    GCFPVUnsigned pvUns(nof_overflow);
    it->second->setValueTimed(string(PROPNAME_NOFOVERFLOW),pvUns, timestamp);
  }
}

void RegisterAccessTask::updateVersion(string scope, string version, double timestamp)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(scope);
  if(it == m_myPropertySetMap.end())
  {
  	LOG_FATAL(formatString("PropertySet not found: %s",scope.c_str()));
  }
  else
  {
    GCFPVString pvString(version);
    it->second->setValueTimed(string(PROPNAME_VERSION),pvString,timestamp);
  }    
}

void RegisterAccessTask::handleMaintenance(string propName, const GCFPValue& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFPVBool pvBool;
  pvBool.copy(value);
  bool maintenanceFlag(pvBool.getValue());
  
  // strip last part of the property name to get the resource name.
  int pos=propName.find_last_of("_.");
  string resource = propName.substr(0,pos);
  m_physicalModel.inMaintenance(maintenanceFlag,resource);
}

void RegisterAccessTask::handleCommand(string propName, const GCFPValue& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  TMyPropertySetMap::iterator propsetIt = getPropertySetFromScope(propName);
  if(propsetIt == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",propName.c_str()));
  }
  else
  {
    GCFPVString pvString;
    pvString.copy(value);
    string command(pvString.getValue());
    
    // strip last part of the property name to get the resource name.
    int pos=propName.find_last_of("_.");
    string resource = propName.substr(0,pos);
    string result("Command not supported");
    TCommandResultCode resultCode(COMMAND_RESULT_ERROR);
    
    if(commandGetID == command.substr(0,commandGetID.length()))
    {
      // determine the property
      if(resource.find("HFA") != string::npos)
      {
        //TODO int rcuNr = getRCUHardwareNr(resource);
        //TODO RSPTestGetHBAIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.rcu = rcuNr;
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("RCU") != string::npos)
      {
        //TODO int rcuNr = getRCUHardwareNr(resource);
        //TODO RSPTestGetRCUIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.rcu = rcuNr;
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("Board") != string::npos)
      {
        //TODO RSPTestGetRSPIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("SubRack") != string::npos)
      {
        //TODO RSPTestGetTDSIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("TBB") != string::npos)
      {
        //TODO RSPTestGetTBBIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("PIC") != string::npos)
      {
        //TODO RSPTestGetGPSIDEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else
      {
        resultCode = COMMAND_RESULT_ERROR;
        result="Command not supported for the selected resource";
      }
    }
    else if(commandTestRegisterReadWrite == command.substr(0,commandTestRegisterReadWrite.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestPPS == command.substr(0,commandTestPPS.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadSattelitePositions == command.substr(0,commandReadSattelitePositions.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadTimeConstant == command.substr(0,commandReadTimeConstant.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadConfiguration == command.substr(0,commandReadConfiguration.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadStatistics == command.substr(0,commandReadStatistics.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadPPSlockStatus == command.substr(0,commandReadPPSlockStatus.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadPLLlockStatus == command.substr(0,commandReadPLLlockStatus.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReadCurrent == command.substr(0,commandReadCurrent.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestRSPlinkSpeed == command.substr(0,commandTestRSPlinkSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestRCUlinkSpeed == command.substr(0,commandTestRCUlinkSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestSerdesSpeed == command.substr(0,commandTestSerdesSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestTBBlinkSpeed == command.substr(0,commandTestTBBlinkSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestBoundaryScan == command.substr(0,commandTestBoundaryScan.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestWaveform == command.substr(0,commandTestWaveform.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestTransient == command.substr(0,commandTestTransient.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestClock == command.substr(0,commandTestClock.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestFPGAlinkSpeed == command.substr(0,commandTestFPGAlinkSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestEthernetLoopBack == command.substr(0,commandTestEthernetLoopBack.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestSerdesLoopBack == command.substr(0,commandTestSerdesLoopBack.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestFPGAmemoryRandom == command.substr(0,commandTestFPGAmemoryRandom.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestFPGAmemory == command.substr(0,commandTestFPGAmemory.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestDataReception == command.substr(0,commandTestDataReception.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandTestRoundTripSpeed == command.substr(0,commandTestRoundTripSpeed.length()))
    {
      //TODO resultCode = COMMAND_RESULT_EXECUTING;
      //TODO result="Executing...";
      LOG_FATAL("TODO: send operational test event messages to RSP Driver");
      resultCode = COMMAND_RESULT_ERROR;
      result="Not implemented";
    }
    else if(commandReset == command.substr(0,commandReset.length()))
    {
      // determine the property
      if(resource.find("RCU") != string::npos)
      {
        //TODO int rcuNr = getRCUHardwareNr(resource);
        //TODO RSPResetRCUEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.rcu = rcuNr;
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("Board") != string::npos)
      {
        //TODO int rcuNr = getRCUHardwareNr(resource);
        //TODO int rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr;
        //TODO getRCURelativeNumbers(rcuNr,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
        //TODO RSPResetRSPEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.rack = rackRelativeNr;
        //TODO commandEvent.subrack = subRackRelativeNr;
        //TODO commandEvent.board = boardRelativeNr;
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else if(resource.find("TBB") != string::npos)
      {
        //TODO RSPResetTBBEvent commandEvent;
        //TODO commandEvent.timestamp.setNow();
        //TODO commandEvent.handle = m_commandHandle;
        m_pendingCommands[m_commandHandle++] = propName;
        //TODO m_RSPclient.send(commandEvent);
        //TODO resultCode = COMMAND_RESULT_EXECUTING;
        //TODO result="Executing...";
        LOG_FATAL("TODO: send operational test event messages to RSP Driver");
        resultCode = COMMAND_RESULT_ERROR;
        result="Not implemented";
      }
      else
      {
        resultCode = COMMAND_RESULT_ERROR;
        result="Command not supported for the selected resource";
      }
    }
  
    // set the result property.
    propsetIt->second->setValue(string(PROPNAME_RESULT_CODE),GCFPVInteger(resultCode));
    propsetIt->second->setValue(string(PROPNAME_RESULT),GCFPVString(result));
  }
}

GCFEvent::TResult RegisterAccessTask::handleCommandResult(GCFEvent& /*e*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  GCFEvent::TResult status = GCFEvent::HANDLED;

/*TODO
  RSPCommandresultEvent commandResultEvent(e);
  
  double timestamp = double(commandResultEvent.timestamp);
  time_t curTime=(time_t)timestamp;
  LOG_INFO(formatString("CommandResult:time:%s:status:%d:value:%s:handle:%d", 
      ctime(&curTime),
      commandResultEvent.status,
      commandResultEvent.value.c_str(),
      commandResultEvent.handle));

  map<uint32,string>::iterator it=m_pendingCommands.find(commandResultEvent.handle);
  if(it != m_pendingCommands.end())
  {
    TMyPropertySetMap::iterator propsetIt = getPropertySetFromScope(it->second);
    if(propsetIt == m_myPropertySetMap.end())
    {
      LOG_FATAL(formatString("PropertySet not found: %s. Unable to write command result property",it->second.c_str()));
    }
    else
    {
      // set the result property.
      propsetIt->second->setValueTimed(string(PROPNAME_RESULT_CODE),GCFPVInteger(commandResultEvent.status),timestamp);
      propsetIt->second->setValueTimed(string(PROPNAME_RESULT),GCFPVString(commandResultEvent.value),timestamp);
    }
    m_pendingCommands.erase(it);
  }
  else
  {
    LOG_ERROR(formatString("Command handle %d not found. Unable to write command result property",commandResultEvent.handle));
  }
*/
  return status;
}

void RegisterAccessTask::handleRCUSettings(string propName, const int bitnr, const GCFPValue& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  boost::shared_ptr<GCFPVBool> pvLBAEnable;
  boost::shared_ptr<GCFPVBool> pvHBAEnable;
  boost::shared_ptr<GCFPVBool> pvBandSel;
  boost::shared_ptr<GCFPVBool> pvFilSel0;
  boost::shared_ptr<GCFPVBool> pvFilSel1;
  boost::shared_ptr<GCFPVBool> pvVLEnable;
  boost::shared_ptr<GCFPVBool> pvVHEnable;
  boost::shared_ptr<GCFPVBool> pvVddVccEn;
  
  TMyPropertySetMap::iterator propsetIt = getPropertySetFromScope(propName);
  if(propsetIt != m_myPropertySetMap.end())
  {
    // get old register values
    pvLBAEnable.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_LBAENABLE))); // bit 0
    pvHBAEnable.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_HBAENABLE))); // bit 1
    pvBandSel.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_BANDSEL)));   // bit 2
    pvFilSel0.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_FILSEL0)));   // bit 3
    pvFilSel1.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_FILSEL1)));   // bit 4
    pvVLEnable.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_VLENABLE)));  // bit 5
    pvVHEnable.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_VHENABLE)));  // bit 6
    pvVddVccEn.reset(static_cast<GCFPVBool*>(propsetIt->second->getValue(PROPNAME_VDDVCCEN)));  // bit 7
    
    // modify the new value
    switch(bitnr)
    {
      case BIT_LBAENABLE:
        pvLBAEnable->copy(value);
        break;
      case BIT_HBAENABLE:
        pvHBAEnable->copy(value);
        break;
      case BIT_BANDSEL:
        pvBandSel->copy(value);
        break;
      case BIT_FILSEL0:
        pvFilSel0->copy(value);
        break;
      case BIT_FILSEL1:
        pvFilSel1->copy(value);
        break;
      case BIT_VLENABLE:
        pvVLEnable->copy(value);
        break;
      case BIT_VHENABLE:
        pvVHEnable->copy(value);
        break;
      case BIT_VDDVCCEN:
        pvVddVccEn->copy(value);
        break;
      default:
        break;
    }
    
    int rcucontrol = 0;
    if(pvLBAEnable->getValue())
      rcucontrol |= 0x01;
    if(pvHBAEnable->getValue())
      rcucontrol |= 0x02;
    if(pvBandSel->getValue())
      rcucontrol |= 0x04;
    if(pvFilSel0->getValue())
      rcucontrol |= 0x08;
    if(pvFilSel1->getValue())
      rcucontrol |= 0x10;
    if(pvVLEnable->getValue())
      rcucontrol |= 0x20;
    if(pvVHEnable->getValue())
      rcucontrol |= 0x40;
    if(pvVddVccEn->getValue())
      rcucontrol |= 0x80;
    
    int rcu = getRCUHardwareNr(propName);
    if(rcu>=0)
    {
      RSPSetrcuEvent setrcu;
      setrcu.timestamp = RTC::Timestamp(0,0);
      setrcu.rcumask = 0;
      setrcu.rcumask.set(rcu);
          
      setrcu.settings().resize(1);
      setrcu.settings()(0).value = rcucontrol;
    
      m_RSPclient.send(setrcu);
    }
    else
    {
      LOG_FATAL(formatString("rcu for property %s not found in local administration",propName.c_str()));
    }
  }
  else
  {
    LOG_FATAL(formatString("property %s not found in local administration",propName.c_str()));
  }
}

void RegisterAccessTask::getBoardRelativeNumbers(int boardNr,int& rackNr,int& subRackNr,int& relativeBoardNr)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  rackNr          = boardNr / (m_n_subracks_per_rack*m_n_boards_per_subrack);
  subRackNr       = (boardNr / m_n_boards_per_subrack) % m_n_subracks_per_rack;
  relativeBoardNr = boardNr % m_n_boards_per_subrack;
}

void RegisterAccessTask::getRCURelativeNumbers(int rcuNr,int& rackRelativeNr,int& subRackRelativeNr,int& boardRelativeNr,int& apRelativeNr,int& rcuRelativeNr)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  rackRelativeNr    = rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack*m_n_subracks_per_rack);
  subRackRelativeNr = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack) ) % m_n_subracks_per_rack;
  boardRelativeNr   = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board) ) % m_n_boards_per_subrack;
  apRelativeNr      = ( rcuNr / (m_n_rcus_per_ap) ) % m_n_aps_per_board;
  rcuRelativeNr     = ( rcuNr % m_n_rcus_per_ap);
}

int RegisterAccessTask::getRCUHardwareNr(const string& property)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  int rcu=-1;
  // strip property and systemname, propertyset name remains
  int posBegin=property.find_first_of(":")+1;
  int posEnd=property.find_last_of(".");
  string propertySetName = property.substr(posBegin,posEnd-posBegin);
  
  bool rcuFound=false;
  map<string,int>::iterator it = m_propertySet2RCUMap.begin();
  while(!rcuFound && it != m_propertySet2RCUMap.end())
  {
    if(propertySetName == it->first)
    {
      rcuFound=true;
      rcu = it->second;
    }
    else
    {
      ++it;
    }
  }
  return rcu;
}

RegisterAccessTask::TMyPropertySetMap::iterator RegisterAccessTask::getPropertySetFromScope(const string& property)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // strip property and systemname, propertyset name remains
  int posBegin=property.find_first_of(":")+1;
  int posEnd=property.find_last_of(".");
  string propertySetName = property.substr(posBegin,posEnd-posBegin);
  
  bool rcuFound=false;
  TMyPropertySetMap::iterator it = m_myPropertySetMap.begin();
  while(!rcuFound && it != m_myPropertySetMap.end())
  {
    if(propertySetName == it->first)
    {
      rcuFound=true;
    }
    else
    {
      ++it;
    }
  }
  return it;
  
}


void RegisterAccessTask::_addStatistics(TStatistics& statistics, uint32 statsHandle)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  if(statsHandle == m_subStatsHandleSubbandPower)
  {
    if(m_integrationTime == 0)
    {
      _writeStatistics(statistics, statsHandle);
    }
    else
    {
      if(m_integratingStatisticsSubband.size() == 0)
      {
        m_integratingStatisticsSubband.resize(statistics.shape());
        m_integratingStatisticsSubband = 0;
      }
    
      m_integratingStatisticsSubband += statistics;
      m_numStatisticsSubband++;

      stringstream statisticsStream;
      statisticsStream << m_integratingStatisticsSubband(blitz::Range(0,2),blitz::Range(0,2));
      LOG_DEBUG(formatString("subband: n_stats:%d; statistics:%s",m_numStatisticsSubband, statisticsStream.str().c_str()));
    }
  }
  else if(statsHandle == m_subStatsHandleBeamletPower)
  {
    if(m_integrationTime == 0)
    {
      _writeStatistics(statistics, statsHandle);
    }
    else
    {
      if(m_integratingStatisticsBeamlet.size() == 0)
      {
        m_integratingStatisticsBeamlet.resize(statistics.shape());
        m_integratingStatisticsBeamlet = 0;
      }
    
      m_integratingStatisticsBeamlet += statistics;
      m_numStatisticsBeamlet++;

      stringstream statisticsStream;
      statisticsStream << m_integratingStatisticsBeamlet(blitz::Range(0,2),blitz::Range(0,2));
      LOG_DEBUG(formatString("beamlet: n_stats:%d; statistics:%s",m_numStatisticsBeamlet, statisticsStream.str().c_str()));
    }
  }
}

void RegisterAccessTask::_integrateStatistics()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  TStatistics statisticsSubband;
  TStatistics statisticsBeamlet;
  TXcStatistics xcstatistics;
  
  if(m_numStatisticsSubband != 0)
  {
    statisticsSubband.resize(m_integratingStatisticsSubband.shape());
    statisticsSubband = 0;
    
    switch(m_integrationMethod)
    {
      case 0: // average
      default:
        statisticsSubband = m_integratingStatisticsSubband/static_cast<double>(m_numStatisticsSubband);
        break;
  //    case 1: // NYI
  //      break;
    }
  
    stringstream statisticsStream;
    statisticsStream << statisticsSubband(blitz::Range(0,2),blitz::Range(0,2));
    LOG_DEBUG(formatString("subband integrated: n_stats:%d; statistics:%s",m_numStatisticsSubband, statisticsStream.str().c_str()));

    m_integratingStatisticsSubband = 0;
    m_numStatisticsSubband=0;

    _writeStatistics(statisticsSubband, m_subStatsHandleSubbandPower);
  }
  if(m_numStatisticsBeamlet != 0)
  {
    statisticsBeamlet.resize(m_integratingStatisticsBeamlet.shape());
    statisticsBeamlet = 0;
    
    switch(m_integrationMethod)
    {
      case 0: // average
      default:
        statisticsBeamlet = m_integratingStatisticsBeamlet/static_cast<double>(m_numStatisticsBeamlet);
        break;
  //    case 1: // NYI
  //      break;
    }

    stringstream statisticsStream;
    statisticsStream << statisticsSubband(blitz::Range(0,2),blitz::Range(0,2));
    LOG_DEBUG(formatString("beamlet integrated: n_stats:%d; statistics:%s",m_numStatisticsBeamlet, statisticsStream.str().c_str()));

    m_integratingStatisticsBeamlet.free();
    m_numStatisticsBeamlet=0;
    _writeStatistics(statisticsBeamlet, m_subStatsHandleBeamletPower);
  }
  if(m_numXcStatistics != 0)
  {
    xcstatistics.resize(m_integratingXcStatistics.shape());
    xcstatistics = 0;
    
    switch(m_integrationMethod)
    {
      case 0: // average
      default:
        xcstatistics = m_integratingXcStatistics/static_cast<double>(m_numXcStatistics);
        break;
  //    case 1: // NYI
  //      break;
    }
  
    // log the first elements
    stringstream statisticsStream;
    statisticsStream << xcstatistics(blitz::Range(0,2),blitz::Range(0,2),blitz::Range(0,2),blitz::Range(0,2));
    LOG_DEBUG(formatString("subband integrated: n_stats:%d; statistics:%s",m_numXcStatistics, statisticsStream.str().c_str()));

    m_integratingXcStatistics = 0;
    m_numXcStatistics=0;

    _writeXcStatistics(xcstatistics, m_subXcStatsHandle);
  }
}

void RegisterAccessTask::_writeStatistics(TStatistics& statistics, uint32 statsHandle)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  int maxRCUs     = statistics.ubound(blitz::firstDim) - statistics.lbound(blitz::firstDim) + 1;
  int maxSubbands = statistics.ubound(blitz::secondDim) - statistics.lbound(blitz::secondDim) + 1;
  LOG_DEBUG(formatString("maxRCUs:%d maxSubbands:%d",maxRCUs,maxSubbands));
    
  if(m_centralized_stats)
  {
    LOG_DEBUG("Writing statistics to a dynamic array of doubles");
    // build a vector of doubles that will be stored in one datapoint
    GCFPValueArray valuePointerVector;
  
    // first elements indicate the length of the array
    valuePointerVector.push_back(new GCFPVDouble(maxRCUs));
    valuePointerVector.push_back(new GCFPVDouble(maxSubbands));
  
    // then for each rcu, the statistics of the beamlets or subbands
    int rcu,subband;
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
      if(statsHandle == m_subStatsHandleSubbandPower)
      {
        propSetIt->second->setValue(string(PROPNAME_STATISTICSSUBBANDPOWER),dynamicArray);
      }
      else if(statsHandle == m_subStatsHandleBeamletPower)
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
  else
  {
    LOG_DEBUG("Writing statistics to one string");
    // statistics will be stored as a string in an element of each rcu
    // then for each rcu, the statistics of the beamlets or subbands
    int rcu,subband;
    for(rcu=statistics.lbound(blitz::firstDim);rcu<=statistics.ubound(blitz::firstDim);rcu++)
    {
      LOG_DEBUG(formatString("rcu:%d",rcu));
      
      stringstream statisticsStream;
      statisticsStream.setf(ios_base::fixed);
      for(subband=statistics.lbound(blitz::secondDim);subband<=statistics.ubound(blitz::secondDim);subband++)
      {
        double stat = statistics(rcu,subband);
        statisticsStream << subband << " ";
        statisticsStream << stat << endl;
      }
      LOG_DEBUG(formatString("first part of statistics:%s",statisticsStream.str().substr(0,30).c_str()));
      
      GCFPVString statisticsString(statisticsStream.str());
      // set the property
      char scopeString[300];
      int rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr;
      getRCURelativeNumbers(rcu,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      TMyPropertySetMap::iterator propSetIt=m_myPropertySetMap.find(scopeString);
      if(propSetIt != m_myPropertySetMap.end())
      {
        if(statsHandle == m_subStatsHandleSubbandPower)
        {
          TGCFResult res = propSetIt->second->setValue(string(PROPNAME_STATISTICSSUBBANDPOWER),statisticsString);
          LOG_DEBUG(formatString("Writing subband statistics to %s returned %d",propSetIt->second->getScope().c_str(),res));
        }
        else if(statsHandle == m_subStatsHandleBeamletPower)
        {
          TGCFResult res = propSetIt->second->setValue(string(PROPNAME_STATISTICSBEAMLETPOWER),statisticsString);
          LOG_DEBUG(formatString("Writing beamlet statistics to %s returned %d",propSetIt->second->getScope().c_str(),res));
        }
      }
    }
  }
}

void RegisterAccessTask::_addXcStatistics(TXcStatistics& statistics, uint32 statsHandle)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  if(statsHandle == m_subXcStatsHandle)
  {
    if(m_integrationTime == 0)
    {
      _writeXcStatistics(statistics, statsHandle);
    }
    else
    {
      if(m_integratingXcStatistics.size() == 0)
      {
        m_integratingXcStatistics.resize(statistics.shape());
        m_integratingXcStatistics = 0;
      }
    
      m_integratingXcStatistics += statistics;
      m_numXcStatistics++;

      // log the first elements
      stringstream statisticsStream;
      statisticsStream << m_integratingXcStatistics(blitz::Range(0,2),blitz::Range(0,2),blitz::Range(0,2),blitz::Range(0,2));
      LOG_DEBUG(formatString("Crosslet: n_stats:%d; statistics:%s",m_numXcStatistics, statisticsStream.str().c_str()));
    }
  }
}

void RegisterAccessTask::_writeXcStatistics(TXcStatistics& statistics, uint32 statsHandle)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  int dim1 = statistics.ubound(blitz::firstDim) - statistics.lbound(blitz::firstDim) + 1;
  int dim2 = statistics.ubound(blitz::secondDim) - statistics.lbound(blitz::secondDim) + 1;
  int dim3 = statistics.ubound(blitz::thirdDim) - statistics.lbound(blitz::thirdDim) + 1;
  int dim4 = statistics.ubound(blitz::fourthDim) - statistics.lbound(blitz::fourthDim) + 1;
  LOG_DEBUG(formatString("dimensions: %d x %d x %d x %d",dim1,dim2,dim3,dim4));
    
  if(!m_centralized_stats)
  {
    LOG_FATAL("NOT Writing XC statistics to one string because the string would be far too big");
  }
  
  LOG_DEBUG("Writing statistics to a dynamic array of doubles");
  // build a vector of doubles that will be stored in one datapoint
  GCFPValueArray valuePointerVector;

  // first elements indicate the dimensions
  valuePointerVector.push_back(new GCFPVDouble(dim1));
  valuePointerVector.push_back(new GCFPVDouble(dim2));
  valuePointerVector.push_back(new GCFPVDouble(dim3));
  valuePointerVector.push_back(new GCFPVDouble(dim4));

  // then add all stats
  for(dim1=statistics.lbound(blitz::firstDim);dim1<=statistics.ubound(blitz::firstDim);dim1++)
  {
    for(dim2=statistics.lbound(blitz::firstDim);dim2<=statistics.ubound(blitz::firstDim);dim2++)
    {
      for(dim3=statistics.lbound(blitz::firstDim);dim3<=statistics.ubound(blitz::firstDim);dim3++)
      {
        for(dim4=statistics.lbound(blitz::firstDim);dim4<=statistics.ubound(blitz::firstDim);dim4++)
        {
          complex<double> stat = statistics(dim1,dim2,dim3,dim4);
          valuePointerVector.push_back(new GCFPVDouble(stat.real()));
          valuePointerVector.push_back(new GCFPVDouble(stat.imag()));
        }
      }
    }
  }

  // convert the vector of unsigned values to a dynamic array
  GCFPVDynArr dynamicArray(LPT_DOUBLE,valuePointerVector);
  
  // set the property
  TMyPropertySetMap::iterator propSetIt=m_myPropertySetMap.find(string(SCOPE_PIC));
  if(propSetIt != m_myPropertySetMap.end())
  {
    if(statsHandle == m_subXcStatsHandle)
    {
      propSetIt->second->setValue(string(PROPNAME_XCSTATISTICS),dynamicArray);
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

void RegisterAccessTask::_refreshFunctionality()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  int rack, subrack, board, ap, rcu;
  char scopeString[300];
  
  int racksDefect(0);
  for(rack=0;rack<m_n_racks;rack++)
  {
    int subracksDefect(0);
    for(subrack=0;subrack<m_n_subracks_per_rack;subrack++)
    {
      int boardsDefect(0);
      for(board=0;board<m_n_boards_per_subrack;board++)
      {
        int apsDefect(0);
        int ethDefect(0);
        int bpDefect(0);
        for(ap=0;ap<m_n_aps_per_board;ap++)
        {
          int rcusDefect(0);
          for(rcu=0;rcu<m_n_rcus_per_ap;rcu++)
          {
            int lfasDefect(0);
            int hfasDefect(0);
            
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rack,subrack,board,ap,rcu);
            lfasDefect+=_isDefect(scopeString);
            _setFunctionality(scopeString, (lfasDefect == 0));
            
            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA,rack,subrack,board,ap,rcu);
            hfasDefect+=_isDefect(scopeString);
            _setFunctionality(scopeString, (hfasDefect == 0));

            sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
            int isDefect = _isDefect(scopeString);
            
            if(isDefect==1 || (lfasDefect>0&&hfasDefect>0))
            {
              rcusDefect++;
              _setFunctionalityRCU(rack,subrack,board,ap,rcu, false);
            }
            else
            {
              _setFunctionality(scopeString, true);
            }
          }
          sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
          int isDefect = _isDefect(scopeString);
          
          if(isDefect==1 || rcusDefect >= m_n_rcus_per_ap)
          {
            apsDefect++;
            _setFunctionalityAP(rack,subrack,board,ap, false);
          }
          else
          {
            _setFunctionality(scopeString, true);
          }
        }

        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
        bpDefect+=_isDefect(scopeString);
        _setFunctionality(scopeString, (bpDefect == 0));

        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
        ethDefect+=_isDefect(scopeString);
        _setFunctionality(scopeString, (ethDefect == 0));

        sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
        int isDefect = _isDefect(scopeString);
        
        if(isDefect==1 || bpDefect>0 || ethDefect>0 || apsDefect >= m_n_aps_per_board)
        {
          boardsDefect++;
          _setFunctionalityBoard(rack,subrack,board,false);
        }
        else
        {
          _setFunctionality(scopeString, true);
        }
      }
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
      int isDefect = _isDefect(scopeString);
      
      if(isDefect==1 || boardsDefect >= m_n_boards_per_subrack)
      {
        subracksDefect++;
        _setFunctionalitySubRack(rack,subrack,false);
      }
      else
      {
        _setFunctionality(scopeString, true);
      }
    }  
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    int isDefect = _isDefect(scopeString);
    
    if(isDefect==1 || subracksDefect >= m_n_subracks_per_rack)
    {
      racksDefect++;
      _setFunctionalityRack(rack,false);
    }
    else
    {
      _setFunctionality(scopeString, true);
    }
  }
  sprintf(scopeString,SCOPE_PIC);
  int isDefect = _isDefect(scopeString);
  
  if(isDefect==1 || racksDefect >= m_n_racks)
  {
    _setFunctionalityStation(false);
  }
  else
  {
    _setFunctionality(scopeString, true);
  }
}

int RegisterAccessTask::_isDefect(char* scopeString)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  int isDefect(1);
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(string(scopeString));
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scopeString));
  }
  else
  {
    boost::shared_ptr<GCFPVInteger> pvStatus(static_cast<GCFPVInteger*>(it->second->getValue(PROPNAME_STATUS)));
    if(!pvStatus)
    {
      LOG_FATAL(formatString("PropertySet not found: %s",scopeString));
    }
    else
    {
      if(pvStatus->getValue()==RS_DEFECT)
      {
        isDefect=1;
      }
      else
      {
        isDefect=0;
      }
    }
  }
  return isDefect;
}

void RegisterAccessTask::_setFunctionality(char* scopeString, bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %s=%s",getName().c_str(),scopeString,(functional?"true":"false")).c_str());
  
  TMyPropertySetMap::iterator it=m_myPropertySetMap.find(string(scopeString));
  if(it == m_myPropertySetMap.end())
  {
    LOG_FATAL(formatString("PropertySet not found: %s",scopeString));
  }
  else
  {
    GCFPVBool pvBool(functional);
    it->second->setValue(PROPNAME_FUNCTIONALITY,pvBool);
  }
}

void RegisterAccessTask::_setFunctionalityRCU(int rack,int subrack,int board,int ap,int rcu, bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %d,%d,%d,%d,%d,%s",getName().c_str(),rack,subrack,board,ap,rcu,(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rack,subrack,board,ap,rcu);
  _setFunctionality(scopeString, functional);
  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_HFA,rack,subrack,board,ap,rcu);
  _setFunctionality(scopeString, functional);
}

void RegisterAccessTask::_setFunctionalityAP(int rack,int subrack,int board,int ap,bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %d,%d,%d,%d,%s",getName().c_str(),rack,subrack,board,ap,(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  for(int r=0;r<m_n_rcus_per_ap;r++)
  {
    _setFunctionalityRCU(rack,subrack,board,ap,r, false);
  }
}

void RegisterAccessTask::_setFunctionalityBoard(int rack,int subrack,int board,bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %d,%d,%d,%s",getName().c_str(),rack,subrack,board,(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
  _setFunctionality(scopeString, functional);
  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
  _setFunctionality(scopeString, functional);
  for(int a=0;a<m_n_aps_per_board;a++)
  {
    _setFunctionalityAP(rack,subrack,board,a,false);
  }
}

void RegisterAccessTask::_setFunctionalitySubRack(int rack,int subrack,bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %d,%d,%s",getName().c_str(),rack,subrack,(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  for(int b=0;b<m_n_boards_per_subrack;b++)
  {
    _setFunctionalityBoard(rack,subrack,b,false);
  }
}

void RegisterAccessTask::_setFunctionalityRack(int rack,bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %d,%s",getName().c_str(),rack,(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC_RackN,rack);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  for(int s=0;s<m_n_subracks_per_rack;s++)
  {
    _setFunctionalitySubRack(rack,s,false);
  }
}

void RegisterAccessTask::_setFunctionalityStation(bool functional)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - %s",getName().c_str(),(functional?"true":"false")).c_str());
  
  char scopeString[300];

  sprintf(scopeString,SCOPE_PIC);
  _setFunctionality(scopeString, functional);
  // also set functionality of underlying resources to false
  for(int r=0;r<m_n_racks;r++)
  {
    _setFunctionalityRack(r,false);
  }
}

} // namespace ARA


} // namespace LOFAR

