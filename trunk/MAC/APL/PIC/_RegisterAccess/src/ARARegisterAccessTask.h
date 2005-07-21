//#  ARARegisterAccessTask.h: class definition for the Beam Server task.
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

#ifndef ARAREGISTERACCESSTASK_H_
#define ARAREGISTERACCESSTASK_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <Common/LofarTypes.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <blitz/array.h>

#include "ARAAnswer.h"
#include "ARAPhysicalModel.h"


namespace LOFAR
{

namespace ARA
{
  class RegisterAccessTask : public GCF::TM::GCFTask
  {
    public:
      /**
       * The constructor of the RegisterAccessTask task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      RegisterAccessTask(string name);
      virtual ~RegisterAccessTask();
  
      // state methods
      
      /**
       * @return true if ready to transition to the connected
       * state.
       */
      bool isConnected();
      
      /**
       * The initial state. This state is used to connect the client
       * and board ports. When they are both connected a transition
       * to the enabled state is made.
       */
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);
      
      /**
       * The myPropSetsLoaded state. In this state the propertysets are loaded and the task 
       * waits for a client to connect
       */
      GCF::TM::GCFEvent::TResult myPropSetsLoaded(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);

      /**
       * The myAPCsLoaded state. In this state the propertysets are loaded and the task 
       * waits for a client to connect
       */
      GCF::TM::GCFEvent::TResult APCsLoaded(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);

      /**
       * The connected state. In this state the task can receive
       * commands.
       */
      GCF::TM::GCFEvent::TResult connected(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);
      /**
       * The subscribing states. In each state a SubStats message is sent
       */
      GCF::TM::GCFEvent::TResult subscribingStatsSubbandPower(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);
      GCF::TM::GCFEvent::TResult subscribingStatsBeamletPower(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);
      /**
       * The operational state. In this state the task can receives
       * status and statistics updates from the rsp driver
       */
      GCF::TM::GCFEvent::TResult operational(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface &p);
    
    private:
      // action methods
      /**
       * Handle the update status event
       */
      GCF::TM::GCFEvent::TResult handleUpdStatus(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& port);
      /**
       * Handle the update stats event
       */
      GCF::TM::GCFEvent::TResult handleUpdStats(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& port);

      /**
       * Handle a change of the Maintenance status field
       */
      void handleMaintenance(string propName, const GCF::Common::GCFPValue& value);
      
      /**
       * Handle a change of one of the RCU settings fields
       */
      void handleRCUSettings(string propName, const int bitnr, const GCF::Common::GCFPValue& value);
      
    private:
      // internal types
      // gcf 3.1: needs specific apc's for every resource
      // in gcf4.0, only the top level apc has to be loaded, and links to other
      // apc's are in the apc's themselves
      typedef map<string,boost::shared_ptr<GCF::PAL::GCFMyPropertySet> > TMyPropertySetMap;
      
      /**
       * create propertyset object, add it to the map
       */
      void addMyPropertySet(
          const char* scope,
          const char* type, 
          GCF::Common::TPSCategory category, 
          const GCF::Common::TPropertyConfig propconfig[],
          GCF::PAL::GCFMyPropertySet::TDefaultUse defaultUse = GCF::PAL::GCFMyPropertySet::USE_MY_DEFAULTS);

      void updateBoardProperties(string scope,
                                 uint8  voltage_15,
                                 uint8  voltage_33,
                                 uint8  ffi0,
                                 uint8  ffi1,
                                 double timestamp);
      /**
       * update eth properties based on status bits
       */
      void updateETHproperties(string scope,uint32 frames,
                                            uint32 errors,
                                            uint8  lastError,
                                            uint8  ffi0,
                                            uint8  ffi1,
                                            uint8  ffi2,
                                            double timestamp);
      /**
       * update MEP status properties 
       */
      void updateMEPStatusProperties(string scope,uint32 seqnr,
                                                  uint8  error,
                                                  uint8  ffi0,
                                                  double timestamp);
      /**
       * update SYNC status properties 
       */
      void updateSYNCStatusProperties(string scope,uint32 sample_count,
                                                   uint32 sync_count,
                                                   uint32 error_count,
                                                   double timestamp);
      /**
       * update fpga board properties based on status bits
       */
      void updateFPGAboardProperties(string scope,double timestamp);
      /**
       * update fpga properties based on status bits
       */
      void updateFPGAproperties(string scope, uint8 status, 
                                              uint8 temp,
                                              double timestamp);
      /**
       * update rcu board properties
       */
      void updateBoardRCUproperties(string scope,uint8  ffi0,
                                                 uint8  ffi1,
                                                 double timestamp);
      /**
       * update rcu properties based on status bits
       */
      void updateBoardRCUproperties(string scope,uint8 status,
                                                 uint32 nof_overflow,
                                                 double timestamp);
      void updateRCUproperties(string scope,uint8 status,double timestamp);
      /**
       * update version string
       */
      void updateVersion(string scope, string version, double timestamp);
      
    private:
      typedef blitz::Array<double, 2> TStatistics;
      
      void getBoardRelativeNumbers(int boardNr,int& rackNr,int& subRackNr,int& relativeBoardNr);
      void getRCURelativeNumbers(int rcuNr,int& rackRelativeNr,int& subRackRelativeNr,int& boardRelativeNr,int& apRelativeNr,int& rcuRelativeNr);    
      int getRCUHardwareNr(const string& property);
      TMyPropertySetMap::iterator getPropertySetFromScope(const string& property);
      void _addStatistics(TStatistics& statistics, uint32 statsHandle);
      void _integrateStatistics();
      void _writeStatistics(TStatistics& statistics, uint32 statsHandle);
      
      // member variables
      ARAAnswer   m_answer;
      
      TMyPropertySetMap m_myPropertySetMap;
      
      bool              m_myPropsLoaded;
      unsigned long     m_myPropsLoadCounter;

      // ports
      static string     m_RSPserverName;
      GCF::TM::GCFPort  m_RSPclient;
      ARAPhysicalModel  m_physicalModel;
    
      map<string,int>   m_propertySet2RCUMap;
      
      // subscriptions
      uint32            m_subStatusHandle;
      uint32            m_subStatsHandleSubbandPower;
      uint32            m_subStatsHandleBeamletPower;

      int               m_n_racks;
      int               m_n_subracks_per_rack;
      int               m_n_boards_per_subrack;
      int               m_n_aps_per_board;
      int               m_n_rcus_per_ap;
      int               m_n_rcus;
      int               m_status_update_interval;
      int               m_stats_update_interval;
      bool              m_centralized_stats;
      
      int32             m_integrationTime;
      int32             m_integrationMethod;
      TStatistics       m_integratingStatisticsSubband;
      int32             m_numStatisticsSubband;
      TStatistics       m_integratingStatisticsBeamlet;
      int32             m_numStatisticsBeamlet;
      unsigned long     m_integrationTimerID;
  };
};

} // namespace LOFAR

     
#endif /* ARAREGISTERACCESSTASK_H_ */
