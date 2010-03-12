//# ExprVisData.cc:
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

ExprVisData::ExprVisData(const VisData::Ptr &chunk, const baseline_t &baseline)
    :   itsChunk(chunk)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    itsBaselineIndex = dims.getBaselineIndex(baseline);
}

const JonesMatrix ExprVisData::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    vector<pair<size_t, size_t> > axisMapping[2];

    const VisDimensions &dims = itsChunk->getDimensions();
    const Grid &visGrid = dims.getGrid();
    axisMapping[FREQ] = makeAxisMapping(request[grid][FREQ], visGrid[FREQ]);
    axisMapping[TIME] = makeAxisMapping(request[grid][TIME], visGrid[TIME]);

    FlagArray flags00(copyFlags(request[grid], "XX", axisMapping));
    FlagArray flags01(copyFlags(request[grid], "XY", axisMapping));
    FlagArray flags10(copyFlags(request[grid], "YX", axisMapping));
    FlagArray flags11(copyFlags(request[grid], "YY", axisMapping));

    JonesMatrix result;
    result.setFlags(flags00 | flags01 | flags10 | flags11);
    result.assign(0, 0, copyData(request[grid], "XX", axisMapping));
    result.assign(0, 1, copyData(request[grid], "XY", axisMapping));
    result.assign(1, 0, copyData(request[grid], "YX", axisMapping));
    result.assign(1, 1, copyData(request[grid], "YY", axisMapping));

    return result;
}

vector<pair<size_t, size_t> >
ExprVisData::makeAxisMapping(const Axis::ShPtr &from, const Axis::ShPtr &to)
    const
{
    vector<pair<size_t, size_t> > mapping;

    const double overlapStart = std::max(from->start(), to->start());
    const double overlapEnd = std::min(from->end(), to->end());

    if(overlapStart >= overlapEnd || casa::near(overlapStart, overlapEnd))
    {
        return mapping;
    }

    const size_t start = from->locate(overlapStart);
    const size_t end = from->locate(overlapEnd, false, start);

    // Intervals are inclusive by convention.
    const size_t nCells = end - start + 1;
    mapping.reserve(nCells);

    // Special case for the first cell: cell center may be located outside of
    // the overlap between the "from" and "to" axis.
    size_t target = 0;
    double center = from->center(start);
    if(center > overlapStart || casa::near(center, overlapStart))
    {
        target = to->locate(center);
        mapping.push_back(make_pair(start, target));
    }

    for(size_t i = start + 1; i < end; ++i)
    {
        target = to->locate(from->center(i), true, target);
        mapping.push_back(make_pair(i, target));
    }

    if(nCells > 1)
    {
        // Special case for the last cell: cell center may be located outside of
        // the overlap between the "from" and "to" axis.
        center = from->center(end);
        if(center < overlapEnd && !casa::near(center, overlapEnd))
        {
            target = to->locate(center, false, target);
            mapping.push_back(make_pair(end, target));
        }
    }

    return mapping;
}

FlagArray ExprVisData::copyFlags(const Grid &grid, const string &product,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    FlagArray result;

    const VisDimensions &dims = itsChunk->getDimensions();
    if(dims.hasPolarization(product))
    {
        // Get polarization product index.
        const unsigned int productIndex = dims.getPolarizationIndex(product);

        // Allocate space for the result and initialize to 1.
        const unsigned int nChannels = grid[FREQ]->size();
        const unsigned int nTimeslots = grid[TIME]->size();
        result = FlagArray(nChannels, nTimeslots, FlagType(1));

        FlagArray::iterator begin = result.begin();

        // Insanely complicated boost::multi_array types...
        typedef boost::multi_array<FlagType, 4>::index_range FRange;
        typedef boost::multi_array<FlagType, 4>::const_array_view<2>::type
            FView;

        // Copy flags.
        FView flags = itsChunk->vis_flag[boost::indices[itsBaselineIndex]
            [FRange()][FRange()][productIndex]];
        for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
        {
            const pair<size_t, size_t> &tmap = mapping[TIME][t];
            FlagArray::iterator offset = begin + tmap.first * nChannels;

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

Matrix ExprVisData::copyData(const Grid &grid, const string &product,
    const vector<pair<size_t, size_t> > (&mapping)[2]) const
{
    Matrix result;

    const VisDimensions &dims = itsChunk->getDimensions();
    if(dims.hasPolarization(product))
    {
        // Get polarization product index.
        const unsigned int productIndex = dims.getPolarizationIndex(product);

        // Allocate space for the result and initialize to 0.0 + 0.0i.
        const unsigned int nChannels = grid[FREQ]->size();
        const unsigned int nTimeslots = grid[TIME]->size();
        result = Matrix(dcomplex(0.0, 0.0), nChannels, nTimeslots);

        // Get pointers to the data.
        double *re = 0, *im = 0;
        result.dcomplexStorage(re, im);

        // Insanely complicated boost::multi_array types...
        typedef boost::multi_array<sample_t, 4>::index_range DRange;
        typedef boost::multi_array<sample_t, 4>::const_array_view<2>::type
            DView;

        // Copy visibility data.
        DView data = itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [DRange()][DRange()][productIndex]];
        for(unsigned int t = 0; t < mapping[TIME].size(); ++t)
        {
            const pair<size_t, size_t> &tmap = mapping[TIME][t];
            double *destRe = re + tmap.first * nChannels;
            double *destIm = im + tmap.first * nChannels;

            for(unsigned int f = 0; f < mapping[FREQ].size(); ++f)
            {
                const pair<size_t, size_t> &fmap = mapping[FREQ][f];
                const sample_t sample = data[tmap.second][fmap.second];

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
