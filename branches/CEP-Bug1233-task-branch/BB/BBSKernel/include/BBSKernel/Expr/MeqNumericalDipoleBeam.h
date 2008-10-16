//# NumericalDipoleBeam.h: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna"
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

#ifndef Expr_MEQNUMERICALDIPOLEBEAM_H
#define Expr_MEQNUMERICALDIPOLEBEAM_H

#include <BBSKernel/Expr/MeqExpr.h>
#include <BBSKernel/Expr/MeqJonesExpr.h>
#include <BBSKernel/Expr/MeqJonesResult.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_complex.h>

#include <boost/multi_array.hpp>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup Expr
// @{

class BeamCoeff
{
public:
    BeamCoeff()
        : freqAvg(0.0), freqRange(1.0)
    {}

    BeamCoeff(double avg, double range,
        const shared_ptr<const boost::multi_array<dcomplex, 4> > &ptr)
        :   freqAvg(avg),
            freqRange(range),
            coeff(ptr)
    {}

    double  freqAvg, freqRange;
    shared_ptr<const boost::multi_array<dcomplex, 4> > coeff;
};


class NumericalDipoleBeam: public JonesExprRep
{
public:
    enum
    {
        IN_AZEL,
        IN_ORIENTATION,
        N_InputPort
    } InputPort;
    
    NumericalDipoleBeam(const BeamCoeff &coeff, const Expr &azel,
        const Expr &orientation);

    virtual JonesResult getJResult(const Request &request);

    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, const Matrix &in_orientation,
        Matrix &out_E11, Matrix &out_E12, 
        Matrix &out_E21, Matrix &out_E22);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

private:
    BeamCoeff   itsBeamCoeff;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
