//# CommandHandlerReducer.h: Controls execution of processing steps on (a part
//# of) the visibility data.
//#
//# Copyright (C) 2012
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

#ifndef LOFAR_BBSCONTROL_COMMANDHANDLERREDUCER_H
#define LOFAR_BBSCONTROL_COMMANDHANDLERREDUCER_H

// \file
// Controls execution of processing steps on (a part of) the visibility data.

#include <BBSControl/Command.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/ProcessGroup.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Evaluator.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmDBLog.h>
#include <ParmDB/SourceDB.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <casa/OS/Path.h>

namespace LOFAR
{
namespace BBS
{
class SingleStep;

// \addtogroup BBSControl
// @{

class CommandHandlerReducer: public CommandVisitor
{
public:
    CommandHandlerReducer(const ProcessGroup &group,
      const Measurement::Ptr &measurement, const ParmDB &parmDB,
      const SourceDB &sourceDB, const casa::Path &logPath);

    // Returns true if a FinalizeCommand has been received.
    bool hasFinished() const;

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
    CommandResult unsupported(const Command &command) const;
    CommandResult simulate(const SingleStep &command, Evaluator::Mode mode);

    // Find the combined frequency axis of the calibration group the process
    // belongs to, given the specified partition of processes into groups.
    Axis::ShPtr getCalGroupFreqAxis(const vector<uint32> &groups) const;

    // Create a CorrelationMask from the correlation selection specified in the
    // parset.
    CorrelationMask makeCorrelationMask(const vector<string> &selection) const;

    // Create buffers and load precomputed visbilities on demand.
    void loadPrecomputedVis(const vector<string> &patches);

    ProcessGroup                          itsProcessGroup;
    Measurement::Ptr                      itsMeasurement;
    ParmDB                                itsParmDB;
    SourceDB                              itsSourceDB;
    casa::Path                            itsLogPath;

    bool                                  itsHasFinished;
    int                                   itsChunkCount;
    string                                itsInputColumn;
    VisSelection                          itsChunkSelection;
    BufferMap                             itsBuffers;
    shared_ptr<BlobStreamableConnection>  itsEstimatorConnection;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
