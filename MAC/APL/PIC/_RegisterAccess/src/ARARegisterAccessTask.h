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
       * @return true if ready to transition to the enabled
       * state.
       */
      bool isEnabled();
      
      /**
       * The initial state. This state is used to connect the client
       * and board ports. When they are both connected a transition
       * to the enabled state is made.
       */
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);
      
      /**
       * The enabled state. In this state the task can receive
       * commands.
       */
      GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &p);
    
    private:
      // action methods
      /**
       * Handle the write register event
       */
      GCFEvent::TResult handleWriteRegister(GCFEvent& e, GCFPortInterface& port);
      
      /**
       * Handle the read register event
       */
      GCFEvent::TResult handleReadRegister(GCFEvent& e, GCFPortInterface& port);

      /**
       * send the register value event
       */
      void sendRegisterValue(
        GCFPortInterface& port, 
        unsigned int board, 
        unsigned int BP,
        unsigned int AP,
        unsigned int ETH,
        unsigned int RCU, 
        unsigned long value);
      
    private:
      // internal types
      typedef struct
      {
        unsigned int          boardStatus;
        vector<unsigned int>  BPStatus;
        vector<unsigned int>  APStatus;
        vector<unsigned int>  ETHStatus;
        vector<unsigned int>  RCUStatus;
      } TRSPStatus;
      
      /**
       * query the property sets and create an RSPStatus struct
       */
      void readRegisterPropertySets(RegisterAccessTask::TRSPStatus& RSPStatus);
      
      /**
       * update the property sets with the current RSPStatus
       */
      void writeRegisterPropertySets(RegisterAccessTask::TRSPStatus RSPStatus);
      
    private:
      // member variables

      // ports
      GCFPort       m_testDriverServer;

  };

};
     
#endif /* ARAREGISTERACCESSTASK_H_ */
