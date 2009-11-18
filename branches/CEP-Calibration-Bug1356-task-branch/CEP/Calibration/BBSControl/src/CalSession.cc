//# CalSession.cc: Control state that is shared between the controller and all
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

#include <lofar_config.h>
#include <BBSControl/CalSession.h>
#include <BBSControl/CalSessionTransactors.h>
#include <BBSControl/Exceptions.h>

#include <MWCommon/VdsDesc.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_numeric.h>
#include <Common/lofar_string.h>

#include <Common/ParameterSet.h>
#include <BBSControl/Step.h>

// gethostname() and getpid()
#include <unistd.h>

// numeric_limits<int32>
// TODO: Create lofar_limits.h in Common.
#include <limits>

#include <pqxx/except>

// Now here's an ugly kludge: libpqxx defines four different top-level
// exception classes. In order to avoid a lot of code duplication we clumped
// together four catch blocks in order to catch all pqxx related exceptions.
// A DatabaseException will be thrown, containing the original pqxx
// exception class type and the description.
#if defined(CATCH_PQXX_AND_RETHROW)
# error CATCH_PQXX_AND_RETHROW is already defined and should not be redefined
#else
# define CATCH_PQXX_AND_RETHROW					\
  catch (pqxx::broken_connection& e) {				\
    THROW (DatabaseException, "pqxx::broken_connection:\n"	\
	   << e.what());					\
  }								\
  catch (pqxx::sql_error& e) {					\
    THROW (DatabaseException, "pqxx::sql_error:\n"		\
	   << "Query: " << e.query() << endl << e.what());	\
  }								\
  catch (pqxx::in_doubt_error& e) {				\
    THROW (DatabaseException, "pqxx::in_doubt_error:\n"		\
	   << e.what());					\
  }								\
  catch (pqxx::internal_error& e) {				\
    THROW (DatabaseException, "pqxx::internal_error:\n"		\
	   << e.what());					\
  }
#endif

namespace LOFAR
{
namespace BBS 
{

namespace
{
    timeval asTimeval(double d)
    {
        timeval tv;
        tv.tv_sec = static_cast<long>(d);
        tv.tv_usec = static_cast<long>(round((d-tv.tv_sec)*1e6));
        if(tv.tv_usec > 999999)
        {
            tv.tv_sec += 1;
            tv.tv_usec -= 1000000;
        }
        if (tv.tv_usec < -999999)
        {
            tv.tv_sec -= 1;
            tv.tv_usec += 1000000;
        }
        return tv;
    }
}

ostream& operator<<(ostream& os, const ProcessId &obj)
{
    return os << obj.hostname << ":" << obj.pid;
}

CalSession::CalSession(const string &key, const string &db, const string &user,
    const string &password, const string &host, const string &port)
    :   itsSessionId(-1)
{
    // Determine the ProcessId of this worker.
    char hostname[512];
    int status = gethostname(hostname, 512);
    ASSERT(status == 0);    
    itsProcessId = ProcessId(string(hostname), getpid());

    // Build connection string.
    string opts("dbname='" + db + "' user='" + user + "' host='" + host
      + "' port=" + port);
    if(!password.empty()) {
      opts += " password='" + password + "'";
    }
    
    try
    {
        LOG_DEBUG_STR("Connecting to database: " << opts);
        itsConnection.reset(new pqxx::connection(opts));

        // Ensure the database representation of this shared state is
        // initialized and get the corresponding ID.
        itsConnection->perform(PQInitSession(key, itsSessionId));
    }
    CATCH_PQXX_AND_RETHROW
    
    registerTrigger(Trigger::WorkerRegisterModified);
    syncWorkerRegister(true);
}

CalSession::~CalSession()
{
}

ProcessId CalSession::getProcessId() const
{
    return itsProcessId;
}

bool CalSession::registerAsControl()
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQRegisterAsControl(itsSessionId, itsProcessId,
            status));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return status == 0;
}

bool CalSession::registerAsKernel(const string &filesys, const string &path,
    const Grid &grid)
{
    int32 status = -1;

    try
    {
        itsConnection->perform(PQRegisterAsKernel(itsSessionId, itsProcessId,
            filesys, path, grid, status));
    }
    CATCH_PQXX_AND_RETHROW;

    return status == 0;
}

