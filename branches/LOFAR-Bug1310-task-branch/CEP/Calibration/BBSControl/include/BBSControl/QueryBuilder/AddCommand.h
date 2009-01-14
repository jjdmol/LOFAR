//# AddCommand.h: Query builder for adding a command to the command queue.
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

#ifndef LOFAR_BBSCONTROL_QUERYBUILDER_ADDCOMMAND_H
#define LOFAR_BBSCONTROL_QUERYBUILDER_ADDCOMMAND_H

// \file 
// Query builder for adding a command to the command queue.

#include <BBSControl/CommandVisitor.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward declarations
    class Command;
    class Step;

    namespace QueryBuilder
    {
      // Concrete CommandVisitor class, implements visit() methods for
      // composing the query to add a concrete Command object to the
      // CommandQueue. Knowledge about the database structure of the command
      // queue and its stored procedures are kept within this class.
      class AddCommand : public CommandVisitor
      {
      public:
        // Destructor.
        virtual ~AddCommand();

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
        virtual void visit(const NoiseStep &command);
        // @}

        // Return the composed query.
        const string& getQuery() const;

      private:
        // Return the SELECT clause.
        // @{
        string selectClause(const Command& command) const;
        string selectClause(const Strategy&) const;
        // @}

        // Start the argument list
        // @{
        string beginArgumentList(const Command&) const;
        // @}

        // End the argument list
        // @{
        string endArgumentList(const Command&) const;
        // @}

        // Return the argument list.
        // @{
        string argumentList(const Command& command) const;
        string argumentList(const NextChunkCommand& command) const;
        string argumentList(const Step& command) const;
        string argumentList(const Strategy& command) const;
        // @}

        // String holding the composed query.
        string itsQuery;
      };

    } //# namespace QueryBuilder

  } //# namespace BBS

} //# namespace LOFAR

#endif
