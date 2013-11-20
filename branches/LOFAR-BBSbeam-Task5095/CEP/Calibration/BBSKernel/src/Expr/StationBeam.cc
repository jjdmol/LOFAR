//# StationBeamFormer.cc: Beam forming the signals from individual dipoles
//# or tiles into a single station beam.
//#
//# Copyright (C) 2011
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
#include <BBSKernel/Expr/StationBeam.h>
#include <BBSKernel/Exceptions.h>
//#include <Common/lofar_algorithm.h>

#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

StationBeam::StationBeam(const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &station0,
    const Expr<Vector<3> >::ConstPtr &tile0,
    const StationResponse::Station::ConstPtr &station)
    :   BasicTernaryExpr<Vector<3>, Vector<3>, Vector<3>,
            JonesMatrix>(direction, station0, tile0),
        itsStation(station),
        itsEnableArrayFactor(true),
        itsEnableElementResponse(true),
        itsUseChannelFreq(true),
        itsReferenceFreq(0.0)
{
}

void StationBeam::setReferenceFreq(double freq)
{
    itsReferenceFreq = freq;
    itsUseChannelFreq = false;
}

void StationBeam::setEnableElementResponse(bool enabled)
{
    itsEnableElementResponse = enabled;
}

void StationBeam::setEnableArrayFactor(bool enabled)
{
    itsEnableArrayFactor = enabled;
}

const JonesMatrix::View StationBeam::evaluateImpl(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &station0,
    const Vector<3>::View &tile0) const
{
    // Check preconditions.
    const size_t nTime = grid[TIME]->size();

    ASSERT(!direction(0).isComplex() && direction(0).nx() == 1
        && static_cast<size_t>(direction(0).ny()) == nTime);
    ASSERT(!direction(1).isComplex() && direction(1).nx() == 1
        && static_cast<size_t>(direction(1).ny()) == nTime);
    ASSERT(!direction(2).isComplex() && direction(2).nx() == 1
        && static_cast<size_t>(direction(2).ny()) == nTime);

    ASSERT(!station0(0).isComplex() && station0(0).nx() == 1
        && static_cast<size_t>(station0(0).ny()) == nTime);
    ASSERT(!station0(1).isComplex() && station0(1).nx() == 1
        && static_cast<size_t>(station0(1).ny()) == nTime);
    ASSERT(!station0(2).isComplex() && station0(2).nx() == 1
        && static_cast<size_t>(station0(2).ny()) == nTime);

    ASSERT(!tile0(0).isComplex() && tile0(0).nx() == 1
        && static_cast<size_t>(tile0(0).ny()) == nTime);
    ASSERT(!tile0(1).isComplex() && tile0(1).nx() == 1
        && static_cast<size_t>(tile0(1).ny()) == nTime);
    ASSERT(!tile0(2).isComplex() && tile0(2).nx() == 1
        && static_cast<size_t>(tile0(2).ny()) == nTime);

    if(itsEnableArrayFactor && itsEnableElementResponse)
    {
        return evaluateImplResponse(grid, direction, station0, tile0);
    }
    else if(!itsEnableElementResponse)
    {
        return evaluateImplArrayFactor(grid, direction, station0, tile0);
    }
    else if(!itsEnableArrayFactor)
    {
        return evaluateImplElementResponse(grid, direction);
    }
    else
    {
        Matrix zero(dcomplex(0.0, 0.0));
        return JonesMatrix::View(zero, zero, zero, zero);
    }
}

const JonesMatrix::View StationBeam::evaluateImplResponse(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &station0,
    const Vector<3>::View &tile0) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Get pointers to input and output data.
    const double *p_direction[3] = {direction(0).doubleStorage(),
        direction(1).doubleStorage(), direction(2).doubleStorage()};
    const double *p_station0[3] = {station0(0).doubleStorage(),
        station0(1).doubleStorage(), station0(2).doubleStorage()};
    const double *p_tile0[3] = {tile0(0).doubleStorage(),
        tile0(1).doubleStorage(), tile0(2).doubleStorage()};

    Matrix E00, E01, E10, E11;
    double *E00_re, *E00_im;
    E00.setDCMat(nFreq, nTime);
    E00.dcomplexStorage(E00_re, E00_im);

    double *E01_re, *E01_im;
    E01.setDCMat(nFreq, nTime);
    E01.dcomplexStorage(E01_re, E01_im);

    double *E10_re, *E10_im;
    E10.setDCMat(nFreq, nTime);
    E10.dcomplexStorage(E10_re, E10_im);

    double *E11_re, *E11_im;
    E11.setDCMat(nFreq, nTime);
    E11.dcomplexStorage(E11_re, E11_im);

    // Evaluate station response.
    for(size_t i = 0; i < nTime; ++i)
    {
        double time = grid[TIME]->center(i);

        StationResponse::vector3r_t v_direction = {{*p_direction[0]++,
            *p_direction[1]++, *p_direction[2]++}};
        StationResponse::vector3r_t v_station0 = {{*p_station0[0]++,
            *p_station0[1]++, *p_station0[2]++}};
        StationResponse::vector3r_t v_tile0 = {{*p_tile0[0]++, *p_tile0[1]++,
            *p_tile0[2]++}};

        for(size_t j = 0; j < nFreq; ++j)
        {
            double freq = grid[FREQ]->center(j);
            double referenceFreq = itsUseChannelFreq ? freq : itsReferenceFreq;

            StationResponse::matrix22c_t response = itsStation->response(time,
                freq, v_direction, referenceFreq, v_station0, v_tile0);

            *E00_re++ = real(response[0][0]);
            *E00_im++ = imag(response[0][0]);
            *E01_re++ = real(response[0][1]);
            *E01_im++ = imag(response[0][1]);
            *E10_re++ = real(response[1][0]);
            *E10_im++ = imag(response[1][0]);
            *E11_re++ = real(response[1][1]);
            *E11_im++ = imag(response[1][1]);
        }
    }

    return JonesMatrix::View(E00, E01, E10, E11);
}

