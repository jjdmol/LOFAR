//#  CommandQueue.h: Command queue of the blackboard system.
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
//#  $Id$

#ifndef LOFAR_BBS_COMMANDQUEUE_H
#define LOFAR_BBS_COMMANDQUEUE_H

// \file
// Command queue of the blackboard system.

//# Includes
#if defined(HAVE_PQXX)
# include <pqxx/connection>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  //# Forward Declarations
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    //# Forward Declarations
    class BBSStep;
    class BBSStrategy;

    // \addtogroup BBSControl
    // @{
    // Command queue of the blackboard system.
    class CommandQueue
    {
    public:
      // Construct the command queue. The command queue is stored in a part of
      // the blackboard DBMS, identified by the name \a dbname.  The arguments
      // \a user, \a host, and \a port are optional, but have "sensible"
      // defaults.
      CommandQueue(const string& dbname,
		   const string& user="postgres",
		   const string& host="dop50.nfra.nl",
		   const string& port="5432");

      // Add a BBSStep to the command queue. Once in the command queue,
      // this step represents a "unit of work", a.k.a. \e workorder. This
      // method is typically used by the global controller.
      // \pre \a step must a BBSSingleStep.
      // \return The unique command-id associated with \a step.
      int addStep(const BBSStep& step) const;

      // Get the next BBSStep from the command queue. When this step is
      // retrieved from the database, its status will be set to "active"
      // ([TBD]). This method is typically used by the local controller.
      //
      // \attention Currently, a BBSStep object is reconstructed using one \e
      // or \e more queries. Although each query is executed as a transaction,
      // multiple queries are \e not executed as one transaction. So beware!
      //
      // \todo Wrap multiple queries (needed for, e.g., reconstructing a
      // BBSSolveStep) in one transaction.
      const BBSStep* getNextStep();

      // Set the BBSStrategy in the command queue. All information, \e except
      // the BBSStep objects within the BBSStrategy are stored in the
      // database. This "meta data" is needed to (re)start a BBS run. This
      // method is typically used by the global controller.
      void setStrategy(const BBSStrategy&) const;

      // Retrieve the BBSStrategy for this BBS run. The information in the
      // database consists of the "meta data" of a BBSStrategy object
      // (i.e. all information \e except the BBSStep objects). This method is
      // typically called by the local controller.
      const BBSStrategy* getStrategy() const;

      // Check to see if we're starting a new run. The local controller needs
      // to do a few extra checks; these checks will be done when \a
      // isGlobalCtrl is \c false.
      bool isNewRun(bool isGlobalCtrl) const;

    private:
      // CommandQueueTrigger needs to have access to itsConnection, so we make
      // it a friend.
      friend class CommandQueueTrigger;

      // Execute \a query. The result will be returned as a ParameterSet.
      ACC::APS::ParameterSet execQuery(const string& query) const;

      // Connection to the PostgreSQL database. The pqxx::connection object
      // will be destroyed when \c *this goes out of scope.
      //
      // \remarks A pqxx::connection object can only be "configured" during
      // construction. However, since we want to be able to defer
      // configuration of the pqxx::connection until after \c *this has been
      // constructed, we need to wrap it into a managed pointer class.
      scoped_ptr<pqxx::connection> itsConnection;

      // Keep track of the ID of the current command.
      uint itsCurrentId;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
