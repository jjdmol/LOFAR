//# ExprVisData.h: Make visibility data from an observation available for use
//# in an expression tree.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRVISDATA_H
#define LOFAR_BBSKERNEL_EXPR_EXPRVISDATA_H

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/VisData.h>

// \file
// Make visibility data from an observation available for use in an expression
// tree.

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ExprVisData: public NullaryExpr<JonesMatrix>
{
public:
    typedef shared_ptr<ExprVisData>        Ptr;
    typedef shared_ptr<const ExprVisData>  ConstPtr;

    ExprVisData(const VisData::Ptr &chunk, const baseline_t &baseline,
        Correlation::Type element00 = Correlation::XX,
        Correlation::Type element01 = Correlation::XY,
        Correlation::Type element10 = Correlation::YX,
        Correlation::Type element11 = Correlation::YY);

//    void setCorrelations(Correlation element00, Correlation element01, Correlation element10,
//        Correlation element11);

protected:
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache&,
        unsigned int grid) const;

private:
    template <typename T_ITER>
    void makeAxisMapping(const Axis::ShPtr &from, const Axis::ShPtr &to,
        T_ITER out) const;

    void setCorrelation(size_t element, Correlation::Type correlation);

    FlagArray copyFlags(const Grid &grid, size_t element,
        const vector<pair<size_t, size_t> > (&mapping)[2]) const;

    Matrix copyData(const Grid &grid, size_t element,
        const vector<pair<size_t, size_t> > (&mapping)[2]) const;

    VisData::Ptr    itsChunk;
    size_t          itsBaseline;
    bool            itsCorrMask[4];
    size_t          itsCorr[4];
};

// @}

// -------------------------------------------------------------------------- //
// - ExprVisData implementation                                             - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
void ExprVisData::makeAxisMapping(const Axis::ShPtr &from,
    const Axis::ShPtr &to, T_ITER out) const
{
    Interval<double> overlap(std::max(from->start(), to->start()),
        std::min(from->end(), to->end()));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return;
    }

    Interval<size_t> domain;
    domain.start = from->locate(overlap.start);
    domain.end = from->locate(overlap.end, false, domain.start);

    // Intervals are inclusive by convention.
    const size_t nCells = domain.end - domain.start + 1;

    // Special case for the first cell: cell center may be located outside of
    // the overlap between the "from" and "to" axis.
    size_t target = 0;
    double center = from->center(domain.start);
    if(center > overlap.start || casa::near(center, overlap.start))
    {
        target = to->locate(center);
        *out++ = make_pair(domain.start, target);
    }

    for(size_t i = domain.start + 1; i < domain.end; ++i)
    {
        target = to->locate(from->center(i), true, target);
        *out++ = make_pair(i, target);
    }

    if(nCells > 1)
    {
        // Special case for the last cell: cell center may be located outside of
        // the overlap between the "from" and "to" axis.
        center = from->center(domain.end);
        if(center < overlap.end && !casa::near(center, overlap.end))
        {
            target = to->locate(center, false, target);
            *out++ = make_pair(domain.end, target);
        }
    }
}

} // namespace BBS
} // namespace LOFAR

#endif
