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
#include "RSP_Protocol.ph"

#include "../src/ARAConstants.h"
#include "../src/ARAPropertyDefines.h"
#include "APLCommon/APL_Defines.h"
#include "ARATestDriverTask.h"

#include <stdio.h>
#include <blitz/array.h>
#include <time.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/ParameterSet.h>

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR
{

namespace ARA
{

string ARATestDriverTask::m_taskName("ARATestDriver");

ARATestDriverTask::ARATestDriverTask() :
  GCFTask((State)&ARATestDriverTask::initial, m_taskName),
  m_answer(),
  m_RSPserver(),
  m_propMap(),
  m_systemStatus(),
  m_stats(),
  m_substatusPeriod(0.0),
  m_substatsPeriod(0.0),
  m_updStatusTimerId(0),
  m_updStatsTimerId(0),
  m_updStatsHandleSP(0),
  m_updStatsHandleSM(0),
  m_updStatsHandleBP(0),
  m_updStatsHandleBM(0),
  n_racks(1),
  n_subracks_per_rack(1),
  n_boards_per_subrack(1),
  n_aps_per_board(1),
  n_rcus_per_ap(1),
  n_rcus(1)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  m_answer.setTask(this);

  ParameterSet::instance()->adoptFile("RegisterAccess.conf");

  n_racks               = ParameterSet::instance()->getInt(PARAM_N_RACKS);
  n_subracks_per_rack   = ParameterSet::instance()->getInt(PARAM_N_SUBRACKS_PER_RACK);
  n_boards_per_subrack  = ParameterSet::instance()->getInt(PARAM_N_BOARDS_PER_SUBRACK);
  n_aps_per_board       = ParameterSet::instance()->getInt(PARAM_N_APS_PER_BOARD);
  n_rcus_per_ap         = ParameterSet::instance()->getInt(PARAM_N_RCUS_PER_AP);
  n_rcus                = n_rcus_per_ap*
                              n_aps_per_board*
                              n_boards_per_subrack*
                              n_subracks_per_rack*
                              n_racks;
  
  m_systemStatus.board().resize(n_boards_per_subrack);
  m_systemStatus.rcu().resize(n_rcus);

	EPA_Protocol::BoardStatus boardStatus;
	memset(&boardStatus,0,sizeof(boardStatus));
  m_systemStatus.board()(blitz::Range::all()) = boardStatus;

	EPA_Protocol::RCUStatus rcuStatus;
	memset(&rcuStatus,0,sizeof(rcuStatus));
  m_systemStatus.rcu()(blitz::Range::all()) = rcuStatus;
  
  m_stats().resize(1,n_rcus,RSP_Protocol::MAX_N_BLPS);
  
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
    boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scope.c_str(),TYPE_LCU_PIC,&m_answer));
    propSetPtr->load();
		unsigned int i=0;
		while(PROPS_Station[i].propName!=0)    
    {
      propSetPtr->subscribeProp(PROPS_Station[i].propName);
      i++;
    }
    m_propMap[scope]=propSetPtr;
  }
  else if(scope == string(SCOPE_PIC_RackN))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Rack,&m_answer));
      propSetPtr->load();
			unsigned int i=0;
			while(PROPS_Rack[i].propName!=0)    
      {
        propSetPtr->subscribeProp(PROPS_Rack[i].propName);
        i++;
      }
      m_propMap[scopeString]=propSetPtr;
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_SubRack,&m_answer));
        propSetPtr->load();
				unsigned int i=0;
				while(PROPS_SubRack[i].propName!=0)    
        {
          propSetPtr->subscribeProp(PROPS_SubRack[i].propName);
          i++;
        }
        m_propMap[scopeString]=propSetPtr;
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Board,&m_answer));
          propSetPtr->load();
					unsigned int i=0;
					while(PROPS_Board[i].propName!=0)    
          {
            propSetPtr->subscribeProp(PROPS_Board[i].propName);
            i++;
          }
          m_propMap[scopeString]=propSetPtr;
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_ETH))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Ethernet,&m_answer));
          propSetPtr->load();
					unsigned int i=0;
					while(PROPS_Ethernet[i].propName!=0)    
          {
            propSetPtr->subscribeProp(PROPS_Ethernet[i].propName);
            i++;
          }
          m_propMap[scopeString]=propSetPtr;
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_BP))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_FPGA,&m_answer));
          propSetPtr->load();
					unsigned int i=0;
					while(PROPS_FPGA[i].propName!=0)    
          {
            propSetPtr->subscribeProp(PROPS_FPGA[i].propName);
            i++;
          }
          m_propMap[scopeString]=propSetPtr;
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          for(ap=1;ap<=n_aps_per_board;ap++)
          {
            for(rcu=1;rcu<=n_rcus_per_ap;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_RCU,&m_answer));
              propSetPtr->load();
							unsigned int i=0;
							while(PROPS_RCU[i].propName!=0)    
              {
                propSetPtr->subscribeProp(PROPS_RCU[i].propName);
                i++;
              }
              m_propMap[scopeString]=propSetPtr;
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_ADCStatistics))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          for(ap=1;ap<=n_aps_per_board;ap++)
          {
            for(rcu=1;rcu<=n_rcus_per_ap;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_ADCStatistics,&m_answer));
              propSetPtr->load();
							unsigned int i=0;
							while(PROPS_ADCStatistics[i].propName!=0)    
              {
                propSetPtr->subscribeProp(PROPS_ADCStatistics[i].propName);
                i++;
              }
              m_propMap[scopeString]=propSetPtr;
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_Maintenance))
  {
    boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scope.c_str(),TYPE_LCU_PIC_Maintenance,&m_answer));
    propSetPtr->load();
		unsigned int i=0;
		while(PROPS_Maintenance[i].propName!=0)    
    {
      propSetPtr->subscribeProp(PROPS_Maintenance[i].propName);
      i++;
    }
    m_propMap[scope]=propSetPtr;
  }
  else if(scope == string(SCOPE_PIC_RackN_Maintenance))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Maintenance,&m_answer));
      propSetPtr->load();
			unsigned int i=0;
			while(PROPS_Maintenance[i].propName!=0)    
      {
        propSetPtr->subscribeProp(PROPS_Maintenance[i].propName);
        i++;
      }
      m_propMap[scopeString]=propSetPtr;
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_Maintenance))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Maintenance,&m_answer));
        propSetPtr->load();
				unsigned int i=0;
				while(PROPS_Maintenance[i].propName!=0)    
        {
          propSetPtr->subscribeProp(PROPS_Maintenance[i].propName);
          i++;
        }
        m_propMap[scopeString]=propSetPtr;
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Maintenance,&m_answer));
          propSetPtr->load();
					unsigned int i=0;
					while(PROPS_Maintenance[i].propName!=0)    
          {
            propSetPtr->subscribeProp(PROPS_Maintenance[i].propName);
            i++;
          }
          m_propMap[scopeString]=propSetPtr;
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          for(ap=1;ap<=n_aps_per_board;ap++)
          {
            for(rcu=1;rcu<=n_rcus_per_ap;rcu++)
            {
              sprintf(scopeString,scope.c_str(),rack,subrack,board,ap,rcu);
              boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Maintenance,&m_answer));
              propSetPtr->load();
							unsigned int i=0;
							while(PROPS_Maintenance[i].propName!=0)    
              {
                propSetPtr->subscribeProp(PROPS_Maintenance[i].propName);
                i++;
              }
              m_propMap[scopeString]=propSetPtr;
            }
          }
        }
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_Alert))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      sprintf(scopeString,scope.c_str(),rack);
      boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Alert,&m_answer));
      propSetPtr->load();
			unsigned int i=0;
			while(PROPS_Alert[i].propName!=0)    
      {
        propSetPtr->subscribeProp(PROPS_Alert[i].propName);
        i++;
      }
      m_propMap[scopeString]=propSetPtr;
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_Alert))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        sprintf(scopeString,scope.c_str(),rack,subrack);
        boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Alert,&m_answer));
        propSetPtr->load();
				unsigned int i=0;
				while(PROPS_Alert[i].propName!=0)    
        {
          propSetPtr->subscribeProp(PROPS_Alert[i].propName);
          i++;
        }
        m_propMap[scopeString]=propSetPtr;
      }
    }
  }
  else if(scope == string(SCOPE_PIC_RackN_SubRackN_BoardN_Alert))
  {
    for(rack=1;rack<=n_racks;rack++)
    {
      for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
      {
        for(board=1;board<=n_boards_per_subrack;board++)
        {
          sprintf(scopeString,scope.c_str(),rack,subrack,board);
          boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_Alert,&m_answer));
          propSetPtr->load();
					unsigned int i=0;
					while(PROPS_Alert[i].propName!=0)    
          {
            propSetPtr->subscribeProp(PROPS_Alert[i].propName);
            i++;
          }
          m_propMap[scopeString]=propSetPtr;
        }
      }
    }
  }
}

