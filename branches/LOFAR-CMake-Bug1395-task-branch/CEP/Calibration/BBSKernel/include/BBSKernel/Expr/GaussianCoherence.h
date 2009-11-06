//# GaussianCoherence.h: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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

#ifndef EXPR_GAUSSIANCOHERENCE_H
#define EXPR_GAUSSIANCOHERENCE_H

// \file
// Spatial coherence function of an elliptical gaussian source.

#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/StatUVW.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class GaussianCoherence: public JonesExprRep
{
public:
    GaussianCoherence(const GaussianSource::ConstPointer &source,
        const StatUVW::ConstPointer &station1,
        const StatUVW::ConstPointer &station2);

    // Calculate the results for the given domain.
    virtual JonesResult getJResult(const Request &request);

private:
    Matrix computeCoherence(const Request &request, const Matrix &uBaseline,
        const Matrix &vBaseline, const Matrix &major, const Matrix &minor,
        const Matrix &phi);

    StatUVW::ConstPointer           itsStation1, itsStation2;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
