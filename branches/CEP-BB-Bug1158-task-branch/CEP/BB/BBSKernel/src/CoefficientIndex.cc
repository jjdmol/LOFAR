//# CoefficientIndex.cc:
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

#include <BBSKernel/CoefficientIndex.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobSTL.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;
using LOFAR::operator>>;


const CoeffInterval &CoeffIndex::insert(const string &parm, uint32 length)
{
    pair<IndexType::iterator, bool> result =
        itsIntervals.insert(make_pair(parm,
            CoeffInterval(itsCount, length)));

    DBGASSERT(result.second || (result.first)->second.length == length);

    if(result.second)
    {
        itsCount += length;
    }

    return (result.first)->second;
}


void CoeffIndex::set(const string &parm, const CoeffInterval &interval)
{
    pair<IndexType::iterator, bool> result =
        itsIntervals.insert(make_pair(parm, interval));

    DBGASSERT(result.second || (result.first)->second == interval);

    if(result.second)
    {
        itsCount += interval.length;
    }
}


void CoeffIndex::set(const string &parm, uint32 start, uint32 length)
{
    pair<IndexType::iterator, bool> result =
        itsIntervals.insert(make_pair(parm, CoeffInterval(start, length)));

    DBGASSERT(result.second || ((result.first)->second.start == start
        && (result.first)->second.length == length));

    if(result.second)
    {
        itsCount += length;
    }
}

ostream &operator<<(ostream &out, const CoeffIndex &obj)
{
    for(CoeffIndex::const_iterator it = obj.begin(), end = obj.end();
        it != end;
        ++it)
    {
        out << it->first;

        if(it->second.length == 0)
        {
            out << ":N/A ";
        }
        else
        {
            out << ":" << it->second.start << "-" << it->second.start
                + it->second.length - 1 << " ";
        }
    }
    
    return out;
}
                    

BlobIStream &operator>>(BlobIStream &in, CoeffIndex &obj)
{
    return (in >> obj.itsCount >> obj.itsIntervals);
}


BlobOStream &operator<<(BlobOStream &out, const CoeffIndex &obj)
{
    return (out << obj.itsCount << obj.itsIntervals);
}


BlobIStream &operator>>(BlobIStream &in, CoeffInterval &obj)
{
    return (in >> obj.start >> obj.length);
}


BlobOStream &operator<<(BlobOStream &out, const CoeffInterval &obj)
{
    return (out << obj.start << obj.length);
}

} // namespace BBS
} // namespace LOFAR