void ARATestDriverTask::updateETHstatus(string& propName,const GCFPValue* pvalue)
{
  LOG_INFO(formatString("updateETHstatus %s", propName.c_str()));
  
  GCFPVUnsigned pvUnsigned;
  pvUnsigned.copy(*pvalue);

  vector<int> hardwareIndexes;
  getHardwareIndexes(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_ETH,hardwareIndexes);
  
//  int rack		=	hardwareIndexes[0];
//  int subrack	=	hardwareIndexes[1];
  int board		= hardwareIndexes[2];
  
  EPA_Protocol::ETHStatus ethStatus = m_systemStatus.board()(board-1).eth;

  // layout eth status: 
  // 31......24  23.....16  15........8  7........0       
  // #RX[15..8]  #RX[7..0]  #Err[15..8]  #Err[7..0]  
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_FRAMESRECEIVED),0) != string::npos)
  {
    if(ethStatus.nof_frames != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.nof_frames = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_FRAMESERROR),0) != string::npos)
  {
    if(ethStatus.nof_errors != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.nof_errors = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_LASTERROR),0) != string::npos)
  {
    if(ethStatus.last_error != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.last_error = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_FFI0),0) != string::npos)
  {
    if(ethStatus.ffi0 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi0 = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_FFI1),0) != string::npos)
  {
    if(ethStatus.ffi1 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi1 = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_FFI2),0) != string::npos)
  {
    if(ethStatus.ffi2 != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).eth.ffi2 = pvUnsigned.getValue();
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

  vector<int> hardwareIndexes;
  getHardwareIndexes(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_APN,hardwareIndexes);
  
  int rack		=	hardwareIndexes[0];
  int subrack	=	hardwareIndexes[1];
  int board		= hardwareIndexes[2];
  int ap      = hardwareIndexes[3];
  
  LOG_INFO(formatString("updateAPstatus hier?? %d (rack=%d,subrack=%d,board=%d,ap=%d)", ++testI,rack,subrack,board,ap));

  EPA_Protocol::FPGAStatus fpgaStatus = m_systemStatus.board()(board-1).fpga;

  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_ALIVE),0) != string::npos)
  {
    if(ap==1 && fpgaStatus.ap1_status != pvUnsigned.getValue())
      m_systemStatus.board()(board-1).fpga.ap1_status = pvUnsigned.getValue();
    else if(ap==2 && fpgaStatus.ap2_status != pvUnsigned.getValue())
      m_systemStatus.board()(board-1).fpga.ap2_status = pvUnsigned.getValue();
    else if(ap==3 && fpgaStatus.ap3_status != pvUnsigned.getValue())
      m_systemStatus.board()(board-1).fpga.ap3_status = pvUnsigned.getValue();
    else if(ap==4 && fpgaStatus.ap4_status != pvUnsigned.getValue())
      m_systemStatus.board()(board-1).fpga.ap4_status = pvUnsigned.getValue();
  }
  else if(propName.find(string(PROPNAME_TEMPERATURE),0) != string::npos)
  {
    if(ap==1)
      m_systemStatus.board()(board-1).fpga.ap1_temp = (uint8)(pvDouble.getValue()*100);
    else if(ap==2)
      m_systemStatus.board()(board-1).fpga.ap2_temp = (uint8)(pvDouble.getValue()*100);
    else if(ap==3)
      m_systemStatus.board()(board-1).fpga.ap3_temp = (uint8)(pvDouble.getValue()*100);
    else
        m_systemStatus.board()(board-1).fpga.ap4_temp = (uint8)(pvDouble.getValue()*100);
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
  
  vector<int> hardwareIndexes;
  getHardwareIndexes(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_BP,hardwareIndexes);
  
//  int rack		=	hardwareIndexes[0];
//  int subrack	=	hardwareIndexes[1];
  int board		= hardwareIndexes[2];

  EPA_Protocol::FPGAStatus fpgaStatus = m_systemStatus.board()(board-1).fpga;

  // layout fpga status: 
  // 15..9 8       7........0       
  // ----- alive   temperature
  if(propName.find(string(PROPNAME_STATUS),0) != string::npos)
  {
    // nothing to be done here
  }
  else if(propName.find(string(PROPNAME_ALIVE),0) != string::npos)
  {
    if(fpgaStatus.bp_status != pvUnsigned.getValue())
    {
      m_systemStatus.board()(board-1).fpga.bp_status = pvUnsigned.getValue();
    }
  }
  else if(propName.find(string(PROPNAME_TEMPERATURE),0) != string::npos)
  {
    m_systemStatus.board()(board-1).fpga.bp_temp = (uint8)(pvDouble.getValue()*100);
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
  GCFPVUnsigned pvUnsigned;
  pvBool.copy(*pvalue);
  pvUnsigned.copy(*pvalue);
  
  vector<int> hardwareIndexes;
  getHardwareIndexes(propName.c_str(),SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,hardwareIndexes);
  
//  int rack		=	hardwareIndexes[0];
//  int subrack	=	hardwareIndexes[1];
  int board		= hardwareIndexes[2];
  int ap			= hardwareIndexes[3];
  int rcu			= hardwareIndexes[4];
  int rcuNumber = rcu + n_rcus_per_ap*(ap-1) + n_rcus_per_ap*n_aps_per_board*(board-1);
  
  uint8 rcuStatus;
  
  rcuStatus = m_systemStatus.rcu()(rcuNumber-1).status;
  
  // layout rcu status (for both statusX and statusY): 
  // 7         6         5         4        3        2       1          0
  // VDDVCCEN VHENABLE VLENABLE FILSELB FILSELA BANDSEL HBAENABLE LBAENABLE
  if(propName.find(string(PROPNAME_VDDVCCEN),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0x7f; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<7);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_VHENABLE),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xbf; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<6);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_VLENABLE),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xdf; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<5);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_FILSELB),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xef; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<4);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_FILSELA),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xf7; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<3);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_BANDSEL),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xfb; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<2);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_HBAENABLE),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xd; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<1);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
  else if(propName.find(string(PROPNAME_LBAENABLE),0) != string::npos)
  {
    uint8 tempStatus = rcuStatus & 0xfe; // reset bits
    tempStatus = tempStatus | ((uint8 )(pvBool.getValue())<<0);
    if(rcuStatus != tempStatus)
    {
      m_systemStatus.rcu()(rcuNumber-1).status = tempStatus;
    }
  }
}

