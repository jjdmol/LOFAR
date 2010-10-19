//# Equator.h: Generate normal equations that tie a model to an observation.
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

#ifndef LOFAR_BB_BBS_EQUATOR_H
#define LOFAR_BB_BBS_EQUATOR_H

// \file
// Generate normal equations that tie a model to an observation.

#include <BBSKernel/Model.h>
//#include <BBSKernel/Expr/Result.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <BBSKernel/SolverInterfaceTypes.h>
#include <BBSKernel/VisData.h>
#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// @{

class Equator
{
public:
    // Constructor. NB: nThreads is ignored when compiled without OPENMP.
    Equator(const VisData::Pointer &chunk, const Model::Pointer &model,
        const CoeffIndex &index, const Grid &solGrid, uint nMaxCells,
        uint nThreads = 1);
    ~Equator();

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

    // Generate normal equations for the given (inclusive) range of cells in
    // the solution grid.
    void process(vector<CellEquation> &result, const Location &start,
        const Location &end);

private:    
    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the observation (chunk) grid.
    void makeGridMapping();

    // Create a mapping that maps each (parmId, coeffId) combination to an
    // index.
    void makeCoeffMapping(const CoeffIndex &index);

    // Pre-allocate thread private casa::LSQFit instances.
    void makeContexts();
    
    // Generate normal equations for a single baseline.
    void blConstruct(uint threadId, const baseline_t &baseline,
        const Location &cellStart, const Location &cellEnd,
        const Location &visStart);

    void resetTimers();
    void printTimers();

    // Nested class containing thread private data structures. Each thread gets
    // its own private data structures, to avoid unnecessary locking.
    class ThreadContext
    {
    public:
        ThreadContext();
        ~ThreadContext();
        void resize(uint nCoeff, uint nMaxCells);
        void clear(bool clearEq = false);

        // Normal matrices (one per cell).
        vector<casa::LSQFit*>   eq;
        // Mapping from available perturbed values (parmId, coeffId) to a
        // coefficient index in the normal matrix.
        vector<uint>            index;
        // One over the perturbation (delta) used to compute the perturbed
        // values. This number is used to approximate the partial derivatives
        // of the model with respect to the solvables using forward differences.
        vector<double>          inversePert;
        // Pointers to the real and imaginary components of the perturbed
        // values.
//        vector<const double*>   pertRe, pertIm;
        vector<ValueSet::ConstPtr>    pertIt;
        // Value of the (approximated) partial derivatives.
        // @{
        vector<double>          partialRe, partialIm;
        // @}

        // Statistics
        unsigned long long      count;

        enum ThreadTimer
        {
            MODEL_EVAL,
            PROCESS,
            BUILD_INDEX,
            DERIVATIVES,
            MAKE_NORM,
            N_ThreadTimer
        };
        
        static string           timerNames[N_ThreadTimer];
        NSTimer                 timers[N_ThreadTimer];
    };
    
    // Nested struct that represents an interval of cells along an axis. Used to
    // create the mapping from cells in the solution grid to intervals of cells
    // in the observation grid.
    struct Interval
    {
        Interval()
            :   start(0),
                end(0)
        {
        }
        
        uint    start, end;                
    };
    
    // Observed visibilities.
    VisData::Pointer                    itsChunk;
    // Model of the sky and the instrument.
    Model::Pointer                      itsModel;
    // Solution grid.
    Grid                                itsSolGrid;
    // Maximum number of cells in the solution grid that can be processed
    // simultaneously.
    uint                                itsMaxCellCount;
    // Requested number of threads.
    uint                                itsThreadCount;

    // Selection of the visibility data to process.
    vector<baseline_t>                  itsBaselines;
    int                                 itsProductMask[4];
    
    // Location in the solution grid of the current chunk's start and end cell.
    Location                            itsStartCell, itsEndCell;
    // Mapping of cells in the solution grid to intervals of cells along the
    // observation grid's axes.
    vector<Interval>                    itsFreqIntervals, itsTimeIntervals;

    // Mapping from (parmId, coeffId) to a coefficient index in the normal
    // equations.
    map<PValueKey, uint>                itsCoeffMap;

    // Thread private data structures.
    boost::multi_array<casa::LSQFit, 2> itsThreadEq;
    vector<ThreadContext>               itsContexts;

    // Timers
    enum Timer
    {
        PROCESS,
        PRECOMPUTE,
        COMPUTE,
        MERGE,
        N_Timer
    };

    NSTimer                             itsTimers[N_Timer];
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
