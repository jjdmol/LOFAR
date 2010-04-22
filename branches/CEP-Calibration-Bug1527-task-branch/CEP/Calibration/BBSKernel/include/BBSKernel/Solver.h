//# Solver.h:
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

#ifndef LOFAR_BBSKERNEL_SOLVER_H
#define LOFAR_BBSKERNEL_SOLVER_H

#include <BBSKernel/SolverInterfaceTypes.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_smartptr.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Solver
{
public:
    typedef shared_ptr<Solver>          Ptr;
    typedef shared_ptr<const Solver>    ConstPtr;

    Solver();

    // (Re)set the solver's state. This allows a solver instance to be
    // re-used.
    void reset(size_t maxIter = 10, double epsValue = 1e-9,
        double epsDerivative = 1e-9, double colFactor = 1e-9,
        double lmFactor = 1.0, bool balanced = false, bool useSvd = true);

    // Set the (local) coefficient index of a kernel.
    void setCoeffIndex(uint32 kernelId, const CoeffIndex &local);
    // Get the merged (global) coefficient index.
    const CoeffIndex &getCoeffIndex() const;

    // Set the initial coefficients of a kernel.
    void setCoeff(uint32 kernelId, const vector<CellCoeff> &local);

    // Set the equations of a kernel.
    void setEquations(uint32 kernelId, const vector<CellEquation> &local);
    // Get the merged equations (meant for debugging purposes).
    void getEquations(vector<CellEquation> &global);

    // Perform an iteration for all available cells.
    bool iterate(vector<CellSolution> &global);

private:
    struct Cell
    {
        casa::LSQFit    solver;
        vector<double>  coeff;
    };

    map<uint32, Cell>               itsCells;
    CoeffIndex                      itsCoeffIndex;
    map<uint32, vector<uint32> >    itsCoeffMapping;

    double                          itsEpsValue;
    double                          itsEpsDerivative;
    size_t                          itsMaxIter;
    double                          itsColFactor;
    double                          itsLmFactor;
    bool                            itsBalanced;
    bool                            itsUseSvd;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
