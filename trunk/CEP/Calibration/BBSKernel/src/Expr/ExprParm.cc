//# ExprParm.cc: Parameter that can be used in an expression.
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

ExprParm::ExprParm(const ParmProxy::ConstPtr &parm)
    :   itsParm(parm),
        itsPValueFlag(false)
{
}

void ExprParm::setPValueFlag()
{
    itsPValueFlag = true;
}

bool ExprParm::getPValueFlag() const
{
    return itsPValueFlag;
}

void ExprParm::clearPValueFlag()
{
    itsPValueFlag = false;
}

bool ExprParm::isDependent() const
{
    return itsPValueFlag;
}

const Scalar ExprParm::evaluateExpr(const Request &request, Cache &cache,
    unsigned int grid) const
{
    // Get the result from the Parm.
    vector<casa::Array<double> > buffers;
    itsParm->getResult(buffers, request[grid], getPValueFlag()); //false);
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
        DBGASSERT(static_cast<unsigned int>(shape(0)) == request[grid][FREQ]->size()
            && static_cast<unsigned int>(shape(1)) == request[grid][TIME]->size());
        result.assign(Matrix(storage, shape(0), shape(1)));
    }
    buffers[0].freeStorage(storage, deleteStorage);

    // Copy the perturbed values if necessary.
    if(getPValueFlag())
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
    scalar.setValueSet(result);
    return scalar;
}

} //# namespace BBS
} //# namespace LOFAR
