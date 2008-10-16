//# GaussianCoherence.h: Spatial coherence function of an elliptical
//#     gaussian source.
//#
//# Copyright (C) 2005
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

#ifndef EXPR_GAUSSIANCOHERENCY_H
#define EXPR_GAUSSIANCOHENRECY_H

// \file
// Spatial coherence function of an elliptical gaussian source.

//# Includes
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/StatUVW.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup Expr
// @{

//# Forward Declarations


class GaussianCoherence: public JonesExprRep
{
public:
    enum
    {
        IN_I,
        IN_Q,
        IN_U,
        IN_V,
        IN_MAJOR,
        IN_MINOR,
        IN_PHI,
        N_InputPort
    } InputPort;

    GaussianCoherence(const GaussianSource *source, StatUVW *station1, StatUVW *station2);
    ~GaussianCoherence();

    // Calculate the results for the given domain.
    virtual JonesResult getJResult(const Request &request);

private:
#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    Matrix computeCoherence(const Request &request,
        const Matrix &uBaseline, const Matrix &vBaseline,
        const Matrix &major, const Matrix &minor, const Matrix &phi);

    const GaussianSource    *itsSource;
    StatUVW                 *itsStation1, *itsStation2;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
