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
  m_systemStatus()
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);

  m_systemStatus.board().resize(N_BOARDS_PER_SUBRACK);
  m_systemStatus.rcu().resize(N_RCUS);

  // fill APCs map
  addPropertySet(SCOPE_PIC);
  addPropertySet(SCOPE_PIC_Maintenance);
  addPropertySet(SCOPE_PIC_RackN);
  addPropertySet(SCOPE_PIC_RackN_Alert);
  addPropertySet(SCOPE_PIC_RackN_Maintenance);
  addPropertySet(SCOPE_PIC_RackN_SubRackN);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_Maintenance);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_Alert);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_Alert);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_ETH);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_BP);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_APN);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics);
  addPropertySet(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance);
  
  m_RSPserver.init(*this, "ARAtestRSPserver", GCFPortInterface::SPP, RSP_PROTOCOL);
  
}

ARATestDriverTask::~ARATestDriverTask()
{
}

void ARATestDriverTask::addPropertySet(string scope)
{
  char scopeString[300];
  int rack;
  int subrack;
  int board;
  int ap;
  int rcu;
  
  if(scope == string(SCOPE_PIC))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Station),sizeof(PROPS_Station)/sizeof(PROPS_Station[0]));
  }
  else if(scope == string(SCOPE_PIC_RackN))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Rack),sizeof(PROPS_Rack)/sizeof(PROPS_Rack[0]));
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        addAllProperties(scopeString,static_cast<TProperty*>(PROPS_SubRack),sizeof(PROPS_SubRack)/sizeof(PROPS_SubRack[0]));
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Board),sizeof(PROPS_Board)/sizeof(PROPS_Board[0]));
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_ETH))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Ethernet),sizeof(PROPS_Ethernet)/sizeof(PROPS_Ethernet[0]));
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_BP))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          addAllProperties(scopeString,static_cast<TProperty*>(PROPS_FPGA),sizeof(PROPS_FPGA)/sizeof(PROPS_FPGA[0]));
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          for(ap=1;ap<=N_APS_PER_BOARD;ap++)
          {
            for(rcu=1;rcu<=N_RCUS_PER_AP;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              addAllProperties(scopeString,static_cast<TProperty*>(PROPS_RCU),sizeof(PROPS_RCU)/sizeof(PROPS_RCU[0]));
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          for(ap=1;ap<=N_APS_PER_BOARD;ap++)
          {
            for(rcu=1;rcu<=N_RCUS_PER_AP;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              addAllProperties(scopeString,static_cast<TProperty*>(PROPS_ADCStatistics),sizeof(PROPS_ADCStatistics)/sizeof(PROPS_ADCStatistics[0]));
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_Maintenance))
  {
    addAllProperties(scope,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
  }
  else if(scope == string(SCOPE_PIC_RackN_Maintenance))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_Maintenance))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          for(ap=1;ap<=N_APS_PER_BOARD;ap++)
          {
            for(rcu=1;rcu<=N_RCUS_PER_AP;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Maintenance),sizeof(PROPS_Maintenance)/sizeof(PROPS_Maintenance[0]));
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_Alert))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Alert),sizeof(PROPS_Alert)/sizeof(PROPS_Alert[0]));
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_Alert))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Alert),sizeof(PROPS_Alert)/sizeof(PROPS_Alert[0]));
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_Alert))
  {
    for(rack=1;rack<=N_RACKS;rack++)
    {
      for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
      {
        for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          addAllProperties(scopeString,static_cast<TProperty*>(PROPS_Alert),sizeof(PROPS_Alert)/sizeof(PROPS_Alert[0]));
        }
      }
    }
  }
}

void ARATestDriverTask::addAllProperties(string scope, TProperty* ptp, int numProperties)
{
  for(int i=0;i<numProperties;i++)
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

void ARATestDriverTask::updateETHstatus(string& propName,const GCFPValue* pvalue)
{
  LOG_INFO(formatString("updateETHstatus %s", propName.c_str()));
  
  GCFPVUnsigned pvUnsigned;
  pvUnsigned.copy(*pvalue);
  
  int rack;
  int subrack;
  int board;

  sscanf(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_ETH,&rack,&subrack,&board);
  
  EPA_Protocol::ETHStatus ethStatus = m_systemStatus.board()(board-1).eth;

  // layout eth status: 
  // 31......24  23.....16  15........8  7........0       
  // #RX[15..8]  #RX[7..0]  #Err[15..8]  #Err[7..0]  
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_PACKETSRECEIVED),0) != string::npos)
  {
    if(ethStatus.nof_frames != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.nof_frames = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_PACKETSERROR),0) != string::npos)
  {
    if(ethStatus.nof_errors != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.nof_errors = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_LASTERROR),0) != string::npos)
  {
    if(ethStatus.last_error != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.last_error = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_FFI0),0) != string::npos)
  {
    if(ethStatus.ffi0 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi0 = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_FFI1),0) != string::npos)
  {
    if(ethStatus.ffi1 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi1 = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_FFI2),0) != string::npos)
  {
    if(ethStatus.ffi2 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi2 = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
}
  
void ARATestDriverTask::updateAPstatus(string& propName,const GCFPValue* pvalue)
{
  LOG_INFO(formatString("updateAPstatus %s", propName.c_str()));
 
  int testI(0); 
  GCFPVUnsigned pvUnsigned;
  GCFPVDouble   pvDouble;
  pvUnsigned.copy(*pvalue);
  pvDouble.copy(*pvalue);
  
  LOG_INFO(formatString("updateAPstatus hier?? %d", ++testI));

  int rack;
  int subrack;
  int board;
  int ap;

  sscanf(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_APN,&rack,&subrack,&board,&ap);
  
  LOG_INFO(formatString("updateAPstatus hier?? %d (rack=%d,subrack=%d,board=%d,ap=%d)", ++testI,rack,subrack,board,ap));

  EPA_Protocol::FPGAStatus apStatus = m_systemStatus.board()(board-1).ap[ap-1];

  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_ALIVE),0) != string::npos)
  {
    if(apStatus.status != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).ap[ap-1].status = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_TEMPERATURE),0) != string::npos)
  {
    m_systemStatus.board()(board-1).ap[ap-1].temp = (uint8)(pvDouble.getValue()*100);
    updateSystemStatus();
  }
  if(propName.find(string(PROPNAME_VERSION),0) != string::npos)
  {
    // nothing to be done here
  }
}

