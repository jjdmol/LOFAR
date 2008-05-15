//# Solver.h: 
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BB_BBSKERNEL_SOLVER_H
#define LOFAR_BB_BBSKERNEL_SOLVER_H

#include <BBSKernel/SolverInterfaceTypes.h>
#include <Common/LofarTypes.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{
    class Solver
    {
    public:
        Solver()
            :   itsEpsValue(1e-9),
                itsEpsDerivative(1e-9),
                itsMaxIter(10),
                itsColFactor(1e-9),
                itsLmFactor(1.0),
                itsBalanced(false),
                itsUseSvd(true)
        {}

        void reset(double epsValue = 1e-9, double epsDerivative = 1e-9,
            size_t maxIter = 10, double colFactor = 1e-9, double lmFactor = 1.0,
            bool balanced = false, bool useSvd = true);

        void setCoeffIndex(uint32 kernelId, const CoeffIndex &local);
        void getCoeffIndex(CoeffIndex &global) const;

        void setCoeff(uint32 kernelId, const vector<CellCoeff> &local);

        void setEquations(uint32 kernelId, const vector<CellEquation> &local);
        void getEquations(vector<CellEquation> &global);

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

} //# namespace BBS
} //# namespace LOFAR

#endif
