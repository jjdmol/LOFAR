//# CommandHandlerEstimator.h: Controls execution of processing steps by a
//# solver process.
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

#ifndef LOFAR_BBSCONTROL_COMMANDHANDLERESTIMATOR_H
#define LOFAR_BBSCONTROL_COMMANDHANDLERESTIMATOR_H

// \file
// Controls execution of processing steps by a solver process.

#include <BBSControl/Command.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/SharedEstimator.h>
#include <BBSControl/ProcessGroup.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class CommandHandlerEstimator: public CommandVisitor
{
public:
    CommandHandlerEstimator(const ProcessGroup &group,
      const SharedEstimator::Ptr &estimator);

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

    bool                      itsHasFinished;
    ProcessGroup              itsProcessGroup;
    SharedEstimator::Ptr      itsEstimator;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
