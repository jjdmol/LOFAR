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
#include <BBSKernel/PatchExpr.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Expr.h>
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

    MeasurementExprLOFAR(SourceDB &sourceDB,
        const BufferMap &buffers,
        const ModelConfig &config,
        const Instrument::ConstPtr &instrument,
        const BaselineSeq &baselines,
        double refFreq,
        const casa::MDirection &refPhase,
        const casa::MDirection &refDelay,
        const casa::MDirection &refTile,
        bool circular = false);

    MeasurementExprLOFAR(SourceDB &sourceDB,
        const BufferMap &buffers,
        const ModelConfig &config,
        const VisBuffer::Ptr &buffer,
        const BaselineMask &mask,
        bool inverse = false,
        bool useMMSE = false,
        double sigmaMMSE = 0.0);

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
    void makeForwardExpr(SourceDB &sourceDB,
        const BufferMap &buffers,
        const ModelConfig &config,
        const Instrument::ConstPtr &instrument,
        double refFreq,
        const casa::MDirection &refPhase,
        const casa::MDirection &refDelay,
        const casa::MDirection &refTile,
        bool circular);

    void makeInverseExpr(SourceDB &sourceDB,
        const BufferMap &buffers,
        const ModelConfig &config,
        const VisBuffer::Ptr &buffer,
        bool useMMSE,
        double sigmaMMSE);

    void setCorrelations(bool circular);

    vector<string> makePatchList(SourceDB &sourceDB, vector<string> patterns);

    PatchExprBase::Ptr makePatchExpr(const string &name,
        const casa::MDirection &phaseRef,
        SourceDB &sourceDB,
        const BufferMap &buffers);

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
