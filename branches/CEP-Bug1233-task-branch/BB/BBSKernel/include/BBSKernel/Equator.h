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
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/SolverInterfaceTypes.h>
#include <BBSKernel/VisData.h>
#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Equator
{
public:
    Equator(const VisData::Pointer &chunk, const Model::Pointer &model,
        const CoeffIndex &index, const Grid &solGrid, uint nMaxCells,
        uint nThreads = 1);
    ~Equator();

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

//    void setCells(const Location &start, const Location &end);

    void process(vector<CellEquation> &result, const Location &start,
        const Location &end);

private:    
    void makeContexts();
    void makeGridMapping();
    void makeCoeffMapping(const CoeffIndex &index);
    
    void blConstruct(uint threadId, const baseline_t &baseline,
        const Request &request, const Location &cellStart,
        const Location &cellEnd, const Location &visStart);

    void resetTimers();
    void printTimers();

    enum Timer
    {
        PROCESS,
        PRECOMPUTE,
        COMPUTE,
        MERGE,
        N_Timer
    };

    class ThreadContext
    {
    public:
        enum Timer
        {
            MODEL_EVAL,
            PROCESS,
            BUILD_INDEX,
            DERIVATIVES,
            MAKE_NORM,
            N_Timer
        };
        
        ThreadContext();
        ~ThreadContext();
        void resize(uint nCoeff, uint nMaxCells);
        void clear(bool clearEq = false);

        static string           timerNames[N_Timer];
        NSTimer                 timers[N_Timer];
        unsigned long long      count;

        vector<casa::LSQFit*>   eq;
        vector<uint>            index;
        vector<double>          inversePert;
        vector<const double*>   pertRe, pertIm;
        vector<double>          partialRe, partialIm;
    };
    
    struct Interval
    {
        Interval()
            :   start(0),
                end(0)
        {}
        
        uint    start, end;                
    };
    
    VisData::Pointer                    itsChunk;
    Model::Pointer                      itsModel;

    vector<baseline_t>                  itsBaselines;
    int                                 itsProductMask[4];
    
    // Global solution grid.
    Grid                                itsSolGrid;
    // Mapping of local solution grid cells to intervals along the chunk
    // grid's axes.
    vector<Interval>                    itsFreqIntervals, itsTimeIntervals;
    // Location in the global solution grid of the current chunk's start and
    // end cell.
    Location                            itsStartCell, itsEndCell;
    boost::multi_array<casa::LSQFit, 2> itsThreadEq;
    vector<ThreadContext>               itsContexts;
    uint                                itsMaxCellCount;

//    vector<size_t>                      itsCoeffIndex;
    uint                                itsThreadCount;
    map<PValueKey, uint>                itsCoeffMap;
    NSTimer                             itsTimers[N_Timer];
//    vector<size_t>                      itsCellIdxFreq, itsCellIdxTime;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
