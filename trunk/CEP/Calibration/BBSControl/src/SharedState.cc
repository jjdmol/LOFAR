//# SharedState.cc: Control state that is shared between the controller and all
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

#include <lofar_config.h>
#include <BBSControl/SharedState.h>
#include <BBSControl/SharedStateTransactors.h>
#include <BBSControl/Exceptions.h>

#include <MWCommon/VdsDesc.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_numeric.h>
#include <Common/lofar_string.h>

#include <Common/ParameterSet.h>
#include <BBSControl/Step.h>
//# For gethostname() and getpid().
//#include <cunistd>
#include <unistd.h>

#include <pqxx/except>

//# Now here's an ugly kludge: libpqxx defines four different top-level
//# exception classes. In order to avoid a lot of code duplication we clumped
//# together four catch blocks in order to catch all pqxx related exceptions.
//# A DatabaseException will be thrown, containing the original pqxx
//# exception class type and the description.
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

    struct MatchWorkerOnIndex: public std::unary_function<SharedState::WorkerDescriptor, bool>
    {
        MatchWorkerOnIndex(SharedState::WorkerType type, int index)
            :   itsType(type),
                itsIndex(index)
        {
        }
        
        bool operator()(const SharedState::WorkerDescriptor &worker) const
        {
            return worker.type == itsType && worker.index == itsIndex;
        }
        
    private:
        SharedState::WorkerType  itsType;
        int         itsIndex;
    };
    struct MatchWorkerOnId: public std::unary_function<SharedState::WorkerDescriptor, bool>
    {
        MatchWorkerOnId(const ProcessId &id)
            :   itsProcessId(id)
        {
        }
        
        bool operator()(const SharedState::WorkerDescriptor &worker) const
        {
            return worker.id == itsProcessId;
        }
        
    private:
        const ProcessId &itsProcessId;
    };
}

SharedState::SharedState(const string &key, const string &db,
    const string &user, const string &host, const string &port)
    :   itsStateId(-1)
{
    // Determine the ProcessId of this worker.
    char hostname[512];
    int status = gethostname(hostname, 512);
    ASSERT(status == 0);    
    itsProcessId = ProcessId(string(hostname), getpid());

    string opts("dbname=" + db  + " user=" + user + " host=" + host + " port="
        + port);

    try
    {
        LOG_DEBUG_STR("Connecting to database: " << opts);
        itsConnection.reset(new pqxx::connection(opts));

        // Ensure the database representation of this shared state is
        // initialized and get the corresponding ID.
        itsConnection->perform(PQInitSharedState(key, itsStateId));
    }
    CATCH_PQXX_AND_RETHROW
    
    registerTrigger(Trigger::RegisterModified);
    syncRegister(true);
}

SharedState::~SharedState()
{
}

bool SharedState::setRunState(RunState state)
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQSetRunState(itsStateId, itsProcessId, status,
            state));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return status == 0;
}

SharedState::RunState SharedState::getRunState() const
{
    int32 status = -1;
    RunState result;
    
    try
    {
        itsConnection->perform(PQGetRunState(itsStateId, status, result));
    }
    CATCH_PQXX_AND_RETHROW;
    
    if(status != 0)
    {
        THROW(SharedStateException, "Unable to get run state");
    }
    
    return result;
}

bool SharedState::registerAsControl()
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQRegisterAsControl(itsStateId, itsProcessId,
            status));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return status == 0;
}

bool SharedState::registerAsKernel(const string &filesystem, const string &path,
    const Grid &grid)
{
    int32 status = -1;

    try
    {
        itsConnection->perform(PQRegisterAsKernel(itsStateId, itsProcessId,
            filesystem, path, grid, status));
    }
    CATCH_PQXX_AND_RETHROW;

    return status == 0;
}

bool SharedState::registerAsSolver(size_t port)
{
    int32 status = -1;

    try
    {
        itsConnection->perform(PQRegisterAsSolver(itsStateId, itsProcessId, port,
            status));
    }
    CATCH_PQXX_AND_RETHROW;

    return status == 0;
}

void SharedState::initRegister(const CEP::VdsDesc &vds, bool useSolver)
{
    try
    {
        itsConnection->perform(PQInitRegister(itsStateId, itsProcessId, vds,
            useSolver));
    }
    CATCH_PQXX_AND_RETHROW;
}    

