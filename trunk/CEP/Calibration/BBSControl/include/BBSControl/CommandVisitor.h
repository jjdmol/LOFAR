//# CommandVisitor.h: Abstract visitor class for the Command class
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

#ifndef LOFAR_BBSCONTROL_COMMANDVISITOR_H
#define LOFAR_BBSCONTROL_COMMANDVISITOR_H

// \file 
// Abstract visitor class for the Command class

#include <BBSControl/CommandResult.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward declarations
    class InitializeCommand;
    class FinalizeCommand;
    class NextChunkCommand;
    class RecoverCommand;
    class SynchronizeCommand;
    class MultiStep;
    class PredictStep;
    class SubtractStep;
    class AddStep;
    class CorrectStep;
    class SolveStep;
    class ShiftStep;
    class RefitStep;
    class NoiseStep;

    // \addtogroup BBSControl
    // @{

    // Abstract Visitor class, declares visit() operations for each concrete
    // Command class. It helps to retrieve lost type information when handling
    // Command or Step references. Concrete visitor classes must implement
    // a visit() method for each concrete Command class.
    //
    // \see Gamma e.a., <em>Design Patterns: Elements of Reusable
    // Object-Oriented Software</em>, Addison-Wesley, 1995
    // \see <a href="http://hillside.net/patterns/papers/type-laundering.html">
    // John Vlissides, <em>Type Laundering</em>, C++ Report, February 1997</a>
    class CommandVisitor
    {
    public:
      virtual ~CommandVisitor() = 0;

      virtual CommandResult visit(const InitializeCommand &command) = 0;
      virtual CommandResult visit(const FinalizeCommand &command) = 0;
      virtual CommandResult visit(const NextChunkCommand &command) = 0;
      virtual CommandResult visit(const RecoverCommand &command) = 0;
      virtual CommandResult visit(const SynchronizeCommand &command) = 0;
      virtual CommandResult visit(const MultiStep &command) = 0;
      virtual CommandResult visit(const PredictStep &command) = 0;
      virtual CommandResult visit(const SubtractStep &command) = 0;
      virtual CommandResult visit(const AddStep &command) = 0;
      virtual CommandResult visit(const CorrectStep &command) = 0;
      virtual CommandResult visit(const SolveStep &command) = 0;
      virtual CommandResult visit(const ShiftStep &command) = 0;
      virtual CommandResult visit(const RefitStep &command) = 0;
      virtual CommandResult visit(const NoiseStep &command) = 0;
    };

    //# Pure virtual destructor has to be defined...
    inline CommandVisitor::~CommandVisitor()
    {
    }

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
