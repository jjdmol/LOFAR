//#  AddCommand.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/QueryBuilder/AddCommand.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/SynchronizeCommand.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/RefitStep.h>
#include <BBSControl/NoiseStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    namespace QueryBuilder
    {

      //##--------   P u b l i c   m e t h o d s   --------##//

      AddCommand::~AddCommand()
      {
      }


      void AddCommand::visit(const InitializeCommand &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const FinalizeCommand &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const NextChunkCommand &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const RecoverCommand &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const SynchronizeCommand &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const Strategy &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const MultiStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        THROW (CommandQueueException, 
               command.type() << " cannot be added to the command queue.");
      }


      void AddCommand::visit(const PredictStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const SubtractStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const CorrectStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const SolveStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const ShiftStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const RefitStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const NoiseStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      const string& AddCommand::getQuery() const
      {
        return itsQuery;
      }


      //##--------   P r i v a t e   m e t h o d s   --------##//

      string AddCommand::selectClause(const Command&) const
      {
        return "SELECT * FROM blackboard.add_command";
      }


      string AddCommand::selectClause(const Strategy&) const
      {
        return "SELECT * FROM blackboard.set_strategy";
      }


      string AddCommand::beginArgumentList(const Command&) const
      {
        return "(";
      }


      string AddCommand::endArgumentList(const Command&) const
      {
        return ") AS result";
      }


      string AddCommand::argumentList(const Command& cmd) const
      {
        return "'" + toLower(cmd.type()) + "'";
      }


      string AddCommand::argumentList(const NextChunkCommand& cmd) const
      {
        ostringstream oss;
        ParameterSet  ps;
        oss << argumentList(static_cast<const Command&>(cmd))
            << ",''"
            << ",'" << (ps << cmd) << "'";
        return oss.str();
      }

      string AddCommand::argumentList(const Step& step) const
      {
        ostringstream oss;
        ParameterSet  ps;
        oss << argumentList(static_cast<const Command&>(step))
            << ",'" << step.name()                           << "'"
            << ",'" << (ps << step)                          << "'";
        return oss.str();
      }

      string AddCommand::argumentList(const Strategy& strategy) const
      {
        ostringstream oss;
        oss << "'"  << strategy.dataSet()                    << "'"
            << ",'" << strategy.parmDB().sky                 << "'"
            << ",'" << strategy.parmDB().instrument          << "'"
            << ",'" << strategy.parmDB().history             << "'"
            << ",'" << strategy.stations()                   << "'"
            << ",'" << strategy.inputColumn()                << "'"
            << ",'" << strategy.regionOfInterest().freq      << "'"
            << ",'" << strategy.regionOfInterest().time      << "'"
            << ",'" << strategy.chunkSize()                  << "'"
            << ",'" << strategy.correlation().selection      << "'"
            << ",'" << strategy.correlation().type           << "'";
        return oss.str();
      }


    } // namespace QueryBuilder

  } // namespace BBS

} // namespace LOFAR
