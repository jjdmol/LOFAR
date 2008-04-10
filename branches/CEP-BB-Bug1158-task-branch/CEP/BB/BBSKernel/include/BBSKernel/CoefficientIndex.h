//# CoefficientIndex.h:
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


#ifndef LOFAR_BB_BBSKERNEL_COEFFICIENTINDEX_H
#define LOFAR_BB_BBSKERNEL_COEFFICIENTINDEX_H

#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>
#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
class BlobIStream;
class BlobOStream;

namespace BBS
{
    class CoeffInterval
    {
    public:
        CoeffInterval()
            :   start(0),
                length(0)
        {
        }

        CoeffInterval(uint32 s, uint32 l)
            :   start(s),
                length(l)
        {
        }

        bool operator==(const CoeffInterval &other) const
        {
            return (start == other.start && length == other.length);
        }

        uint32  start;
        uint32  length;
    };


    class CellCoeffIndex
    {
    private:
        typedef map<uint32, CoeffInterval> IndexType;

    public:
        typedef IndexType::const_iterator const_iterator;

        CellCoeffIndex()
//            :   itsCellId(0),
            : itsCount(0)
        {
        }
/*
        CellCoeffIndex(uint32 cellId)
            :   itsCellId(cellId),
                itsCount(0)
        {
        }
*/
        const CoeffInterval &setInterval(uint32 parmId, uint32 length);
        void setInterval(uint32 parmId, const CoeffInterval &interval);

/*
        uint32 getCellId() const
        {
            return itsCellId;
        }
*/
        uint32 getCount() const
        {
            return itsCount;
        }

        const_iterator begin() const
        {
            return itsIntervals.begin();
        }

        const_iterator end() const
        {
            return itsIntervals.end();
        }

        // iostream I/O
        friend ostream &operator<<(ostream &out,
            const CellCoeffIndex &obj);

        // BlobStream I/O
        friend BlobIStream &operator>>(BlobIStream &in,
            CellCoeffIndex &obj);
        friend BlobOStream &operator<<(BlobOStream &out,
            const CellCoeffIndex &obj);

    private:
//        uint32                              itsCellId;
        uint32                              itsCount;
        IndexType                           itsIntervals;
    };


    class CoefficientIndex
    {
    public:
        typedef shared_ptr<CoefficientIndex>    Pointer;
        typedef map<string, uint32>             ParmIndexType;
        typedef map<uint32, CellCoeffIndex>     CoeffIndexType;
        
        CoefficientIndex()
        {}

        pair<uint32, bool> insertParm(const string &name);
        pair<uint32, bool> findParm(const string &name) const;
        size_t getParmCount() const
        { return itsParameters.size(); }
        
        ParmIndexType::const_iterator beginParm() const
        { return itsParameters.begin(); }
        ParmIndexType::const_iterator endParm() const
        { return itsParameters.end(); }

//        pair<CellCoeffIndex&, bool> insertCell(uint32 id);
//        pair<CellCoeffIndex&, bool> findCell(uint32 id);
        size_t getCellCount() const
        { return itsCellCoeffIndices.size(); }

        CellCoeffIndex &operator[](uint32 id)
        { return itsCellCoeffIndices[id]; }
        
        CoeffIndexType::const_iterator begin() const
        { return itsCellCoeffIndices.begin(); }
        CoeffIndexType::const_iterator end() const
        { return itsCellCoeffIndices.end(); }
         

        // iostream I/O
        friend ostream &operator<<(ostream &out, const CoefficientIndex &obj);

        // BlobStream I/O
        friend BlobIStream &operator>>(BlobIStream &in, CoefficientIndex &obj);
        friend BlobOStream &operator<<(BlobOStream &out,
            const CoefficientIndex &obj);

    private:
        // Forbid copying.
        // <group>
//        CoefficientIndex(const CoefficientIndex &other);
//        CoefficientIndex &operator=(const CoefficientIndex &other);
        // </group>

        ParmIndexType   itsParameters;
        CoeffIndexType  itsCellCoeffIndices;
    };


    // iostream I/O
    ostream &operator<<(ostream &out, const CoefficientIndex &obj);
    ostream &operator<<(ostream &out, const CellCoeffIndex &obj);

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CoefficientIndex &obj);
    BlobOStream &operator<<(BlobOStream &out, const CoefficientIndex &obj);
    BlobIStream &operator>>(BlobIStream &in, CellCoeffIndex &obj);
    BlobOStream &operator<<(BlobOStream &out, const CellCoeffIndex &obj);
    BlobIStream &operator>>(BlobIStream &in, CoeffInterval &obj);
    BlobOStream &operator<<(BlobOStream &out, const CoeffInterval &obj);

} // namespace BBS
} // namespace LOFAR

#endif