bool CalSession::registerAsSolver(size_t port)
{
    int32 status = -1;

    try
    {
        itsConnection->perform(PQRegisterAsSolver(itsSessionId, itsProcessId,
            port, status));
    }
    CATCH_PQXX_AND_RETHROW;

    return status == 0;
}

void CalSession::setState(State state)
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQSetState(itsSessionId, itsProcessId, state,
            status));
    }
    CATCH_PQXX_AND_RETHROW;
    
    if(status != 0)
    {
        THROW(CalSessionException, "Unable to set session state");
    }
}

CalSession::State CalSession::getState() const
{
    int32 status = -1;
    State result;
    
    try
    {
        itsConnection->perform(PQGetState(itsSessionId, status, result));
    }
    CATCH_PQXX_AND_RETHROW;
    
    if(status != 0)
    {
        THROW(CalSessionException, "Unable to get session state");
    }
    
    return result;
}

void CalSession::initWorkerRegister(const CEP::VdsDesc &vds, bool useSolver)
{
    try
    {
        itsConnection->perform(PQInitWorkerRegister(itsSessionId, itsProcessId,
            vds, useSolver));
    }
    CATCH_PQXX_AND_RETHROW;
}    

void CalSession::setWorkerIndex(const ProcessId &worker, size_t index)
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQSetWorkerIndex(itsSessionId, itsProcessId,
            worker, index, status));
    }
    CATCH_PQXX_AND_RETHROW;
    
    if(status != 0)
    {
        THROW(CalSessionException, "Unable to set index of worker: " << worker
            << " to: " << index);
    }
}

CommandId CalSession::postCommand(const Command& cmd, WorkerType addressee)
    const
{
    int32 status = -1;
    CommandId id(-1);

    try
    {
        itsConnection->perform(PQPostCommand(itsSessionId, itsProcessId,
            addressee, cmd, status, id));
    }
    CATCH_PQXX_AND_RETHROW;

    if(status != 0)
    {
        THROW(CalSessionException, "Unable to post command");
    }

    return id;
}

pair<CommandId, shared_ptr<Command> > CalSession::getCommand() const
{
    int32 status = -1;
    pair<CommandId, shared_ptr<Command> > cmd(CommandId(-1),
        shared_ptr<Command>());
    
    try
    {
        itsConnection->perform(PQGetCommand(itsSessionId, itsProcessId, status,
            cmd));
    }
    CATCH_PQXX_AND_RETHROW;

    // Check for failure. A return value of -2 means that the command queue is
    // empty. This is not considered an error.
    if(status != 0 && status != -2)
    {
        THROW(CalSessionException, "Unable to get command");
    }

    return cmd;
}

void CalSession::postResult(const CommandId &id, const CommandResult &result)
    const
{        
    int32 status = -1;
      
    try
    {
        itsConnection->perform(PQPostResult(itsSessionId, itsProcessId, id,
            result, status));
    }
    CATCH_PQXX_AND_RETHROW;

    if(status != 0)
    {
        THROW(CalSessionException, "Unable to post result");
    }
}

CommandStatus CalSession::getCommandStatus(const CommandId &id) const
{
    int32 status = -1;
    WorkerType addressee;
    CommandStatus cmdStatus;

    try
    {
        itsConnection->perform(PQGetCommandStatus(id, status, addressee,
            cmdStatus));
    }
    CATCH_PQXX_AND_RETHROW;

    if(status != 0)
    {
        THROW(CalSessionException, "Attempt to get status for command with"
            " invalid command id: " << id);
    }

    return cmdStatus;
}

