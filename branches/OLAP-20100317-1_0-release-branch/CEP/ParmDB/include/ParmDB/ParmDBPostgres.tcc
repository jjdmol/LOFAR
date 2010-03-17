//# ParmDBPostgres.tcc: Utility function templates for Postgres ParmDB 
//# implementation.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_PARMDB_PARMDBPOSTGRES_TCC
#define LOFAR_PARMDB_PARMDBPOSTGRES_TCC

#if defined(HAVE_PQXX)
# include <pqxx/transactor>
# include <pqxx/binarystring>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <Common/DataFormat.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace ParmDB
{

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
    uchar format = static_cast<uchar>(LOFAR::dataFormat());

    //# Copy the contents of the input vector.
    uchar tmp[in.size()];
    for(size_t i = 0; i < in.size(); ++i)
        tmp[i] = static_cast<uchar>(in[i]);

    return transaction.esc_raw(&format, 1)
        + transaction.esc_raw(tmp, sizeof(bool) * in.size());
}


//# TODO: sizeof(bool) == 1 cannot assumed to be 1!
template<>
inline vector<bool> unpack_vector<>(const pqxx::binarystring &in)
{
    ASSERT(sizeof(bool) == 1);
    ASSERT((in.size() - 1) % sizeof(bool) == 0);

    const uchar *bytes = in.data();
    const size_t size = (in.size() - 1) / sizeof(bool);
    vector<bool> result(size);

    const bool *data = reinterpret_cast<const bool*>(&bytes[1]);
    for(size_t i = 0; i < size; ++i)
        result[i] = data[i];

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
