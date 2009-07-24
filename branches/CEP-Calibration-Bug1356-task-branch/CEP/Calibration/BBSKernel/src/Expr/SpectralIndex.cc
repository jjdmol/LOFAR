//# SpectralIndex.cc: Frequency dependent scale factor for the base flux given
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

#include <lofar_config.h>
#include <BBSKernel/Expr/SpectralIndex.h>

namespace LOFAR
{
namespace BBS
{

SpectralIndex::~SpectralIndex()
{
    for(unsigned int i = 0; i < itsCoeff.size(); ++i)
    {
        disconnect(itsCoeff[i]);
    }
    disconnect(itsRefFreq);
}

unsigned int SpectralIndex::nArguments() const
{
    return itsCoeff.size() + 1;
}

ExprBase::ConstPtr SpectralIndex::argument(unsigned int i) const
{
    DBGASSERT(i < nArguments());
    if(i == 0)
    {
        return itsRefFreq;
    }
    else
    {
        return itsCoeff[i - 1];
    }
}

const Scalar SpectralIndex::evaluateExpr(const Request &request, Cache &cache)
    const
{
    // Allocate result.
    Scalar result;

    // Evaluate arguments.
    const unsigned int nArg = nArguments();
    vector<FlagArray> flags;
    flags.reserve(nArg);

    const Scalar refFreq = itsRefFreq->evaluate(request, cache);
    flags.push_back(refFreq.flags());

    vector<Scalar> coeff;
    coeff.reserve(nArg - 1);
    for(unsigned int i = 0; i < nArg - 1; ++i)
    {
        coeff.push_back(itsCoeff[i]->evaluate(request, cache));
        flags.push_back(coeff[i].flags());
    }

    // Evaluate flags.
    result.setFlags(mergeFlags(flags.begin(), flags.end()));

    // Compute main value.
    vector<Scalar::view> coeffValue;
    coeffValue.reserve(nArg - 1);
    for(unsigned int i = 0; i < nArg - 1; ++i)
    {
        coeffValue.push_back(coeff[i].value());
    }
    result.assign(evaluateImpl(request, refFreq.value(), coeffValue));

    // Compute perturbed values.
    ExprBase::const_solvable_iterator it = begin();
    while(it != end())
    {
        for(unsigned int i = 0; i < nArg - 1; ++i)
        {
            coeffValue[i] = coeff[i].value(*it);
        }

        result.assign(*it, evaluateImpl(request, refFreq.value(*it),
            coeffValue));
        ++it;
    }

    return result;
}

const Scalar::view SpectralIndex::evaluateImpl(const Request &request,
    const Scalar::view &refFreq, const vector<Scalar::view> &coeff) const
{
    Scalar::view result;

    // Special case: No coefficients or a single real coefficient with value
    // zero.
    if(coeff.empty() || (coeff.size() == 1 && !coeff[0]().isArray()
        && !coeff[0]().isComplex() && coeff[0]().getDouble() == 0.0))
    {
        result.assign(Matrix(1.0));
        return result;
    }

    // Create a 2-D Matrix that contains the frequency for each sample. (We
    // _must_ expand to a 2-D buffer because 1-D buffers are not supported).
    const size_t nFreq = request[FREQ]->size();
    const size_t nTime = request[TIME]->size();

    Matrix freq;
    double *it = freq.setDoubleFormat(nFreq, nTime);

    for(unsigned int t = 0; t < nTime; ++t)
    {
        for(unsigned int f = 0; f < nFreq; ++f)
        {
            *it++ = request[FREQ]->center(f);
        }
    }

    // Compute flux scale factor as:
    // (v / v0) ^ (-1.0 * [c0 + c1 * log(v / v0) + c2 * log(v / v0)^2 + ...])
    // Where v is the frequency and v0 is the reference frequency.

    // Compute log(v / v0).
    Matrix base = log(freq / refFreq());

    // In the following, we depend on coeff not being empty. It shouldn't be,
    // because that special case is handled above. But to guard against
    // oversights during software maintenance, we recheck the condition here.
    DBGASSERT(!coeff.empty());

    // Compute c0 + log(v / v0) * c1 + log(v / v0)^2 * c2 + ... using Horner's
    // rule.
    Matrix exponent = coeff[coeff.size() - 1]();
    for(unsigned int i = 1; i < coeff.size(); ++i)
    {
        exponent = exponent * base + coeff[coeff.size() - 1 - i]();
    }

    // Compute (v / v0) ^ -exponent.
    result.assign(exp(base * (-exponent)));
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
