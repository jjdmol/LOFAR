//# Solver.cc:
//#
//# Copyright (C) 2007
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
#include <BBSKernel/Solver.h>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/Timer.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

SolverOptions::SolverOptions()
    :   maxIter(0),
        epsValue(0.0),
        epsDerivative(0.0),
        colFactor(0.0),
        lmFactor(0.0),
        balancedEq(false),
        useSVD(false)
{
}

ostream& operator<<(ostream& os, const SolverOptions& obj)
{
    os << "Solver options:";
    Indent id;
    os << endl << indent << "Max nr. of iterations: " << obj.maxIter
        << endl << indent << "Epsilon value: " << obj.epsValue
        << endl << indent << "Epsilon derivative: " << obj.epsDerivative
        << endl << indent << "Colinearity factor: " << obj.colFactor
        << endl << indent << "LM factor: " << obj.lmFactor
        << boolalpha
        << endl << indent << "Balanced equations: " << obj.balancedEq
        << endl << indent << "Use SVD: " << obj.useSVD
        << noboolalpha;

    return os;
}

Solver::Solver()
{
    SolverOptions options;
    reset(options);
}

Solver::Solver(const SolverOptions &options)
{
    reset(options);
}

void Solver::reset(const SolverOptions &options)
{
    itsMaxIter = options.maxIter;
    itsEpsValue = options.epsValue;
    itsEpsDerivative = options.epsDerivative;
    itsColFactor = options.colFactor;
    itsLMFactor = options.lmFactor;
    itsBalancedEq = options.balancedEq;
    itsUseSVD = options.useSVD;

    itsCells.clear();
    itsCoeffIndex.clear();
    itsCoeffMapping.clear();
}

void Solver::setCoeffIndex(size_t kernelId, const CoeffIndex &local)
{
    vector<casa::uInt> &mapping = itsCoeffMapping[kernelId];
    mapping.resize(local.getCoeffCount());
    for(CoeffIndex::const_iterator it = local.begin(), end = local.end();
        it != end; ++it)
    {
        const CoeffInterval &interval =
            itsCoeffIndex.insert(it->first, it->second.length);

        for(size_t i = 0; i < interval.length; ++i)
        {
            mapping[it->second.start + i] = interval.start + i;
        }
    }
}

CoeffIndex Solver::getCoeffIndex() const
{
    return itsCoeffIndex;
}

} // namespace BBS
} // namespace LOFAR
