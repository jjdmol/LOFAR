//# GaussianSource.h: Class holding the expressions defining a gaussian
//# source.
//#
//# Copyright (C) 2002
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

#ifndef MNS_MEQGAUSSIANSOURCE_H
#define MNS_MEQGAUSSIANSOURCE_H

// \file
// Class holding the expressions defining a gaussian source.

#include <BBSKernel/MNS/MeqSource.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup MNS
// @{

class GaussianSource: public Source
{
public:
    typedef shared_ptr<GaussianSource>       Pointer;
    typedef shared_ptr<const GaussianSource> ConstPointer;

    GaussianSource(const string& name, const Expr &ra, const Expr &dec,
        const Expr &I, const Expr &Q, const Expr &U, const Expr &V,
        const Expr &major, const Expr &minor, const Expr &phi);

    virtual ~GaussianSource();

    const Expr &getI() const
    { return itsI; }
    const Expr &getQ() const
    { return itsQ; }
    const Expr &getU() const
    { return itsU; }
    const Expr &getV() const
    { return itsV; }
    const Expr &getMajor() const
    { return itsMajor; }
    const Expr &getMinor() const
    { return itsMinor; }
    const Expr &getPhi() const
    { return itsPhi; }

private:
    Expr itsI;
    Expr itsQ;
    Expr itsU;
    Expr itsV;
    Expr itsMajor;
    Expr itsMinor;
    Expr itsPhi;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