const JonesMatrix::View StationBeam::evaluateImplArrayFactor(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &station0,
    const Vector<3>::View &tile0) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Get pointers to input and output data.
    const double *p_direction[3] = {direction(0).doubleStorage(),
        direction(1).doubleStorage(), direction(2).doubleStorage()};
    const double *p_station0[3] = {station0(0).doubleStorage(),
        station0(1).doubleStorage(), station0(2).doubleStorage()};
    const double *p_tile0[3] = {tile0(0).doubleStorage(),
        tile0(1).doubleStorage(), tile0(2).doubleStorage()};

    Matrix E00, E01(dcomplex(0.0, 0.0)), E10(dcomplex(0.0, 0.0)), E11;
    double *E00_re, *E00_im;
    E00.setDCMat(nFreq, nTime);
    E00.dcomplexStorage(E00_re, E00_im);

    double *E11_re, *E11_im;
    E11.setDCMat(nFreq, nTime);
    E11.dcomplexStorage(E11_re, E11_im);

    // Evaluate station array factor.
    for(size_t i = 0; i < nTime; ++i)
    {
        double time = grid[TIME]->center(i);

        StationResponse::vector3r_t v_direction = {{*p_direction[0]++,
            *p_direction[1]++, *p_direction[2]++}};
        StationResponse::vector3r_t v_station0 = {{*p_station0[0]++,
            *p_station0[1]++, *p_station0[2]++}};
        StationResponse::vector3r_t v_tile0 = {{*p_tile0[0]++, *p_tile0[1]++,
            *p_tile0[2]++}};

        for(size_t j = 0; j < nFreq; ++j)
        {
            double freq = grid[FREQ]->center(j);
            double referenceFreq = itsUseChannelFreq ? freq : itsReferenceFreq;

            StationResponse::diag22c_t af = itsStation->arrayFactor(time, freq,
                v_direction, referenceFreq, v_station0, v_tile0);

            *E00_re++ = real(af[0]);
            *E00_im++ = imag(af[0]);
            *E11_re++ = real(af[1]);
            *E11_im++ = imag(af[1]);
        }
    }

    return JonesMatrix::View(E00, E01, E10, E11);
}

const JonesMatrix::View
StationBeam::evaluateImplElementResponse(const Grid &grid,
    const Vector<3>::View &direction) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Get pointers to input and output data.
    const double *p_direction[3] = {direction(0).doubleStorage(),
        direction(1).doubleStorage(), direction(2).doubleStorage()};

    Matrix E00, E01, E10, E11;
    double *E00_re, *E00_im;
    E00.setDCMat(nFreq, nTime);
    E00.dcomplexStorage(E00_re, E00_im);

    double *E01_re, *E01_im;
    E01.setDCMat(nFreq, nTime);
    E01.dcomplexStorage(E01_re, E01_im);

    double *E10_re, *E10_im;
    E10.setDCMat(nFreq, nTime);
    E10.dcomplexStorage(E10_re, E10_im);

    double *E11_re, *E11_im;
    E11.setDCMat(nFreq, nTime);
    E11.dcomplexStorage(E11_re, E11_im);

    // Evaluate element response.
    for(size_t i = 0; i < nTime; ++i)
    {
        double time = grid[TIME]->center(i);

        StationResponse::vector3r_t v_direction = {{*p_direction[0]++,
            *p_direction[1]++, *p_direction[2]++}};

        for(size_t j = 0; j < nFreq; ++j)
        {
            // For a station with multiple antenna fields, need to select
            // for which field the element response will be evaluated. Here
            // the first field of the station is always selected.
            StationResponse::AntennaField::ConstPtr field =
                *itsStation->beginFields();
            StationResponse::matrix22c_t response =
                field->singleElementResponse(time, grid[FREQ]->center(j),
                v_direction);

            *E00_re++ = real(response[0][0]);
            *E00_im++ = imag(response[0][0]);
            *E01_re++ = real(response[0][1]);
            *E01_im++ = imag(response[0][1]);
            *E10_re++ = real(response[1][0]);
            *E10_im++ = imag(response[1][0]);
            *E11_re++ = real(response[1][1]);
            *E11_im++ = imag(response[1][1]);
        }
    }

    return JonesMatrix::View(E00, E01, E10, E11);
}

} //# namespace BBS
} //# namespace LOFAR
