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
#include <BBSKernel/PatchExpr.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprValue.h>
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

    StationExprLOFAR(SourceDB &sourceDB, const BufferMap &buffers,
        const ModelConfig &config, const Instrument::ConstPtr &instrument,
        double refFreq, const casa::MDirection &refPhase,
        const casa::MDirection &refDelay, const casa::MDirection &refTile,
        bool inverse = false, bool useMMSE = false, double sigmaMMSE = 0.0);

    StationExprLOFAR(SourceDB &sourceDB, const BufferMap &buffers,
        const ModelConfig &config, const VisBuffer::Ptr &buffer,
        bool inverse = false, bool useMMSE = false, double sigmaMMSE = 0.0);

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
    void initialize(SourceDB &sourceDB,
        const BufferMap &buffers,
        const ModelConfig &config,
        const Instrument::ConstPtr &instrument,
        double refFreq,
        const casa::MDirection &refPhase,
        const casa::MDirection &refDelay,
        const casa::MDirection &refTile,
        bool inverse,
        bool useMMSE,
        double sigmaMMSE);

    PatchExprBase::Ptr makePatchExpr(const string &name,
        const casa::MDirection &refPhase,
        SourceDB &sourceDB,
        const BufferMap &buffers);

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
