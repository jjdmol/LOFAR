//# CalSessionTransactors.cc: Transactors responsible for calling stored
//# procedures and for all required parameter/return value conversions.
//#
//# Copyright (C) 2009
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
#include <BBSControl/CalSessionTransactors.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Step.h>

#include <LMWCommon/VdsDesc.h>
#include <LMWCommon/VdsPartDesc.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

PQInitSession::PQInitSession(const string &key, int32 &id)
    :   pqxx::transactor<>("PQInitSession"),
        itsKey(key),
        itsId(&id)
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
    *itsId = itsQueryResult[0]["_id"].as<int32>();
}


PQSetState::PQSetState(int32 id, const ProcessId &pid, CalSession::State state,
    int32 &status)
    :   pqxx::transactor<>("PQSetState"),
        itsId(id),
        itsProcessId(pid),
        itsState(state),
        itsStatus(&status)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetState::PQGetState(int32 id, int32 &status, CalSession::State &state)
    :   pqxx::transactor<>("PQGetState"),
        itsId(id),
        itsStatus(&status),
        itsState(&state)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        int32 state = itsQueryResult[0]["_state"].as<int32>();
        if(state < CalSession::FAILED || state >= CalSession::N_State)
        {
            THROW(TranslationException, "Invalid session state: " << state);
        }

        *itsState = static_cast<CalSession::State>(state);
    }
}


PQSetAxisTime::PQSetAxisTime(int32 id, const ProcessId &pid,
    const Axis::ShPtr &axis, int32 &status)
    :   pqxx::transactor<>("PQSetAxisTime"),
        itsId(id),
        itsProcessId(pid),
        itsAxis(axis),
        itsStatus(&status)
{
}

void PQSetAxisTime::operator()(argument_type &transaction)
{
    // The E' syntax avoids a warning on non-standard use of \\ in a string
    // literal. The E'' syntax is postgresql specific (i.e. non-standard) but
    // avoids the warning. It explicitly enables the use of \\ in a string
    // literal. The best solution would be to change the way escaping is done,
    // but that is part of the libpqxx implementation.

    ostringstream query;
    query << "SELECT * FROM blackboard.set_axis_time("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ",E'"
        << pack_vector(transaction, itsAxis->lowers()) << "',E'"
        << pack_vector(transaction, itsAxis->uppers()) << "')";

    LOG_DEBUG_STR("Query string size (MB): " << query.str().size()
        / (1024.0 * 1024.0));

    itsQueryResult = transaction.exec(query.str());
}

void PQSetAxisTime::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetAxisTime::PQGetAxisTime(int32 id, int32 &status,
    Axis::ShPtr &axis)
    :   pqxx::transactor<>("PQGetAxisTime"),
        itsId(id),
        itsStatus(&status),
        itsAxis(&axis)
{
}

void PQGetAxisTime::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_axis_time(" << itsId << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetAxisTime::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        ASSERT(!itsQueryResult[0]["_axis_time_lower"].is_null());
        ASSERT(!itsQueryResult[0]["_axis_time_upper"].is_null());

        // Unpack frequency axis.
        pqxx::binarystring timeLower(itsQueryResult[0]["_axis_time_lower"]);
        pqxx::binarystring timeUpper(itsQueryResult[0]["_axis_time_upper"]);
        *itsAxis = Axis::ShPtr(new OrderedAxis(unpack_vector<double>(timeLower),
            unpack_vector<double>(timeUpper), true));
    }
}


PQSetParset::PQSetParset(int32 id, const ProcessId &pid,
    const ParameterSet &parset, int32 &status)
    :   pqxx::transactor<>("PQSetParset"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(&status)
{
    parset.writeBuffer(itsParset);
}

void PQSetParset::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.set_parset("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ",'"
        << transaction.esc(itsParset) << "')";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQSetParset::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetParset::PQGetParset(int32 id, int32 &status, ParameterSet &parset)
    :   itsId(id),
        itsStatus(&status),
        itsParset(&parset)
{
}

void PQGetParset::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_parset(" << itsId << ")";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}


void PQGetParset::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
    itsParset->adoptBuffer(itsQueryResult[0]["_parset"].as<string>());
}


PQInitWorkerRegister::PQInitWorkerRegister(int32 id, const ProcessId &pid,
    const CEP::VdsDesc &vds, bool useSolver)
    :   pqxx::transactor<>("PQInitWorkerRegister"),
        itsId(id),
        itsProcessId(pid),
        itsVdsDesc(&vds),
        itsUseSolver(useSolver)
{
}

void PQInitWorkerRegister::operator()(argument_type &transaction)
{
    ostringstream query;

    const vector<CEP::VdsPartDesc> &parts = itsVdsDesc->getParts();
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
        itsStatus(&status)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQRegisterAsKernel::PQRegisterAsKernel(int32 id, const ProcessId &pid,
    const string &filesys, const string &path, const Axis::ShPtr &freqAxis,
    const Axis::ShPtr &timeAxis, int32 &status)
    :   pqxx::transactor<>("PQRegisterAsKernel"),
        itsId(id),
        itsProcessId(pid),
        itsFilesys(filesys),
        itsPath(path),
        itsFreqAxis(freqAxis),
        itsTimeAxis(timeAxis),
        itsStatus(&status)
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
        << transaction.esc(itsPath) << "',"
        << setprecision(20)
        << itsFreqAxis->start() << ","
        << itsFreqAxis->end() << ","
        << itsTimeAxis->start() << ","
        << itsTimeAxis->end() << ",E'"
        << pack_vector(transaction, itsFreqAxis->lowers()) << "',E'"
        << pack_vector(transaction, itsFreqAxis->uppers()) << "',E'"
        << pack_vector(transaction, itsTimeAxis->lowers()) << "',E'"
        << pack_vector(transaction, itsTimeAxis->uppers()) << "')";

    LOG_DEBUG_STR("Query string size (MB): " << query.str().size()
        / (1024.0 * 1024.0));

    itsQueryResult = transaction.exec(query.str());
}

void PQRegisterAsKernel::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}