vector<pair<ProcessId, CommandResult> > CalSession::getResults(const CommandId
    &id) const
{
    vector<pair<ProcessId, CommandResult> > results;
    
    try
    {
        itsConnection->perform(PQGetResults(id, results));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return results;
}

bool CalSession::slotsAvailable() const
{
    // Note: syncWorkerRegister() already called in getWorkerCount().
    const size_t count = getWorkerCount();
    return (itsRegister.size() < count);
}

size_t CalSession::getWorkerCount() const
{
    syncWorkerRegister();
    return accumulate(itsSlotCount.begin(), itsSlotCount.end(), 0);
}

size_t CalSession::getWorkerCount(WorkerType type) const
{
    syncWorkerRegister();
    return itsSlotCount[static_cast<size_t>(type)];
}

bool CalSession::isWorker(const ProcessId &id) const
{
    try
    {
        // Note: syncWorkerRegister() already called in getWorkerById().
        getWorkerById(id);
    }
    catch(CalSessionException &e)
    {
        return false;
    }

    return true;
}

bool CalSession::isKernel(const ProcessId &id) const
{
    try
    {
        // Note: syncWorkerRegister() already called in getWorkerById().
        return (getWorkerById(id).type == KERNEL);
    }
    catch(CalSessionException &e)
    {
        return false;
    }
}

bool CalSession::isSolver(const ProcessId &id) const
{
    try
    {
        // Note: syncWorkerRegister() already called in getWorkerById().
        return (getWorkerById(id).type == SOLVER);
    }
    catch(CalSessionException &e)
    {
        return false;
    }
}

size_t CalSession::getIndex() const
{
    // Note: syncWorkerRegister() already called via getWorkerIndex(ProcessId).
    return getIndex(getProcessId());
}

size_t CalSession::getIndex(const ProcessId &id) const
{
    // Note: syncWorkerRegister() already called in getWorkerById().
    int32 index = getWorkerById(id).index;
    if(index < 0)
    {
        THROW(CalSessionException, "Index not assigned yet");
    }
    return static_cast<size_t>(index);
}

size_t CalSession::getPort(const ProcessId &id) const
{
    // Note: syncWorkerRegister() already called in getWorkerById().
    const Worker &worker = getWorkerById(id);
    if(worker.type != SOLVER)
    {
        THROW(CalSessionException, "Worker " << id << " is not of type SOLVER");
    }
    return worker.port;
}

string CalSession::getFilesys(const ProcessId &id) const
{
    // Note: syncWorkerRegister() already called in getWorkerById().
    const Worker &worker = getWorkerById(id);
    if(worker.type != KERNEL)
    {
        THROW(CalSessionException, "Worker " << id << " is not of type KERNEL");
    }
    return worker.filesys;
}

string CalSession::getPath(const ProcessId &id) const
{
    // Note: syncWorkerRegister() already called in getWorkerById().
    const Worker &worker = getWorkerById(id);
    if(worker.type != KERNEL)
    {
        THROW(CalSessionException, "Worker " << id << " is not of type KERNEL");
    }
    return worker.path;
}

Grid CalSession::getGrid(const ProcessId &id) const
{
    // Note: syncWorkerRegister() already called in getWorkerById().
    const Worker &worker = getWorkerById(id);
    if(worker.type != KERNEL)
    {
        THROW(CalSessionException, "Worker " << id << " is not of type KERNEL");
    }
    return worker.grid;
}

vector<ProcessId> CalSession::getWorkersByType(WorkerType type) const
{
    syncWorkerRegister();

    vector<ProcessId> result;
    vector<Worker>::const_iterator it = itsRegister.begin();
    vector<Worker>::const_iterator itEnd = itsRegister.end();
    while(it != itEnd)
    {
        if(it->type == type)
        {
            result.push_back(it->id);
        }
        ++it;
    }

    return result;
}

ProcessId CalSession::getWorkerByIndex(WorkerType type, size_t index) const
{
    ASSERT(index <= static_cast<size_t>(std::numeric_limits<int32>::max()));
    syncWorkerRegister();
    
    vector<Worker>::const_iterator it = itsRegister.begin();
    vector<Worker>::const_iterator itEnd = itsRegister.end();
    while(it != itEnd)
    {
        if(it->type == type && it->index == static_cast<int32>(index))
        {
            return it->id;
        }        
        ++it;
    }
    
    THROW(CalSessionException, "No worker found of type: "
        << static_cast<int>(type) << " with index: " << index);
}

Axis::ShPtr CalSession::getGlobalTimeAxis() const
{
    syncWorkerRegister();

    Axis::ShPtr axis;
    vector<Worker>::const_iterator it = itsRegister.begin();
    vector<Worker>::const_iterator itEnd = itsRegister.end();
    while(it != itEnd)
    {
        if(it->type == KERNEL)
        {
            if(!it->grid[1])
            {
                continue;
            }
            
            if(axis)
            {
                int s1, e1, s2, e2;
                axis = axis->combine(*(it->grid[1]), s1, e1, s2, e2);
            }
            else
            {
                axis = it->grid[1];
            }
        }        
        ++it;
    }
    
    return axis;        
}

void CalSession::syncWorkerRegister(bool force) const
{
    if(!force && !waitForTrigger(Trigger::WorkerRegisterModified, 0))
    {
        return;
    }
    
    vector<size_t> tmpSlotCount(N_WorkerType, 0);
    vector<Worker> tmpRegister;
    
    try
    {
        itsConnection->perform(PQGetWorkerRegister(itsSessionId, tmpSlotCount,
            tmpRegister));
    }
    CATCH_PQXX_AND_RETHROW;
    
    itsSlotCount = tmpSlotCount;
    itsRegister = tmpRegister;
    
    // Recreate the mapping from ProcessId to index in the register.
    itsRegisterMap.clear();
    for(size_t i = 0; i < itsRegister.size(); ++i)
    {
        itsRegisterMap[itsRegister[i].id] = i;
    }
}

const CalSession::Worker &CalSession::getWorkerById(const ProcessId &id) const
{
    syncWorkerRegister();
    
    map<ProcessId, size_t>::const_iterator index = itsRegisterMap.find(id);
    if(index == itsRegisterMap.end())
    {
        THROW(CalSessionException, "Unknown worker: " << id);
    }

    ASSERT(index->second < itsRegister.size());
    return itsRegister[index->second];
}

bool CalSession::waitForCommand(double timeOut) const
{
    if(!isRegistered(Trigger::Command))
    {
        registerTrigger(Trigger::Command);
        return true;
    }

    return waitForTrigger(Trigger::Command, timeOut);
}

bool CalSession::waitForResult(double timeOut) const
{
    if(!isRegistered(Trigger::Result))	
    {
        registerTrigger(Trigger::Result);
        return true;
    }

    return waitForTrigger(Trigger::Result, timeOut);
}

bool CalSession::registerTrigger(Trigger::Type type) const
{
    shared_ptr<Trigger> trigger(new Trigger(*this, type));
    return itsTriggers.insert(make_pair(type, trigger)).second;
}

bool CalSession::dropTrigger(Trigger::Type type) const
{
    return itsTriggers.erase(type);
}

bool CalSession::isRegistered(Trigger::Type type) const
{
    return itsTriggers.find(type) != itsTriggers.end();
}

bool CalSession::waitForTrigger(Trigger::Type type, double timeOut) const
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

    // Check if we already received a notification earlier.
    if (Trigger::testAndClearFlags(type) == type)
    {
        return true;
    }
    else
    {
        LOG_TRACE_COND("Waiting for notification");
        uint notifs;
        if(timeOut < 0)
        {
            notifs = itsConnection->await_notification();
        }
        else
        {
            timeval tv = asTimeval(timeOut);
            notifs = itsConnection->await_notification(tv.tv_sec, tv.tv_usec);
        }
    }
    
    return (Trigger::testAndClearFlags(type) == type);
}

