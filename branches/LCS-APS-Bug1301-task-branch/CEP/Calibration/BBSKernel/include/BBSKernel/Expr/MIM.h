//# MIM.h: Ionospheric disturbance of a (source,station) combination.
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

#ifndef EXPR_MIM_H
#define EXPR_MIM_H

#include <BBSKernel/Expr/JonesNode.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

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
    static const uint NPARMS = 5;

    MIM(const Expr &pp, const vector<Expr> &MIMParms, const Expr &ref_pp);
    virtual ~MIM();

    // Calculate the result of its members.
    virtual JonesResult getJResult (const Request&);

private:
    void evaluate(const Request &request, const Matrix &in_x,
        const Matrix &in_y, const Matrix &in_z, const Matrix &in_alpha,
        const vector<const Matrix*> &MIMParms, const Matrix &in_refx,
        const Matrix &in_refy, const Matrix &in_refz, Matrix &out_11,
        Matrix &out_22);

    double calculate_mim_function(const vector<double> &parms, double x,
        double y, double z, double alpha, double ref_x,
        double ref_y, double ref_z);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
