//# HamakerDipole.h: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna".
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

#ifndef LOFAR_BBSKERNEL_EXPR_HAMAKERDIPOLE_H
#define LOFAR_BBSKERNEL_EXPR_HAMAKERDIPOLE_H

// \file
// Implementation of J.P. Hamaker's memo "Mathematical-physical analysis of the
// generic dual-dipole antenna".

#include <BBSKernel/Expr/BasicExpr.h>
#include <Common/lofar_complex.h>

#include <casa/Arrays.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class HamakerBeamCoeff
{
public:
    HamakerBeamCoeff();

    void init(const string &filename);

    // Center frequency used to scale frequency to range [-1.0, 1.0].
    double center() const;
    // Width used to scale frequency to range [-1.0, 1.0].
    double width() const;

    unsigned int shape(unsigned int i) const;

    dcomplex operator()(unsigned int element, unsigned int harmonic,
        unsigned int powTheta, unsigned int powFreq) const;

private:
    double                  itsCenter, itsWidth;
    casa::Array<dcomplex>   itsCoeff;
};

class HamakerDipole: public BasicBinaryExpr<Vector<2>, Scalar, JonesMatrix>
{
public:
    typedef shared_ptr<HamakerDipole>       Ptr;
    typedef shared_ptr<const HamakerDipole> ConstPtr;

    HamakerDipole(const HamakerBeamCoeff &coeff,
        const Expr<Vector<2> >::ConstPtr &azel,
        const Expr<Scalar>::ConstPtr &orientation);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const Vector<2>::View &azel, const Scalar::View &orientation) const;

private:
    HamakerBeamCoeff    itsCoeff;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: HamakerBeamCoeff                                       - //
// -------------------------------------------------------------------------- //

inline double HamakerBeamCoeff::center() const
{
    return itsCenter;
}

inline double HamakerBeamCoeff::width() const
{
    return itsWidth;
}

inline unsigned int HamakerBeamCoeff::shape(unsigned int i) const
{
    // Reverse axes because casa::Array<> uses fortran order.
    DBGASSERT(i < 4);
    return itsCoeff.shape()(3 - i);
}

inline dcomplex HamakerBeamCoeff::operator()(unsigned int element,
    unsigned int harmonic, unsigned int powTheta, unsigned int powFreq) const
{
    // Reverse axes because casa::Array<> uses fortran order.
    return itsCoeff(casa::IPosition(4, powFreq, powTheta, harmonic, element));
}

} //# namespace BBS
} //# namespace LOFAR

#endif
