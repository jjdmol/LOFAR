//# StationExprLOFAR.h: Expression for the response (Jones matrix) of a set of
//# LOFAR stations.
//#
//# Copyright (C) 2010
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

#ifndef LOFAR_BBSKERNEL_STATIONEXPRLOFAR_H
#define LOFAR_BBSKERNEL_STATIONEXPRLOFAR_H

// \file
// Expression for the response (Jones matrix) of a set of LOFAR stations.

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/IonosphereExpr.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprValue.h>
#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/Scope.h>
#include <ParmDB/SourceDB.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class StationExprLOFAR: public ExprSet<JonesMatrix>
{
public:
    typedef shared_ptr<StationExprLOFAR>        Ptr;
    typedef shared_ptr<const StationExprLOFAR>  ConstPtr;

    StationExprLOFAR(SourceDB &sourceDB, const ModelConfig &config,
        const Instrument::ConstPtr &instrument,
        const casa::MDirection &phaseReference, double referenceFreq,
        bool inverse = false);

    StationExprLOFAR(SourceDB &sourceDB, const ModelConfig &config,
        const VisBuffer::Ptr &buffer, bool inverse = false);

    // \name ExprSet interface implementation
    // These methods form an implementation of the ExprSet interface.
    //
    // @{
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
    void initialize(SourceDB &sourceDB, const ModelConfig &config,
        bool inverse);

    // Expressions related to positions.
    Expr<Vector<2> >::Ptr makeSourcePositionExpr(const string &name);
    Expr<Vector<2> >::Ptr makePatchPositionExpr(SourceDB &sourceDB,
        const string &patch);
    Expr<Vector<2> >::Ptr makeRefPositionExpr(const casa::MDirection &position)
        const;
    Expr<Vector<2> >::Ptr makeAzElExpr(const Station::ConstPtr &station,
        const Expr<Vector<2> >::Ptr &direction) const;
    Expr<Vector<2> >::Ptr makeITRFExpr(const AntennaField::ConstPtr &field,
        const Expr<Vector<2> >::Ptr &direction) const;

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
        const Expr<Vector<2> >::Ptr &exprRefRaDec,
        const Expr<Vector<2> >::Ptr &exprRaDec) const;
    Expr<JonesMatrix>::Ptr makeIonosphereExpr(const Station::ConstPtr &station,
        const casa::MPosition &refPosition,
        const Expr<Vector<2> >::Ptr &exprAzEl,
        const IonosphereExpr::Ptr &exprIonosphere) const;
    Expr<JonesMatrix>::Ptr
        makeFaradayRotationExpr(const Station::ConstPtr &station,
            const string &patch);

    // Right multiply accumulator by effect. Return effect if accumulator is
    // uninitialized.
    Expr<JonesMatrix>::Ptr compose(const Expr<JonesMatrix>::Ptr &accumulator,
        const Expr<JonesMatrix>::Ptr &effect) const;

    // Load element beam model coefficients from disk.
    HamakerBeamCoeff loadBeamModelCoeff(casa::Path path,
        const AntennaField::ConstPtr &field) const;

    // Attributes.
    Instrument::ConstPtr            itsInstrument;
    casa::MDirection                itsPhaseReference;
    double                          itsReferenceFreq;

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
