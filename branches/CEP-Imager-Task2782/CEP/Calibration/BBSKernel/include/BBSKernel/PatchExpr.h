//# PatchExpr.h: Factory classes that construct expressions for the visibilities
//# of a patch on a given baseline, sharing station bound sub-expressions.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_PATCHEXPR_H
#define LOFAR_BBSKERNEL_PATCHEXPR_H

// \file
// Factory classes that construct expressions for the visibilities of a patch on
// a given baseline, sharing station bound sub-expressions.

#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Source.h>
#include <ParmDB/SourceDB.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class PatchExprBase
{
public:
    typedef shared_ptr<PatchExprBase>       Ptr;
    typedef shared_ptr<const PatchExprBase> ConstPtr;

    virtual ~PatchExprBase();

    // Return the name of the patch.
    virtual const string &name() const = 0;

    // Return an expression that represents the (centroid) position of the
    // patch.
    virtual Expr<Vector<2> >::Ptr position() const = 0;

    // Construct an expression that represents the visibilities of the patch for
    // the given baseline.
    virtual Expr<JonesMatrix>::Ptr coherence(const baseline_t &baseline,
        const Expr<Vector<3> >::Ptr &uvwLHS,
        const Expr<Vector<3> >::Ptr &uvwRHS) const = 0;
};

class PatchExpr: public PatchExprBase
{
public:
    typedef shared_ptr<PatchExpr>       Ptr;
    typedef shared_ptr<const PatchExpr> ConstPtr;

    PatchExpr(Scope &scope, SourceDB &sourceDB, const string &name,
        const casa::MDirection &refPhase);

    // \name PatchExprBase interface implementation.
    // These methods form an implementation of the PatchExprBase interface. See
    // that class for function documentation.
    //
    // @{
    virtual const string &name() const;
    virtual Expr<Vector<2> >::Ptr position() const;
    virtual Expr<JonesMatrix>::Ptr coherence(const baseline_t &baseline,
        const Expr<Vector<3> >::Ptr &uvwLHS,
        const Expr<Vector<3> >::Ptr &uvwRHS) const;
    // @}
    
private:
    size_t nSources() const;
    void initSourceList(Scope &scope, SourceDB &sourceDB, const string &name);
    void initPositionExpr(const vector<Source::Ptr> &sources);
    void initLMNExpr(const vector<Source::Ptr> &sources,
        const casa::MDirection &refPhase);

    Expr<Vector<2> >::Ptr makeStationShiftExpr(unsigned int station,
        unsigned int source, const Expr<Vector<3> >::Ptr &uvw) const;

    string                                  itsName;
    vector<Source::Ptr>                     itsSourceList;
    Expr<Vector<2> >::Ptr                   itsPosition;
    vector<Expr<Vector<3> >::Ptr>           itsLMN;
    mutable vector<Expr<Vector<2> >::Ptr>   itsShift;
};

class StoredPatchExpr: public PatchExprBase
{
public:
    typedef shared_ptr<StoredPatchExpr>         Ptr;
    typedef shared_ptr<const StoredPatchExpr>   ConstPtr;

    StoredPatchExpr(const string &name, const VisBuffer::Ptr &buffer);

    // \name PatchExprBase interface implementation.
    // These methods form an implementation of the PatchExprBase interface. See
    // that class for function documentation.
    //
    // @{
    virtual const string &name() const;
    virtual Expr<Vector<2> >::Ptr position() const;
    virtual Expr<JonesMatrix>::Ptr coherence(const baseline_t &baseline,
        const Expr<Vector<3> >::Ptr &uvwLHS,
        const Expr<Vector<3> >::Ptr &uvwRHS) const;
    // @}

private:
    string                                  itsName;
    VisBuffer::Ptr                          itsBuffer;
    Expr<Vector<2> >::Ptr                   itsPosition;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
