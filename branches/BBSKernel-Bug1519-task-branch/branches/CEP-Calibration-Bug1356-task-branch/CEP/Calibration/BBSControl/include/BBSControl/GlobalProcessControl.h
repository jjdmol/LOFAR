//# GlobalProcessControl.h: Implementation of ACC/PLC ProcessControl class.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H

// \file
// Implementation of ACC/PLC ProcessControl class

//# Includes
#include <BBSControl/Strategy.h>
#include <BBSControl/CalSession.h>

#include <Common/lofar_smartptr.h>
#include <PLC/ProcessControl.h>
#include <ParmDB/Axis.h>
#include <MWCommon/VdsDesc.h>

namespace LOFAR
{
  namespace BBS
  {
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
      enum State {
        UNDEFINED = -1,
        NEXT_CHUNK,
        NEXT_CHUNK_WAIT,
        RUN,
        FINALIZE,
        FINALIZE_WAIT,
        QUIT,
        //# Insert new types HERE !!
        N_State
      };

      // Get the global time axis and verify consistency across all parts.
      Axis::ShPtr getGlobalTimeAxis() const;

      // Assign an index to each kernel process starting from 0. The index is
      // sorted on start frequency. Also, each solver process is assigned an
      // index (starting from 0, sorted on worker id).
      void createWorkerIndex();

      // Set run state to \a state
      void setState(State state);

      // Return the run state as a string.
      const string& showState() const;

      // State of the control process controller.
      State                     itsState;

      // The strategy that will be executed by this controller.
      Strategy                  itsStrategy;

      // Iterator used to iterate over the leaf-nodes (SingleSteps) of the
      // Strategy.
      StrategyIterator          itsStrategyIterator;

      // Calibration session information.
      scoped_ptr<CalSession>    itsCalSession;

      // Id of the command that the controller is waiting for.
      CommandId                 itsWaitId;

      CEP::VdsDesc              itsVdsDesc;
      Axis::ShPtr               itsGlobalTimeAxis;
      double                    itsFreqStart, itsFreqEnd;
      size_t                    itsTimeStart, itsTimeEnd;
      size_t                    itsChunkStart, itsChunkSize;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
