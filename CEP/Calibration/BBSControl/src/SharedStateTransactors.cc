//# SharedStateTransactors.cc: Transactors responsible for calling stored
//# procedures and for all required parameter/return value conversions.
//#
//# Copyright (C) 2009
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
#include <BBSControl/SharedStateTransactors.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Step.h>

#include <MWCommon/VdsDesc.h>
#include <MWCommon/VdsPartDesc.h>

#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{


//----------------------------------------------------------------------------//

PQInitSharedState::PQInitSharedState(const string &key, int32 &id)
    :   pqxx::transactor<>("PQInitSharedState"),
        itsKey(key),
        itsId(id)
{
}
        
void PQInitSharedState::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.init_shared_state('"
        << transaction.esc(itsKey) << "')";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQInitSharedState::on_commit()
{
    itsId = itsQueryResult[0]["_id"].as<int32>();
}


PQSetRunState::PQSetRunState(int32 id, const ProcessId &pid, int32 &status,
    SharedState::RunState state)
    :   pqxx::transactor<>("PQSetRunState"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(status),
        itsRunState(state)
{
}
        
void PQSetRunState::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.set_run_state("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ","
        << static_cast<int32>(itsRunState) << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQSetRunState::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetRunState::PQGetRunState(int32 id, int32 &status,
    SharedState::RunState &state)
    :   pqxx::transactor<>("PQGetRunState"),
        itsId(id),
        itsStatus(status),
        itsRunState(state)
{
}

void PQGetRunState::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_run_state(" << itsId << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetRunState::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
    
    if(itsStatus == 0)
    {
        int32 runState = itsQueryResult[0]["_run_state"].as<int32>();

        if(runState < SharedState::FAILED
            || runState >= SharedState::N_RunState)
        {
            THROW(TranslationException, "Invalid run state: " << runState);
        }
        LOG_DEBUG_STR("RUN STATE: " << runState);
        itsRunState = static_cast<SharedState::RunState>(runState);
    }
}


PQInitRegister::PQInitRegister(int32 id, const ProcessId &pid,
    const CEP::VdsDesc &vds, bool useSolver)
    :   pqxx::transactor<>("PQInitRegister"),
        itsId(id),
        itsProcessId(pid),
        itsVdsDesc(vds),
        itsUseSolver(useSolver)
{
}    

void PQInitRegister::operator()(argument_type &transaction)
{
    ostringstream query;
        
    const vector<CEP::VdsPartDesc> &parts = itsVdsDesc.getParts();
    for(size_t i = 0; i < parts.size(); ++i)
    {
        query.str("");
        query << "SELECT blackboard.create_kernel_slot("
            << itsId << ",'"
            << transaction.esc(itsProcessId.hostname) << "',"
            << itsProcessId.pid << ",'"
            << transaction.esc(parts[i].getFileSys()) << "','"
            << transaction.esc(parts[i].getFileName()) << "')";
        LOG_DEBUG_STR("Query: " << query.str());
        transaction.exec(query.str());
    }
    
    if(itsUseSolver)
    {
        query.str("");
        query << "SELECT blackboard.create_solver_slot("
            << itsId << ",'"
            << transaction.esc(itsProcessId.hostname) << "',"
            << itsProcessId.pid << ")";
        LOG_DEBUG_STR("Query: " << query.str());
        transaction.exec(query.str());
    }
}


PQRegisterAsControl::PQRegisterAsControl(int32 id, const ProcessId &pid,
    int32 &status)
    :   pqxx::transactor<>("PQRegisterAsControl"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(status)
{
}

void PQRegisterAsControl::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.register_as_control(" << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQRegisterAsControl::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQRegisterAsKernel::PQRegisterAsKernel(int32 id, const ProcessId &pid,
    const string &filesystem, const string &path, const Grid &grid,
    int32 &status)
    :   pqxx::transactor<>("PQRegisterAsKernel"),
        itsId(id),
        itsProcessId(pid),
        itsFilesystem(filesystem),
        itsPath(path),
        itsGrid(grid),
        itsStatus(status)
{
}    

void PQRegisterAsKernel::operator()(argument_type &transaction)
{
    // The E' syntax avoids a warning on non-standard use of \\ in a string
    // literal. The E'' syntax is postgresql specific (i.e. non-standard) but
    // avoids the warning. It explicitly enables the use of \\ in a string
    // literal. The best solution would be to change the way escaping is done,
    // but that is part of the libpqxx implementation.

    ostringstream query;
    query << "SELECT * FROM blackboard.register_as_kernel("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ",'"
        << transaction.esc(itsFilesystem) << "','"
        << transaction.esc(itsPath) << "',E'"
        << pack_vector(transaction, itsGrid[0]->lowers()) << "',E'"
        << pack_vector(transaction, itsGrid[0]->uppers()) << "',E'"
        << pack_vector(transaction, itsGrid[1]->lowers()) << "',E'"
        << pack_vector(transaction, itsGrid[1]->uppers()) << "')";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQRegisterAsKernel::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}

PQRegisterAsSolver::PQRegisterAsSolver(int32 id, const ProcessId &pid,
    size_t port, int32 &status)
    :   pqxx::transactor<>("PQRegisterAsSolver"),
        itsId(id),
        itsProcessId(pid),
        itsPort(port),
        itsStatus(status)
{
}    

void PQRegisterAsSolver::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.register_as_solver("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ","
        << itsPort << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQRegisterAsSolver::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetRegister::PQGetRegister(int32 id, vector<size_t> &count,
    vector<SharedState::WorkerDescriptor> &workers)
    :   pqxx::transactor<>("PQGetRegister"),
        itsId(id),
        itsCount(count),
        itsWorkers(workers)
{
    ASSERT(itsCount.size() == SharedState::N_WorkerType);
}
        
void PQGetRegister::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_register(" << itsId << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetRegister::on_commit()
{
    fill(itsCount.begin(), itsCount.end(), 0);
    
    pqxx::result::const_iterator rowIt = itsQueryResult.begin();
    const pqxx::result::const_iterator rowItEnd = itsQueryResult.end();
    while(rowIt != rowItEnd)
    {
        // Parse worker type.
        SharedState::WorkerType workerType;

        string type = rowIt["worker_type"].as<string>();
        if(type.size() != 1)
        {
            THROW(TranslationException, "Invalid worker type: " << type);
        }

        if(type[0] == 'K')
        {
            workerType = SharedState::KERNEL;
        }
        else if(type[0] == 'S')
        {
            workerType = SharedState::SOLVER;
        }
        else
        {
            THROW(TranslationException, "Invalid worker type: " << type);
        }

        // Update slot counter.
        ++itsCount[workerType];

        // Check if slot is empty.
        if(rowIt["hostname"].is_null() || rowIt["pid"].is_null())
        {
            LOG_DEBUG_STR("Empty slot.");
            ++rowIt;
            continue;
        }

        // Parse worker type specific information.
        SharedState::WorkerDescriptor descriptor;
        descriptor.id.hostname = rowIt["hostname"].as<string>();
        descriptor.id.pid = rowIt["pid"].as<int64>();
        descriptor.type = workerType;
        descriptor.index = rowIt["index"].as<int32>(-1);

        if(workerType == SharedState::KERNEL)
        {
            if(rowIt["path"].is_null()
                || rowIt["axis_freq_lower"].is_null()
                || rowIt["axis_freq_upper"].is_null()
                || rowIt["axis_time_lower"].is_null()
                || rowIt["axis_time_upper"].is_null())
            {
                THROW(TranslationException, "Kernel information should not be"
                    " NULL for worker " << descriptor.id.hostname << ":"
                    << descriptor.id.pid);
            }

            // Get measurement part.
            descriptor.filesystem = rowIt["filesystem"].as<string>();
            descriptor.path = rowIt["path"].as<string>();
            
            // Unpack frequency axis.
            pqxx::binarystring freqLower(rowIt["axis_freq_lower"]);
            pqxx::binarystring freqUpper(rowIt["axis_freq_upper"]);
            Axis::ShPtr freqAxis(new OrderedAxis(
                unpack_vector<double>(freqLower),
                unpack_vector<double>(freqUpper),
                true));
            
            // Unpack time axis.
            pqxx::binarystring timeLower(rowIt["axis_time_lower"]);
            pqxx::binarystring timeUpper(rowIt["axis_time_upper"]);
            Axis::ShPtr timeAxis(new OrderedAxis(
                unpack_vector<double>(timeLower),
                unpack_vector<double>(timeUpper),
                true));
            
            descriptor.grid = Grid(freqAxis, timeAxis);
        }
        else
        {
            ASSERT(workerType == SharedState::SOLVER);

            if(rowIt["port"].is_null())
            {
                THROW(TranslationException, "Solver information should not be"
                    " NULL for worker " << descriptor.id.hostname << ":"
                    << descriptor.id.pid);
            }
            
            descriptor.port = rowIt["port"].as<int32>();
        }

        itsWorkers.push_back(descriptor);
        LOG_DEBUG_STR("Found worker... Type: " << type);
        
        ++rowIt;
    }
}


PQSetIndex::PQSetIndex(int32 id, const ProcessId &pid, const ProcessId &target,
    int32 index, int32 &status)
    :   pqxx::transactor<>("PQSetIndex"),
        itsId(id),
        itsProcessId(pid),
        itsTarget(target),
        itsIndex(index),
        itsStatus(status)
{
    ASSERT(index >= 0);
}

void PQSetIndex::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.set_index("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ",'"
        << transaction.esc(itsTarget.hostname) << "',"
        << itsTarget.pid << ","
        << itsIndex << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQSetIndex::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQAddCommand::PQAddCommand(int32 id, const ProcessId &pid, SharedState::WorkerType target,
    const Command &cmd, int32 &status, int32 &cmdId)
    :   pqxx::transactor<>("PQAddCommand"),
        itsId(id),
        itsProcessId(pid),
        itsTarget(target),
        itsCommand(cmd),
        itsStatus(status),
        itsCommandId(cmdId)
{
}

void PQAddCommand::operator()(argument_type &transaction)
{
    string target;
    if(itsTarget >= SharedState::N_WorkerType)
    {
        target = "NULL";
    }
    else if(itsTarget == SharedState::KERNEL)
    {
        target = "'K'";
    }
    else
    {
        target = "'S'";
    }
    
    ostringstream query;

    query << "SELECT * FROM blackboard.add_command("
        << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "'"
        << "," << itsProcessId.pid
        << "," << target
        << ",'" << transaction.esc(toLower(itsCommand.type())) << "'";

    // Only Step and its derivatives have an attribute 'name'. As this is also
    // used to distinguish between Step (or a derived class) and Command (or a
    // derived class) we have to supply it.
    try
    {
        const Step &step = dynamic_cast<const Step&>(itsCommand);
        query << ",'" << transaction.esc(step.name()) << "'";
    }
    catch(std::bad_cast)
    {
        query << ",NULL";
    }

    // Add the command's arguments as a parset.
    ParameterSet ps;
    query << ",'" << (ps << itsCommand) << "')";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQAddCommand::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(itsStatus == 0)
    {
        itsCommandId = itsQueryResult[0]["_id"].as<CommandId>();
    }
}


PQAddResult::PQAddResult(int32 id, const ProcessId &pid, const CommandId &cmdId,
    const CommandResult &cmdResult, int32 &status)
    :   pqxx::transactor<>("PQAddResult"),
        itsId(id),
        itsProcessId(pid),
        itsCommandId(cmdId),
        itsCommandResult(cmdResult),
        itsStatus(status)
{
}        

void PQAddResult::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.add_result("
        << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "'"
        << "," << itsProcessId.pid
        << "," << itsCommandId
        << "," << itsCommandResult.asInt()
        << ",'" << transaction.esc(itsCommandResult.message()) << "')";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQAddResult::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetCommand::PQGetCommand(int32 id, const ProcessId &pid, int32 &status,
    pair<CommandId, shared_ptr<Command> > &cmd)
    :   pqxx::transactor<>("PQGetCommand"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(status),
        itsCommand(cmd)
{
}
    
void PQGetCommand::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_command("
        << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "'"
        << "," << itsProcessId.pid << ")";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQGetCommand::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(itsStatus < 0)
    {
        // Failure or empty queue.
        itsCommand = make_pair(CommandId(-1), shared_ptr<Command>());
        return;
    }
    
    string type = itsQueryResult[0]["_type"].as<string>();

    // Get the command-id.
    CommandId id = itsQueryResult[0]["_id"].as<int32>();

    LOG_DEBUG_STR("Next command: " << type << " (id=" << id << ")");

    // Only steps have names, so this is a way to differentiate between ordinary
    // commands and steps.
    bool isStep = !itsQueryResult[0]["_name"].is_null();

    // Get command arguments.
    ParameterSet ps;
    ps.adoptBuffer(itsQueryResult[0]["_args"].as<string>(string()));

    if(isStep)
    {
        // Create a new Step.
        string name = itsQueryResult[0]["_name"].as<string>();
        shared_ptr<Command> cmd = Step::create(name, ps, 0);
        itsCommand = make_pair(id, cmd);
    }
    else
    {
        // Here we handle all other commands. They can be constructed using
        // the CommandFactory and initialized using read().
        shared_ptr<Command> cmd(CommandFactory::instance().create(type));
        if(!cmd)
        {
            THROW(TranslationException, "Failed to create a '" << type << "'"
                << " command object");
        }
        
        cmd->read(ps);
        itsCommand = make_pair(id, cmd);
    }
}


PQGetCommandStatus::PQGetCommandStatus(const CommandId &id, int32 &status,
    SharedState::WorkerType &target, CommandStatus &commandStatus)
    :   pqxx::transactor<>("PQGetCommandStatus"),
        itsCommandId(id),
        itsStatus(status),
        itsTarget(target),
        itsCommandStatus(commandStatus)
{
}
        
void PQGetCommandStatus::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_command_status("
        << itsCommandId << ")";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQGetCommandStatus::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(itsStatus == 0)
    {
        if(itsQueryResult[0]["_target"].is_null())
        {
            itsTarget = SharedState::N_WorkerType;
        }
        else
        {
            string tmp = itsQueryResult[0]["_target"].as<string>();
            if(tmp.size() != 1)
            {
                THROW(TranslationException, "Invalid worker type: " << tmp);
            }

            if(tmp[0] == 'K')
            {
                itsTarget = SharedState::KERNEL;
            }
            else if(tmp[0] == 'S')
            {
                itsTarget = SharedState::SOLVER;
            }
            else
            {
                THROW(TranslationException, "Invalid worker type: " << tmp);
            }
        }

        itsCommandStatus.nResults =
            itsQueryResult[0]["_result_count"].as<size_t>();
        itsCommandStatus.nFail = itsQueryResult[0]["_fail_count"].as<size_t>();
    }
}


PQGetResults::PQGetResults(const CommandId &id,
    vector<pair<ProcessId, CommandResult> > &results)
    :   pqxx::transactor<>("PQGetResults"),
        itsCommandId(id),
        itsResults(results)
{
}
    
void PQGetResults::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_results(" << itsCommandId << ")";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQGetResults::on_commit()
{
    itsResults.clear();
    
    pqxx::result::const_iterator rowIt = itsQueryResult.begin();
    const pqxx::result::const_iterator rowItEnd = itsQueryResult.end();
    while(rowIt != rowItEnd)
    {
        pair<ProcessId, CommandResult> res;

        try
        {
            res.first = ProcessId(rowIt["hostname"].as<string>(),
                rowIt["pid"].as<int64>());
            res.second = CommandResult(rowIt["result_code"].as<int>(),
                rowIt["message"].as<string>());
        }
        catch(std::logic_error &e)
        {
            THROW(TranslationException, "Unable to parse result.");
        }

        itsResults.push_back(res);
        ++rowIt;
    }                
}

} //# namespace BBS
} //# namespace LOFAR
