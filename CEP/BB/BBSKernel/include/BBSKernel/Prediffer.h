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

#include <BBSKernel/Measurement.h>
#include <BBSKernel/Model.h>
#include <BBSKernel/VisSelection.h>
#include <BBSKernel/VisData.h>

#include <BBSKernel/BBSKernelStructs.h>
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

// Prediffer calculates the equations for the solver.
// It reads the measured data and predicts the data from the model.
// It subtracts them from each other and calculates the derivatives.
// These results can be sent to the solver to find better parameter values.

struct ProcessingContext
{
    ProcessingContext()
        :   domainSize(0, 0),
            domainCount(0, 0),
            unknownCount(0)
    {}

    set<baseline_t>                 baselines;
    set<size_t>                     polarizations;
    string                          outputColumn;

    // 
    // Information for the SOLVE operation (valid if unknownCount > 0).
    //     

    // Nominal solve domain size along each axis in #channels, #timeslots.
    std::pair<size_t, size_t>       domainSize;
    // Number of solve domains along each axis.
    std::pair<size_t, size_t>       domainCount;
    // Solve domains.
    vector<MeqDomain>               domains;
//    boost::multi_array<MeqDomain, 2>    domains;
    
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
};


class Prediffer
{
public:
    Prediffer(const string &measurement, size_t subband,
        const string &inputColumn,
        const string &skyDBase, const string &instrumentDBase,
        const string &historyDBase, bool readUVW);

    // Destructor
    ~Prediffer();

    // Lock and unlock the parm database tables.
    // The user does not need to lock/unlock, but it can increase performance
    // if many small accesses have to be done.
    // <group>
    void lock(bool lockForWrite = true)
    {
        itsInstrumentDBase->lock(lockForWrite);
        itsSkyDBase->lock(lockForWrite);
    }

    void unlock()
    {
        itsInstrumentDBase->unlock();
        itsSkyDBase->unlock();
    }
    // </group>

    void setSelection(VisSelection selection);
    void setChunkSize(size_t time);
    bool nextChunk();

    bool setContext(const PredictContext &context);
    bool setContext(const SubtractContext &context);
    bool setContext(const CorrectContext &context);
    bool setContext(const GenerateContext &context);

    void predict();
    void subtract();
    void correct();
    void generate(pair<size_t, size_t> start, pair<size_t, size_t> end,
        vector<casa::LSQFit> &solvers);

    // Get the actual work domain for this MS (after a setStrategy).
    const MeqDomain &getWorkDomain() const
        { return itsWorkDomain; }

    // Get the local data domain.
//    MeqDomain getLocalDataDomain() const;

    // There are three ways to update the solvable parms after the solver
    // found a new solution.
    // <group>
    // Update the values of solvable parms.
    // Using its spid index each parm takes its values from the vector.
//    void updateSolvableParms (const vector<double>& values);

    // Update the given values (for the current domain) of solvable parms
    // matching the corresponding parm name in the vector.
    // Vector values with a name matching no solvable parm are ignored.
//    void updateSolvableParms (const ParmDataInfo& parmData);

    // Update the solvable parm values (reread from table).
    void updateSolvableParms();
    void updateUnknowns(size_t domain, const vector<double> &unknowns);

    // Log the updated values of a single solve domain.
    //void logIteration(const string &stepName, size_t solveDomainIndex,
    //    double rank, double chiSquared, double LMFactor);

    // Write the solved parms.
    void writeParms();
    void writeParms(size_t solveDomainIndex);

    const vector<vector<double > > &getUnknowns() const
    { return itsContext.unknowns; }

    pair<size_t, size_t> getSolveDomainGridSize() const
    { return itsContext.domainCount; }

#ifdef EXPR_GRAPH
    void writeExpressionGraph(const string &fileName, baseline_t baseline);
#endif

private:
    // Copy constructor and assignment are not allowed.
    // <group>
    Prediffer (const Prediffer& other);
    Prediffer& operator= (const Prediffer& other);
    // </group>

    bool setContext(const Context &context);
    void initSolveDomains(std::pair<size_t, size_t> size);
    void initPolarizationMap();
    
    // Make all parameters non-solvable.
    void clearSolvableParms();

    // Read the polcs for all parameters for the current work domain.
    void readParms();

    // Define the signature of a function processing a baseline.
    typedef void (Prediffer::*BaselineProcessor) (int threadnr, void* arguments,
        VisData::Pointer chunk, pair<size_t, size_t> offset,
        const MeqRequest& request, baseline_t baseline, bool showd);

    // Loop through all data and process each baseline by ProcessFuncBL.
    void process(bool useFlags, bool precalc, bool derivatives,
        pair<size_t, size_t> start, pair<size_t, size_t> end,
        BaselineProcessor processor, void *arguments);

    // Write the predicted data of a baseline.
    void copyBaseline(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);

    // Subtract the data of a baseline.
    void subtractBaseline(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);

    // Generate equations for a baseline.
    void generateBaseline(int threadnr, void* arguments, VisData::Pointer chunk,
        pair<size_t, size_t> offset, const MeqRequest& request,
        baseline_t baseline, bool showd = false);

//    void testFlagsBaseline(int threadnr, void* arguments,
//        VisData::Pointer chunk, const MeqRequest& request, baseline_t
//baseline,
//        bool showd);

    string                              itsInputColumn;
    bool                                itsReadUVW;
    size_t                              itsChunkSize;
    size_t                              itsChunkPosition;

    vector<int>                         itsPolarizationMap;

    scoped_ptr<ParmDB::ParmDB>          itsInstrumentDBase;
    scoped_ptr<ParmDB::ParmDB>          itsSkyDBase;
    scoped_ptr<ParmDB::ParmDB>          itsHistoryDBase;

    //# Container for all parameters.
    MeqParmGroup                        itsParmGroup;
    //# All parm values in current work domain.
    map<string, ParmDB::ParmValueSet>   itsParmValues;

    //# Phase reference position in J2000 coordinates.
    MeqPhaseRef                         itsPhaseRef;
    MeqDomain                           itsWorkDomain;

    //# Thread private buffers.
    vector<ThreadContext>               itsThreadContexts;

    NSTimer                             itsPredTimer, itsEqTimer;

    Measurement::Pointer                itsMeasurement;
    Model::Pointer                      itsModel;
    VisGrid                             itsGrid;
    VisSelection                        itsChunkSelection;
    VisData::Pointer                    itsChunkData;
    ProcessingContext                   itsContext;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