bool SharedState::setIndex(const ProcessId &target, size_t index)
{
    int32 status = -1;
    
    try
    {
        itsConnection->perform(PQSetIndex(itsStateId, itsProcessId, target, index,
            status));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return status == 0;
}

CommandId SharedState::addCommand(const Command& cmd, WorkerType target) const
{
    int32 status = -1;
    CommandId cmdId(-1);

    try
    {
        itsConnection->perform(PQAddCommand(itsStateId, itsProcessId, target, cmd, status,
            cmdId));
    }
    CATCH_PQXX_AND_RETHROW;

    if(status != 0)
    {
        THROW(SharedStateException, "Unable to add command");
    }

    return cmdId;
}

pair<CommandId, shared_ptr<Command> > SharedState::getCommand() const
{
    int32 status = -1;
    pair<CommandId, shared_ptr<Command> > cmd(CommandId(-1),
        shared_ptr<Command>());
    
    try
    {
        itsConnection->perform(PQGetCommand(itsStateId, itsProcessId, status, cmd));
    }
    CATCH_PQXX_AND_RETHROW;

    // Check for failure. A return value of -2 means that the command queue is
    // empty. This is not considered an error.
    if(status != 0 && status != -2)
    {
        THROW(SharedStateException, "Unable to get command");
    }

    return cmd;
}

CommandStatus SharedState::getCommandStatus(const CommandId &cmdId) const
{
    int32 status = -1;
    WorkerType target;
    CommandStatus cmdStatus;

    try
    {
        itsConnection->perform(PQGetCommandStatus(cmdId, status, target, cmdStatus));
    }
    CATCH_PQXX_AND_RETHROW;

    if(status != 0)
    {
        THROW(SharedStateException, "Attempt to get status for command with"
            " invalid command id: " << cmdId);
    }

    return cmdStatus;
}

bool SharedState::addResult(const CommandId &cmdId, const CommandResult &result)
    const
{        
    int32 status = -1;
      
    try
    {
        itsConnection->perform(PQAddResult(itsStateId, itsProcessId, cmdId, result,
            status));
    }
    CATCH_PQXX_AND_RETHROW;

    return status == 0;
}

vector<pair<ProcessId, CommandResult> > SharedState::getResults(const CommandId
    &cmdId) const
{
    vector<pair<ProcessId, CommandResult> > results;
    
    try
    {
        itsConnection->perform(PQGetResults(cmdId, results));
    }
    CATCH_PQXX_AND_RETHROW;
    
    return results;
}

bool SharedState::slotsAvailable() const
{
    syncRegister();
    
    LOG_DEBUG_STR("#kernel slots: " << itsSlotCount[KERNEL]);
    LOG_DEBUG_STR("#solver slots: " << itsSlotCount[SOLVER]);
    
    const size_t count =
        accumulate(itsSlotCount.begin(), itsSlotCount.end(), 0);
    
    return (itsRegister.size() < count);
}


Axis::ShPtr SharedState::getGlobalTimeAxis() const
{
    syncRegister();
    
    vector<WorkerDescriptor> kernels = getWorkersByType(KERNEL);
    if(kernels.empty())
    {
        return Axis::ShPtr();
    }

    int s1, e1, s2, e2;
    Axis::ShPtr axis = kernels.front().grid[1];
    for(size_t i = 1; i < kernels.size(); ++i)
    {
        axis = axis->combine(*kernels[i].grid[1], s1, e1, s2, e2);
    }

    return axis;
}

void SharedState::syncRegister(bool force) const
{
    if(!force && !waitForTrigger(Trigger::RegisterModified, 0))
    {
        return;
    }
    
    vector<size_t> tmpSlotCount(N_WorkerType, 0);
    vector<WorkerDescriptor> tmpRegister;
    
    try
    {
        itsConnection->perform(PQGetRegister(itsStateId, tmpSlotCount, tmpRegister));
    }
    CATCH_PQXX_AND_RETHROW;
    
    itsSlotCount = tmpSlotCount;
    itsRegister = tmpRegister;
}

bool SharedState::waitForCommand(double timeOut) const
{
    if(!isRegistered(Trigger::Command))
    {
        registerTrigger(Trigger::Command);
        return true;
    }

    return waitForTrigger(Trigger::Command, timeOut);
}

bool SharedState::waitForResult(double timeOut) const
{
    if(!isRegistered(Trigger::Result))
    {
        registerTrigger(Trigger::Result);
        return true;
    }

    return waitForTrigger(Trigger::Result, timeOut);
}

vector<SharedState::WorkerDescriptor> SharedState::getWorkersByType(WorkerType type) const
{
    syncRegister();

    vector<WorkerDescriptor> result;

    for(size_t i = 0; i < itsRegister.size(); ++i)
    {
        if(itsRegister[i].type == type)
        {
            result.push_back(itsRegister[i]);
        }
    }
    return result;
}


SharedState::WorkerDescriptor SharedState::getWorkerByIndex(WorkerType type, size_t index)
    const
{
    syncRegister();
    
    vector<WorkerDescriptor>::const_iterator it =
        find_if(itsRegister.begin(), itsRegister.end(),
            MatchWorkerOnIndex(type, index));
    
    if(it == itsRegister.end())
    {
        THROW(SharedStateException, "No worker found of type: "
            << static_cast<int>(type) << " with index: " << index);
    }

    return *it;
}


SharedState::WorkerDescriptor SharedState::getWorkerById(const ProcessId &id) const
{
    syncRegister();
    
    vector<WorkerDescriptor>::const_iterator it =
        find_if(itsRegister.begin(), itsRegister.end(),
            MatchWorkerOnId(id));
    
    if(it == itsRegister.end())
    {
        THROW(SharedStateException, "No worker found of with id: "
            << id.hostname << ":" << id.pid);
    }

    return *it;
}

bool SharedState::isRegistered(const ProcessId &id) const
{
    try
    {
        getWorkerById(id);
    }
    catch(SharedStateException &e)
    {
        return false;
    }

    return true;
}

bool SharedState::isKernel(const ProcessId &id) const
{
    WorkerDescriptor worker;

    try
    {
        worker = getWorkerById(id);
    }
    catch(SharedStateException &e)
    {
        return false;
    }
    
    return worker.type == KERNEL;    
}

bool SharedState::isSolver(const ProcessId &id) const
{
    WorkerDescriptor worker;

    try
    {
        worker = getWorkerById(id);
    }
    catch(SharedStateException &e)
    {
        return false;
    }
    
    return worker.type == SOLVER;
}

size_t SharedState::getIndex() const
{
    int32 index = getWorkerById(itsProcessId).index;
    if(index < 0)
    {
        THROW(SharedStateException, "Index not assigned yet");
    }
    return static_cast<size_t>(index);
}

size_t SharedState::getWorkerCount() const
{
    syncRegister();
    return accumulate(itsSlotCount.begin(), itsSlotCount.end(), 0);
}

size_t SharedState::getWorkerCount(WorkerType type) const
{
    syncRegister();
    return itsSlotCount[static_cast<size_t>(type)];
}

bool SharedState::registerTrigger(Trigger::Type type) const
{
    shared_ptr<Trigger> trigger(new Trigger(*this, type));
    return itsTriggers.insert(make_pair(type, trigger)).second;
}

bool SharedState::deregisterTrigger(Trigger::Type type) const
{
    return itsTriggers.erase(type);
}

bool SharedState::isRegistered(Trigger::Type type) const
{
    return itsTriggers.find(type) != itsTriggers.end();
}

bool SharedState::waitForTrigger(Trigger::Type type, double timeOut) const
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

/*
    if(notifs)
    {
        LOG_TRACE_COND("Received notification ...");
        if (Trigger::testAndClearFlags(type) == type)
        {
            LOG_TRACE_COND("... of the correct type");
            return true;
        }
    }
*/    
}

SharedState::Trigger::Init::Init()
{
    theirTypes[Command] = "insert_command";
    theirTypes[Result]  = "insert_result";
    theirTypes[RegisterModified] = "modify_register";
}

SharedState::Trigger::Trigger(const SharedState& state, Type type)
    :   pqxx::trigger(*state.itsConnection,
            theirTypes.find(type) == theirTypes.end() ? "" : theirTypes[type])
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    ASSERT(theirTypes.find(type) != theirTypes.end());
    LOG_DEBUG_STR("Created trigger of type: " << theirTypes[type]);
}

void SharedState::Trigger::operator()(int be_pid)
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

double                          SharedState::theirDefaultTimeOut = 5.0;
SharedState::Trigger::TypeMap   SharedState::Trigger::theirTypes;
SharedState::Trigger::Type      SharedState::Trigger::theirFlags;
SharedState::Trigger::Init      SharedState::Trigger::theirInit;

} //# namespace BBS
} //# namespace LOFAR
