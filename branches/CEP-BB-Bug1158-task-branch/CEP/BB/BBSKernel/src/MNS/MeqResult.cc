//# MeqResult.cc: The result of an expression for a domain.
//#
//# Copyright (C) 2002
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
#include <BBSKernel/MNS/MeqMatrixTmp.h>
//#include <BBSKernel/MNS/Pool.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>

#include <Common/lofar_algorithm.h>

namespace LOFAR
{
namespace BBS
{

/*
Pool<MeqResultRep> theirPool;
#pragma omp threadprivate(theirPool)

void *MeqResultRep::operator new(size_t)
{
  return theirPool.allocate();
}

void MeqResultRep::operator delete(void *rep)
{
  theirPool.deallocate((MeqResultRep *) rep);
}
*/


MeqResultRep::MeqResultRep(int nspid)
    :   itsCount            (0),
        itsSpidCount        (nspid),
        itsPerturbedValues  (nspid),
        itsPerturbedParms   (nspid)
{
    fill(itsPerturbedParms.begin(), itsPerturbedParms.end(),
        static_cast<const MeqParmFunklet*>(0));
}


MeqResultRep::~MeqResultRep()
{
}


void MeqResultRep::clear()
{
    itsPerturbedValues.clear();
    itsPerturbedValues.resize(itsSpidCount);

    fill(itsPerturbedParms.begin(), itsPerturbedParms.end(),
        static_cast<const MeqParmFunklet*>(0));
}

  
double MeqResultRep::getPerturbation(int i, int j) const
{
    const MeqParmFunklet *parm = getPerturbedParm(i);
    DBGASSERT(parm != 0);
    
    int coeffInx = i - parm->getPertInx();
    DBGASSERT(coeffInx >= 0 && coeffInx < parm->getFunklets()[j]->ncoeff());

    return parm->getFunklets()[j]->getPerturbation(coeffInx);
}


void MeqResultRep::setPerturbedValue(int i, const MeqMatrix &value)
{
    DBGASSERT(i < itsPerturbedValues.size());
    itsPerturbedValues[i] = value;
}


void MeqResultRep::setPerturbedParm(int i, const MeqParmFunklet *parm)
{
    DBGASSERT(i < itsPerturbedValues.size());
    itsPerturbedParms[i] = parm;
}


void MeqResultRep::show (ostream& os) const
{
    os << "Value: " << itsValue << endl;
    
    for(int i = 0; i < itsSpidCount; ++i)
    {
        if(isDefined(i))
        {
            os << "Perturbed parm: " << i << endl;
            os << "   " << (itsPerturbedValues[i] - itsValue) << endl;
        }
    }
}


// -----------------------------------------------------------------------------
MeqResult::MeqResult(int nspid)
    :   itsRep(new MeqResultRep(nspid))
{
    itsRep->link();
}


MeqResult::MeqResult(const MeqResult &that)
    :   itsRep(that.itsRep)
{
    if(itsRep != 0)
    {
        itsRep->link();
    }
}


MeqResult& MeqResult::operator=(const MeqResult &that)
{
    if(this != &that)
    {
        MeqResultRep::unlink(itsRep);
        itsRep = that.itsRep;
        if(itsRep != 0)
        {
            itsRep->link();
        }
    }
    return *this;
}

} // namespace BBS
} // namespace LOFAR
