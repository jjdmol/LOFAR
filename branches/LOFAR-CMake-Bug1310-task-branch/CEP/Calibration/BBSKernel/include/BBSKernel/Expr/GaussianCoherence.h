//# GaussianCoherence.h: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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
    ~GaussianCoherence();

    // Calculate the results for the given domain.
    virtual JonesResult getJResult(const Request &request);

private:
#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    Matrix computeCoherence(const Request &request, const Matrix &uBaseline,
        const Matrix &vBaseline, const Matrix &major, const Matrix &minor,
        const Matrix &phi);

    GaussianSource::ConstPointer    itsSource;
    StatUVW::ConstPointer           itsStation1, itsStation2;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
