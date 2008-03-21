//# Prediffer.h: Read and predict read visibilities
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
// Read and predict read visibilities

//#include <casa/BasicSL/Complex.h>
//#include <casa/Arrays/Matrix.h>

#include <BBSKernel/Grid.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/VisSelection.h>
#include <BBSKernel/VisData.h>

#include <BBSKernel/Types.h>
//#include <BBSKernel/ParmData.h>
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

#include <utility>

namespace casa
{
    class LSQFit;
}

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

//typedef Grid<CellCenteredAxis<RegularSeries>,
//    CellCenteredAxis<IrregularSeries> >         SolutionGrid;
    
// Prediffer calculates the equations for the solver.
// It reads the measured data and predicts the data from the model.
// It subtracts them from each other and calculates the derivatives.
// These results can be sent to the solver to find better parameter values.

struct ProcessingContext
{
    ProcessingContext()
        :   unknownCount(0)
    {}


    // 
    // Information for the SOLVE operation (valid if unknownCount > 0).
    //     
    Grid<double>                    solutionGrid;
    Grid<size_t>                    cellGrid;
    
    Location                        chunkStart, chunkEnd;

    // Sum of the maximal number (over all solve domains) of unknowns of each
    // parameter.
    size_t                          unknownCount;
    // The set of unknowns for each solve domain.
    vector<vector<double> >         unknowns;
//    boost::multi_array<double, 3>   unknowns;

    // The set of parameters included in the SOLVE operation.
//    vector<MeqPExpr>                parameters;
    // Index into the unknown vector of the first (solvable) coefficient of a
    // given parameter.
//    vector<size_t>                  parameterOffset;
};


struct ThreadContext
{
    vector<casa::LSQFit*>   solvers;
    vector<uint>            unknownIndex;
    vector<const double*>   perturbedRe, perturbedIm;
    vector<double>          partialRe, partialIm;
    NSTimer                 evaluationTimer, operationTimer;
};


struct Selection
{
    vector<baseline_t>      baselines;
    set<size_t>             polarizations;
};


struct ModelConfiguration
{
    vector<string>          components;
    vector<string>          sources;
};


class Prediffer
{
public:
    enum Operation
    {
        SIMULATE,
        SUBTRACT,
        CORRECT,
        CONSTRUCT,
        N_Operation        
    };
    
    Prediffer(Measurement::Pointer measurement,
        ParmDB::ParmDB sky,
        ParmDB::ParmDB instrument);

    ~Prediffer();

    // Attach/detach the chunk of data to process.
    // <group>
    void attachChunk(VisData::Pointer chunk);
    void detachChunk();
    // </group>

    // Select a subset of the data for processing. An empty vector matches
    // anything. A selection remains active until it is changed or until
    // attachChunk() is called.
    bool setSelection(const string &filter,
        const vector<string> &stations1,
        const vector<string> &stations2,
        const vector<string> &polarizations);
    
    void setModelConfiguration(const vector<string> &components,
        const vector<string> &sources);
        
    // Operations that can be performed on the data.
    // <group>
    void setOperation(Operation type);
/*
    void simulate();
    void subtract();
    void correct();
*/    
    // </group>
    
    // Solving....
    // <group>
    bool setSolutionGrid(const Grid<double> &solutionGrid);

    bool setParameterSelection(const vector<string> &include,
        const vector<string> &exclude);
        
    void clearParameterSelection();

/*
    void getParameterIndex(ParameterIndex &parameters) const;
    void setParameterIndex(const ParameterIndex &parameters);

    void getCoefficientIndices(vector<CoefficientIndex> &indices) const;
    void setCoefficientIndices(const vector<CoefficientIndex> &indices);

    void getCoefficients(const GridLocation &start,
        const GridLocation &end, vector<Coefficients> &coefficients);
    void setCoefficients(const vector<Coefficients> &coefficients,
        bool useIndex);
*/
//    void construct(const Location &start, const Location &end,
//        vector<Equations> &equations);
    // </group>

/*
    // Commit cached parameter values to the parameter database.
    void storeParameterValues();

#ifdef EXPR_GRAPH
    void writeExpressionGraph(const string &fileName, baseline_t baseline);
#endif
*/
private:
    // Copy construction and assignment are not allowed.
    // <group>
    Prediffer(const Prediffer& other);
    Prediffer &operator=(const Prediffer& other);
    // </group>

    void initPolarizationMap();

/*  
    void initSolveDomains(std::pair<size_t, size_t> size);
    
    // Get all parameter values that intersect the current chunk.
    void fetchParameterValues();

    void resetTimers();
    void printTimers(const string &operationName);

    // Define the signature of a function processing a baseline.
    typedef void (Prediffer::*BaselineProcessor) (int threadnr, void* arguments,
        VisData::Pointer chunk, pair<size_t, size_t> offset,
        const MeqRequest& request, baseline_t baseline, bool showd);

    // Loop through all data and process each baseline by ProcessFuncBL.
    void process(bool useFlags, bool precalc, bool derivatives,
        pair<size_t, size_t> start, pair<size_t, size_t> end,
        BaselineProcessor processor, void *arguments);

    // Copy the predicted visibilities of a single baseline.
    void copyBL(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);

    // Subtract the predicted visibilities from the observed data of a
    // single baseline.
    void subtractBL(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);

    // Construct equations for a single baseline.
    void constructBL(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);
*/

    Measurement::Pointer                itsMeasurement;
    VisData::Pointer                    itsChunk;
    //# Mapping from internal polarization (XX,XY,YX,YY) to polarizations
    //# found in the chunk (which may be a subset, and in a different order).
    vector<int>                         itsPolarizationMap;

    ParmDB::ParmDB                      itsInstrumentDb;
    ParmDB::ParmDB                      itsSkyDb;
    //# All parameter values that intersect the chunk.
    map<string, ParmDB::ParmValueSet>   itsParameterValues;

    Model::Pointer                      itsModel;
    //# Container for the model parameters (the leaf nodes of the model).
    MeqParmGroup                        itsParameters;

    Selection                           itsSelection;
    ModelConfiguration                  itsModelConfiguration;
    Operation                           itsOperation;
    //# TODO: Remove this.
    ProcessingContext                   itsContext;

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