CalSession::Trigger::Init::Init()
{
    // Couple trigger type and notification name.
    // (see sql/create_blackboard_triggers.sql)
    theirTypes[Command] = "insert_command";
    theirTypes[Result]  = "insert_result";
    theirTypes[WorkerRegisterModified] = "modify_worker_register";
}

CalSession::Trigger::Trigger(const CalSession& session, Type type)
    :   pqxx::trigger(*session.itsConnection,
            theirTypes.find(type) == theirTypes.end() ? "" : theirTypes[type])
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    ASSERT(theirTypes.find(type) != theirTypes.end());
    LOG_DEBUG_STR("Created trigger of type: " << theirTypes[type]);
}

void CalSession::Trigger::operator()(int be_pid)
{
    LOG_DEBUG_STR("Received notification: " << name() << "; pid = " << be_pid);
    TypeMap::const_iterator it;
    for(it = theirTypes.begin(); it != theirTypes.end(); ++it)
    {
        if(it->second == name())
        {
            LOG_TRACE_COND_STR("Raising flag [" << it->first << ","
                << it->second << "]");
            raiseFlags(it->first);
            break;
        }
    }
}

double                          CalSession::theirDefaultTimeOut = 5.0;
CalSession::Trigger::TypeMap    CalSession::Trigger::theirTypes;
CalSession::Trigger::Type       CalSession::Trigger::theirFlags;
CalSession::Trigger::Init       CalSession::Trigger::theirInit;

} //# namespace BBS
} //# namespace LOFAR
