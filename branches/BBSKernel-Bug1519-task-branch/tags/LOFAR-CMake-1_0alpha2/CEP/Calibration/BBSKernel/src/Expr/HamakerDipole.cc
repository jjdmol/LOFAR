//# HamakerDipole.cc: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna"
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/lofar_iomanip.h>

#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

HamakerDipole::HamakerDipole(const BeamCoeff &coeff, const Expr &azel,
    const Expr &orientation)
    : itsBeamCoeff(coeff)
{
    ASSERT(itsBeamCoeff.coeff);
    ASSERT(itsBeamCoeff.coeff->size() > 0);

    addChild(azel);
    addChild(orientation);
}

JonesResult HamakerDipole::getJResult(const Request &request)
{
    // Evaluate children.
    ResultVec tmpAzel;
    Result tmpOrientation;

    const ResultVec &resAzel = getChild(0).getResultVecSynced(request, tmpAzel);
    const Result &resOrientation =
        getChild(1).getResultSynced(request, tmpOrientation);

    const Matrix &az = resAzel[0].getValue();
    const Matrix &el = resAzel[1].getValue();
    const Matrix &orientation = resOrientation.getValue();

    // Create result.
    JonesResult result;
    result.init();

    Result &resXX = result.result11();
    Result &resXY = result.result12();
    Result &resYX = result.result21();
    Result &resYY = result.result22();

    // Compute main value.
    evaluate(request, az, el, orientation, resXX.getValueRW(),
        resXY.getValueRW(), resYX.getValueRW(), resYY.getValueRW());

    // Compute the perturbed values.
    enum PValues
    {
        PV_AZ, PV_EL, PV_ORIENTATION, N_PValues
    };

    const Result *pvSet[N_PValues] = {&(resAzel[0]), &(resAzel[1]),
        &resOrientation};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &pvAz = pvIter.value(PV_AZ);
        const Matrix &pvEl = pvIter.value(PV_EL);
        const Matrix &pvOrientation = pvIter.value(PV_ORIENTATION);

        evaluate(request, pvAz, pvEl, pvOrientation,
            resXX.getPerturbedValueRW(pvIter.key()),
            resXY.getPerturbedValueRW(pvIter.key()),
            resYX.getPerturbedValueRW(pvIter.key()),
            resYY.getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }

    return result;
}

void HamakerDipole::evaluate(const Request &request,
    const Matrix &in_az, const Matrix &in_el,
    const Matrix &in_orientation,
    Matrix &out_E11, Matrix &out_E12,
    Matrix &out_E21, Matrix &out_E22)
{
    const size_t nChannels = request.getChannelCount();
    const size_t nTimeslots = request.getTimeslotCount();

    // Check preconditions.
    ASSERT(static_cast<size_t>(in_az.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_el.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_orientation.nelements()) == 1);

    // Get pointers to input and output data.
    const double *az = in_az.doubleStorage();
    const double *el = in_el.doubleStorage();
    const double orientation = in_orientation.getDouble(0, 0);

    double *E11_re, *E11_im;
    out_E11.setDCMat(nChannels, nTimeslots);
    out_E11.dcomplexStorage(E11_re, E11_im);

    double *E12_re, *E12_im;
    out_E12.setDCMat(nChannels, nTimeslots);
    out_E12.dcomplexStorage(E12_re, E12_im);

    double *E21_re, *E21_im;
    out_E21.setDCMat(nChannels, nTimeslots);
    out_E21.dcomplexStorage(E21_re, E21_im);

    double *E22_re, *E22_im;
    out_E22.setDCMat(nChannels, nTimeslots);
    out_E22.dcomplexStorage(E22_re, E22_im);

    // Evaluate beam.
    const size_t nHarmonics = itsBeamCoeff.coeff->shape()[1];
    const size_t nTheta = itsBeamCoeff.coeff->shape()[2];
    const size_t nFreq = itsBeamCoeff.coeff->shape()[3];

    const boost::multi_array<dcomplex, 4> &coeff = *itsBeamCoeff.coeff;

    size_t sample = 0;
    Axis::ShPtr freqAxis(request.getGrid()[FREQ]);
    for(size_t t = 0; t < nTimeslots; ++t)
    {
        // Correct azimuth for dipole orientation and sign flip (because in the
        // definition of the beam model positive azimuth is defined as North
        // over West).
        const double phi = -(az[t] - orientation);

        // NB: The model is parameterized in terms of zenith angle. The
        // appropriate conversion is taken care of below.
        const double theta = casa::C::pi_2 - el[t];

        for(size_t f = 0; f < nChannels; ++f)
        {
            // NB: The model is parameterized in terms of a normalized
            // frequency in the range [-1, 1]. The appropriate conversion is
            // taken care of below.
            const double scaledFreq = (freqAxis->center(f)
                - itsBeamCoeff.freqAvg) / itsBeamCoeff.freqRange;

            // J-jones matrix (2x2 complex matrix)
            dcomplex J[2][2];
            J[0][0] = J[0][1] = J[1][0] = J[1][1] = makedcomplex(0.0, 0.0);

            for(size_t k = 0; k < nHarmonics; ++k)
            {
                // Compute diagonal projection matrix P for the current
                // harmonic.
                dcomplex P[2];
                P[0] = P[1] = makedcomplex(0.0, 0.0);

                dcomplex inner[2];
                for(long i = nTheta - 1; i >= 0; --i)
                {
                    inner[0] = coeff[0][k][i][nFreq - 1];
                    inner[1] = coeff[1][k][i][nFreq - 1];

                    for(long j = nFreq - 2; j >= 0; --j)
                    {
                        inner[0] = inner[0] * scaledFreq + coeff[0][k][i][j];
                        inner[1] = inner[1] * scaledFreq + coeff[1][k][i][j];
                    }
                    P[0] = P[0] * theta + inner[0];
                    P[1] = P[1] * theta + inner[1];
                }

                // Compute J-jonex matrix for this harmonic by rotating
                // P over kappa * phi and add it to the result.
                const double kappa = ((k & 1) == 0 ? 1.0 : -1.0) * (2.0 * k + 1.0);
                const double cphi = std::cos(kappa * phi);
                const double sphi = std::sin(kappa * phi);

                J[0][0] += cphi * P[0];
                J[0][1] += -sphi * P[1];
                J[1][0] += sphi * P[0];
                J[1][1] += cphi * P[1];
            }

            E11_re[sample] = real(J[0][0]);
            E11_im[sample] = imag(J[0][0]);
            E12_re[sample] = real(J[0][1]);
            E12_im[sample] = imag(J[0][1]);
            E21_re[sample] = real(J[1][0]);
            E21_im[sample] = imag(J[1][0]);
            E22_re[sample] = real(J[1][1]);
            E22_im[sample] = imag(J[1][1]);

            // Move to next sample.
            ++sample;
        }
    }
}



} //# namespace BBS
} //# namespace LOFAR
