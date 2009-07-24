//# SpectralIndex.h: Frequency dependent scale factor for the base flux given
//# for a specific reference frequency.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBS_EXPR_SPECTRALINDEX_H
#define LOFAR_BBS_EXPR_SPECTRALINDEX_H

// \file
// Frequency dependent scale factor for the base flux given for a specific
// reference frequency.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class SpectralIndex: public Expr<Scalar>
{
public:
    typedef shared_ptr<SpectralIndex>       Ptr;
    typedef shared_ptr<const SpectralIndex> ConstPtr;

    template <typename T_ITERATOR>
    SpectralIndex(const Expr<Scalar>::ConstPtr &refFreq, T_ITERATOR coeffBegin,
        T_ITERATOR coeffEnd)
        :   itsRefFreq(refFreq),
            itsCoeff(coeffBegin, coeffEnd)
    {
        connect(itsRefFreq);
        for(unsigned int i = 0; i < itsCoeff.size(); ++i)
        {
            connect(itsCoeff[i]);
        }
    }

    virtual ~SpectralIndex();

private:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Scalar evaluateExpr(const Request &request, Cache &cache)
        const;

    const Scalar::view evaluateImpl(const Request &request,
        const Scalar::view &refFreq, const vector<Scalar::view> &coeff) const;

    const Expr<Scalar>::ConstPtr    itsRefFreq;
    vector<Expr<Scalar>::ConstPtr>  itsCoeff;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
