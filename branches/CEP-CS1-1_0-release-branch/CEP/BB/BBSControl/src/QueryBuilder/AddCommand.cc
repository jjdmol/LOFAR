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
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSMultiStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSShiftStep.h>
#include <BBSControl/BBSRefitStep.h>
#include <BBSControl/Exceptions.h>
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


      void AddCommand::visit(const BBSStrategy &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSMultiStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        THROW (CommandQueueException, 
               command.type() << " cannot be added to the command queue.");
      }


      void AddCommand::visit(const BBSPredictStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSSubtractStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSCorrectStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSSolveStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSShiftStep &command)
      {
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, command.type());
        itsQuery = 
          selectClause(command) + 
          beginArgumentList(command)   +
          argumentList(command) +
          endArgumentList(command);
      }


      void AddCommand::visit(const BBSRefitStep &command)
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

      string AddCommand::selectClause(const Command& command) const
      {
        return 
          "SELECT * FROM blackboard.add_" + 
          toLower(command.type())         +
          "_command";
      }


      string AddCommand::selectClause(const BBSStrategy&) const
      {
        return "SELECT * FROM blackboard.set_strategy";
      }


      string AddCommand::beginArgumentList(const Command&) const
      {
        return "(";
      }


      string AddCommand::beginArgumentList(const BBSSingleStep&) const
      {
        return "(ROW(";
      }


      string AddCommand::endArgumentList(const Command&) const
      {
        return ") AS result";
      }


      string AddCommand::endArgumentList(const BBSSingleStep&) const
      {
        return ")) AS result";
      }


      string AddCommand::argumentList(const Command&) const
      {
        return "";
      }


      string AddCommand::argumentList(const BBSStrategy& strategy) const
      {
        ostringstream oss;
        oss << "'"  << strategy.dataSet()                    << "'"
            << ",'" << strategy.parmDB().localSky            << "'"
            << ",'" << strategy.parmDB().instrument          << "'"
            << ",'" << strategy.parmDB().history             << "'"
            << ",'" << strategy.stations()                   << "'"
            << ",'" << strategy.inputData()                  << "'"
            << ",'" << strategy.regionOfInterest().frequency << "'"
            << ",'" << strategy.regionOfInterest().time      << "'"
            << ",'" << strategy.domainSize().bandWidth       << "'"
            << ",'" << strategy.domainSize().timeInterval    << "'"
            << ",'" << strategy.correlation().selection      << "'"
            << ",'" << strategy.correlation().type           << "'";
        return oss.str();
      }


      string AddCommand::argumentList(const BBSSingleStep& step) const
      {
        ostringstream oss;
        oss << "'"  << step.getName()               << "'"
            << ",'" << step.operation()             << "'"
            << ",'" << step.baselines().station1    << "'"
            << ",'" << step.baselines().station2    << "'"
            << ",'" << step.correlation().selection << "'"
            << ",'" << step.correlation().type      << "'"
            << ",'" << step.sources()               << "'"
            << ",'" << step.instrumentModels()      << "'"
            << ",'" << step.outputData()            << "'";

        return oss.str();
      }


      string AddCommand::argumentList(const BBSSolveStep& step) const
      {
        ostringstream oss;
        oss << argumentList(static_cast<const BBSSingleStep&>(step)) 
            << ",'" << step.maxIter()                  << "'"
            << ",'" << step.epsilon()                  << "'"
            << ",'" << step.minConverged()             << "'"
            << ",'" << step.parms()                    << "'"
            << ",'" << step.exclParms()                << "'"
            << ",'" << step.domainSize().bandWidth     << "'"
            << ",'" << step.domainSize().timeInterval  << "'";
        return oss.str();
      }


    } // namespace QueryBuilder

  } // namespace BBS

} // namespace LOFAR
