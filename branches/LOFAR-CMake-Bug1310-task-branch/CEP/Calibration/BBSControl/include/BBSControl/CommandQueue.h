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

#ifndef LOFAR_BBSCONTROL_COMMANDQUEUE_H
#define LOFAR_BBSCONTROL_COMMANDQUEUE_H

// \file
// Command queue of the blackboard system.

//# Includes
#if defined(HAVE_PQXX)
# include <pqxx/connection>
# include <pqxx/result>
# include <pqxx/transactor>
# include <pqxx/trigger>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

//# The following #includes are not really necessary to compile the code,
//# but it avoids the need to #include them whenever any of the public
//# typedefs \c NextCommandType, \c ResultType, or \a ResultMapType are used.
#include <BBSControl/Command.h>
#include <BBSControl/CommandResult.h>
#include <BBSControl/SenderId.h>
#include <BBSControl/Types.h>

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  //# Forward Declarations
  class ParameterSet;

  namespace BBS
  {
    //# Forward Declarations
    class Step;
    class Strategy;

    // \addtogroup BBSControl
    // @{

    // @name Typedefs
    // @{

    // Return type of the function CommandQueue::getNextCommand(). It pairs a
    // (managed) pointer to a Command with its ID.
    typedef pair<shared_ptr<const Command>, CommandId> NextCommandType;

    // Return type of the function CommandQueue::getNewResults(const
    // CommandId&). It pairs a command result with the local controller that
    // executed that command.
    typedef pair<SenderId, CommandResult> ResultType;
    
    // Return type of the function CommandQueue::getNewResults(). It binds a
    // command-id and the results received from the local controllers.
    typedef map<const CommandId, vector<ResultType> > ResultMapType;

    // @}


    // Command queue of the blackboard system.
    class CommandQueue
    {
    public:
      // Trigger class handles notification of triggers received from the
      // database backend by raising the flag associated with the trigger.
      class Trigger : public pqxx::trigger
      {
      public:
        // Valid trigger types.
        typedef enum {
          Command = 1L << 0,
          Result  = 1L << 1
        } Type;

        // Construct a trigger handler for trigger of type \a type.
        Trigger(const CommandQueue& queue, Type type);

        // Destructor. Reimplemented only because of no-throw specification in
        // the base class.
        virtual ~Trigger() throw() {}

        // Handle the notification, by raising the flag associated with the
        // received trigger.
        virtual void operator()(int be_pid);

        // Test if any of the \a flags are raised.
        static Type testFlags(Type flags) 
        { return Type(theirFlags & flags); }

        // Raise \a flags.
        static void raiseFlags(Type flags) 
        { theirFlags = Type(theirFlags | flags); }

        // Clear \a flags.
        static void clearFlags(Type flags)
        { theirFlags = Type(theirFlags & ~flags); }

        // Test if any of the \a flags are raised and clear them.
        static Type testAndClearFlags(Type flags) {
          Type tp = testFlags(flags);
          clearFlags(flags);
          return tp;
        }

      private:
        // Map associating trigger types with their string representation.
        // @{
        typedef map<Type, string> TypeMap;
        static TypeMap theirTypes;
        // @}

        // Keep track of flags that were raised as a result of a handled
        // notification. Note that this is a \e shared variable, because we're
        // not really interested in who responds to a raised flag; it's
        // important that it is handled, but not by whom.
        static Type theirFlags;

        // Initializer struct. The default constructor contains code to
        // initialize the static data members theirTypes and theirFlags.
        struct Init {
          Init();
        };

        // Static instance of Init, triggers the initialization of static data
        // members during its construction.
        static Init theirInit;
      }; //# class Trigger


      // Construct the command queue. The command queue is stored in a part of
      // the blackboard DBMS, identified by the name \a dbname.  The arguments
      // \a user, \a host, and \a port are optional, but have "sensible"
      // defaults.
      CommandQueue(const string& dbname,
		   const string& user="postgres",
		   const string& host="dop50.nfra.nl",
		   const string& port="5432");

      //!!-- temporary "hack", I don't know if I'll need this method --!!//
      bool allDone() const;

      //!!-- temporary "hack", I don't know if I'll need this method --!!//
      bool setStrategyDone() const;

      // Add a Command to the command queue. Once in the command queue,
      // the command represents a "unit of work", a.k.a. \e workorder. This
      // method is typically used by the global controller.
      // \return The unique command-id associated with \a cmd.
      CommandId addCommand(const Command& cmd) const;

      // Forward this call to setStrategy(), in order to avoid the risk that
      // the return value of the stored procedure being called is ignored.
      void addCommand(const Strategy& cmd) const
      { setStrategy(cmd); }

      // Get the next Command from the command queue. When this command is
      // retrieved from the database, its status will be set to "active"
      // ([TBD]). This method is typically used by the local controller.
      //
      // \return A pair consisting of a pointer to the next command and the ID
      // of this command. When there are no more commands left in the queue a
      // null pointer will be returned and the ID is set to -1.
      //
      // \attention Currently, a Command object is reconstructed using one \e
      // or \e more queries. Although each query is executed as a transaction,
      // multiple queries are \e not executed as one transaction. So beware!
      //
      // \todo Wrap multiple queries (needed for, e.g., reconstructing a
      // SolveStep) in one transaction.
      NextCommandType getNextCommand() const;

      // Set the Strategy in the command queue. All information, \e except
      // the Step objects within the Strategy are stored in the
      // database. This "meta data" is needed to (re)start a BBS run. This
      // method is typically used by the global controller.
      void setStrategy(const Strategy&) const;

      // Retrieve the Strategy for this BBS run. The information in the
      // database consists of the "meta data" of a Strategy object
      // (i.e. all information \e except the Step objects). This method is
      // typically called by the local controller.
      shared_ptr<const Strategy> getStrategy() const;

      // Add the result \a result for the command (identified by) \a commandId
      // to the blackboard result table. \a commandId must be the ID of the
      // first command in the queue for which no result has been set yet.
      // \throw CommandQueueException when insertion failed (e.g., a wrong \a
      // commandId was specified).
      void addResult(const CommandId& commandId, 
                     const CommandResult& result,
                     const SenderId& senderId) const;

      // Get all new results from the database.
      ResultMapType getNewResults() const;

      // Get new results from the database for the given command-id.
      vector<ResultType> getNewResults(const CommandId& id) const; 

      // Check to see if we're starting a new run. The local controller needs
      // to do a few extra checks; these checks will be done when \a
      // isGlobalCtrl is \c false.
      bool isNewRun(bool isGlobalCtrl) const;

      // Register for the trigger of type \a type. Once registered, we will
      // receive notification from the database when a new result for a
      // command of type \a type is available.
      bool registerTrigger(Trigger::Type type);

      // De-register for the result trigger of \a command. We will no longer
      // receive notifications when a new result for \a command is available.
      bool deregisterTrigger(Trigger::Type type);

      // Wait for a trigger from the database backend. Only triggers that were
      // previously registered will get through. When a trigger arrives the
      // associated trigger flag will be raised. This way, we decoupled
      // reception of the trigger from any action to be taken.
      bool waitForTrigger(Trigger::Type type, 
                          double timeOut = theirDefaultTimeOut) const;

    private:
      // Functor class for executing a query as a transaction.
      class ExecQuery : public pqxx::transactor<>
      {
      public:
        // Constructor for insert/update-like queries. These queries do not
        // return a result.
        explicit ExecQuery(const string& query);

        // Constructor for select-like queries. The result is returned as a
        // string.
        ExecQuery(const string& query, ParameterSet& result);

        // This method will be invoked by the perform() method of your
        // pqxx::connection class to execute the query stored in itsQuery. The
        // result, if any, will be stored in itsPQResult.
        void operator()(argument_type& transaction);

        // This method will be invoked by the perform() method of your
        // pqxx::connection class, when the transaction succeeded. The result
        // of the query, stored in itsPQResult, will be converted to a
        // ParameterSet and assigned to itsResult. Each key is uniquely
        // defined as "_row(<row-number>).<column-name>", i.e. "_row(0)." for
        // the first row, "_row(1)." for the second, etc. The key "_nrows"
        // will contain the number of rows in the result.
        void on_commit();

      private:
        // Empty ParameterSet, used to initialize itsResult properly, when the
        // one-argument constructor is used.
        static ParameterSet emptyResult;

        // String containing the query to be executed.
        const string itsQuery;

        // Reference to the ParameterSet that will hold the query result.
        ParameterSet& itsResult;

        // The result of the executed query must be stored internally, because
        // it will be written in operator() and will be read in on_commit().
        pqxx::result itsPQResult;
      }; //# class ExecQuery


      // Execute \a query. The result will be returned as a ParameterSet. The
      // optional argument \a doAlwaysPrefix indicates whether keys should
      // always be prefixed with \c _row(<row-nr>) or not. By default, when
      // only one row is returned, the prefix \c _row(0). is dropped.
      ParameterSet execQuery(const string& query,
                                       bool doAlwaysPrefix = false) const;

      // Connection to the PostgreSQL database. The pqxx::connection object
      // will be destroyed when \c *this goes out of scope.
      //
      // \remarks A pqxx::connection object can only be "configured" during
      // construction. However, since we want to be able to defer
      // configuration of the pqxx::connection until after \c *this has been
      // constructed, we need to wrap it into a managed pointer class.
      scoped_ptr<pqxx::connection> itsConnection;

      // Triggers that have been registered with the command queue.
      // \note Triggers cannot be copied, so we must use a shared pointer.
      map<Trigger::Type, shared_ptr<Trigger> > itsTriggers;

      // Default time-out value
      static double theirDefaultTimeOut;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
