//# VisExpr.h: ExprSet interface to observed visibility data.
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

#ifndef LOFAR_BBSKERNEL_VISEXPR_H
#define LOFAR_BBSKERNEL_VISEXPR_H

// \file
// ExprSet interface to observed visibility data.

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Expr/Expr.h>

#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class VisExpr: public ExprSet<JonesMatrix>
{
public:
    typedef shared_ptr<VisExpr>         Ptr;
    typedef shared_ptr<const VisExpr>   ConstPtr;

    VisExpr(const Instrument &instrument,
      const casa::MDirection &reference, const VisData::Ptr &chunk,
      const vector<baseline_t> &baselines, bool shift,
      const casa::MDirection &target, bool resample,
      double flagDensityThreshold);

    virtual unsigned int size() const;
    virtual Box domain() const;

    virtual ParmGroup getParms() const;
    virtual ParmGroup getSolvableParms() const;
    virtual void setSolvableParms(const ParmGroup&);
    virtual void clearSolvableParms();

    virtual void setEvalGrid(const Grid &grid);
    virtual const JonesMatrix evaluate(unsigned int i);

private:
    vector<unsigned int>
        makeUsedStationList(const vector<baseline_t> &baselines) const;

    Request                                 itsRequest;
    VisData::Ptr                            itsChunk;
    bool                                    itsResampleFlag;
    vector<Expr<JonesMatrix>::Ptr>          itsExpr;
    Cache                                   itsCache;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
