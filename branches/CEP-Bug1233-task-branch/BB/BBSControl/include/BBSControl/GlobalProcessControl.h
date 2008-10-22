//# GlobalProcessControl.h: Implementation of ACC/PLC ProcessControl class.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H

// \file
// Implementation of ACC/PLC ProcessControl class

//# Includes
#include <BBSControl/CommandResult.h>
#include <BBSControl/CommandQueue.h>

#include <ParmDB/Axis.h>
#include <BBSKernel/MetaMeasurement.h>

#include <PLC/ProcessControl.h>

#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations.
    class Strategy;
    class Step;
    class CommandQueue;

    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl interface for the global BBS
    // controller.
    class GlobalProcessControl : public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      GlobalProcessControl();

      // Destructor.
      virtual ~GlobalProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool release();
      virtual tribool quit();
      virtual tribool pause(const string& condition);
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

    private:
        enum RunState {
          UNDEFINED = -1,
          NEXT_CHUNK,
          NEXT_CHUNK_WAIT,
          RUN,
          WAIT,
          RECOVER,
          FINALIZE,
          FINALIZE_WAIT,
          QUIT,
          //# Insert new types HERE !!
          N_States
        };

      // Set run state to \a state
      void setState(RunState state);

      // Return the current state as a string.
      const string& showState() const;

#if 0
      // Post the command \a cmd to the command queue and wait until all local
      // controllers have executed the command.
      bool execCommand(const Command& cmd);

      // Number of local controllers.
      uint itsNrLocalCtrls;
#endif

      // Wait for results for the command with id \a cmdId. If new results
      // arrive within the time-out period (which is a modifiable property of
      // the CommandQueue), then \c itsResults will be updated. These new
      // results will also be returned as a vector of ResultType.
      vector<ResultType> waitForResults(const CommandId& cmdId);

      // Wait for results for any command. If new results arrive within the
      // time-out period (which is a modifiable property of the CommandQueue),
      // then \c itsResults will be updated. These new results will also be
      // returned as ResultMapType.
      ResultMapType waitForResults();

      // State of the global process controller.
      RunState itsState;

      // Keep track of the status of the commands sent to the command queue.
      // Once a local controller has executed a command, it will post a
      // result, which we can use to update our administration.
      ResultMapType itsResults;

      // The strategy that will be executed by this controller.
      scoped_ptr<Strategy> itsStrategy;

      // Vector containing all the separate steps, in sequential order, that
      // the strategy consists of.
      vector< shared_ptr<const Step> >  itsSteps;

      // Iterator for keeping track where we left while traversing the vector
      // \c itsSteps. We need this iterator, because the run() method will be
      // invoked several times by ACCmain. In each call to run() we must
      // execute one Step.
      vector< shared_ptr<const Step> >::const_iterator itsStepsIterator;

      // CommandQueue where strategies and steps can be "posted".
      scoped_ptr<CommandQueue> itsCommandQueue;
      
      MetaMeasurement       itsMetaMeasurement;
      double                itsFreqStart, itsFreqEnd;
      size_t                itsTimeStart, itsTimeEnd;
      size_t                itsChunkStart, itsChunkSize;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
