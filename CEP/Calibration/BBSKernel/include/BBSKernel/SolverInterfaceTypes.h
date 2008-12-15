//# SolverInterfaceTypes.h: 
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

#ifndef LOFAR_BB_BBSKERNEL_SOLVERINTERFACETYPES_H
#define LOFAR_BB_BBSKERNEL_SOLVERINTERFACETYPES_H

#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

#include <scimath/Fitting/LSQFit.h>

    
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


    class CoeffIndex
    {
    private:
        typedef map<string, CoeffInterval>  IndexType;
    
    public:
        typedef IndexType::const_iterator   const_iterator;

        CoeffIndex()
            : itsCount(0)
        {}

        void clear()
        {
            itsCount = 0;
            itsIntervals.clear();
        }
        
        const CoeffInterval &insert(const string &parm, uint32 length);
        void set(const string &parm, const CoeffInterval &interval);
        void set(const string &parm, uint32 start, uint32 length);

        const_iterator find(const string &parm) const
        { return itsIntervals.find(parm); }
        const_iterator begin() const
        { return itsIntervals.begin(); }
        const_iterator end() const
        { return itsIntervals.end(); }

        size_t getParmCount() const
        { return itsIntervals.size(); }
        
        size_t getCoeffCount() const
        { return itsCount; }

    private:
        friend BlobIStream &operator>>(BlobIStream &in, CoeffIndex &obj);
        friend BlobOStream &operator<<(BlobOStream &out, const CoeffIndex &obj);

        uint32      itsCount;
        IndexType   itsIntervals;
    };

    // iostream I/O
    ostream &operator<<(ostream &out, const CoeffIndex &obj);

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CoeffIndex &obj);
    BlobOStream &operator<<(BlobOStream &out, const CoeffIndex &obj);
    BlobIStream &operator>>(BlobIStream &in, CoeffInterval &obj);
    BlobOStream &operator<<(BlobOStream &out, const CoeffInterval &obj);


// -----------------------------------------------------------------------------
    class CellCoeff
    {
    public:
        CellCoeff()
            : id(0)
        {}
        
        CellCoeff(uint32 id)
            : id(id)
        {}

        uint32          id;
        vector<double>  coeff;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellCoeff &obj);
    BlobOStream &operator<<(BlobOStream &out, const CellCoeff &obj);


// -----------------------------------------------------------------------------
    class CellEquation
    {
    public:
        CellEquation()
            : id(0)
        {}
        
        CellEquation(uint32 id)
            : id(id)
        {}

        uint32          id;
        casa::LSQFit    equation;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellEquation &obj);
    BlobOStream &operator<<(BlobOStream &out, const CellEquation &obj);


// -----------------------------------------------------------------------------
    class CellSolution
    {
    public:
        CellSolution()
            : id(0)
        {}
        
        CellSolution(uint32 id)
            : id(id)
        {}

        uint32          id;
        vector<double>  coeff;
        uint32          result;
        string          resultText;
        uint32          rank;
        double          chiSqr;
        double          lmFactor;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellSolution &obj);
    BlobOStream &operator<<(BlobOStream &out, const CellSolution &obj);

} // namespace BBS
} // namespace LOFAR

#endif

