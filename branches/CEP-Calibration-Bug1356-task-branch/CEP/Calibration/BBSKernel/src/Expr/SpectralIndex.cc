//# SpectralIndex.cc: Frequency dependent flux.
//#
//# Copyright (C) 2009
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

#include <lofar_config.h>
#include <BBSKernel/Expr/SpectralIndex.h>

namespace LOFAR
{
namespace BBS
{

SpectralIndex::~SpectralIndex()
{
    typedef vector<Expr<Scalar>::ConstPtr>::const_reverse_iterator _iter;
    for(_iter it = itsCoeff.rbegin(), end = itsCoeff.rend(); it != end; ++it)
    {
        disconnect(*it);
    }
    disconnect(itsRefStokes);
    disconnect(itsRefFreq);
}

unsigned int SpectralIndex::nArguments() const
{
    return itsCoeff.size() + 2;
}

ExprBase::ConstPtr SpectralIndex::argument(unsigned int i) const
{
    DBGASSERT(i < nArguments());
    switch(i)
    {
    case 0:
        return itsRefFreq;
    case 1:
        return itsRefStokes;
    default:
        return itsCoeff[i - 2];
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

    const Scalar refStokes = itsRefStokes->evaluate(request, cache);
    flags.push_back(refFreq.flags());

    vector<Scalar> coeff;
    coeff.reserve(nArg - 2);
    for(unsigned int i = 0; i < nArg - 2; ++i)
    {
        coeff.push_back(itsCoeff[i]->evaluate(request, cache));
        flags.push_back(coeff[i].flags());
    }

    // Evaluate flags.
    result.setFlags(mergeFlags(flags.begin(), flags.end()));

    // Compute main value.
    vector<Scalar::View> coeffValue;
    coeffValue.reserve(nArg - 2);
    for(unsigned int i = 0; i < nArg - 2; ++i)
    {
        coeffValue.push_back(coeff[i].view());
    }
    result.assign(evaluateImpl(request, refFreq.view(), refStokes.view(),
        coeffValue));

    // Compute perturbed values.
    Scalar::Iterator refFreqIt(refFreq);
    Scalar::Iterator refStokesIt(refStokes);
    bool atEnd = refFreqIt.atEnd() && refStokesIt.atEnd();

    vector<Scalar::Iterator> coeffIt;
    coeffIt.reserve(nArg - 2);
    for(unsigned int i = 0; i < nArg - 2; ++i)
    {
        coeffIt.push_back(Scalar::Iterator(coeff[i]));
        atEnd = atEnd && coeffIt.back().atEnd();
    }

    PValueKey key;
    while(!atEnd)
    {
        key = std::min(refFreqIt.key(), refStokesIt.key());
        for(unsigned int i = 0; i < nArg - 2; ++i)
        {
            key = std::min(key, coeffIt[i].key());
        }

        for(unsigned int i = 0; i < nArg - 2; ++i)
        {
            coeffValue[i] = coeffIt[i].value(key);
        }

        result.assign(key, evaluateImpl(request, refFreqIt.value(key),
            refStokesIt.value(key), coeffValue));

        refFreqIt.advance(key);
        refStokesIt.advance(key);
        atEnd = refFreqIt.atEnd() && refStokesIt.atEnd();
        for(unsigned int i = 0; i < nArg - 2; ++i)
        {
            coeffIt[i].advance(key);
            atEnd = atEnd && coeffIt[i].atEnd();
        }
    }

    return result;
}

const Scalar::View SpectralIndex::evaluateImpl(const Request &request,
    const Scalar::View &refFreq, const Scalar::View &refStokes,
    const vector<Scalar::View> &coeff) const
{
    Scalar::View result;

    // Special case: No coefficients or a single real coefficient with value
    // zero.
    if(coeff.empty() || (coeff.size() == 1 && !coeff[0]().isArray()
        && !coeff[0]().isComplex() && coeff[0]().getDouble() == 0.0))
    {
        result.assign(refStokes());
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

    // Compute spectral index as:
    // (v / v0) ^ (c0 + c1 * log10(v / v0) + c2 * log10(v / v0)^2 + ...)
    // Where v is the frequency and v0 is the reference frequency.

    // Compute log10(v / v0).
    Matrix base = log10(freq) - log10(refFreq());

    // Compute c0 + log10(v / v0) * c1 + log10(v / v0)^2 * c2 + ... using
    // Horner's rule.
    Matrix exponent = coeff[coeff.size() - 1]();
    for(unsigned int i = 1; i < coeff.size(); ++i)
    {
        exponent = exponent * base + coeff[coeff.size() - 1 - i]();
    }

    // Compute I0 * (v / v0) ^ exponent, where I0 is the value of the Stokes
    // parameter at the reference frequency.
    result.assign(refStokes() * pow10(base * exponent));
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
