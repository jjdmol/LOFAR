//# PValueIterator.h: Iterators that iterate over the perturbed values of a (set
//# of) result(s).
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

#ifndef EXPR_PVALUEITERATOR_H
#define EXPR_PVALUEITERATOR_H

// \file
// Iterators that iterate over the perturbed values of a (set of) result(s).

#include <BBSKernel/Expr/Result.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PValueConstIterator
{
public:
    PValueConstIterator()
        : itsResultRep(0)
    {
    }

    // A pointer to result is stored (instead of a reference) such that
    // PValueIterator is Assignable. This allows it to be stored in std
    // containers.
    PValueConstIterator(const Result &result)
        : itsResultRep(result.itsRep)
    {
        ASSERT(itsResultRep);
        itsIter = itsResultRep->itsPValueMap.begin();
    }

    void rewind()
    { itsIter = itsResultRep->itsPValueMap.begin(); }
    
    bool atEnd() const
    { return itsIter == itsResultRep->itsPValueMap.end(); }
    
    const PValueKey &key() const
    { return itsIter->first; }
    
    const Matrix &value() const
    { return itsResultRep->itsPValues[itsIter->second]; }

    void next()
    { ++itsIter; }

private:
    const ResultRep                       *itsResultRep;
    ResultRep::PValueMap::const_iterator  itsIter;
};

class PValueIterator
{
public:
    PValueIterator()
        : itsResultRep(0)
    {
    }

    // A pointer to result is stored (instead of a reference) such that
    // PValueIterator is Assignable. This allows it to be stored in std
    // containers.
    PValueIterator(Result &result)
        : itsResultRep(result.itsRep)
    {
        ASSERT(itsResultRep);
        itsIter = itsResultRep->itsPValueMap.begin();
    }

    void rewind()
    { itsIter = itsResultRep->itsPValueMap.begin(); }
    
    bool atEnd() const
    { return itsIter == itsResultRep->itsPValueMap.end(); }
    
    const PValueKey &key() const
    { return itsIter->first; }
    
    Matrix &value() const
    { return itsResultRep->itsPValues[itsIter->second]; }
    
    void next()
    { ++itsIter; }

private:
    ResultRep                        *itsResultRep;
    ResultRep::PValueMap::iterator   itsIter;
};

template <size_t _Count>
class PValueSetIterator
{
public:
    PValueSetIterator(const Result* const (&results)[_Count])
        :   itsResults(results),
            itsCurrKey(std::numeric_limits<size_t>::max(), 0),
            itsAtEnd(false)
    {
        for(size_t i = 0; i < _Count; ++i)
        {
            itsIterators[i] = PValueConstIterator(*itsResults[i]);
        }
        updateState();
    }

    void rewind()
    {
        for(size_t i = 0; i < _Count; ++i)
        {
            itsIterators[i].rewind();
        }
    }

    bool atEnd() const
    { return itsAtEnd; }
    
    bool hasPValue(size_t n) const
    { return !itsIterators[n].atEnd() && itsIterators[n].key() == itsCurrKey; }
    
    const PValueKey &key() const
    { return itsCurrKey; }
    
    const Matrix &value(size_t n) const
    {
        return hasPValue(n) ? itsIterators[n].value()
            : itsResults[n]->getValue();
    }

    void next()
    {
        for(size_t i = 0; i < _Count; ++i)
        {
            if(hasPValue(i))
            {
                itsIterators[i].next();
            }
        }
        updateState();
    }

private:
    void updateState()
    {
        itsAtEnd = true;
        itsCurrKey = PValueKey(std::numeric_limits<size_t>::max(), 0);

        for(size_t i = 0; i < _Count; ++i)
        {
            if(!itsIterators[i].atEnd() && itsIterators[i].key().parmId
                < itsCurrKey.parmId)
            {
                itsCurrKey = itsIterators[i].key();
                itsAtEnd = false;
            }
        }
    }

    const Result* const     (&itsResults)[_Count];
    PValueConstIterator     itsIterators[_Count];
    PValueKey               itsCurrKey;
    bool                    itsAtEnd;
};

class PValueSetIteratorDynamic
{
public:
    PValueSetIteratorDynamic(const vector<const Result*> &results)
        :   itsResults(results),
            itsCurrKey(std::numeric_limits<size_t>::max(), 0),
            itsAtEnd(false)
    {
        itsIterators.resize(itsResults.size());
        for(size_t i = 0; i < itsResults.size(); ++i)
        {
            itsIterators[i] = PValueConstIterator(*itsResults[i]);
        }
        updateState();
    }

    void rewind()
    {
        for(size_t i = 0; i < itsIterators.size(); ++i)
        {
            itsIterators[i].rewind();
        }
    }

    bool atEnd() const
    { return itsAtEnd; }
    
    bool hasPValue(size_t n) const
    { return !itsIterators[n].atEnd() && itsIterators[n].key() == itsCurrKey; }
    
    const PValueKey &key() const
    { return itsCurrKey; }
    
    const Matrix &value(size_t n) const
    {
        return hasPValue(n) ? itsIterators[n].value()
            : itsResults[n]->getValue();
    }

    void next()
    {
        for(size_t i = 0; i < itsIterators.size(); ++i)
        {
            if(hasPValue(i))
            {
                itsIterators[i].next();
            }
        }
        updateState();
    }

private:
    void updateState()
    {
        itsAtEnd = true;
        itsCurrKey = PValueKey(std::numeric_limits<size_t>::max(), 0);

        for(size_t i = 0; i < itsIterators.size(); ++i)
        {
            if(!itsIterators[i].atEnd() && itsIterators[i].key().parmId
                < itsCurrKey.parmId)
            {
                itsCurrKey = itsIterators[i].key();
                itsAtEnd = false;
            }
        }
    }

    vector<const Result*>    itsResults;
    vector<PValueConstIterator> itsIterators;
    PValueKey                   itsCurrKey;
    bool                        itsAtEnd;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
