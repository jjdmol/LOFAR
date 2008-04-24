//# Prediffer.h: Calibrate observed data.
//#
//# Copyright (C) 2004
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

#ifndef LOFAR_BB_BBS_PREDIFFER_H
#define LOFAR_BB_BBS_PREDIFFER_H

// \file
// Calibrate observed data.

#include <BBSKernel/CoefficientIndex.h>
#include <BBSKernel/Grid.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Messages.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/VisSelection.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Types.h>

#include <BBSKernel/MNS/MeqDomain.h>
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqParm.h>
#include <BBSKernel/MNS/MeqPhaseRef.h>
#include <BBSKernel/MNS/MeqRequest.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/ParmDomain.h>

#include <Common/Timer.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

// Prediffer calculates the equations for the solver.
// It reads the measured data and predicts the data from the model.
// It subtracts them from each other and calculates the derivatives.
// These results can be sent to the solver to find better parameter values.


class Prediffer
{
public:
    enum OperationType
    {
        UNSET = 0,
        SIMULATE,
        SUBTRACT,
        CORRECT,
        CONSTRUCT,
        N_OperationType        
    };

    Prediffer(uint32 id,
        Measurement::Pointer measurement,
        ParmDB::ParmDB sky,
        ParmDB::ParmDB instrument);

    // Attach/detach the chunk of data to process.
    // <group>
    void attachChunk(VisData::Pointer chunk);
    void detachChunk();
    // </group>

    // Select a subset of the data for processing. An empty vector matches
    // anything. The selection remains active until it is changed or until
    // attachChunk() is called.
    bool setSelection(const string &filter,
        const vector<string> &stations1,
        const vector<string> &stations2,
        const vector<string> &polarizations);
    
    void setModelConfig(OperationType operation,
        const vector<string> &components,
        const vector<string> &sources);
        
    // Operations that can be performed on the data. The setModelConfig()
    // function should be called prior to calling any of these functions. This
    // ensures the model is properly initialized. Of course, the operation
    // function called should match the operation specified in the call to
    // setModelConfig().
    //
    // NOTE: Prior to calling construct(), the parameters to solve for should be
    // selected and a solution cell grid should be set (see
    // setParameterSelection() and setCellGrid() below).
    // <group>
    void simulate();
    void subtract();
    void correct();
    EquationMsg::Pointer construct(Location start, Location end);
    // </group>
    
    // (Distributed) solving.
    // <group>
    void setParameterSelection(const vector<string> &include,
        const vector<string> &exclude);        
    void clearParameterSelection();

    bool setCellGrid(const Grid<double> &cellGrid);

    CoeffIndexMsg::Pointer getCoefficientIndex() const;
    void setCoefficientIndex(CoeffIndexMsg::Pointer msg);

    CoefficientMsg::Pointer getCoefficients(Location start, Location end) const;
    void setCoefficients(SolutionMsg::Pointer msg);
    // </group>

    // Commit cached parameter values to the parameter database.
    void storeParameterValues();

/*
#ifdef EXPR_GRAPH
    void writeExpressionGraph(const string &fileName, baseline_t baseline);
#endif
*/

private:
    
    struct ThreadContext
    {
        enum
        {
            MODEL_EVALUATION = 0,
            PROCESS,
            GRID_LOOKUP,
            INV_DELTA,
            BUILD_INDEX,
            DERIVATIVES,
            MAKE_NORM,
            N_Timer
        } Timer;

        static string           timerNames[N_Timer];
        NSTimer                 timers[N_Timer];
        unsigned long long      visCount;

        vector<casa::LSQFit*>   equations;
        vector<size_t>          coeffIndex;
        vector<const double*>   perturbedRe, perturbedIm;
        vector<double>          partialRe, partialIm;
    };
    
    //# Copy construction and assignment are not allowed.
    Prediffer(const Prediffer& other);
    Prediffer &operator=(const Prediffer& other);

    //# Define the signature for a function that processes the data of a single
    //# baseline.
    typedef void (Prediffer::*BlProcessor) (size_t threadNr,
        const baseline_t &baseline,
        const Location &offset,
        const MeqRequest &request,
        void *arguments);

    //# Process (a subset of) the observed data.
    void process(bool checkFlags, bool precalc, const Location &start,
        const Location &end, BlProcessor processor, void *arguments = 0);

    //# Copy the simulated visibilities for a single baseline.
    void copyBl(size_t threadNr, const baseline_t &baseline,
        const Location &offset, const MeqRequest &request, void *arguments = 0);

    //# Subtract the simulated visibilities from the observed data for a
    //# single baseline.
    void subtractBl(size_t threadNr, const baseline_t &baseline,
        const Location &offset, const MeqRequest &request, void *arguments = 0);

    //# Construct equations for a single baseline.
    void constructBl(size_t threadNr, const baseline_t &baseline,
        const Location &offset, const MeqRequest &request, void *arguments = 0);

    //# Create a look-up table that maps external (measurement) polarization
    //# indices to internal polarization indices.
    void initPolarizationMap();

    //# Get all parameter values that intersect the current chunk.
    void loadParameterValues();

    //# Reset and/or print various timers and statistics.
    void resetTimers();
    void printTimers(const string &operation);


    uint32                              itsId;
    Measurement::Pointer                itsMeasurement;
    VisData::Pointer                    itsChunk;
    //# Mapping from internal polarization (XX,XY,YX,YY) to polarizations
    //# found in the chunk (which may be a subset, and in a different order).
    vector<int>                         itsPolarizationMap;

    ParmDB::ParmDB                      itsSkyDb;
    ParmDB::ParmDB                      itsInstrumentDb;
    //# All parameter values that intersect the chunk.
    map<string, ParmDB::ParmValueSet>   itsParameterValues;

    Model::Pointer                      itsModel;
    //# Container for the model parameters (the leaf nodes of the model).
    MeqParmGroup                        itsParameters;

    OperationType                       itsOperation;
    vector<baseline_t>                  itsBaselineSelection;
    vector<size_t>                      itsPolarizationSelection;

    //# Information required for the CONSTRUCT operation.
    //# ------------------------------------------------------------------------
    Grid<double>                        itsGlobalCellGrid;
    Grid<uint32>                        itsCellGrid;
    Location                            itsStartCell, itsEndCell;

    //# Sum of the maximal number (over all solution cells) of coefficients of
    //# each parameter.
    uint32                              itsCoeffCount;
    vector<MeqPExpr>                    itsParameterSelection;
    map<uint32, CellCoeffIndex>         itsCellCoeffIndices;
    //# ------------------------------------------------------------------------

    vector<ThreadContext>               itsThreadContexts;

    //# Timers for performance measurement.
    NSTimer                             itsPrecalcTimer, itsProcessTimer;
    
    //# TODO: Remove this.
    //# Phase reference position in J2000 coordinates.
    MeqPhaseRef                         itsPhaseRef;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