void ARATestDriverTask::updateBPstatus(string& propName,const GCFPValue* pvalue)
{
  LOG_INFO(formatString("updateBPstatus %s", propName.c_str()));
  
  GCFPVUnsigned pvUnsigned;
  GCFPVDouble   pvDouble;
  pvUnsigned.copy(*pvalue);
  pvDouble.copy(*pvalue);
  
  int rack;
  int subrack;
  int board;

  sscanf(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_BP,&rack,&subrack,&board);
  
  EPA_Protocol::FPGAStatus bpStatus = m_systemStatus.board()(board-1).bp;

  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_ALIVE),0) != string::npos)
  {
    if(bpStatus.status != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).bp.status = pvUnsigned.getValue();
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_TEMPERATURE),0) != string::npos)
  {
    m_systemStatus.board()(board-1).bp.temp = (uint8)(pvDouble.getValue()*100);
    updateSystemStatus();
  }
  if(propName.find(string(PROPNAME_VERSION),0) != string::npos)
  {
    // nothing to be done here
  }
}

void ARATestDriverTask::updateRCUstatus(string& propName,const GCFPValue* pvalue)
{
  LOG_INFO(formatString("updateRCUstatus %s", propName.c_str()));
  
  GCFPVBool pvBool;
  pvBool.copy(*pvalue);
  
  int rack;
  int subrack;
  int board;
  int ap;
  int rcu;

  sscanf(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,&rack,&subrack,&board,&ap,&rcu);
  int rcuNumber = rcu + N_RCUS_PER_AP*(ap-1) + N_RCUS_PER_AP*N_APS_PER_BOARD*(board-1);
  
  uint8 rcuStatus;
  rcuStatus = m_systemStatus.rcu()(rcuNumber).status;
  rcuStatus = m_systemStatus.rcu()(rcuNumber-1).status;
  
  // layout rcu status: 
  // 7 6       5       4       3 2 1 0
  // - ringerr rcuHerr rcuVerr - - - statsReady
  if(propName.find(string(PROPNAME_RINGERR),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0x00000031; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<6);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_RCUHERR),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0x00000051; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<5);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
      updateSystemStatus();
    }
  }
  else if(propName.find(string(PROPNAME_RCUVERR),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0x00000061; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<4);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
      updateSystemStatus();
    }
  }
  if(propName.find(string(PROPNAME_STATSREADY),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0x00000070; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<0);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
      updateSystemStatus();
    }
  }
}

void ARATestDriverTask::updateSystemStatus()
{
  // send new status to RA application
  RSPUpdstatusEvent updStatusEvent;
  struct timeval timeValNow;
  time(&timeValNow.tv_sec);
  timeValNow.tv_usec=0;
  updStatusEvent.timestamp.set(timeValNow);
  updStatusEvent.status=0; // ignore ??
  updStatusEvent.handle=1; // ignore
  updStatusEvent.sysstatus = m_systemStatus;

  m_RSPserver.send(updStatusEvent);
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

      if(propName.find(string("_Maintenance_"),0) == string::npos)
      {
        if(propName.find(string("_ETH_"),0) != string::npos)
        {
          updateETHstatus(propName,pPropAnswer->pValue);
        }
        else if(propName.find(string("_BP_"),0) != string::npos)
        {
          updateBPstatus(propName,pPropAnswer->pValue);
        }
        else if(propName.find(string("_RCU"),0) != string::npos)
        {
          updateRCUstatus(propName,pPropAnswer->pValue);
        }
        else if(propName.find(string("_AP"),0) != string::npos)
        {
          updateAPstatus(propName,pPropAnswer->pValue);
        }
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("ARATestDriverTask(%s)::enabled, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

