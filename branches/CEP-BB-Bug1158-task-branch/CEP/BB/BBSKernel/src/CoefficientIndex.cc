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

// -------------------------------------------------------------------------- //
    void CellCoeffIndex::setInterval(uint32 parmId,
        const CoeffInterval &interval)
    {
        pair<IndexType::iterator, bool> result =
            itsIntervals.insert(make_pair(parmId, interval));

        DBGASSERT(result.second || (result.first)->second == interval);

        if(result.second)
        {
            itsCount += interval.length;
        }
    }


    const CoeffInterval &CellCoeffIndex::setInterval(uint32 parmId,
        uint32 length)
    {
        pair<IndexType::iterator, bool> result =
            itsIntervals.insert(make_pair(parmId,
                CoeffInterval(itsCount, length)));

        DBGASSERT(result.second || (result.first)->second.length == length);

        if(result.second)
        {
            itsCount += length;
        }

        return (result.first)->second;
    }


    pair<uint32, bool> CoefficientIndex::insertParm(const string &name)
    {
        pair<ParmIndexType::iterator, bool> result =
            itsParameters.insert(make_pair(name, itsParameters.size()));

        return make_pair((result.first)->second, result.second);
    }

    
    pair<uint32, bool> CoefficientIndex::findParm(const string &name) const
    {
        ParmIndexType::const_iterator it = itsParameters.find(name);
            
        const bool found = (it != itsParameters.end());
        return make_pair((found ? it->second : 0), found);
    }
// -------------------------------------------------------------------------- //


// -------------------------------------------------------------------------- //
    ostream &operator<<(ostream &out, const CoefficientIndex &obj)
    {
        for(CoefficientIndex::ParmIndexType::const_iterator it =
            obj.beginParm(), end = obj.endParm();
            it != end;
            ++it)
        {
            out << "[" << it->second << "] " << it->first << endl;
        }

        for(CoefficientIndex::CoeffIndexType::const_iterator it = obj.begin(),
            end = obj.end();
            it != end;
            ++it)
        {
            cout << "Cell: " << it->first << " Index: " << it->second << endl;
        }
        
        return out;
    }
                    

    ostream &operator<<(ostream &out, const CellCoeffIndex &obj)
    {
        for(CellCoeffIndex::const_iterator it = obj.begin(),
            end = obj.end();
            it != end;
            ++it)
        {
            if(it->second.length == 0)
            {
                out << it->first << ":N/A ";
            }
            else
            {
                out << it->first << ":" << it->second.start << "-"
                    << it->second.start + it->second.length - 1 << " ";
            }
        }

        return out;
    }
// -------------------------------------------------------------------------- //


// -------------------------------------------------------------------------- //
    BlobIStream &operator>>(BlobIStream &in, CoefficientIndex &obj)
    {
        return (in >> obj.itsParameters >> obj.itsCellCoeffIndices);
    }

    
    BlobOStream &operator<<(BlobOStream &out, const CoefficientIndex &obj)
    {
        return (out << obj.itsParameters << obj.itsCellCoeffIndices);
    }


    BlobIStream &operator>>(BlobIStream &in, CellCoeffIndex &obj)
    {
        return (in >> obj.itsCount >> obj.itsIntervals);
    }


    BlobOStream &operator<<(BlobOStream &out, const CellCoeffIndex &obj)
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
// -------------------------------------------------------------------------- //

} // namespace BBS
} // namespace LOFAR
