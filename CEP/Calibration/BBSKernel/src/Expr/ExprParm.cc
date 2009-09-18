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

ExprParm::ExprParm(const ParmProxy::ConstPointer &parm)
    :   itsParm(parm),
        itsPValueFlag(false)
{
}

ExprParm::~ExprParm()
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

Result ExprParm::getResult(const Request &request)
{
    // Get the result from the Parm.
    vector<casa::Array<double> > buffers;
    itsParm->getResult(buffers, request.getGrid(), itsPValueFlag
        && request.getPValueFlag());
    ASSERT(buffers.size() > 0);

    // Transform into a Result.
    Result result;
    result.init();

    bool deleteStorage;
    const double *storage = 0;
    
    // Copy the main value.
    storage = buffers[0].getStorage(deleteStorage);
    ASSERT(storage);
    if(buffers[0].nelements() == 1)
    {
        result.setValue(Matrix(storage[0]));
    }
    else
    {
        const casa::IPosition &shape = buffers[0].shape();
        result.setValue(Matrix(storage, shape(0), shape(1)));
    }
    buffers[0].freeStorage(storage, deleteStorage);

    // Copy the perturbed values if necessary.
    if(itsPValueFlag && request.getPValueFlag())
    {
        const size_t nCoeff = itsParm->getCoeffCount();
        ASSERT(buffers.size() == nCoeff + 1);
        
        for(size_t i = 0; i < nCoeff; ++i)
        {
            storage = buffers[i + 1].getStorage(deleteStorage);
            ASSERT(storage);
            if(buffers[i + 1].nelements() == 1)
            {
                result.setPerturbedValue(PValueKey(itsParm->getId(), i),
                    Matrix(storage[0]));
            }
            else
            {
                const casa::IPosition &shape = buffers[i + 1].shape();
                result.setPerturbedValue(PValueKey(itsParm->getId(), i),
                    Matrix(storage, shape(0), shape(1)));
            }
            buffers[i + 1].freeStorage(storage, deleteStorage);
        }
    }
    
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
