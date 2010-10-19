//# MIM.h: Ionospheric disturbance of a (source, station) combination.
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

// \file
// Ionospheric disturbance of a (source, station) combination.

#include <BBSKernel/Expr/Expr.h>

#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class MIM: public Expr<Scalar>
{
public:
    typedef shared_ptr<MIM>         Ptr;
    typedef shared_ptr<const MIM>   ConstPtr;

    template <typename T_ITERATOR>
    MIM(const casa::MPosition &refStation, const Expr<Vector<4> >::ConstPtr &pp,
        T_ITERATOR coeffBegin, T_ITERATOR coeffEnd)
        :   itsRefStation(casa::MPosition::Convert(refStation,
                casa::MPosition::ITRF)()),
            itsPiercePoint(pp),
            itsCoeff(coeffBegin, coeffEnd)
    {
        connect(itsPiercePoint);
        for(unsigned int i = 0; i < itsCoeff.size(); ++i)
        {
            connect(itsCoeff[i]);
        }
    }

    virtual ~MIM();

private:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Scalar evaluateExpr(const Request &request, Cache &cache)
        const;

    const Scalar::view evaluateImpl(const Request &request,
        const Vector<4>::view &pp, const vector<Scalar::view> &coeff)
        const;

//    void evaluate(const Request &request, const Matrix &in_x,
//        const Matrix &in_y, const Matrix &in_z, const Matrix &in_alpha,
//        const vector<const Matrix*> &MIMParms, const double refx,
//        const double refy, const double refz, Matrix &out_11,
//        Matrix &out_22);

//    double calculate_mim_function(const vector<double> &parms, double x,
//        double y, double z, double alpha, double ref_x,
//        double ref_y, double ref_z);

    casa::MPosition                 itsRefStation;
    Expr<Vector<4> >::ConstPtr      itsPiercePoint;
    vector<Expr<Scalar>::ConstPtr > itsCoeff;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
