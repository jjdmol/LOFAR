//# Result.cc: The result of the evaluation of an expression on a given request
//# grid.
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
#include <BBSKernel/MNS/MeqResult.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

ResultRep::ResultRep()
    : itsRefCount(0)
{
}

ResultRep::~ResultRep()
{
}

void ResultRep::setPerturbedValue(const PValueKey &key,
    const Matrix &value)
{
    pair<PValueMap::iterator, bool> status =
        itsPValueMap.insert(make_pair(key, itsPValues.size()));

    if(status.second)
    {
        itsPValues.push_back(value);
    }
    else
    {
        itsPValues[status.first->second] = value;
    }        
}

Matrix &ResultRep::getPerturbedValueRW(const PValueKey &key)
{
    pair<PValueMap::iterator, bool> status =
        itsPValueMap.insert(make_pair(key, itsPValues.size()));

    if(status.second)
    {
        itsPValues.push_back(Matrix());
    }
    
    return itsPValues[status.first->second];
}

const Matrix &ResultRep::getPerturbedValue(const PValueKey &key) const
{
    PValueMap::const_iterator it = itsPValueMap.find(key);
    return it == itsPValueMap.end() ? itsValue : itsPValues[it->second];
}

bool ResultRep::isPerturbed(const PValueKey &key) const
{
    PValueMap::const_iterator it = itsPValueMap.find(key);
    return (it != itsPValueMap.end());
}

void ResultRep::clearPerturbedValues()
{
    itsPValues.clear();
    itsPValueMap.clear();
}

// -------------------------------------------------------------------------- //

Result::Result()
    : itsRep(0)
{
}

Result::~Result()
{
    ResultRep::unlink(itsRep);
}

Result::Result(const Result &other)
    : itsRep(other.itsRep)
{
    if(itsRep != 0)
    {
        itsRep->link();
    }        
}

Result &Result::operator=(const Result &other)
{
    if(this != &other)
    {
        ResultRep::unlink(itsRep);
        itsRep = other.itsRep;
        if(itsRep != 0)
        {
            itsRep->link();
        }
    }
    
    return *this;
}    

void Result::init()
{
    if(itsRep == 0)
    {
        itsRep = new ResultRep();
        itsRep->link();
    }
}

} //# namespace BBS
} //# namespace LOFAR
