//# MeasurementExprLOFARGen.h: Helper functions to generate a LOFAR measurement
//# expression from a set of model configuration options.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFARGEN_H
#define LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFARGEN_H

// \file
// Helper functions to generate a LOFAR measurement expression from a set of
// model configuration options.

#include <BBSKernel/Instrument.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/Source.h>
#include <Common/lofar_vector.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

vector<Expr<JonesMatrix>::Ptr>
makeMeasurementExpr(Scope &scope, const vector<Source::Ptr> &sources,
    const Instrument::ConstPtr &instrument, const BaselineSeq &baselines,
    const ModelConfig &config, double refFreq, const casa::MDirection &refPhase,
    const casa::MDirection &refDelay, const casa::MDirection &refTile,
    bool circular = false);

vector<Expr<JonesMatrix>::Ptr>
makeStationExpr(Scope &scope, const casa::MDirection &direction,
    const Instrument::ConstPtr &instrument, const ModelConfig &config,
    double refFreq, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool inverse = true, bool useMMSE = false,
    double sigmaMMSE = 0.0);

vector<Expr<JonesMatrix>::Ptr>
makeStationExpr(Scope &scope, const vector<Source::Ptr> &sources,
    const Instrument::ConstPtr &instrument, const ModelConfig &config,
    double refFreq, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool inverse = true, bool useMMSE = false,
    double sigmaMMSE = 0.0);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