void ARATestDriverTask::updateSystemStatus()
{
  // send new status to RA application
  RSPUpdstatusEvent updStatusEvent;
  updStatusEvent.timestamp.setNow();
  updStatusEvent.status=SUCCESS;
  updStatusEvent.handle=1; // ignore
  updStatusEvent.sysstatus.board().reference(m_systemStatus.board().copy());
  updStatusEvent.sysstatus.rcu().reference(m_systemStatus.rcu().copy());

  m_RSPserver.send(updStatusEvent);
}

void ARATestDriverTask::updateStats()
{
  // send new stats to RA application
  RSPUpdstatsEvent updStatsEvent;
  updStatsEvent.timestamp.setNow();
  updStatsEvent.status=SUCCESS;
  
  int i=0;
  int j;
  int k;
  for(j=0;j<n_rcus;j++)
  {
    for(k=0;k<RSP_Protocol::MAX_N_BLPS;k++)
    {
		  double noise=(double)(rand()%1000)/500.0;
		  if(k==10)
      	m_stats()(i,j,k) = 4000+noise*500.0;
			else		  
      	m_stats()(i,j,k) = noise;
    }
  }
  
  updStatsEvent.stats().reference(m_stats().copy());
  
  if(m_updStatsHandleSP != 0)
  {
    updStatsEvent.handle=m_updStatsHandleSP; 
    m_RSPserver.send(updStatsEvent);
  }
  if(m_updStatsHandleSM != 0)
  {
    updStatsEvent.handle=m_updStatsHandleSM; 
    m_RSPserver.send(updStatsEvent);
  }
  if(m_updStatsHandleBP != 0)
  {
    updStatsEvent.handle=m_updStatsHandleBP; 
    m_RSPserver.send(updStatsEvent);
  }
  if(m_updStatsHandleBM != 0)
  {
    updStatsEvent.handle=m_updStatsHandleBM;
    m_RSPserver.send(updStatsEvent);
  }
}

