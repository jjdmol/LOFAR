//# MIM.h: Ionospheric disturbance of a (source,station) combination.
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

#ifndef EXPR_MIM_H
#define EXPR_MIM_H

#include <BBSKernel/Expr/JonesNode.h>
#include <BBSKernel/Instrument.h>


namespace LOFAR
{
namespace BBS
{
class Request;
class Matrix;

// \ingroup Expr
// @{

class MIM: public JonesExprRep
{
public:
  uint NPARMS;
  
  MIM(const Expr &pp, const vector<Expr> &MIMParms, const Station &ref_station);
  virtual ~MIM();
  
  // Calculate the result of its members.
  virtual JonesResult getJResult (const Request&);

private:
    void evaluate(const Request &request, const Matrix &in_x,
        const Matrix &in_y, const Matrix &in_z, const Matrix &in_alpha,
        const vector<const Matrix*> &MIMParms, const double refx,
        const double refy, const double refz, Matrix &out_11,
        Matrix &out_22);

    double calculate_mim_function(const vector<double> &parms, double x,
        double y, double z, double alpha, double ref_x,
        double ref_y, double ref_z);

    Station                 itsRefStation;

};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
