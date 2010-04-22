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

ExprVisData::ExprVisData(const VisData::Ptr &chunk, const baseline_t &baseline,
    Correlation element00, Correlation element01, Correlation element10,
    Correlation element11)
//ExprVisData::ExprVisData(const VisData::Ptr &chunk, const baseline_t &baseline)
    :   itsChunk(chunk)
{
    itsBaseline = chunk->baselines().index(baseline);
    if(itsBaseline == chunk->nBaselines())
    {
        THROW(BBSKernelException, "No data available for baseline: "
            << baseline.first << "-" << baseline.second);
    }

    setCorrelation(0, element00);
    setCorrelation(1, element01);
    setCorrelation(2, element10);
    setCorrelation(3, element11);

//    // By default, use linear correlations.
//    setCorrelations(XX, XY, YX, YY);
}

//void ExprVisData::setCorrelations(Correlation element00, Correlation element01,
//    Correlation element10, Correlation element11)
//{
//    setCorrelation(0, element00);
//    setCorrelation(1, element01);
//    setCorrelation(2, element10);
//    setCorrelation(3, element11);
//}

void ExprVisData::setCorrelation(size_t element, Correlation correlation)
{
    if(isDefined(correlation))
    {
        itsCorr[element] = itsChunk->correlations().index(correlation);
        itsCorrMask[element] = itsCorr[0] != itsChunk->nCorrelations();
    }
    else
    {
        itsCorrMask[element] = false;
    }
}

const JonesMatrix ExprVisData::evaluateExpr(const Request &request,
    Cache&, unsigned int grid) const
{
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

    return result;
}

FlagArray ExprVisData::copyFlags(const Grid &grid, size_t element,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    FlagArray result;

    if(itsCorrMask[element])
    {
        // Get polarization product index.
        const size_t cr = itsCorr[element];

        // Allocate space for the result and initialize to 1.
        const size_t nFreq = grid[FREQ]->size();
        const size_t nTime = grid[TIME]->size();
        result = FlagArray(nFreq, nTime, FlagType(1));

        FlagArray::iterator begin = result.begin();

        // Insanely complicated boost::multi_array types...
        typedef boost::multi_array<FlagType, 4>::index_range FRange;
        typedef boost::multi_array<FlagType, 4>::const_array_view<2>::type
            FSlice;

        // Copy flags.
        FSlice flags = itsChunk->vis_flag[boost::indices[itsBaseline]
            [FRange()][FRange()][cr]];
        for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
        {
            const pair<size_t, size_t> &tmap = mapping[TIME][t];
            FlagArray::iterator offset = begin + tmap.first * nFreq;

            // TODO: Check for tslot_flag and skip if true?
            for(unsigned int f = 0; f < mapping[FREQ].size(); ++f)
            {
                const pair<size_t, size_t> &fmap = mapping[FREQ][f];
                *(offset + fmap.first) = flags[tmap.second][fmap.second];
            }
        }
    }
    else
    {
        result = FlagArray(FlagType(0));
    }

    return result;
}

Matrix ExprVisData::copyData(const Grid &grid, size_t element,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    Matrix result;

    if(itsCorrMask[element])
    {
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
        typedef boost::multi_array<sample_t, 4>::index_range SRange;
        typedef boost::multi_array<sample_t, 4>::const_array_view<2>::type
            SSlice;

        // Copy visibility data.
        SSlice samples = itsChunk->vis_data[boost::indices[itsBaseline]
            [SRange()][SRange()][cr]];
        for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
        {
            const pair<size_t, size_t> &tmap = mapping[TIME][t];
            double *destRe = re + tmap.first * nFreq;
            double *destIm = im + tmap.first * nFreq;

            for(unsigned int f = 0; f < mapping[FREQ].size(); ++f)
            {
                const pair<size_t, size_t> &fmap = mapping[FREQ][f];
                const sample_t sample = samples[tmap.second][fmap.second];

                destRe[fmap.first] = real(sample);
                destIm[fmap.first] = imag(sample);
            }
        }
    }
    else
    {
        result = Matrix(dcomplex(0.0, 0.0));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
