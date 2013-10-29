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
//using LOFAR::min;

/*
namespace
{
void makeFieldArrayFactorAtReferenceFreq(const Grid &grid,
    const AntennaField::ConstPtr &field, const casa::MPosition &refPosition,
    double refFrequency, const Vector<3>::View &direction,
    const Vector<3>::View &refDirection, Matrix (&AF)[2]);

void makeFieldArrayFactorAtChannelFreq(const Grid &grid,
    const AntennaField::ConstPtr &field, const casa::MPosition &refPosition,
    const Vector<3>::View &direction, const Vector<3>::View &refDirection,
    Matrix (&AF)[2]);
} //# unnamed namespace
*/

StationBeam::StationBeam(const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &station0,
    const Expr<Vector<3> >::ConstPtr &tile0,
    const StationResponse::Station::ConstPtr &station,
    bool conjugate)
    :   BasicTernaryExpr<Vector<3>, Vector<3>, Vector<3>, JonesMatrix>(direction, station0, tile0),
        itsStation(station),
        itsUseChannelFreq(true),
        itsRefFrequency(0.0),
        itsConjugateFlag(conjugate)
{
}

StationBeam::StationBeam(const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &station0,
    const Expr<Vector<3> >::ConstPtr &tile0,
    const StationResponse::Station::ConstPtr &station,
    double refFrequency,
    bool conjugate)
    :   BasicTernaryExpr<Vector<3>, Vector<3>, Vector<3>, JonesMatrix>(direction, station0, tile0),
        itsStation(station),
        itsUseChannelFreq(false),
        itsRefFrequency(refFrequency),
        itsConjugateFlag(conjugate)
{
}

const JonesMatrix::View StationBeam::evaluateImpl(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &station0,
    const Vector<3>::View &tile0) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
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

    // Get pointers to input and output data.
    const double *p_direction[3] = {direction(0).doubleStorage(), direction(1).doubleStorage(), direction(2).doubleStorage()};
    const double *p_station0[3] = {station0(0).doubleStorage(), station0(1).doubleStorage(), station0(2).doubleStorage()};
    const double *p_tile0[3] = {tile0(0).doubleStorage(), tile0(1).doubleStorage(), tile0(2).doubleStorage()};

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

    // Evaluate antenna model.

    for(size_t i = 0; i < nTime; ++i)
    {
        for(size_t j = 0; j < nFreq; ++j)
        {
            StationResponse::vector3r_t q_direction = {{p_direction[0][j], p_direction[1][j], p_direction[2][j]}};
            StationResponse::vector3r_t q_station0 = {{p_station0[0][j], p_station0[1][j], p_station0[2][j]}};
            StationResponse::vector3r_t q_tile0 = {{p_tile0[0][j], p_tile0[1][j], p_tile0[2][j]}};

            StationResponse::matrix22c_t J = itsStation->response(grid[TIME]->center(j), grid[FREQ]->center(j),
                    q_direction, itsRefFrequency, q_station0, q_tile0);

            *E00_re++ = real(J[0][0]);
            *E00_im++ = imag(J[0][0]);
            *E01_re++ = real(J[0][1]);
            *E01_im++ = imag(J[0][1]);
            *E10_re++ = real(J[1][0]);
            *E10_im++ = imag(J[1][0]);
            *E11_re++ = real(J[1][1]);
            *E11_im++ = imag(J[1][1]);
        }
    }

    return JonesMatrix::View(E00, E01, E10, E11);

    /*
    // Allocate result (initialized at 0+0i).
    Matrix E[2][2];
    for(size_t i = 0; i < itsStation->nField(); ++i)
    {
        // Compute field array factor.
        Matrix AF[2];
        if(itsUseChannelFreq)
        {
            makeFieldArrayFactorAtChannelFreq(grid, itsStation->field(i),
                itsStation->position(), direction, reference, AF);
        }
        else
        {
            makeFieldArrayFactorAtReferenceFreq(grid, itsStation->field(i),
                itsStation->position(), itsRefFrequency, direction, reference,
                AF);
        }

        // Normalize.
        const size_t nActiveElement = itsStation->nActiveElement();
        if(nActiveElement > 0)
        {
            AF[0] /= nActiveElement;
            AF[1] /= nActiveElement;
        }

        // Conjugate array factor if required.
        if(itsConjugateFlag)
        {
            AF[0] = conj(AF[0]);
            AF[1] = conj(AF[1]);
        }

        // Multiply array factor with (tile) element response and add.
        if(i == 0)
        {
            E[0][0] = AF[0] * beam[i](0, 0);
            E[0][1] = AF[0] * beam[i](0, 1);
            E[1][0] = AF[1] * beam[i](1, 0);
            E[1][1] = AF[1] * beam[i](1, 1);
        }
        else
        {
            E[0][0] += AF[0] * beam[i](0, 0);
            E[0][1] += AF[0] * beam[i](0, 1);
            E[1][0] += AF[1] * beam[i](1, 0);
            E[1][1] += AF[1] * beam[i](1, 1);
        }
    }

    return JonesMatrix::View(E[0][0], E[0][1], E[1][0], E[1][1]);
    */
}