bool ARATestDriverTask::isEnabled()
{
  return (m_RSPserver.isConnected());
}

GCFEvent::TResult ARATestDriverTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("ARATestDriverTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
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
      LOG_DEBUG(formatString("ARATestDriverTask(%s)::initial, default",getName().c_str()));
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
  LOG_DEBUG(formatString("ARATestDriverTask(%s)::enabled (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      break;
    }

    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
      
      if(timerEvent.id == (unsigned long)m_updStatusTimerId)
      {
        // send updstatus message;
        updateSystemStatus();
        
        m_updStatusTimerId = port.setTimer(m_substatusPeriod);
      }
      else if(timerEvent.id == (unsigned long)m_updStatsTimerId)
      {
        // send updstats message;
        updateStats();
        
        m_updStatsTimerId = port.setTimer(m_substatsPeriod);
      }
      break;
    }
    
    case RSP_SUBSTATUS:
    {
      LOG_INFO("RSP_SUBSTATUS received");
      RSPSubstatusEvent substatus(event);
      
      m_substatusPeriod = (double)substatus.period;
      m_updStatusTimerId = port.setTimer(m_substatusPeriod);

      RSPSubstatusackEvent ack;
      ack.timestamp.setNow();
      ack.status = SUCCESS;
      ack.handle = (int)&ack;
      port.send(ack);
      break;
    }
    
    case RSP_UNSUBSTATUS:
    {
      LOG_INFO("RSP_UNSUBSTATUS received");
      RSPUnsubstatusEvent unsubstatus(event);
      
      m_substatusPeriod = 0.0;
      m_updStatusTimerId = 0;

      RSPUnsubstatusackEvent ack;
      ack.timestamp.setNow();
      ack.status = SUCCESS;
      ack.handle = (int)&ack;
      port.send(ack);
      break;
    }
    
    case RSP_SUBSTATS:
    {
      LOG_INFO("RSP_SUBSTATS received");
      RSPSubstatsEvent substats(event);
      
      RSPSubstatsackEvent ack;
      ack.timestamp.setNow();
      ack.status = SUCCESS;
      if(substats.type == RSP_Protocol::Statistics::SUBBAND_POWER)
      {
        m_updStatsHandleSP = 11;
        ack.handle = m_updStatsHandleSP;
      }
      else if(substats.type == RSP_Protocol::Statistics::BEAMLET_POWER)
      {
        m_updStatsHandleBP = 21;
        ack.handle = m_updStatsHandleBP;
      }
      else
      {
        ack.status = FAILURE;
        ack.handle = 0;
      }      
      m_substatsPeriod = (double)substats.period;
      m_updStatsTimerId = port.setTimer(m_substatsPeriod);

      port.send(ack);
      break;
    }
    
    case RSP_UNSUBSTATS:
    {
      LOG_INFO("RSP_UNSUBSTATS received");
      RSPUnsubstatsEvent unsubstats(event);
      
      m_substatsPeriod = 0.0;
      m_updStatsTimerId = 0;

      RSPUnsubstatsackEvent ack;
      ack.timestamp.setNow();
      ack.status = SUCCESS;
      ack.handle = (int)&ack;
      port.send(ack);
      break;
    }
    
    case RSP_GETVERSION:
    {
      LOG_INFO("RSP_GETVERSION received");
      RSPGetversionEvent getversion(event);
      
      RSP_Protocol::Versions versions;
      versions.rsp().resize(n_racks*n_subracks_per_rack*n_boards_per_subrack);
      versions.bp().resize(n_racks*n_subracks_per_rack*n_boards_per_subrack);
      versions.ap().resize(n_racks*n_subracks_per_rack*n_boards_per_subrack*n_aps_per_board);
      for(int board=0;board<n_racks*n_subracks_per_rack*n_boards_per_subrack;board++)
      {
        versions.rsp()(board) = (board+1);
        LOG_INFO(formatString("board[%d].version = 0x%x",board,versions.rsp()(board)));
        versions.bp()(board) = (board+1)*8;
        LOG_INFO(formatString("bp[%d].version = 0x%x",board,versions.bp()(board)));
        for(int ap=0;ap<EPA_Protocol::N_AP; ap++)
        {
          versions.ap()(board * EPA_Protocol::N_AP + ap) = ((board+1)<<4)+ap;
          LOG_INFO(formatString("ap[%d][%d].version = 0x%x",board,ap,versions.ap()(board * EPA_Protocol::N_AP + ap)));
        }
      }
      
      RSPGetversionackEvent ack;
      ack.timestamp.setNow();
      ack.status = SUCCESS;
      ack.versions.rsp().reference(versions.rsp().copy());
      ack.versions.bp().reference(versions.bp().copy());
      ack.versions.ap().reference(versions.ap().copy());
      
      port.send(ack);
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned pvUnsigned;
      GCFPVDouble   pvDouble;
      GCFPVBool     pvBool;
      GCFPVString   pvString;
      switch(pPropAnswer->pValue->getType())
      {
        case LPT_BOOL:
          pvBool.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%d", pPropAnswer->pPropName, pvBool.getValue()));
          break;
        case LPT_UNSIGNED:
          pvUnsigned.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%d", pPropAnswer->pPropName, pvUnsigned.getValue()));
          break;
        case LPT_DOUBLE:
          pvDouble.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%f", pPropAnswer->pPropName, pvDouble.getValue()));
          break;
        case LPT_STRING:
          pvString.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%s", pPropAnswer->pPropName, pvString.getValue().c_str()));
          break;
        case NO_LPT:
        case LPT_CHAR:
        case LPT_INTEGER:
        case LPT_BIT32:
        case LPT_BLOB:
        case LPT_REF:
        case LPT_DATETIME:
        case LPT_DYNARR:
        case LPT_DYNBOOL:
        case LPT_DYNCHAR:
        case LPT_DYNUNSIGNED:
        case LPT_DYNINTEGER:
        case LPT_DYNBIT32:
        case LPT_DYNBLOB:
        case LPT_DYNREF:
        case LPT_DYNDOUBLE:
        case LPT_DYNDATETIME:
        case LPT_DYNSTRING:
        default:
          break;
      }
      // for now, we are only interested in changes of the BP status, AP status,
      // ETH status or RCU status;
      string propName(pPropAnswer->pPropName);

      if(propName.find(string("_Maintenance"),0) == string::npos)
      {
        if(propName.find(string("_ETH"),0) != string::npos)
        {
          updateETHstatus(propName,pPropAnswer->pValue);
        }
        else if(propName.find(string("_BP"),0) != string::npos)
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
      LOG_DEBUG(formatString("ARATestDriverTask(%s)::enabled, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void ARATestDriverTask::getHardwareIndexes(const string& propName,const string& scope, vector<int>& hardwareIndexes)
{
	hardwareIndexes.clear();
	const int numIndexes=10;
	int indexes[numIndexes];
	int system=0;

	if(0 == propName.find(string("System")))
	{
		string scanScope = string("System%d:")+scope;
	  sscanf(propName.c_str(),scanScope.c_str(),&system,&indexes[0],&indexes[1],&indexes[2],&indexes[3],&indexes[4],&indexes[5],&indexes[6],&indexes[7],&indexes[8],&indexes[9]);
	}	
	else
	{
	  sscanf(propName.c_str(),scope.c_str(),&indexes[0],&indexes[1],&indexes[2],&indexes[3],&indexes[4],&indexes[5],&indexes[6],&indexes[7],&indexes[8],&indexes[9]);
	}
	for(int i=0;i<numIndexes;i++)
	{
		hardwareIndexes.push_back(indexes[i]);
	}
}

} // namespace ARA


} // namespace LOFAR

