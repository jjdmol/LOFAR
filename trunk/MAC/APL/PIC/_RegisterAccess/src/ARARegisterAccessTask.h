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

#include <GCF/GCF_Control.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>
#include <Common/LofarTypes.h>
#include <boost/shared_ptr.hpp>
#include <map>

#include "ARAAnswer.h"
#include "ARAPhysicalModel.h"

namespace ARA
{
  class RegisterAccessTask : public GCFTask
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
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);
      
      /**
       * The myPropSetsLoaded state. In this state the propertysets are loaded and the task 
       * waits for a client to connect
       */
      GCFEvent::TResult myPropSetsLoaded(GCFEvent& e, GCFPortInterface &p);

      /**
       * The myAPCsLoaded state. In this state the propertysets are loaded and the task 
       * waits for a client to connect
       */
      GCFEvent::TResult APCsLoaded(GCFEvent& e, GCFPortInterface &p);

      /**
       * The connected state. In this state the task can receive
       * commands.
       */
      GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface &p);
      /**
       * The subscribing states. In each state a SubStats message is sent
       */
      GCFEvent::TResult subscribingStatsSubbandPower(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult subscribingStatsSubbandMean(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult subscribingStatsBeamletPower(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult subscribingStatsBeamletMean(GCFEvent& e, GCFPortInterface &p);
      /**
       * The operational state. In this state the task can receives
       * status and statistics updates from the rsp driver
       */
      GCFEvent::TResult operational(GCFEvent& e, GCFPortInterface &p);
    
    private:
      // action methods
      /**
       * Handle the update status event
       */
      GCFEvent::TResult handleUpdStatus(GCFEvent& e, GCFPortInterface& port);
      /**
       * Handle the update stats event
       */
      GCFEvent::TResult handleUpdStats(GCFEvent& e, GCFPortInterface& port);

      /**
       * Handle a change of the Maintenance status field
       */
      void handleMaintenance(string propName, const GCFPValue& value);
      
    private:
      // internal types
      // gcf 3.1: needs specific apc's for every resource
      // in gcf4.0, only the top level apc has to be loaded, and links to other
      // apc's are in the apc's themselves
      typedef map<string,boost::shared_ptr<GCFMyPropertySet> > TMyPropertySetMap;
      typedef map<string,boost::shared_ptr<GCFApc> > TAPCMap;
      
      /**
       * create propertyset object, add it to the map
       */
      void addMyPropertySet(const TPropertySet& propset,const char* scope);

      /**
       * create apc object, add it to the map
       */
      void addAPC(string apc,string scope);

      void updateBoardProperties(string scope,
                                 uint8  voltage_15,
                                 uint8  voltage_22,
                                 uint8  ffi);
      /**
       * update eth properties based on status bits
       */
      void updateETHproperties(string scope,uint32 frames,
                                            uint32 errors,
                                            uint8  lastError,
                                            uint8  ffi0,
                                            uint8  ffi1,
                                            uint8  ffi2);
      /**
       * update fpga properties based on status bits
       */
      void updateFPGAproperties(string scope,uint8 status, uint8 temp);
      /**
       * update rcu properties based on status bits
       */
      void updateRCUproperties(string scope,uint8 status);
      /**
       * update version string
       */
      void updateVersion(string scope, string version);
      
    private:
    
      // member variables
      ARAAnswer   m_answer;
      
      TMyPropertySetMap m_myPropertySetMap;
      TAPCMap           m_APCMap;
      
      bool              m_myPropsLoaded;
      unsigned long     m_myPropsLoadCounter;
      bool              m_APCsLoaded;
      unsigned long     m_APCsLoadCounter;

      // ports
      static string     m_RSPserverName;
      GCFPort           m_RSPclient;
      ARAPhysicalModel  m_physicalModel;
      
      // subscriptions
      uint32            m_subStatusHandle;
      uint32            m_subStatsHandleSubbandPower;
      uint32            m_subStatsHandleSubbandMean;
      uint32            m_subStatsHandleBeamletPower;
      uint32            m_subStatsHandleBeamletMean;

      int               m_n_racks;
      int               m_n_subracks_per_rack;
      int               m_n_boards_per_subrack;
      int               m_n_aps_per_board;
      int               m_n_rcus_per_ap;
      int               m_n_rcus;
      int               m_status_update_interval;
      int               m_stats_update_interval;

  };

};
     
#endif /* ARAREGISTERACCESSTASK_H_ */