/*
namespace
{
void makeFieldArrayFactorAtReferenceFreq(const Grid &grid,
    const AntennaField::ConstPtr &field, const casa::MPosition &refPosition,
    double refFrequency, const Vector<3>::View &direction,
    const Vector<3>::View &refDirection, Matrix (&AF)[2])
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Account for the case where the station phase center is not equal to the
    // antenna field center (only applies to core HBA fields).
    const Vector3 &fieldCenter = field->position();
    casa::MVPosition stationCenter = refPosition.getValue();
    Vector3 offsetShift = {{fieldCenter[0] - stationCenter(0),
        fieldCenter[1] - stationCenter(1),
        fieldCenter[2] - stationCenter(2)}};

    // Angular reference frequency.
    const double omega0 = casa::C::_2pi * refFrequency;

    // Compute array factor.
    Matrix shift(makedcomplex(0.0, 0.0), nFreq, nTime);
    AF[0] = Matrix(makedcomplex(0.0, 0.0), nFreq, nTime);
    AF[1] = Matrix(makedcomplex(0.0, 0.0), nFreq, nTime);
    for(size_t i = 0; i < field->nElement(); ++i)
    {
        const AntennaField::Element &element = field->element(i);
        if(element.flag[0] && element.flag[1])
        {
            continue;
        }

        // Compute the offset relative to the delay center.
        Vector3 offset = {{element.offset[0] + offsetShift[0],
            element.offset[1] + offsetShift[1],
            element.offset[2] + offsetShift[2]}};

        // Compute the delay for a plane wave approaching from the direction
        // of interest with respect to the phase center of the element.
        Matrix delay = (direction(0) * offset[0] + direction(1) * offset[1]
            + direction(2) * offset[2]) / casa::C::c;

        // Compute the delay for a plane wave approaching from the phase
        // reference direction with respect to the phase center of the
        // element.
        Matrix delay0 = (refDirection(0) * offset[0]
            + refDirection(1) * offset[1] + refDirection(2) * offset[2])
            / casa::C::c;

        // Compute array factor contribution for this element.
        double *p_re, *p_im;
        shift.dcomplexStorage(p_re, p_im);
        const double *p_delay = delay.doubleStorage();
        const double *p_delay0 = delay0.doubleStorage();
        for(size_t t = 0; t < nTime; ++t)
        {
            const double delay_t = *p_delay++;
            const double phase0 = omega0 * (*p_delay0++);

            for(size_t f = 0; f < nFreq; ++f)
            {
                const double phase = casa::C::_2pi * grid[FREQ]->center(f)
                    * delay_t - phase0;
                *p_re = std::cos(phase);
                *p_im = std::sin(phase);
                ++p_re;
                ++p_im;
            }
        }

        if(!element.flag[0])
        {
            AF[0] += shift;
        }

        if(!element.flag[1])
        {
            AF[1] += shift;
        }
    }
}

void makeFieldArrayFactorAtChannelFreq(const Grid &grid,
    const AntennaField::ConstPtr &field, const casa::MPosition &refPosition,
    const Vector<3>::View &direction, const Vector<3>::View &refDirection,
    Matrix (&AF)[2])
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Instead of computing a phase shift for the pointing direction and a phase
    // shift for the direction of interest and then computing the difference,
    // compute the resultant phase shift in one go. Here we make use of the
    // relation a . b + a . c = a . (b + c). The sign of k is related to the
    // sign of the phase shift.
    Matrix k[3];
    k[0] = direction(0) - refDirection(0);
    k[1] = direction(1) - refDirection(1);
    k[2] = direction(2) - refDirection(2);

    // Account for the case where the station phase center is not equal to the
    // antenna field center (only applies to core HBA fields).
    const Vector3 &fieldCenter = field->position();
    casa::MVPosition stationCenter = refPosition.getValue();
    Vector3 offsetShift = {{fieldCenter[0] - stationCenter(0),
        fieldCenter[1] - stationCenter(1),
        fieldCenter[2] - stationCenter(2)}};

    // Compute array factor.
    Matrix shift(makedcomplex(0.0, 0.0), nFreq, nTime);
    AF[0] = Matrix(makedcomplex(0.0, 0.0), nFreq, nTime);
    AF[1] = Matrix(makedcomplex(0.0, 0.0), nFreq, nTime);
    for(size_t i = 0; i < field->nElement(); ++i)
    {
        const AntennaField::Element &element = field->element(i);
        if(element.flag[0] && element.flag[1])
        {
            continue;
        }

        // Compute the offset relative to the delay center.
        Vector3 offset = {{element.offset[0] + offsetShift[0],
            element.offset[1] + offsetShift[1],
            element.offset[2] + offsetShift[2]}};

        // Compute the effective delay for a plane wave approaching from the
        // direction of interest with respect to the phase center of the element
        // when beam forming in the reference direction.
        Matrix delay = (k[0] * offset[0] + k[1] * offset[1] + k[2] * offset[2])
            / casa::C::c;

        // Compute array factor contribution for this element.
        double *p_re, *p_im;
        shift.dcomplexStorage(p_re, p_im);
        const double *p_delay = delay.doubleStorage();
        for(size_t t = 0; t < nTime; ++t)
        {
            const double delay_t = *p_delay++;

            for(size_t f = 0; f < nFreq; ++f)
            {
                const double phase = casa::C::_2pi * grid[FREQ]->center(f)
                    * delay_t;
                *p_re = std::cos(phase);
                *p_im = std::sin(phase);
                ++p_re;
                ++p_im;
            }
        }

        if(!element.flag[0])
        {
            AF[0] += shift;
        }

        if(!element.flag[1])
        {
            AF[1] += shift;
        }
    }
}
} //# unnamed namespace
*/

} //# namespace BBS
} //# namespace LOFAR
