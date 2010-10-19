//# PiercePoint.h: Pierce point for a direction (az,el) on the sky.
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

#ifndef EXPR_PIERCEPOINT_H
#define EXPR_PIERCEPOINT_H

#include <BBSKernel/Instrument.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ResultVec.h>


namespace LOFAR
{
namespace BBS
{
  
// \ingroup Expr
// @{

const double iono_height=400000.; //height (m) 

class PiercePoint: public ExprRep
{
public:
    PiercePoint(const Station &station, const Expr &direction);
    ResultVec getResultVec(const Request &request);
    
private:
    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, Matrix &out_x, Matrix &out_y, Matrix &out_z,
        Matrix &out_alpha);


    Station                 itsStation;
    double itsLong; //longitude of station
    double itsLat; //latitude of station
    double itsHeight; // height above earth-surface of station
    double itsEarthRadius; // earth radius at long lat of station
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
