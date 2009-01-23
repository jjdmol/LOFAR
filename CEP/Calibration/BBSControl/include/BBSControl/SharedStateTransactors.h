//# SharedStateTransactors.h: Transactors responsible for calling stored
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

#ifndef LOFAR_BBSCONTROL_SHAREDSTATETRANSACTORS_H
#define LOFAR_BBSCONTROL_SHAREDSTATETRANSACTORS_H

// \file
// Transactors responsible for calling stored procedures and for all required
// parameter/return value conversions.


// Have to pass arguments by non-const ref. because functor may be copied.
// Let all pqxx exception propagate.
// Convert error codes (business logic) into either return values or exceptions. -> export them to the outside of the transactor.
// throw a TranslationException when encoutering invalid values.

#if defined(HAVE_PQXX)
# include <pqxx/transactor>
# include <pqxx/binarystring>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <BBSControl/SharedState.h>

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

#include <Common/DataFormat.h>
#include <Common/DataConvert.h>

namespace LOFAR
{
//namespace CEP
//{
//    class VdsDesc;
//}
namespace BBS
{

// \addtogroup BBSControl
// @{

class PQInitSharedState: public pqxx::transactor<>
{
public:
    PQInitSharedState(const string &key, int32 &id);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const string    &itsKey;
    int32           &itsId;
    pqxx::result    itsQueryResult;
};

class PQSetRunState: public pqxx::transactor<>
{
public:
    PQSetRunState(int32 id, const ProcessId &pid, int32 &status,
        SharedState::RunState state);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                 itsId;
    const ProcessId             &itsProcessId;
    int32                       &itsStatus;
    const SharedState::RunState itsRunState;
    pqxx::result                itsQueryResult;
};

class PQGetRunState: public pqxx::transactor<>
{
public:
    PQGetRunState(int32 id, int32 &status, SharedState::RunState &state);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32             itsId;
    int32                   &itsStatus;
    SharedState::RunState   &itsRunState;
    pqxx::result            itsQueryResult;
};

class PQInitRegister: public pqxx::transactor<>
{
public:
    PQInitRegister(int32 id, const ProcessId &pid, const CEP::VdsDesc &vds,
        bool useSolver);//, int &result);
    void operator()(argument_type &transaction);

private:    
    int32               itsId;
    const ProcessId     &itsProcessId;
    const CEP::VdsDesc  &itsVdsDesc;
    bool                itsUseSolver;
};

class PQRegisterAsControl: public pqxx::transactor<>
{
public:
    PQRegisterAsControl(int32 id, const ProcessId &pid, int32 &status);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32     itsId;
    const ProcessId &itsProcessId;
    int32           &itsStatus;
    pqxx::result    itsQueryResult;
};

class PQRegisterAsKernel: public pqxx::transactor<>
{
public:
    PQRegisterAsKernel(int32 id, const ProcessId &pid, const string &filesystem,
        const string &path, const Grid &grid, int32 &status);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32     itsId;
    const ProcessId &itsProcessId;
    const string    &itsFilesystem;
    const string    &itsPath;
    const Grid      &itsGrid;
    int32           &itsStatus;
    pqxx::result    itsQueryResult;
};

class PQRegisterAsSolver: public pqxx::transactor<>
{
public:
    PQRegisterAsSolver(int32 id, const ProcessId &pid, size_t port,
        int32 &status);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32     itsId;
    const ProcessId &itsProcessId;
    const size_t    itsPort;
    int32           &itsStatus;
    pqxx::result    itsQueryResult;
};

class PQGetRegister: public pqxx::transactor<>
{
public:
    PQGetRegister(int32 id, vector<size_t> &count,
        vector<SharedState::WorkerDescriptor> &workers);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                             itsId;
    vector<size_t>                          &itsCount;
    vector<SharedState::WorkerDescriptor>   &itsWorkers;
    pqxx::result                            itsQueryResult;
};

class PQSetIndex: public pqxx::transactor<>
{
public:
    PQSetIndex(int32 id, const ProcessId &pid, const ProcessId &target,
        int32 index, int32 &status);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                 itsId;
    const ProcessId             &itsProcessId;
    const ProcessId             &itsTarget;
    const int32                 itsIndex;
    int32                       &itsStatus;
    pqxx::result                itsQueryResult;
};

class PQAddCommand: public pqxx::transactor<>
{
public:
    PQAddCommand(int32 id, const ProcessId &pid, SharedState::WorkerType target,
        const Command &cmd, int32 &status, CommandId &cmdId);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                 itsId;
    const ProcessId             &itsProcessId;
    const SharedState::WorkerType            itsTarget;
    const Command               &itsCommand;
    int32                       &itsStatus;
    CommandId                   &itsCommandId;
    pqxx::result                itsQueryResult;
};

class PQAddResult: public pqxx::transactor<>
{
public:
    PQAddResult(int32 id, const ProcessId &pid, const CommandId &id,
        const CommandResult &result, int32 &status);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                             itsId;
    const ProcessId                         &itsProcessId;
    const int32                             itsCommandId;
    const CommandResult                     &itsCommandResult;
    int32                                   &itsStatus;
    pqxx::result                            itsQueryResult;
};

class PQGetCommand: public pqxx::transactor<>
{
public:
    PQGetCommand(int32 id, const ProcessId &pid, int32 &status,
        pair<CommandId, shared_ptr<Command> > &cmd);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                             itsId;
    const ProcessId                         &itsProcessId;
    int32                                   &itsStatus;
    pair<CommandId, shared_ptr<Command> >   &itsCommand;
    pqxx::result                            itsQueryResult;
};

class PQGetCommandStatus: public pqxx::transactor<>
{
public:
    PQGetCommandStatus(const CommandId &id, int32 &status,
        SharedState::WorkerType &target, CommandStatus &commandStatus);
    void operator()(argument_type &transaction);
    void on_commit();

private:    
    const int32                             itsCommandId;
    int32                                   &itsStatus;
    SharedState::WorkerType                 &itsTarget;    
    CommandStatus                           &itsCommandStatus;
    pqxx::result                            itsQueryResult;
};

class PQGetResults: public pqxx::transactor<>
{
public:
    PQGetResults(const CommandId &id,
        vector<pair<ProcessId, CommandResult> > &results);
    void operator()(argument_type &transaction);
    void on_commit();

private:
    const int32                             itsCommandId;
    vector<pair<ProcessId, CommandResult> > &itsResults;
    pqxx::result                            itsQueryResult;
};

//# Function templates to pack/unpack a vector of (builtin) types to/from a 
//# Postgres binary string literal (BYTEA).
template<typename T>
inline string pack_vector(pqxx::transaction_base &transaction,
    const vector<T> &in)
{
    uchar format = static_cast<uchar>(LOFAR::dataFormat());

    //# Construct a properly escaped string that consists of one uchar that
    //# represents the host endianness, followed by the data.
    return transaction.esc_raw(&format, 1)
        + transaction.esc_raw(reinterpret_cast<const uchar*>(&in[0]),
            sizeof(T) * in.size());
}

template<typename T>
inline vector<T> unpack_vector(const pqxx::binarystring &in)
{
    ASSERT((in.size() - 1) % sizeof(T) == 0);

    const uchar *bytes = in.data();
    const size_t size = (in.size() - 1) / sizeof(T);
    vector<T> result(size);

    //# Copy data as elements of type T.
    const T *data = reinterpret_cast<const T*>(&bytes[1]);
    for(size_t i = 0; i < size; ++i)
        result[i] = data[i];

    //# Byte swap the result if necessary.
    if(bytes[0] != static_cast<uchar>(LOFAR::dataFormat()))
        dataConvert(static_cast<DataFormat>(bytes[0]),
            &result[0],
            result.size());

    return result;
}

//# Template specialization for vector<bool>. Needed because vector<bool> does 
//# not satisfy all the requirements of an STL container. In particular, it uses 
//# proxies to represent access to individual bits, so &(in[0]) does not work.
template<>
inline string pack_vector<>(pqxx::transaction_base &transaction,
    const vector<bool> &in)
{
    //# The size of a bool may vary, so copy the contents of the input vector
    //# to a type of standard size (sizeof(uchar) == 1).
    vector<uchar> tmp(in.size());
    copy(in.begin(), in.end(), tmp.begin());

    return transaction.esc_raw(&tmp[0], tmp.size());
}


template<>
inline vector<bool> unpack_vector<>(const pqxx::binarystring &in)
{
    // Endianess specifier is not needed because sizeof(uchar) == 1.
    const uchar *bytes = in.data();
    vector<bool> result(in.size());

    for(size_t i = 0; i < in.size(); ++i)
        result[i] = static_cast<bool>(bytes[i]);

    return result;
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
