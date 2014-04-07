//# MeasurementExprLOFAR.h: Measurement equation for the LOFAR telescope and its
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

#ifndef LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFAR_H
#define LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFAR_H

// \file
// Measurement equation for the LOFAR telescope and its environment.


#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/IonosphereExpr.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/Source.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/SourceDB.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

class MeasurementExprLOFAR: public MeasurementExpr
{
public:
    typedef shared_ptr<MeasurementExprLOFAR>        Ptr;
    typedef shared_ptr<const MeasurementExprLOFAR>  ConstPtr;

    MeasurementExprLOFAR(SourceDB &sourceDB, const ModelConfig &config,
        const Instrument::ConstPtr &instrument, const BaselineSeq &baselines,
        const casa::MDirection &phaseReference, double referenceFreq,
        bool circular = false);

    MeasurementExprLOFAR(SourceDB &sourceDB, const ModelConfig &config,
        const VisBuffer::Ptr &buffer, const BaselineMask &mask,
        bool inverse = false);

    // \name MeasurementExpr interface implementation
    // These methods form an implementation of the MeasurementExpr interface
    // (and consequently of the ExprSet interface as well).
    //
    // @{
    virtual const BaselineSeq &baselines() const;
    virtual const CorrelationSeq &correlations() const;

    virtual size_t size() const;
    virtual Box domain() const;

    virtual ParmGroup parms() const;
    virtual size_t nParms() const;
    virtual ParmGroup solvables() const;
    virtual void setSolvables(const ParmGroup &solvables);
    virtual void clearSolvables();
    virtual void solvablesChanged();

    virtual void setEvalGrid(const Grid &grid);
    virtual const JonesMatrix evaluate(unsigned int i);
    // @}

private:
    void makeForwardExpr(const ModelConfig &config,
        const casa::MDirection &phaseReference, double refFreq, bool circular);

    void makeInverseExpr(const ModelConfig &config,
        const VisBuffer::Ptr &buffer, const casa::MDirection &phaseReference,
        double refFreq, bool circular);

    void setCorrelations(bool circular);
    bool isLinear(const VisBuffer::Ptr &buffer) const;
    bool isCircular(const VisBuffer::Ptr &buffer) const;

    vector<string> makePatchList(const vector<string> &patterns);
    vector<Source::Ptr> makeSourceList(const string &patch);

    // Expressions related to positions.
    Expr<Vector<2> >::Ptr makeRefPositionExpr(const casa::MDirection &position)
        const;
    Expr<Vector<2> >::Ptr
        makePatchPositionExpr(const vector<Source::Ptr> &sources) const;
    Expr<Vector<3> >::Ptr makeUVWExpr(const casa::MPosition &array,
        const casa::MPosition &station, const casa::MDirection &direction)
        const;
    Expr<Vector<2> >::Ptr makeAzElExpr(const Station::ConstPtr &station,
        const Expr<Vector<2> >::Ptr &direction) const;

    // Expressions related to patch coherence.
    Expr<Vector<3> >::Ptr makeLMNExpr(const casa::MDirection &reference,
        const Expr<Vector<2> >::Ptr &direction) const;
    Expr<Vector<2> >::Ptr
        makeStationShiftExpr(const Expr<Vector<3> >::Ptr &exprUVW,
            const Expr<Vector<3> >::Ptr &exprLMN) const;
    Expr<JonesMatrix>::Ptr
        makePatchCoherenceExpr(const Expr<Vector<3> >::Ptr &uvwLHS,
            const vector<Expr<Vector<2> >::Ptr> &shiftLHS,
            const Expr<Vector<3> >::Ptr &uvwRHS,
            const vector<Expr<Vector<2> >::Ptr> &shiftRHS,
            const vector<Source::Ptr> &sources) const;

    // Direction independent effects.
    Expr<JonesMatrix>::Ptr makeBandpassExpr(const Station::ConstPtr &station);
    Expr<JonesMatrix>::Ptr makeClockExpr(const Station::ConstPtr &station);
    Expr<JonesMatrix>::Ptr makeGainExpr(const Station::ConstPtr &station,
        bool phasors);

    // Direction dependent effects.
    Expr<JonesMatrix>::Ptr
        makeDirectionalGainExpr(const Station::ConstPtr &station,
            const string &patch, bool phasors);
    Expr<JonesMatrix>::Ptr makeBeamExpr(const Station::ConstPtr &station,
        double referenceFreq, const BeamConfig &config,
        const HamakerBeamCoeff &coeffLBA,
        const HamakerBeamCoeff &coeffHBA,
        const Expr<Vector<2> >::Ptr &exprRaDec,
        const Expr<Vector<2> >::Ptr &exprRefRaDec) const;
    Expr<JonesMatrix>::Ptr makeIonosphereExpr(const Station::ConstPtr &station,
        const casa::MPosition &refPosition,
        const Expr<Vector<2> >::Ptr &exprAzEl,
        const IonosphereExpr::Ptr &exprIonosphere) const;
    Expr<JonesMatrix>::Ptr
        makeFaradayRotationExpr(const Station::ConstPtr &station,
            const string &patch);

    // Right multiply \p accumulator by \p effect. Return \p effect if
    // \p accumulator is uninitialized.
    Expr<JonesMatrix>::Ptr compose(const Expr<JonesMatrix>::Ptr &accumulator,
        const Expr<JonesMatrix>::Ptr &effect) const;

    // Construct \p lhs * \p coherence * (\p rhs)^H. Return \p coherence if
    // either \p lhs or \p rhs are uninitialized.
    Expr<JonesMatrix>::Ptr corrupt(const Expr<JonesMatrix>::Ptr &lhs,
        const Expr<JonesMatrix>::Ptr &coherence,
        const Expr<JonesMatrix>::Ptr &rhs) const;

    SourceDB                        itsSourceDB;
    Instrument::ConstPtr            itsInstrument;

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
