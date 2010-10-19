//# JonesSum.cc: A sum of JonesExpr results
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>

#include <BBSKernel/Expr/JonesSum.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

namespace LOFAR
{
namespace BBS
{

JonesSum::JonesSum(const vector<JonesExpr> &expr)
    : itsExpr(expr)
{
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        addChild(itsExpr[i]);
    }
}

JonesSum::~JonesSum()
{
}

JonesResult JonesSum::getJResult(const Request &request)
{
    int nx = request.getChannelCount();
    int ny = request.getTimeslotCount();

    JonesResult result;
    result.init();

    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();

    // Allocate the result and initialize to 0.
    result11.setValue(Matrix(makedcomplex(0,0), nx, ny));
    result12.setValue(Matrix(makedcomplex(0,0), nx, ny));
    result21.setValue(Matrix(makedcomplex(0,0), nx, ny));
    result22.setValue(Matrix(makedcomplex(0,0), nx, ny));

    // Iterate over all terms.
    for(vector<JonesExpr>::iterator it = itsExpr.begin();
        it != itsExpr.end();
        ++it)
    {
        JonesResult termBuf;
        const JonesResult &term = it->getResultSynced(request, termBuf);

        mergePValues(term.getResult11(), result11);
        mergePValues(term.getResult12(), result12);
        mergePValues(term.getResult21(), result21);
        mergePValues(term.getResult22(), result22);

        // Update main value (location is important!!).
        result11.getValueRW() += term.getResult11().getValue();
        result12.getValueRW() += term.getResult12().getValue();
        result21.getValueRW() += term.getResult21().getValue();
        result22.getValueRW() += term.getResult22().getValue();
    }

    return result;
}

void JonesSum::mergePValues(const Result &in, Result &out)
{
    PValueConstIterator inIter(in);
    PValueIterator outIter(out);

    while(!inIter.atEnd() && !outIter.atEnd())
    {
        if(outIter.key() == inIter.key())
        {
            outIter.value() += inIter.value();
            inIter.next();
            outIter.next();
        }
        else if(outIter.key() < inIter.key())
        {
            outIter.value() += in.getValue();
            outIter.next();
        }
        else
        {
            out.setPerturbedValue(inIter.key(), out.getValue()
                + inIter.value());
            inIter.next();
        }
    }

    while(!inIter.atEnd())
    {
        out.setPerturbedValue(inIter.key(), out.getValue()
            + inIter.value());
        inIter.next();
    }

    while(!outIter.atEnd())
    {
        outIter.value() += in.getValue();
        outIter.next();
    }
}


} // namespace BBS
} // namespace LOFAR
