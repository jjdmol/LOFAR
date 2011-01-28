//# ExprVisData.cc: Make visibility data from an observation available for use
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

#include <lofar_config.h>

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

ExprVisData::ExprVisData(const VisBuffer::Ptr &chunk, const baseline_t &baseline,
    Correlation::Type element00, Correlation::Type element01,
    Correlation::Type element10, Correlation::Type element11)
    :   itsChunk(chunk)
{
    itsBaseline = chunk->baselines().index(baseline);
    ASSERT(itsBaseline < chunk->nBaselines());

    setCorrelation(0, element00);
    setCorrelation(1, element01);
    setCorrelation(2, element10);
    setCorrelation(3, element11);
}

void ExprVisData::setCorrelation(size_t element, Correlation::Type correlation)
{
    const size_t index = itsChunk->correlations().index(correlation);

    itsCorr[element] = index;
    itsCorrMask[element] = Correlation::isDefined(correlation)
        && index < itsChunk->nCorrelations();
}

const JonesMatrix ExprVisData::evaluateExpr(const Request &request,
    Cache&, unsigned int grid) const
{
    EXPR_TIMER_START();

    vector<pair<size_t, size_t> > axisMapping[2];
    makeAxisMapping(request[grid][FREQ], itsChunk->grid()[FREQ],
        back_inserter(axisMapping[FREQ]));
    makeAxisMapping(request[grid][TIME], itsChunk->grid()[TIME],
        back_inserter(axisMapping[TIME]));

    FlagArray flags00(copyFlags(request[grid], 0, axisMapping));
    FlagArray flags01(copyFlags(request[grid], 1, axisMapping));
    FlagArray flags10(copyFlags(request[grid], 2, axisMapping));
    FlagArray flags11(copyFlags(request[grid], 3, axisMapping));

    JonesMatrix result;
    result.setFlags(flags00 | flags01 | flags10 | flags11);
    result.assign(0, 0, copyData(request[grid], 0, axisMapping));
    result.assign(0, 1, copyData(request[grid], 1, axisMapping));
    result.assign(1, 0, copyData(request[grid], 2, axisMapping));
    result.assign(1, 1, copyData(request[grid], 3, axisMapping));

    EXPR_TIMER_STOP();

    return result;
}

FlagArray ExprVisData::copyFlags(const Grid &grid, size_t element,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    if(!itsCorrMask[element])
    {
        return FlagArray(flag_t(0));
    }

    FlagArray result;

    // Get polarization product index.
    const size_t cr = itsCorr[element];

    // Allocate space for the result and initialize to 1.
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();
    result = FlagArray(nFreq, nTime, flag_t(1));

    FlagArray::iterator begin = result.begin();

    // Insanely complicated boost::multi_array types...
    typedef boost::multi_array<flag_t, 4>::index_range FRange;
    typedef boost::multi_array<flag_t, 4>::const_array_view<2>::type FSlice;

    // Copy flags.
    FSlice flags =
        itsChunk->flags[boost::indices[itsBaseline][FRange()][FRange()][cr]];

    for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
    {
        const pair<size_t, size_t> &tmap = mapping[TIME][t];
        FlagArray::iterator offset = begin + tmap.first * nFreq;

        for(unsigned int f = 0; f < mapping[FREQ].size(); ++f)
        {
            const pair<size_t, size_t> &fmap = mapping[FREQ][f];
            *(offset + fmap.first) = flags[tmap.second][fmap.second];
        }
    }

    return result;
}

Matrix ExprVisData::copyData(const Grid &grid, size_t element,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    if(!itsCorrMask[element])
    {
        return Matrix(dcomplex(0.0, 0.0));
    }

    Matrix result;

    // Get polarization product index.
    const size_t cr = itsCorr[element];

    // Allocate space for the result and initialize to 0.0 + 0.0i.
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();
    result = Matrix(dcomplex(0.0, 0.0), nFreq, nTime);

    // Get pointers to the data.
    double *re = 0, *im = 0;
    result.dcomplexStorage(re, im);

    // Insanely complicated boost::multi_array types...
    typedef boost::multi_array<dcomplex, 4>::index_range SRange;
    typedef boost::multi_array<dcomplex, 4>::const_array_view<2>::type
        SSlice;

    // Copy visibility data.
    SSlice samples =
        itsChunk->samples[boost::indices[itsBaseline][SRange()][SRange()][cr]];

    for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
    {
        const pair<size_t, size_t> &tmap = mapping[TIME][t];
        double *destRe = re + tmap.first * nFreq;
        double *destIm = im + tmap.first * nFreq;

        for(unsigned int f = 0; f < mapping[FREQ].size(); ++f)
        {
            const pair<size_t, size_t> &fmap = mapping[FREQ][f];
            const dcomplex sample = samples[tmap.second][fmap.second];

            destRe[fmap.first] = real(sample);
            destIm[fmap.first] = imag(sample);
        }
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
