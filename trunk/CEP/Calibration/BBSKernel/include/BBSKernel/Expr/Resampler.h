//# Resampler.h: Resample input to a different sample grid.
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

#ifndef LOFAR_BBSKERNEL_EXPR_RESAMPLER_H
#define LOFAR_BBSKERNEL_EXPR_RESAMPLER_H

// \file
// Resample input to a different sample grid.

#include <BBSKernel/Expr/Expr.h>

#include <casa/BasicMath/Math.h>
#include <iterator>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// TODO: Make class template <T_EXPR, T_EXPR>.
class Resampler: public UnaryExpr<JonesMatrix, JonesMatrix>
{
public:
    typedef shared_ptr<Resampler>       Ptr;
    typedef shared_ptr<const Resampler> ConstPtr;

    Resampler(const Expr<JonesMatrix>::ConstPtr &arg, unsigned int resolution,
        double flagDensityThreshold = 0.5);

protected:
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    struct Span
    {
        unsigned int    src, dst;
        double          weight;
    };

    template <typename T_ITER>
    void makeAxisMap(const Axis::ShPtr &from, const Axis::ShPtr &to,
        T_ITER map) const;

    FlagArray resampleFlags(const FlagArray &in, const vector<Span> (&map)[2])
        const;
    Matrix resampleWithFlags(const Matrix &in, const FlagArray &flags,
        const vector<Span> (&map)[2]) const;
    Matrix resample(const Matrix &in, const vector<Span> (&map)[2]) const;

    unsigned int itsGridId;
    double       itsFlagDensityThreshold;
};

// @}

template <typename T_ITER>
void Resampler::makeAxisMap(const Axis::ShPtr &from, const Axis::ShPtr &to,
    T_ITER map) const
{
    // Check preconditions: the start and end of the axes should line up (which
    // is guaranteed because the domains of the two grid has to be the same) and
    // they should contain at least a single cell.
    DBGASSERT(from->size() > 0 && to->size() > 0);
    DBGASSERT(casa::near(from->lower(0), to->lower(0)));
    DBGASSERT(casa::near(from->upper(from->size() - 1),
        to->upper(to->size() - 1)));

    const size_t nFrom = from->size();
    const size_t nTo = to->size();

    Span span;
    unsigned int i = 0, j = 0;
    double lower = to->lower(0);
    while(i < nFrom && j < nTo)
    {
        span.src = i;
        span.dst = j;

        if(casa::near(from->upper(i), to->upper(j)))
        {
            span.weight = (to->upper(j) - lower) / from->width(i);
            lower = to->upper(j);
            ++i;
            ++j;
        }
        else if(from->upper(i) < to->upper(j))
        {
            span.weight = (from->upper(i) - lower) / from->width(i);
            lower = from->upper(i);
            ++i;
        }
        else
        {
            span.weight = (to->upper(j) - lower) / from->width(i);
            lower = to->upper(j);
            ++j;
        }

        *map++ = span;
    }

    //# Handle any remaining input cells (which are completely contained within
    //# the last output cell).
    while(i < nFrom)
    {
        span.src = i;
        span.dst = nTo - 1;
        span.weight = 1.0;
        ++i;

        *map++ = span;
    }

    //# Handle any remaining output cells (which are completely contained within
    //# the last input cell.
    while(j < nTo)
    {
        span.src = nFrom - 1;
        span.dst = j;
        span.weight = to->width(j) / from->width(i);
        ++j;

        *map++ = span;
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
