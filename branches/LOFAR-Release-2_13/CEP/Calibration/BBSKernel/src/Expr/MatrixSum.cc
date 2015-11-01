//# MatrixSum.h: Compute the (element-wise) sum of a collection of Jones
//# matrices.
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <lofar_config.h>

#include <BBSKernel/Expr/MatrixSum.h>

namespace LOFAR
{
namespace BBS
{

MatrixSum::~MatrixSum()
{
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        disconnect(itsExpr[i]);
    }
}

void MatrixSum::connect(const Expr<JonesMatrix>::ConstPtr &expr)
{
    ExprBase::connect(expr);
    itsExpr.push_back(expr);
}


unsigned int MatrixSum::nArguments() const
{
    return itsExpr.size();
}

ExprBase::ConstPtr MatrixSum::argument(unsigned int i) const
{
    ASSERT(i < itsExpr.size());
    return itsExpr[i];
}

const JonesMatrix MatrixSum::evaluateExpr(const Request &request, Cache &cache,
    unsigned int grid) const
{
    EXPR_TIMER_START();

    // TODO: Use unsigned integer type for Matrix size.
    int nx = request[grid][FREQ]->size();
    int ny = request[grid][TIME]->size();

    // Allocate Jones matrix elements and initialize the main value to 0 + 0i.
    Element element[2][2];
    element[0][0].assign(Matrix(makedcomplex(0.0, 0.0), nx, ny));
    element[0][1].assign(Matrix(makedcomplex(0.0, 0.0), nx, ny));
    element[1][0].assign(Matrix(makedcomplex(0.0, 0.0), nx, ny));
    element[1][1].assign(Matrix(makedcomplex(0.0, 0.0), nx, ny));

    // Allocate flags.
    bool haveFlags = false;
    FlagArray flags(flag_t(0));

    // Iterate over all terms.
    // TODO: First process all nodes that do not depend on solvables and cache
    // this sum somehow.
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        EXPR_TIMER_STOP();
        const JonesMatrix term = itsExpr[i]->evaluate(request, cache, grid);
        EXPR_TIMER_START();

        // Update flags.
        if(term.hasFlags())
        {
            flags |= term.flags();
            haveFlags = true;
        }

        // Update perturbed values.
        merge(term.element(0, 0), element[0][0]);
        merge(term.element(0, 1), element[0][1]);
        merge(term.element(1, 0), element[1][0]);
        merge(term.element(1, 1), element[1][1]);

        // Update main value (location is important!!).
        element[0][0].value() += term.element(0, 0).value();
        element[0][1].value() += term.element(0, 1).value();
        element[1][0].value() += term.element(1, 0).value();
        element[1][1].value() += term.element(1, 1).value();
    }

    JonesMatrix result(element[0][0], element[0][1], element[1][0],
        element[1][1]);
    if(haveFlags)
    {
        result.setFlags(flags);
    }

    EXPR_TIMER_STOP();

    return result;
}

void MatrixSum::merge(const Element &in, Element &out) const
{
    Element::const_iterator inIter = in.begin();
    Element::const_iterator inEnd = in.end();

    Element::iterator outIter = out.begin();
    Element::iterator outEnd = out.end();

    while(inIter != inEnd && outIter != outEnd)
    {
        if(outIter->first == inIter->first)
        {
            outIter->second += inIter->second;
            ++inIter;
            ++outIter;
        }
        else if(outIter->first < inIter->first)
        {
            outIter->second += in.value();
            ++outIter;
        }
        else
        {
            out.assign(inIter->first, out.value() + inIter->second);
            ++inIter;
        }
    }

    while(inIter != inEnd)
    {
        out.assign(inIter->first, out.value() + inIter->second);
        ++inIter;
    }

    while(outIter != outEnd)
    {
        outIter->second += in.value();
        ++outIter;
    }
}

} // namespace BBS
} // namespace LOFAR
