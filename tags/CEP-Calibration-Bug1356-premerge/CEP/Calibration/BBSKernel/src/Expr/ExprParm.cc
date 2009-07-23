//# ExprParm.cc: Parameter that can be used in an expression.
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
#include <BBSKernel/Expr/ExprParm.h>
//#include <BBSKernel/Expr/Cache.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

ExprParm::ExprParm(const ParmProxy::ConstPtr &parm)
    :   Expr<Scalar>(),
        itsParm(parm),
        itsPValueFlag(false)
{
}

void ExprParm::setPValueFlag()
{
    itsPValueFlag = true;
}

void ExprParm::clearPValueFlag()
{
    itsPValueFlag = false;
}

void ExprParm::updateSolvables(set<PValueKey> &solvables) const
{
    if(getPValueFlag())
    {
        // TODO: The following is incorrect; need to know the id's of all the
        // _solvable_ coefficients (get the mask?).
        const size_t nCoeff = itsParm->getCoeffCount();
        for(size_t i = 0; i < nCoeff; ++i)
        {
            solvables.insert(PValueKey(itsParm->getId(), i));
        }
    }

    itsSolvables.clear();
    itsSolvables.insert(itsSolvables.begin(), solvables.begin(), solvables.end());
}

const Scalar ExprParm::evaluateExpr(const Request &request, Cache &cache) const
{
    // Get the result from the Parm.
    vector<casa::Array<double> > buffers;
    itsParm->getResult(buffers, request.getGrid(), getPValueFlag()); //false);
    ASSERT(buffers.size() > 0);

    // Transform into an ExprResult.
    ValueSet result;

    bool deleteStorage = false;
    const double *storage = 0;

    // Copy the main value.
    storage = buffers[0].getStorage(deleteStorage);
    ASSERT(storage);
    if(buffers[0].nelements() == 1)
    {
        result.assign(Matrix(*storage));
    }
    else
    {
        const casa::IPosition &shape = buffers[0].shape();
        DBGASSERT(static_cast<unsigned int>(shape(0)) == request[FREQ]->size()
            && static_cast<unsigned int>(shape(1)) == request[TIME]->size());
        result.assign(Matrix(storage, shape(0), shape(1)));
    }
    buffers[0].freeStorage(storage, deleteStorage);

    // Copy the perturbed values if necessary.
    // TODO: re-enable flag in request to avoid generating partials even if
    // some parms are set to solvable?
    if(getPValueFlag())// && request.getPValueFlag())
    {
        // TODO: check correctness when some coefficients are masked
        // non-solvable.
        const size_t nCoeff = itsParm->getCoeffCount();
        ASSERT(buffers.size() == nCoeff + 1);

        for(size_t i = 0; i < nCoeff; ++i)
        {
            storage = buffers[i + 1].getStorage(deleteStorage);
            ASSERT(storage);
            if(buffers[i + 1].nelements() == 1)
            {
                result.assign(PValueKey(itsParm->getId(), i),
                    Matrix(storage[0]));
            }
            else
            {
                const casa::IPosition &shape = buffers[i + 1].shape();
                result.assign(PValueKey(itsParm->getId(), i), Matrix(storage,
                    shape(0), shape(1)));
            }
            buffers[i + 1].freeStorage(storage, deleteStorage);
        }
    }

    Scalar scalar;
//    if(itsParm->getId() == 32)
//    {
//        LOG_DEBUG_STR("Setting flags of parm: " << itsParm->getName());
//        tmp.setFlags(FlagArray(3u));
//    }
//
    scalar.setValueSet(result);
    return scalar;
}

} //# namespace BBS
} //# namespace LOFAR
