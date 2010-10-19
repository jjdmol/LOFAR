//# MeqJonesVisData.cc: 
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
#include <BBSKernel/MNS/MeqJonesVisData.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

MeqJonesVisData::MeqJonesVisData(VisData::Pointer vdata, baseline_t baseline)
    : itsVisData(vdata)
{
    itsBaselineIndex = itsVisData->getBaselineIndex(baseline);
}

MeqJonesResult MeqJonesVisData::getJResult (const MeqRequest& request)
{
    typedef boost::multi_array<sample_t, 4>::index_range range;

    size_t nChannels = request.nx();
    size_t nTimeslots = request.ny();
    pair<size_t, size_t> offset = request.offset();
    range freqRange(offset.first, offset.first + nChannels);
    range timeRange(offset.second, offset.second + nTimeslots);

    // Allocate the result matrices.
    size_t pol;
    double *re, *im;
    MeqJonesResult result(request.nspid());
    MeqMatrix& m11 = result.result11().getValueRW();
    MeqMatrix& m12 = result.result12().getValueRW();
    MeqMatrix& m21 = result.result21().getValueRW();
    MeqMatrix& m22 = result.result22().getValueRW();
    m11.setDCMat(nChannels, nTimeslots);
    m12.setDCMat(nChannels, nTimeslots);
    m21.setDCMat(nChannels, nTimeslots);
    m22.setDCMat(nChannels, nTimeslots);

    // Copy 11 elements if available.
    if(itsVisData->hasPolarization("XX") || itsVisData->hasPolarization("LL"))
    {
        m11.dcomplexStorage(re, im);

        try
        {
            pol = itsVisData->getPolarizationIndex("XX");
        }
        catch(BBSKernelException &ex)
        {
            pol = itsVisData->getPolarizationIndex("LL");
        }

        copy(re, im, itsVisData->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][pol]]);
    }
    else
        m11 = MeqMatrix(makedcomplex(0,0), nChannels, nTimeslots);


    // Copy 12 elements if available.
    if(itsVisData->hasPolarization("XY") || itsVisData->hasPolarization("LR"))
    {
        m12.dcomplexStorage(re, im);

        try
        {
            pol = itsVisData->getPolarizationIndex("XY");
        }
        catch(BBSKernelException &ex)
        {
            pol = itsVisData->getPolarizationIndex("LR");
        }

        copy(re, im, itsVisData->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][pol]]);
    }
    else
        m12 = MeqMatrix(makedcomplex(0,0), nChannels, nTimeslots);


    // Copy 21 elements if available.
    if(itsVisData->hasPolarization("YX") || itsVisData->hasPolarization("RL"))
    {
        m21.dcomplexStorage(re, im);

        try
        {
            pol = itsVisData->getPolarizationIndex("YX");
        }
        catch(BBSKernelException &ex)
        {
            pol = itsVisData->getPolarizationIndex("RL");
        }

        copy(re, im, itsVisData->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][pol]]);
    }
    else
        m21 = MeqMatrix(makedcomplex(0,0), nChannels, nTimeslots);


    // Copy 22 elements if available.
    if(itsVisData->hasPolarization("YY") || itsVisData->hasPolarization("RR"))
    {
        m22.dcomplexStorage(re, im);

        try
        {
            pol = itsVisData->getPolarizationIndex("YY");
        }
        catch(BBSKernelException &ex)
        {
            pol = itsVisData->getPolarizationIndex("RR");
        }

        copy(re, im, itsVisData->vis_data[boost::indices[itsBaselineIndex]
            [timeRange][freqRange][pol]]);
    }
    else
        m22 = MeqMatrix(makedcomplex(0,0), nChannels, nTimeslots);

    return result;
}


void MeqJonesVisData::copy(double *re, double *im,
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
