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

#include <BBSKernel/CoefficientIndex.h>
#include <BBSKernel/Grid.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Messages.h>
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
#include <scimath/Fitting/LSQFit.h>


#include <Blob/BlobStreamable.h>

/*
namespace casa
{
    class LSQFit;
}
*/

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

struct ProcessingContext
{
    ProcessingContext()
        :   coefficientCount(0)
    {}


    // 
    // Information for the SOLVE operation (valid if unknownCount > 0).
    //     
    Grid<double>                        solutionGrid;
    Grid<size_t>                        domainGrid;
    
    Location                            chunkStart, chunkEnd;

    // Sum of the maximal number (over all solve domains) of unknowns of each
    // parameter.
    size_t                              coefficientCount;
    vector<MeqPExpr>                    localParmSelection;
    map<uint32, CellCoeffIndex>         localCoeffIndices;
    
    // The set of unknowns for each solve domain.
//    vector<vector<double> >         unknowns;
//    boost::multi_array<double, 3>   unknowns;

    // The set of parameters included in the SOLVE operation.
//    vector<MeqPExpr>                parameters;
    // Index into the unknown vector of the first (solvable) coefficient of a
    // given parameter.
//    vector<size_t>                  parameterOffset;
};


struct ThreadContext
{
    enum
    {
        MODEL_EVALUATION = 0,
        PROCESS,
        GRID_LOOKUP,
        BUILD_INDEX,
        PARTIAL_DERIVATIVES,
        MAKE_NORM,
        N_Timer
    } Timer;

    static string           timerNames[N_Timer];
    NSTimer                 timers[N_Timer];

    vector<casa::LSQFit*>   equations;
    vector<uint>            unknownIndex;
    vector<const double*>   perturbedRe, perturbedIm;
    vector<double>          partialRe, partialIm;
};


struct Selection
{
    vector<baseline_t>      baselines;
    set<size_t>             polarizations;
};


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

    ~Prediffer();

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
    // NOTE: Prior to calling construct(), a solution grid should be set and
    // the parameters to solve for should be selected (see setSolutionGrid()
    // and setParameterSelection() below).
    // <group>
    void simulate();
    void subtract();
    void correct();
    EquationMsg::Pointer construct(const Location &start, const Location &end);
    // </group>
    
    // (Distributed) solving.
    // <group>
    bool setSolutionGrid(const Grid<double> &grid);

    bool setParameterSelection(const vector<string> &include,
        const vector<string> &exclude);        
    void clearParameterSelection();

    CoeffIndexMsg::Pointer getCoefficientIndex() const;
    void setCoefficientIndex(CoeffIndexMsg::Pointer msg);

    CoefficientMsg::Pointer getCoefficients(const Location &start,
        const Location &end) const;
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
    // Copy construction and assignment are not allowed.
    // <group>
    Prediffer(const Prediffer& other);
    Prediffer &operator=(const Prediffer& other);
    // </group>

    void initPolarizationMap();

    //# Get all parameter values that intersect the current chunk.
    void loadParameterValues();

/*  
    void initSolveDomains(std::pair<size_t, size_t> size);
    

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

    Selection                           itsSelection;
    OperationType                       itsOperation;

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
