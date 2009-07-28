//#  KernelProcessControl.h:  Local (kernel) process controller
//#
//#  Copyright (C) 2002-2007
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H

// \file
// Local (kernel) process controller

//# Includes
#include <BBSControl/CalSession.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/Types.h>

#include <BBSKernel/Measurement.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/VisSelection.h>
#include <BBSKernel/VisData.h>

#include <PLC/ProcessControl.h>

#include <ParmDB/SourceDB.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
namespace BBS
{
// \ingroup BBSControl
// @{

// Implementation of the ProcessControl and the CommandVisitor interface for the
// local Kernel controller.
class KernelProcessControl: public ACC::PLC::ProcessControl,
                            public CommandVisitor
{
public:
    // Constructor
    KernelProcessControl();

    // Destructor
    virtual ~KernelProcessControl();

    // @name Implementation of PLC interface.
    // @{
    virtual tribool define();
    virtual tribool init();
    virtual tribool run();
    virtual tribool pause(const string& condition);
    virtual tribool release();
    virtual tribool quit();
    virtual tribool snapshot(const string& destination);
    virtual tribool recover(const string& source);
    virtual tribool reinit(const string& configID);
    virtual string  askInfo(const string& keylist);
    // @}

    // @name Implementation of CommandVisitor interface.
    // @{
    virtual CommandResult visit(const InitializeCommand &command);
    virtual CommandResult visit(const FinalizeCommand &command);
    virtual CommandResult visit(const NextChunkCommand &command);
    virtual CommandResult visit(const RecoverCommand &command);
    virtual CommandResult visit(const SynchronizeCommand &command);
    virtual CommandResult visit(const MultiStep &command);
    virtual CommandResult visit(const PredictStep &command);
    virtual CommandResult visit(const SubtractStep &command);
    virtual CommandResult visit(const AddStep &command);
    virtual CommandResult visit(const CorrectStep &command);
    virtual CommandResult visit(const SolveStep &command);
    virtual CommandResult visit(const ShiftStep &command);
    virtual CommandResult visit(const RefitStep &command);
    // @}

private:
    enum State
    {
      UNDEFINED,
      WAIT,
      RUN,
      //# Insert new types here!
      N_State
    };

    CommandResult unsupported(const Command &command) const;

    // Set run state to \a state
    void setState(State state);

    // Return the current state as a string.
    const string& showState() const;

    bool parseProductSelection(vector<string> &result, const Step &command)
        const;

    bool parseBaselineSelection(vector<baseline_t> &result,
        const Step &command) const;

    State                                   itsState;

    // Calibration session information.
    scoped_ptr<CalSession>                  itsCalSession;

    // 0-based index of this kernel process.
    KernelIndex                             itsKernelIndex;

    // Measurement.
    Measurement::Ptr                    itsMeasurement;
    string                                  itsInputColumn;

    // Global time axis.
    Axis::ShPtr                             itsGlobalTimeAxis;

    // Chunk.
    Box                                     itsDomain;
    VisSelection                            itsChunkSelection;
    VisData::Ptr                        itsChunk;

    // Model
    Model::Ptr                          itsModel;

    // Source Database
    shared_ptr<SourceDB>                    itsSourceDb;

    // Connection to the global solver.
    shared_ptr<BlobStreamableConnection>    itsSolver;
};

//@}

} // namespace BBS
} // namespace LOFAR

#endif
