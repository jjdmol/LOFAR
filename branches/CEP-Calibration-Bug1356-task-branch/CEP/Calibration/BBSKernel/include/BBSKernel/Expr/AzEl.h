//# AzEl.h: Azimuth and elevation for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
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

#ifndef EXPR_AZEL_H
#define EXPR_AZEL_H

// \file
// Azimuth and elevation for a direction (ra,dec) on the sky.

#include <BBSKernel/Instrument.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ResultVec.h>
#include <BBSKernel/Expr/Source.h>

namespace LOFAR
{
namespace BBS
{
class Request;
class Matrix;

// \ingroup Expr
// @{

// AzEl computes azimuth and elevation coordinates for a direction (ra, dec) on
// the sky as seen from a specific location (ITRF) on earth.
//
// \todo The direction on the sky is assumed to be a source and the location
// on earth is assumed to be a station. This could be generalized if necessary.
class AzEl: public ExprRep
{
public:
    AzEl(const Station &station, const Source::ConstPtr &source);
    
    ResultVec getResultVec(const Request &request);
    
private:
    // Compute (az, el) coordinates. This method will be called multiple times
    // if any of the input values (ra, dec) have associated perturbed values.
    void evaluate(const Request &request, const Matrix &in_ra,
        const Matrix &in_dec, Matrix &out_az, Matrix &out_el);

    Station                 itsStation;
    Source::ConstPtr    itsSource;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
