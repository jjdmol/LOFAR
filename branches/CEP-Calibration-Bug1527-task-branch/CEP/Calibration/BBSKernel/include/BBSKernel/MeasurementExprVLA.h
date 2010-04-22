//# MeasurementExprVLA.h: Measurement equation for the VLA telescope and its
//# environment.
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

#ifndef LOFAR_BBSKERNEL_MEASUREMENTEXPRVLA_H
#define LOFAR_BBSKERNEL_MEASUREMENTEXPRVLA_H

// \file
// Measurement equation for the VLA telescope and its environment.

#include <BBSKernel/Instrument.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/VisData.h>

#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/Source.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/SourceDB.h>

#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

#include <casa/Arrays.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

class MeasurementExprVLA: public MeasurementExpr
{
public:
    typedef shared_ptr<MeasurementExprVLA>          Ptr;
    typedef shared_ptr<const MeasurementExprVLA>    ConstPtr;

    MeasurementExprVLA(const Instrument &instrument, const SourceDB &sourceDb,
        const casa::MDirection &reference, double referenceFreq);

    void clear();

    void makeForwardExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    void makeInverseExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    // \name MeasurementExpr interface implementation
    // These methods form an implementation of the MeasurementExpr interface
    // (and consequently of the ExprSet interface as well).
    //
    // @{
    virtual const BaselineSeq &baselines() const;
    virtual const CorrelationSeq &correlations() const;

    virtual unsigned int size() const;
    virtual Box domain() const;

    virtual ParmGroup parms() const;
    virtual ParmGroup solvables() const;
    virtual void setSolvables(const ParmGroup &solvables);
    virtual void clearSolvables();
    virtual void solvablesChanged();

    virtual void setEvalGrid(const Grid &grid);
    virtual const JonesMatrix evaluate(unsigned int i);
    // @}

private:
    void applyCachePolicy(const ModelConfig &config) const;

    vector<unsigned int>
        makeUsedStationList(const vector<baseline_t> &baselines) const;

    pair<unsigned int, unsigned int>
        findStationIndices(const vector<unsigned int> &stations,
            const baseline_t &baseline) const;

    vector<string> makePatchList(const vector<string> &patterns);

    vector<Source::Ptr> makeSourceList(const string &patch);

    Expr<Vector<2> >::ConstPtr
        makePatchCentroidExpr(const vector<Source::Ptr> &sources) const;

    Expr<JonesMatrix>::Ptr
        makePatchCoherenceExpr(const vector<Source::Ptr> &sources,
            const Expr<Vector<3> >::Ptr &uvwLHS,
            const casa::Vector<Expr<Vector<2> >::Ptr> &shiftLHS,
            const Expr<Vector<3> >::Ptr &uvwRHS,
            const casa::Vector<Expr<Vector<2> >::Ptr> &shiftRHS) const;

    casa::Vector<Expr<Vector<3> >::Ptr>
        makeUVWExpr(const vector<unsigned int> &stations);

    casa::Vector<Expr<Vector<2> >::Ptr>
        makeAzElExpr(const vector<unsigned int> &stations,
            const Expr<Vector<2> >::ConstPtr &direction) const;

    casa::Vector<Expr<Vector<2> >::Ptr>
        makeRefAzElExpr(const vector<unsigned int> &stations) const;

    casa::Matrix<Expr<Vector<2> >::Ptr>
        makeStationShiftExpr(const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources,
            const casa::Vector<Expr<Vector<3> >::Ptr> &exprUVW) const;

    void makeGainExpr(const ModelConfig &config,
            const vector<unsigned int> &stations,
            casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator);

    void makeDirectionalGainExpr(const ModelConfig &config,
        const vector<unsigned int> &stations, const string &patch,
        casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator);

    Expr<JonesMatrix>::Ptr compose(const Expr<JonesMatrix>::Ptr &accumulator,
        const Expr<JonesMatrix>::Ptr &effect) const;

    Expr<JonesMatrix>::Ptr corrupt(const Expr<JonesMatrix>::Ptr &lhs,
        const Expr<JonesMatrix>::Ptr &coherence,
        const Expr<JonesMatrix>::Ptr &rhs) const;

    Instrument                      itsInstrument;
    SourceDB                        itsSourceDb;
    casa::MDirection                itsPhaseReference;
    double                          itsReferenceFreq;

    BaselineSeq                     itsBaselines;
    CorrelationSeq                  itsCorrelations;

    Request                         itsRequest;
    Cache                           itsCache;

    vector<Expr<JonesMatrix>::Ptr>  itsExpr;
    Scope                           itsScope;
    CachePolicy::Ptr                itsCachePolicy;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
