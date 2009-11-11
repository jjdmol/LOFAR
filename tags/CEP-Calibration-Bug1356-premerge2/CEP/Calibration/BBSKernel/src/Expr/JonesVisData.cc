//# JonesVisData.cc:
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
#include <BBSKernel/Expr/JonesVisData.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

JonesVisData::JonesVisData(const VisData::Ptr &chunk,
    const baseline_t &baseline)
    : itsChunk(chunk)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    itsBaselineIndex = dims.getBaselineIndex(baseline);
}

const JonesMatrix JonesVisData::evaluateExpr(const Request &request,
    Cache &cache) const
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const Grid &visGrid = dims.getGrid();
    const Grid &reqGrid = request.getGrid();

    // Find the offset of the request grid relative to the chunk grid.
    Box reqBox(reqGrid.getBoundingBox());
    Location start(visGrid.locate(reqBox.lower()));

    // Verify that the request grid is contained within the chunk grid (as it
    // is impossible to return a partial result).
    uint nChannels = request[FREQ]->size();
    uint nTimeslots = request[TIME]->size();
    ASSERT(start.first + nChannels <= visGrid.shape().first);
    ASSERT(start.second + nTimeslots <= visGrid.shape().second);

//    LOG_DEBUG_STR("Offset: (" << start.first << "," << start.second
//        << ") Size: " << nChannels << "x" << nTimeslots);

    // NB: index_ranges are exclusive.
    typedef boost::multi_array<sample_t, 4>::index_range DRange;
    DRange freqRange(start.first, start.first + nChannels);
    DRange timeRange(start.second, start.second + nTimeslots);

    // Allocate the result.
    Matrix m11, m12, m21, m22;

    double *re = 0, *im = 0;

    // Copy 11 elements if available.
    try
    {
        const uint productIndex = dims.getPolarizationIndex("XX");
        m11.setDCMat(nChannels, nTimeslots);
        m11.dcomplexStorage(re, im);
        copyData(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][productIndex]]);
    }
    catch(BBSKernelException &ex)
    {
        m11 = Matrix(makedcomplex(0,0), nChannels, nTimeslots);
    }

    // Copy 12 elements if available.
    try
    {
        const uint productIndex = dims.getPolarizationIndex("XY");
        m12.setDCMat(nChannels, nTimeslots);
        m12.dcomplexStorage(re, im);
        copyData(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][productIndex]]);
    }
    catch(BBSKernelException &ex)
    {
        m12 = Matrix(makedcomplex(0,0), nChannels, nTimeslots);
    }

    // Copy 21 elements if available.
    try
    {
        const uint productIndex = dims.getPolarizationIndex("YX");
        m21.setDCMat(nChannels, nTimeslots);
        m21.dcomplexStorage(re, im);
        copyData(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][productIndex]]);
    }
    catch(BBSKernelException &ex)
    {
        m21 = Matrix(makedcomplex(0,0), nChannels, nTimeslots);
    }


    // Copy 22 elements if available.
    try
    {
        const uint productIndex = dims.getPolarizationIndex("YY");
        m22.setDCMat(nChannels, nTimeslots);
        m22.dcomplexStorage(re, im);
        copyData(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][productIndex]]);
    }
    catch(BBSKernelException &ex)
    {
        m22 = Matrix(makedcomplex(0,0), nChannels, nTimeslots);
    }

    typedef boost::multi_array<sample_t, 4>::index_range FRange;
    FlagArray flags00(nChannels, nTimeslots);
    FlagArray flags01(nChannels, nTimeslots);
    FlagArray flags10(nChannels, nTimeslots);
    FlagArray flags11(nChannels, nTimeslots);

    copyFlags(flags00.begin(),
        itsChunk->vis_flag[boost::indices[itsBaselineIndex]
            [FRange(start.second, start.second + nTimeslots)]
            [FRange(start.first, start.first + nChannels)][0]]);
    copyFlags(flags01.begin(),
        itsChunk->vis_flag[boost::indices[itsBaselineIndex]
            [FRange(start.second, start.second + nTimeslots)]
            [FRange(start.first, start.first + nChannels)][1]]);
    copyFlags(flags10.begin(),
        itsChunk->vis_flag[boost::indices[itsBaselineIndex]
            [FRange(start.second, start.second + nTimeslots)]
            [FRange(start.first, start.first + nChannels)][2]]);
    copyFlags(flags11.begin(),
        itsChunk->vis_flag[boost::indices[itsBaselineIndex]
            [FRange(start.second, start.second + nTimeslots)]
            [FRange(start.first, start.first + nChannels)][3]]);

    JonesMatrix::View proxy;
    proxy.assign(0, 0, m11);
    proxy.assign(0, 1, m12);
    proxy.assign(1, 0, m21);
    proxy.assign(1, 1, m22);

    JonesMatrix result;
    result.setFlags(flags00 | flags01 | flags10 | flags11);
    result.assign(proxy);

    return result;
}


void JonesVisData::copyData(double *re, double *im,
    const boost::multi_array<sample_t, 4>::const_array_view<2>::type &src) const
{
    for(size_t tslot = 0; tslot < src.shape()[0]; ++tslot)
    {
        for(size_t chan = 0; chan < src.shape()[1]; ++chan)
        {
            re[chan] = real(src[tslot][chan]);
            im[chan] = imag(src[tslot][chan]);
        }
        re += src.shape()[1];
        im += src.shape()[1];
    }
}

void JonesVisData::copyFlags(FlagArray::iterator dest,
    const boost::multi_array<flag_t, 4>::const_array_view<2>::type &src) const
{
    for(size_t tslot = 0; tslot < src.shape()[0]; ++tslot)
    {
        for(size_t chan = 0; chan < src.shape()[1]; ++chan)
        {
            *dest++ = src[tslot][chan];
        }
    }
}

} // namespace BBS
} // namespace LOFAR
