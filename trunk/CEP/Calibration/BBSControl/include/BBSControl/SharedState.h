//# SharedState.h: Control state that is shared between the controller and all
//# workers.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSCONTROL_SHAREDSTATE_H
#define LOFAR_BBSCONTROL_SHAREDSTATE_H

// \file
// Control state that is shared between the controller and all workers.

#if defined(HAVE_PQXX)
# include <pqxx/connection>
# include <pqxx/trigger>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

//# The following #includes are not really necessary to compile the code,
//# but it avoids the need to #include them whenever any of the public
//# typedefs \c NextCommandType, \c ResultType, or \a ResultMapType are used.
#include <BBSControl/Command.h>
#include <BBSControl/CommandResult.h>
#include <BBSControl/Types.h>

//#include <Common/lofar_functional.h>
#include <functional>
#include <BBSControl/Exceptions.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>

#include <ParmDB/Grid.h>


namespace LOFAR
{

namespace CEP
{
    class VdsDesc;
    class VdsPartDesc;
}

namespace BBS
{

// \addtogroup BBSControl
// @{

struct ProcessId
{
    ProcessId()
        :   pid(-1)
    {
    }
    
    ProcessId(const string &hostname, int64 pid)
        :   hostname(hostname),
            pid(pid)
    {
    }

    // ProcessIds are sorted on pid first as this is a much faster comparison.
    bool operator<(const ProcessId &rhs) const
    {
        return pid < rhs.pid || (pid == rhs.pid && hostname < rhs.hostname);
    }

    bool operator==(const ProcessId &rhs) const
    {
        return pid == rhs.pid && hostname == rhs.hostname;
    }

    string  hostname;
    int64   pid;
};


struct CommandStatus
{
    size_t  nResults;
    size_t  nFail;
};

class SharedState
{
public:
    enum RunState
    {
        FAILED = -1,
        WAITING_FOR_CONTROL,
        WAITING_FOR_WORKERS,
        COMPUTING_WORKER_INDEX,
        PROCESSING,
        DONE,
        N_RunState
    };
    
    enum WorkerType
    {
        KERNEL,
        SOLVER,
        N_WorkerType
    };

    struct WorkerDescriptor
    {
        ProcessId   id;
        WorkerType  type;
        int32       index;
        int32       port;
        string      filesystem;
        string      path;
        Grid        grid;
    };

    SharedState(const string &key, const string &db, const string &user,
        const string &host = "localhost", const string &port="5432");
    ~SharedState();

    bool registerAsControl();
    bool registerAsKernel(const string &filesystem, const string &path,
        const Grid &grid);
    bool registerAsSolver(size_t port);

    bool setRunState(RunState state);
    RunState getRunState() const;
    
    bool setIndex(const ProcessId &target, size_t index);

    void initRegister(const CEP::VdsDesc &vds, bool useSolver);

    // Add a Command to the command queue. Once in the command queue,
    // the command represents a "unit of work", a.k.a. \e workorder. This
    // method is typically used by the global controller.
    // \return The unique command-id associated with \a cmd.
    CommandId addCommand(const Command &cmd, WorkerType target = N_WorkerType) const;

    // Add the result \a result for the command (identified by) \a commandId
    // to the blackboard result table. \a commandId must be the ID of the
    // first command in the queue for which no result has been set yet.
    // \throw CommandQueueException when insertion failed (e.g., a wrong \a
    // commandId was specified).
    bool addResult(const CommandId &cmdId, const CommandResult &result) const;

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
    pair<CommandId, shared_ptr<Command> > getCommand() const;

    CommandStatus getCommandStatus(const CommandId &cmdId) const;
    
    vector<pair<ProcessId, CommandResult> > getResults(const CommandId &cmdId)
        const;

    bool slotsAvailable() const;

    bool waitForCommand(double timeOut = theirDefaultTimeOut) const;
    bool waitForResult(double timeOut = theirDefaultTimeOut) const;

    vector<WorkerDescriptor> getWorkersByType(WorkerType type) const;
    
    WorkerDescriptor getWorkerByIndex(WorkerType type, size_t index) const;
    
    WorkerDescriptor getWorkerById(const ProcessId &id) const;

    size_t getIndex() const;
    ProcessId getProcessId() const
    { return itsProcessId; }

    bool isRegistered(const ProcessId &id) const;
    bool isKernel(const ProcessId &id) const;
    bool isSolver(const ProcessId &id) const;

    size_t getWorkerCount() const;
    size_t getWorkerCount(WorkerType type) const;

    Axis::ShPtr getGlobalTimeAxis() const;

private:
    // TODO: Should this be private?
    void syncRegister(bool force = false) const;
    
    // Trigger class handles notification of triggers received from the
    // database backend by raising the flag associated with the trigger.
    class Trigger : public pqxx::trigger
    {
    public:
        // Valid trigger types.
        enum Type
        {
            Command = 1L << 0,
            Result  = 1L << 1,
            RegisterModified = 1L << 2
        };

        // Construct a trigger handler for trigger of type \a type.
        Trigger(const SharedState& state, Type type);

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
        static Type testAndClearFlags(Type flags)
        {
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
        struct Init
        {
            Init();
        };

        // Static instance of Init, triggers the initialization of static data
        // members during its construction.
        static Init theirInit;
    }; //# class Trigger

    // Register for the trigger of type \a type. Once registered, we will
    // receive notification from the database when a new result for a
    // command of type \a type is available.
    bool registerTrigger(Trigger::Type type) const;

    // De-register for the result trigger of \a command. We will no longer
    // receive notifications when a new result for \a command is available.
    bool deregisterTrigger(Trigger::Type type) const;

    bool isRegistered(Trigger::Type type) const;
    
    // Wait for a trigger from the database backend. Only triggers that were
    // previously registered will get through. When a trigger arrives the
    // associated trigger flag will be raised. This way, we decoupled
    // reception of the trigger from any action to be taken.
    bool waitForTrigger(Trigger::Type type,
        double timeOut = theirDefaultTimeOut) const;


    int32                                       itsStateId;
    ProcessId                                   itsProcessId;
    scoped_ptr<pqxx::connection>                itsConnection;

    // WorkerRegister cache.
    mutable vector<size_t>                      itsSlotCount;
    mutable vector<WorkerDescriptor>            itsRegister;

    // Triggers that have been registered with the command queue.
    // \note Triggers cannot be copied, so we must use a shared pointer.
    mutable map<Trigger::Type, shared_ptr<Trigger> >    itsTriggers;

    // Default time-out value
    static double theirDefaultTimeOut;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
