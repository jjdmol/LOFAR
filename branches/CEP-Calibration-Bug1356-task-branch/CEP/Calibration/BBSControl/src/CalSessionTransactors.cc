//# CalSessionTransactors.cc: Transactors responsible for calling stored
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
#include <BBSControl/CalSessionTransactors.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Step.h>

#include <MWCommon/VdsDesc.h>
#include <MWCommon/VdsPartDesc.h>

#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

PQInitSession::PQInitSession(const string &key, int32 &id)
    :   pqxx::transactor<>("PQInitSession"),
        itsKey(key),
        itsId(id)
{
}
        
void PQInitSession::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.init_session('"
        << transaction.esc(itsKey) << "')";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQInitSession::on_commit()
{
    itsId = itsQueryResult[0]["_id"].as<int32>();
}


PQSetState::PQSetState(int32 id, const ProcessId &pid, CalSession::State state,
    int32 &status)
    :   pqxx::transactor<>("PQSetState"),
        itsId(id),
        itsProcessId(pid),
        itsState(state),
        itsStatus(status)
{
}
        
void PQSetState::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.set_state("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ","
        << static_cast<int32>(itsState) << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQSetState::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetState::PQGetState(int32 id, int32 &status, CalSession::State &state)
    :   pqxx::transactor<>("PQGetState"),
        itsId(id),
        itsStatus(status),
        itsState(state)
{
}

void PQGetState::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_state(" << itsId << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetState::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
    
    if(itsStatus == 0)
    {
        int32 state = itsQueryResult[0]["_state"].as<int32>();

        if(state < CalSession::FAILED || state >= CalSession::N_State)
        {
            THROW(TranslationException, "Invalid session state: " << state);
        }
        itsState = static_cast<CalSession::State>(state);
    }
}


PQInitWorkerRegister::PQInitWorkerRegister(int32 id, const ProcessId &pid,
    const CEP::VdsDesc &vds, bool useSolver)
    :   pqxx::transactor<>("PQInitWorkerRegister"),
        itsId(id),
        itsProcessId(pid),
        itsVdsDesc(vds),
        itsUseSolver(useSolver)
{
}    

void PQInitWorkerRegister::operator()(argument_type &transaction)
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
    const string &filesys, const string &path, const Grid &grid, int32 &status)
    :   pqxx::transactor<>("PQRegisterAsKernel"),
        itsId(id),
        itsProcessId(pid),
        itsFilesys(filesys),
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
        << transaction.esc(itsFilesys) << "','"
        << transaction.esc(itsPath) << "',E'"
        << pack_vector(transaction, itsGrid[0]->lowers()) << "',E'"
        << pack_vector(transaction, itsGrid[0]->uppers()) << "',E'"
        << pack_vector(transaction, itsGrid[1]->lowers()) << "',E'"
        << pack_vector(transaction, itsGrid[1]->uppers()) << "')";
//    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQRegisterAsKernel::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}

PQRegisterAsSolver::PQRegisterAsSolver(int32 id, const ProcessId &pid,
    int32 port, int32 &status)
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


PQGetWorkerRegister::PQGetWorkerRegister(int32 id, vector<size_t> &count,
    vector<CalSession::Worker> &workers)
    :   pqxx::transactor<>("PQGetWorkerRegister"),
        itsId(id),
        itsCount(count),
        itsWorkers(workers)
{
    ASSERT(itsCount.size() == CalSession::N_WorkerType);
}
        
void PQGetWorkerRegister::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_worker_register(" << itsId << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetWorkerRegister::on_commit()
{
    fill(itsCount.begin(), itsCount.end(), 0);
    
    pqxx::result::const_iterator rowIt = itsQueryResult.begin();
    const pqxx::result::const_iterator rowItEnd = itsQueryResult.end();
    while(rowIt != rowItEnd)
    {
        // Parse worker type.
        int32 type = rowIt["type"].as<int32>();
        if(type < CalSession::KERNEL || type >= CalSession::N_WorkerType)
        {
            THROW(TranslationException, "Invalid worker type: " << type);
        }
        CalSession::WorkerType workerType = 
            static_cast<CalSession::WorkerType>(type);

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
        CalSession::Worker worker;
        worker.id.hostname = rowIt["hostname"].as<string>();
        worker.id.pid = rowIt["pid"].as<int64>();
        worker.type = workerType;
        worker.index = rowIt["index"].as<int32>(-1);

        if(workerType == CalSession::KERNEL)
        {
            if(rowIt["path"].is_null()
                || rowIt["axis_freq_lower"].is_null()
                || rowIt["axis_freq_upper"].is_null()
                || rowIt["axis_time_lower"].is_null()
                || rowIt["axis_time_upper"].is_null())
            {
                THROW(TranslationException, "Kernel information should not be"
                    " NULL for worker " << worker.id.hostname << ":"
                    << worker.id.pid);
            }

            // Get measurement part.
            worker.filesys = rowIt["filesys"].as<string>();
            worker.path = rowIt["path"].as<string>();
            
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
            
            worker.grid = Grid(freqAxis, timeAxis);
        }
        else
        {
            if(rowIt["port"].is_null())
            {
                THROW(TranslationException, "Solver information should not be"
                    " NULL for worker " << worker.id.hostname << ":"
                    << worker.id.pid);
            }

            worker.port = rowIt["port"].as<size_t>();
        }

        itsWorkers.push_back(worker);
        LOG_DEBUG_STR("Found worker... Type: " << type);
        
        ++rowIt;
    }
}


PQSetWorkerIndex::PQSetWorkerIndex(int32 id, const ProcessId &pid,
    const ProcessId &worker, int32 index, int32 &status)
    :   pqxx::transactor<>("PQSetWorkerIndex"),
        itsId(id),
        itsProcessId(pid),
        itsWorker(worker),
        itsIndex(index),
        itsStatus(status)
{
    ASSERT(index >= 0);
}

void PQSetWorkerIndex::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.set_worker_index("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ",'"
        << transaction.esc(itsWorker.hostname) << "',"
        << itsWorker.pid << ","
        << itsIndex << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQSetWorkerIndex::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQPostCommand::PQPostCommand(int32 id, const ProcessId &pid,
    CalSession::WorkerType addressee, const Command &cmd, int32 &status,
    int32 &cmdId)
    :   pqxx::transactor<>("PQPostCommand"),
        itsId(id),
        itsProcessId(pid),
        itsAddressee(addressee),
        itsCommand(cmd),
        itsStatus(status),
        itsCommandId(cmdId)
{
}

void PQPostCommand::operator()(argument_type &transaction)
{
    ostringstream query;

    query << "SELECT * FROM blackboard.post_command("
        << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "'"
        << "," << itsProcessId.pid;
        
    if(itsAddressee != CalSession::N_WorkerType)
    {
        query << "," << static_cast<int32>(itsAddressee);
    }
    else
    {
        query << ",NULL";
    }

    query << ",'" << transaction.esc(toLower(itsCommand.type())) << "'";

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

void PQPostCommand::on_commit()
{
    itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(itsStatus == 0)
    {
        itsCommandId = itsQueryResult[0]["_id"].as<CommandId>();
    }
}


PQPostResult::PQPostResult(int32 id, const ProcessId &pid,
    const CommandId &cmdId, const CommandResult &cmdResult, int32 &status)
    :   pqxx::transactor<>("PQPostResult"),
        itsId(id),
        itsProcessId(pid),
        itsCommandId(cmdId),
        itsCommandResult(cmdResult),
        itsStatus(status)
{
}        

void PQPostResult::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.post_result("
        << itsId
        << ",'" << transaction.esc(itsProcessId.hostname) << "'"
        << "," << itsProcessId.pid
        << "," << itsCommandId
        << "," << itsCommandResult.asInt()
        << ",'" << transaction.esc(itsCommandResult.message()) << "')";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQPostResult::on_commit()
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
    CalSession::WorkerType &addressee, CommandStatus &commandStatus)
    :   pqxx::transactor<>("PQGetCommandStatus"),
        itsCommandId(id),
        itsStatus(status),
        itsAddressee(addressee),
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
        itsAddressee = CalSession::N_WorkerType;

        if(!itsQueryResult[0]["_addressee"].is_null())
        {
            int32 addressee = itsQueryResult[0]["_addressee"].as<int32>();
            if(addressee < 0 || addressee >= CalSession::N_WorkerType)
            {
                THROW(TranslationException, "Invalid addressee: " << addressee);
            }

            itsAddressee = static_cast<CalSession::WorkerType>(addressee);
        }

        itsCommandStatus.finished =
            itsQueryResult[0]["_result_count"].as<size_t>();
        itsCommandStatus.failed = itsQueryResult[0]["_fail_count"].as<size_t>();
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
