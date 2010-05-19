//# LocalSolveController.h: Class that controls the execution of a local solve
//# command.
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

#ifndef LOFAR_BBSCONTROL_LOCAL_SOLVE_CONTROLLER_H
#define LOFAR_BBSCONTROL_LOCAL_SOLVE_CONTROLLER_H

// \file
// Class that controls the execution of a local solve command.

#include <BBSControl/Types.h>
#include <BBSControl/Exceptions.h>

#include <BBSKernel/Equator.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/VisData.h>

#include <Common/lofar_smartptr.h>

#include <ParmDB/ParmDBLog.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class LocalSolveController
{
public:
    LocalSolveController(const ExprSet<JonesMatrix>::Ptr &lhs,
        const ExprSet<JonesMatrix>::Ptr &rhs,
        const SolverOptions &options);
	 ~LocalSolveController();

    void init(const vector<string> &include, const vector<string> &exclude,
        const Grid &evalGrid, const Grid &solGrid, unsigned int cellChunkSize,
        bool propagate);

    void run();
	 void run(ParmDBLog &parmLogger);

private:
    void makeCoeffIndex(const ParmGroup &solvables);
    void makeCoeffMapping(const ParmGroup &solvables, const CoeffIndex &index);

    void getInitialCoeff(vector<CellCoeff> &result, const Location &start,
        const Location &end) const;

    void setSolution(const vector<CellSolution> &solutions,
        const Location &start, const Location &end) const;

    void getCoeff(vector<double> &result, const Location &cell) const;
    void setCoeff(const vector<double> &coeff, const Location &cell) const;
    void setCoeff(const vector<double> &coeff, const Location &cell,
        const vector<unsigned int> &mapping) const;

    ExprSet<JonesMatrix>::Ptr       itsLHS;
    ExprSet<JonesMatrix>::Ptr       itsRHS;
    Solver                          itsSolver;

    bool                            itsInitFlag;
    bool                            itsPropagateFlag;
    Grid                            itsSolGrid;
    unsigned int                    itsCellChunkSize;
    ParmGroup                       itsSolvables;
    CoeffIndex                      itsCoeffIndex;
    scoped_ptr<Equator>             itsEquator;
    vector<unsigned int>            itsSolCoeffMapping;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
