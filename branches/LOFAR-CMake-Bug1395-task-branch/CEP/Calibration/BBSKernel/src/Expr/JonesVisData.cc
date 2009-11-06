//# JonesVisData.cc:
//#
//# Copyright (C) 2007
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

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Expr/JonesVisData.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

JonesVisData::JonesVisData(const VisData::Pointer &chunk,
    const baseline_t &baseline)
    : itsChunk(chunk)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    itsBaselineIndex = dims.getBaselineIndex(baseline);
}

JonesVisData::~JonesVisData()
{
}

JonesResult JonesVisData::getJResult(const Request &request)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const Grid &visGrid = dims.getGrid();
    const Grid &reqGrid = request.getGrid();

    // Find the offset of the request grid relative to the chunk grid.
    Box reqBox(reqGrid.getBoundingBox());
    Location start(visGrid.locate(reqBox.lower()));

    // Verify that the request grid is contained within the chunk grid (as it
    // is impossible to return a partial result).
    uint nChannels = request.getChannelCount();
    uint nTimeslots = request.getTimeslotCount();
    ASSERT(start.first + nChannels <= visGrid.shape().first);
    ASSERT(start.second + nTimeslots <= visGrid.shape().second);

//    LOG_DEBUG_STR("Offset: (" << start.first << "," << start.second
//        << ") Size: " << nChannels << "x" << nTimeslots);

    // NB: index_ranges are exclusive.
    typedef boost::multi_array<sample_t, 4>::index_range Range;
    Range freqRange(start.first, start.first + nChannels);
    Range timeRange(start.second, start.second + nTimeslots);

    // Allocate the result.
    JonesResult result;
    result.init();
    Matrix& m11 = result.result11().getValueRW();
    Matrix& m12 = result.result12().getValueRW();
    Matrix& m21 = result.result21().getValueRW();
    Matrix& m22 = result.result22().getValueRW();

    double *re = 0, *im = 0;

    // Copy 11 elements if available.
    try
    {
        const uint productIndex = dims.getPolarizationIndex("XX");
        m11.setDCMat(nChannels, nTimeslots);
        m11.dcomplexStorage(re, im);
        copy(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
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
        copy(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
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
        copy(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
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
        copy(re, im, itsChunk->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][productIndex]]);
    }
    catch(BBSKernelException &ex)
    {
        m22 = Matrix(makedcomplex(0,0), nChannels, nTimeslots);
    }

    return result;
}


void JonesVisData::copy(double *re, double *im,
    const boost::multi_array<sample_t, 4>::const_array_view<2>::type &src)
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


} // namespace BBS
} // namespace LOFAR
