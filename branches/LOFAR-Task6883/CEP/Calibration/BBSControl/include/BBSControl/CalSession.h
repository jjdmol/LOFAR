//# CalSession.h: Control state that is shared between the controller and all
//# worker processes.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSCONTROL_CALSESSION_H
#define LOFAR_BBSCONTROL_CALSESSION_H

// \file
// Control state that is shared between the controller and all worker processes.

#include <BBSControl/Command.h>
#include <BBSControl/CommandResult.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Types.h>
#include <BBSControl/ProcessId.h>
#include <BBSKernel/Interval.h>
#include <ParmDB/Axis.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <pqxx/connection>
#include <pqxx/trigger>
#include <functional>

namespace LOFAR
{

namespace CEP
{
    class VdsDesc;
    class VdsPartDesc;
}

namespace BBS
{
class PQGetWorkerRegister;

// \addtogroup BBSControl
// @{

class CalSession
{
public:
    enum State
    {
        FAILED = -1,
        WAITING_FOR_CONTROL,
        WAITING_FOR_WORKERS,
        INITIALIZING,
        PROCESSING,
        DONE,
        N_State
    };

    enum WorkerType
    {
        KERNEL,
        SOLVER,
        N_WorkerType
    };

    // Status of a Command.
    struct CommandStatus
    {
        // Number of workers that finished the command.
        unsigned int    finished;
        // Number of workers that reported failure.
        unsigned int    failed;
    };

    CalSession(const string &key, const string &db, const string &user,
        const string &password = "", const string &host = "localhost",
        const string &port = "");

    ProcessId getProcessId() const;

    bool registerAsControl();
    bool registerAsKernel(const string &filesys, const string &path,
        const Axis::ShPtr &freqAxis, const Axis::ShPtr &timeAxis);
    bool registerAsSolver(size_t port);

    // Manipulate the session state.
    // @{
    void setState(State state);
    State getState() const;
    // @}

    // Manipulate the session time axis.
    // @{
    void setTimeAxis(const Axis::ShPtr &axis);
    Axis::ShPtr getTimeAxis();
    // @}

    // Manipulate the session parameter set.
    // @{
    void setParset(const ParameterSet &parset) const;
    ParameterSet getParset() const;
    // @}

    void initWorkerRegister(const CEP::VdsDesc &vds, bool useSolver);
    void setWorkerIndex(const ProcessId &worker, size_t index);

    // Post a Command to the command queue. This method is typically used by the
    // controller.
    // \return The unique command-id associated with \a cmd.
    CommandId postCommand(const Command &cmd,
        WorkerType addressee = N_WorkerType) const;

    // Get the next Command from the command queue. This method is typically
    // used by workers.
    // \return A pair consisting of a unique command-id and a shared_ptr to
    // the command. When there are no more commands left in the queue, and id of
    // -1 and a null shared_ptr are returned.
    pair<CommandId, shared_ptr<Command> > getCommand() const;

    // Post the result \a result for the command (identified by) \a id. The
    // \a id must match the id of the current command for the worker invoking
    // this method.
    // \returns False if post failed, true otherwise.
    void postResult(const CommandId &id, const CommandResult &result) const;

    // Get the current status of the command (identified by) \a id, which must
    // refer to a command posted in the current session.
    CommandStatus getCommandStatus(const CommandId &id) const;

    // Get all the results associated with the command (identified by) \a id.
    vector<pair<ProcessId, CommandResult> > getResults(const CommandId &id)
        const;

    // Querying the worker register.
    // @{
    bool slotsAvailable() const;
    size_t getWorkerCount() const;
    size_t getWorkerCount(WorkerType type) const;

    bool isWorker(const ProcessId &id) const;
    bool isKernel(const ProcessId &id) const;
    bool isSolver(const ProcessId &id) const;

    size_t getIndex() const;
    size_t getIndex(const ProcessId &id) const;
    size_t getPort(const ProcessId &id) const;
    string getFilesys(const ProcessId &id) const;
    string getPath(const ProcessId &id) const;
    Interval<double> getFreqRange(const ProcessId &id) const;
    Interval<double> getTimeRange(const ProcessId &id) const;
    Axis::ShPtr getFreqAxis(const ProcessId &id) const;
    Axis::ShPtr getTimeAxis(const ProcessId &id) const;

    vector<ProcessId> getWorkersByType(WorkerType type) const;
    ProcessId getWorkerByIndex(WorkerType type, size_t index) const;
    // @}

    // Waiting for an event.
    // @{
    bool waitForCommand(double timeOut = theirDefaultTimeOut) const;
    bool waitForResult(double timeOut = theirDefaultTimeOut) const;
    // @}

private:
    // Trigger class handles notifications received from the database backend by
    // raising the flag associated with the notification.
    class Trigger : public pqxx::trigger
    {
    public:
        // Valid trigger types.
        enum Type
        {
            Command = 1L << 0,
            Result  = 1L << 1,
            WorkerRegisterModified = 1L << 2
        };

        // Construct a trigger handler for trigger of type \a type.
        Trigger(const CalSession& session, Type type);

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

        // Keep track of flags that were raised as a result of a notification.
        // Note that this is a \e shared variable, because we're not really
        // interested in who responds to a raised flag; it's important that it
        // is handled, but not by whom.
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

    // Struct that contains all information about a Worker.
    struct Worker
    {
        ProcessId           id;
        WorkerType          type;
        int32               index;
        size_t              port;
        string              filesys;
        string              path;
        Interval<double>    freqRange;
        Interval<double>    timeRange;
    };

    // Trigger administration and waiting.
    // @{

    // Create a the trigger for events of type \a type. Once registered, we will
    // receive a notification from the database whenever an event of type
    // \a type occurs.
    bool registerTrigger(Trigger::Type type) const;

    // Drop the trigger for events of type \a type.
    bool dropTrigger(Trigger::Type type) const;

    // Is a trigger for events of type \a type registered?
    bool isRegistered(Trigger::Type type) const;

    // Wait for a trigger from the database backend. Only triggers that were
    // previously registered will get through. When a trigger arrives the
    // associated trigger flag will be raised. This way, we decouple reception
    // of the trigger from the action to be taken.
    bool waitForTrigger(Trigger::Type type,
        double timeOut = theirDefaultTimeOut) const;
    // @}

    // Synchronize the cached worker register with the database.
    void syncWorkerRegister(bool force = false) const;

    const Worker &getWorkerById(const ProcessId &id) const;

    int32                           itsSessionId;
    ProcessId                       itsProcessId;
    scoped_ptr<pqxx::connection>    itsConnection;

    // WorkerRegister cache.
    mutable vector<size_t>          itsSlotCount;
    mutable map<ProcessId, size_t>  itsRegisterMap;
    mutable vector<Worker>          itsRegister;
    mutable bool                    itsRegisterDirty;

    // Triggers that have been registered with the database.
    // \note Triggers cannot be copied, so we must use a shared pointer.
    mutable map<Trigger::Type, shared_ptr<Trigger> >    itsTriggers;

    // Default time-out value.
    static double theirDefaultTimeOut;

    friend class PQGetWorkerRegister;
};
// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
