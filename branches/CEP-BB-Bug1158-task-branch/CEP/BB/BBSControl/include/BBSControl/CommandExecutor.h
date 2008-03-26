//# CommandExecutor.h: Concrete viistor class for concrete Command classes
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSCONTROL_COMMANDEXECUTOR_H
#define LOFAR_BBSCONTROL_COMMANDEXECUTOR_H

// \file
// Concrete viistor class for concrete Command classes

#include <BBSControl/CommandVisitor.h>
#include <BBSControl/CommandResult.h>
#include <BBSKernel/Prediffer.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
  //# Forward declations
  class BlobStreamable;

  namespace BBS
  {
    //# Forward declations
    class CommandQueue;
    class Prediffer;
    class BlobStreamableConnection;

    // \addtogroup BBSControl
    // @{

    class CommandExecutor: public CommandVisitor
    {
    public:
      CommandExecutor(shared_ptr<CommandQueue> &queue,
                      shared_ptr<BlobStreamableConnection> &solver)
        :   itsCommandQueue(queue),
            itsSolverConnection(solver)
      {
      }

      virtual ~CommandExecutor();

      // @name Implementation of visit() for different commands.
      // @{
      virtual void visit(const InitializeCommand &command);
      virtual void visit(const FinalizeCommand &command);
      virtual void visit(const NextChunkCommand &command);
      virtual void visit(const RecoverCommand &command);
      virtual void visit(const SynchronizeCommand &command);
      virtual void visit(const Strategy &command);
      virtual void visit(const MultiStep &command);
      virtual void visit(const PredictStep &command);
      virtual void visit(const SubtractStep &command);
      virtual void visit(const CorrectStep &command);
      virtual void visit(const SolveStep &command);
      virtual void visit(const ShiftStep &command);
      virtual void visit(const RefitStep &command);
      // @}

      // Get result of the last executed command.
      const CommandResult &getResult() const
      { return itsResult; }


    private:
      // Kernel.
      scoped_ptr<Prediffer>                   itsKernel;

      // CommandQueue.
      shared_ptr<CommandQueue>                itsCommandQueue;

      // Connection to the solver.
      shared_ptr<BlobStreamableConnection>    itsSolverConnection;

      // Result of the last executed command.
      CommandResult                           itsResult;
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
