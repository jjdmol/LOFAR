//# GlobalSolveController.h: Class that controls the execution of a global solve
//# command.
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

#ifndef LOFAR_BBSCONTROL_GLOBAL_SOLVE_CONTROLLER_H
#define LOFAR_BBSCONTROL_GLOBAL_SOLVE_CONTROLLER_H

// \file
// Class that controls the execution of a global solve command.

#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/Types.h>

#include <BBSKernel/Equator.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/VisData.h>

#include <Common/lofar_smartptr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class GlobalSolveController
{
public:
    GlobalSolveController(const KernelIndex &index,
        const VisData::Pointer &chunk, const Model::Pointer &model,
        const shared_ptr<BlobStreamableConnection> &solver);
    ~GlobalSolveController();
    
    void init(const vector<string> &include, const vector<string> &exclude,
        const Grid &solGrid, const vector<baseline_t> &baselines,
        const vector<string> &products, uint cellChunkSize, bool propagate);
    void run();

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
        const vector<uint> &mapping) const;

    KernelIndex                                itsKernelIndex;
    VisData::Pointer                        itsChunk;
    Model::Pointer                          itsModel;
    shared_ptr<BlobStreamableConnection>    itsSolver;

    bool                                    itsInitFlag;
    bool                                    itsPropagateFlag;
    Grid                                    itsSolGrid;
    uint                                    itsCellChunkSize;
    ParmGroup                               itsSolvables;
    CoeffIndex                              itsCoeffIndex;
    scoped_ptr<Equator>                     itsEquator;
    vector<uint>                            itsSolCoeffMapping;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
