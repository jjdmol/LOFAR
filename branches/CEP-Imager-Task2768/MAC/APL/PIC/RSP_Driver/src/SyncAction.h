//#  -*- mode: c++ -*-
//#
//#  SyncAction.h: RSP Driver syncaction class
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

#ifndef SYNCACTION_H_
#define SYNCACTION_H_

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  using GCF::TM::GCFFsm;
  using GCF::TM::GCFPortInterface;
  namespace RSP {

    class SyncAction : public GCFFsm
    {
      // declare Scheduler a friend so it can access doContinue()
      friend class Scheduler;

    public:
      /**
       * Constructors for a SyncAction object.
       * Default direction is read.
       */
      SyncAction(GCFPortInterface& board_port, int board_id, int n_indices);
	  
      /* Destructor for SyncAction. */
      virtual ~SyncAction();

      /*@{*/
      /**
       * The states of the statemachine.
       */
      GCFEvent::TResult idle_state(GCFEvent& event, GCFPortInterface& port);
      GCFEvent::TResult sendrequest_state(GCFEvent& event, GCFPortInterface& port);
      GCFEvent::TResult waitack_state(GCFEvent& event, GCFPortInterface& port);
      /*@}*/

      /*@{*/
      /**
       * Hooks to perform specific actions.
       */
      virtual void sendrequest() = 0;
      virtual void sendrequest_status()   = 0;
      virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port) = 0;
      /*@}*/

      /**
       * Get the board id for this sync action.
       */
      int getBoardId();

      /**
       * Get a reference to the port for SyncAction.
       */
      GCFPortInterface& getBoardPort();

      /*@{*/
      /**
       * Has the state machine reached its final state?
       */
      void setCompleted(bool completed);
      bool hasCompleted() const;
	  void setFinished() { m_completed = true; m_current_index = m_n_indices; }
      /*@}*/

      /**
       * Get index of current local index
       */
      int getCurrentIndex() const { return m_current_index; }
      int getNumIndices() const { return m_n_indices; }

      /**
       * Reset the statemachine to the initial state.
       */
      void reset();

      /*
       * doAtInit
       *
       * When passed true, this action is marked as an
       * initialization action and it is executed during
       * the initialisation phase. If set to false this action
       * is skipped during the initialisation phase.
       */
      void doAtInit(bool atinit = true) { m_atinit = atinit; }

    protected:
      /*@{*/
      /**
       * setContinue(true) is called when no event
       * has been sent and we need to continue to the next index.
       */
      void setContinue(bool cont);
      bool doContinue() const;
      /*@}*/

   private:
      SyncAction();
 
    private:
      GCFPortInterface& m_board_port;
      int               m_board_id;
      bool              m_completed; /** indicates whether the state machine has reached its final state */
      bool              m_continue; /** no event sent, continue to index */
      int               m_n_indices;
      int               m_current_index;
      int               m_retries;
      bool              m_atinit;
    };
  };
};
     
#endif /* SYNCACTION_H_ */