PQRegisterAsSolver::PQRegisterAsSolver(int32 id, const ProcessId &pid,
    int32 port, int32 &status)
    :   pqxx::transactor<>("PQRegisterAsSolver"),
        itsId(id),
        itsProcessId(pid),
        itsPort(port),
        itsStatus(&status)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetWorkerRegister::PQGetWorkerRegister(int32 id, vector<size_t> &count,
    vector<CalSession::Worker> &workers)
    :   pqxx::transactor<>("PQGetWorkerRegister"),
        itsId(id),
        itsCount(&count),
        itsWorkers(&workers)
{
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
    *itsCount = vector<size_t>(CalSession::N_WorkerType, 0);

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
        (*itsCount)[workerType] += 1;

        // Check if slot is empty.
        if(rowIt["hostname"].is_null() || rowIt["pid"].is_null())
        {
//            LOG_DEBUG_STR("Empty slot.");
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
                || rowIt["freq_lower"].is_null()
                || rowIt["freq_upper"].is_null()
                || rowIt["time_lower"].is_null()
                || rowIt["time_upper"].is_null())
            {
                THROW(TranslationException, "Kernel information should not be"
                    " NULL for worker " << worker.id.hostname << ":"
                    << worker.id.pid);
            }

            // Get measurement part.
            worker.filesys = rowIt["filesys"].as<string>();
            worker.path = rowIt["path"].as<string>();

            // Parse frequency and time ranges.
            worker.freqRange.start = rowIt["freq_lower"].as<double>();
            worker.freqRange.end = rowIt["freq_upper"].as<double>();
            worker.timeRange.start = rowIt["time_lower"].as<double>();
            worker.timeRange.end = rowIt["time_upper"].as<double>();
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

        itsWorkers->push_back(worker);
//        LOG_DEBUG_STR("Found worker... Type: " << type);

        ++rowIt;
    }
}


PQGetWorkerAxisFreq::PQGetWorkerAxisFreq(int32 id, const ProcessId &pid,
    int32 &status, Axis::ShPtr &axis)
    :   pqxx::transactor<>("PQGetWorkerAxisFreq"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(&status),
        itsAxis(&axis)
{
}

void PQGetWorkerAxisFreq::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_worker_axis_freq("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetWorkerAxisFreq::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        ASSERT(!itsQueryResult[0]["_axis_freq_lower"].is_null());
        ASSERT(!itsQueryResult[0]["_axis_freq_upper"].is_null());

        // Unpack frequency axis.
        pqxx::binarystring freqLower(itsQueryResult[0]["_axis_freq_lower"]);
        pqxx::binarystring freqUpper(itsQueryResult[0]["_axis_freq_upper"]);
        *itsAxis = Axis::ShPtr(new OrderedAxis(unpack_vector<double>(freqLower),
            unpack_vector<double>(freqUpper), true));
    }
}


PQGetWorkerAxisTime::PQGetWorkerAxisTime(int32 id, const ProcessId &pid,
    int32 &status, Axis::ShPtr &axis)
    :   pqxx::transactor<>("PQGetWorkerAxisTime"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(&status),
        itsAxis(&axis)
{
}

void PQGetWorkerAxisTime::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_worker_axis_time("
        << itsId << ",'"
        << transaction.esc(itsProcessId.hostname) << "',"
        << itsProcessId.pid << ")";
    LOG_DEBUG_STR("Query: " << query.str());

    itsQueryResult = transaction.exec(query.str());
}

void PQGetWorkerAxisTime::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        ASSERT(!itsQueryResult[0]["_axis_time_lower"].is_null());
        ASSERT(!itsQueryResult[0]["_axis_time_upper"].is_null());

        // Unpack frequency axis.
        pqxx::binarystring timeLower(itsQueryResult[0]["_axis_time_lower"]);
        pqxx::binarystring timeUpper(itsQueryResult[0]["_axis_time_upper"]);
        *itsAxis = Axis::ShPtr(new OrderedAxis(unpack_vector<double>(timeLower),
            unpack_vector<double>(timeUpper), true));
    }
}


PQSetWorkerIndex::PQSetWorkerIndex(int32 id, const ProcessId &pid,
    const ProcessId &worker, int32 index, int32 &status)
    :   pqxx::transactor<>("PQSetWorkerIndex"),
        itsId(id),
        itsProcessId(pid),
        itsWorker(worker),
        itsIndex(index),
        itsStatus(&status)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQPostCommand::PQPostCommand(int32 id, const ProcessId &pid,
    CalSession::WorkerType addressee, const Command &cmd, int32 &status,
    int32 &cmdId)
    :   pqxx::transactor<>("PQPostCommand"),
        itsId(id),
        itsProcessId(pid),
        itsAddressee(addressee),
        itsCommand(&cmd),
        itsStatus(&status),
        itsCommandId(&cmdId)
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

    query << ",'" << transaction.esc(toLower(itsCommand->type())) << "'";

    // Only Step and its derivatives have an attribute 'name'. As this is also
    // used to distinguish between Step (or a derived class) and Command (or a
    // derived class) we have to supply it.
    const Step *step = dynamic_cast<const Step*>(itsCommand);
    if(step)
    {
        query << ",'" << transaction.esc(step->name()) << "'";
    }
    else
    {
        query << ",NULL";
    }

    // Add the command's arguments as a parset.
    ParameterSet ps;
    query << ",'" << (ps << (*itsCommand)) << "')";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQPostCommand::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        *itsCommandId = itsQueryResult[0]["_id"].as<CommandId>();
    }
}


PQPostResult::PQPostResult(int32 id, const ProcessId &pid,
    const CommandId &cmdId, const CommandResult &cmdResult, int32 &status)
    :   pqxx::transactor<>("PQPostResult"),
        itsId(id),
        itsProcessId(pid),
        itsCommandId(cmdId),
        itsCommandResult(cmdResult),
        itsStatus(&status)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();
}


PQGetCommand::PQGetCommand(int32 id, const ProcessId &pid, int32 &status,
    pair<CommandId, shared_ptr<Command> > &cmd)
    :   pqxx::transactor<>("PQGetCommand"),
        itsId(id),
        itsProcessId(pid),
        itsStatus(&status),
        itsCommand(&cmd)
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
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus < 0)
    {
        // Failure or empty queue.
        *itsCommand = make_pair(CommandId(-1), shared_ptr<Command>());
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
        *itsCommand = make_pair(id, cmd);
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
        *itsCommand = make_pair(id, cmd);
    }
}


PQGetCommandStatus::PQGetCommandStatus(const CommandId &id, int32 &status,
    CalSession::WorkerType &addressee, CalSession::CommandStatus &commandStatus)
    :   pqxx::transactor<>("PQGetCommandStatus"),
        itsCommandId(id),
        itsStatus(&status),
        itsAddressee(&addressee),
        itsCommandStatus(&commandStatus)
{
}

void PQGetCommandStatus::operator()(argument_type &transaction)
{
    ostringstream query;
    query << "SELECT * FROM blackboard.get_command_status(" << itsCommandId
        << ")";

    LOG_DEBUG_STR(query.str());
    itsQueryResult = transaction.exec(query.str());
}

void PQGetCommandStatus::on_commit()
{
    *itsStatus = itsQueryResult[0]["_status"].as<int32>();

    if(*itsStatus == 0)
    {
        *itsAddressee = CalSession::N_WorkerType;

        if(!itsQueryResult[0]["_addressee"].is_null())
        {
            int32 addressee = itsQueryResult[0]["_addressee"].as<int32>();
            if(addressee < 0 || addressee >= CalSession::N_WorkerType)
            {
                THROW(TranslationException, "Invalid addressee: " << addressee);
            }

            *itsAddressee = static_cast<CalSession::WorkerType>(addressee);
        }

        itsCommandStatus->finished =
            itsQueryResult[0]["_result_count"].as<size_t>();
        itsCommandStatus->failed =
            itsQueryResult[0]["_fail_count"].as<size_t>();
    }
}


PQGetResults::PQGetResults(const CommandId &id,
    vector<pair<ProcessId, CommandResult> > &results)
    :   pqxx::transactor<>("PQGetResults"),
        itsCommandId(id),
        itsResults(&results)
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
    pqxx::result::const_iterator rowIt = itsQueryResult.begin();
    const pqxx::result::const_iterator rowItEnd = itsQueryResult.end();
    while(rowIt != rowItEnd)
    {
        pair<ProcessId, CommandResult> result;

        try
        {
            result.first = ProcessId(rowIt["hostname"].as<string>(),
                rowIt["pid"].as<int64>());
            result.second = CommandResult(rowIt["result_code"].as<int>(),
                rowIt["message"].as<string>());
        }
        catch(std::logic_error &e)
        {
            THROW(TranslationException, "Unable to parse result.");
        }

        itsResults->push_back(result);
        ++rowIt;
    }
}

} //# namespace BBS
} //# namespace LOFAR
