//# HamakerDipole.cc: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna".
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
#include <BBSKernel/Exceptions.h>

#include <casa/BasicSL/Constants.h>
#include <casa/OS/Path.h>

namespace LOFAR
{
namespace BBS
{

HamakerBeamCoeff::HamakerBeamCoeff()
    :   itsCenter(0.0),
        itsWidth(1.0)
{
}

void HamakerBeamCoeff::init(const casa::Path &coeffFile)
{
    // Open file.
    casa::String path = coeffFile.expandedName();
    ifstream in(path.c_str());
    if(!in)
    {
        THROW(BBSKernelException, "" << path << ": unable to open file");
    }

    // Read file header.
    string header, token0, token1, token2, token3, token4, token5;
    getline(in, header);

    size_t nElements, nHarmonics, nPowerTime, nPowerFreq;
    double freqAvg, freqRange;

    istringstream iss(header);
    iss >> token0 >> nElements >> token1 >> nHarmonics >> token2 >> nPowerTime
        >> token3 >> nPowerFreq >> token4 >> freqAvg >> token5 >> freqRange;

    if(!in || !iss || token0 != "d" || token1 != "k" || token2 != "pwrT"
        || token3 != "pwrF" || token4 != "freqAvg" || token5 != "freqRange")
    {
        THROW(BBSKernelException, "" << path << ": unable to parse header");
    }

    if(nElements * nHarmonics * nPowerTime * nPowerFreq == 0)
    {
        THROW(BBSKernelException, "" << path << ": the number of coefficients"
            " should be larger than zero.");
    }

    LOG_DEBUG_STR("" << path << ": nElements: " << nElements << " nHarmonics: "
        << nHarmonics << " nPowerTime: " << nPowerTime << " nPowerFreq: "
        << nPowerFreq);

    // Allocate coefficient matrix.
    itsCenter = freqAvg;
    itsWidth = freqRange;
    itsCoeff = casa::Array<dcomplex>(casa::IPosition(4, nPowerFreq, nPowerTime,
        nHarmonics, nElements));

    size_t nCoeff = 0;
    while(in.good())
    {
        // Read line from file.
        string line;
        getline(in, line);

        // Skip lines that contain only whitespace.
        if(line.find_last_not_of(" ") == string::npos)
        {
            continue;
        }

        // Parse line.
        size_t element, harmonic, powerTime, powerFreq;
        double re, im;

        iss.clear();
        iss.str(line);
        iss >> element >> harmonic >> powerTime >> powerFreq >> re >> im;

        if(!iss || element >= nElements || harmonic >= nHarmonics
            || powerTime >= nPowerTime || powerFreq >= nPowerFreq)
        {
            THROW(BBSKernelException, "" << path << ": errror reading file.");
        }

        // Store coefficient.
        itsCoeff(casa::IPosition(4, powerFreq, powerTime, harmonic, element)) =
            makedcomplex(re, im);

        // Update coefficient counter.
        ++nCoeff;
    }

    if(!in.eof())
    {
        THROW(BBSKernelException, "" << path << ": error reading file.");
    }

    if(nCoeff != nElements * nHarmonics * nPowerTime * nPowerFreq)
    {
        THROW(BBSKernelException, "" << path << ": the number of coefficients"
            " specified in the header does not match the number of coefficients"
            " in the file.");
    }
}

HamakerDipole::HamakerDipole(const HamakerBeamCoeff &coeff,
    const Expr<Vector<2> >::ConstPtr &target,
    const Expr<Scalar>::ConstPtr &orientation)
    :   BasicBinaryExpr<Vector<2>, Scalar, JonesMatrix>(target, orientation),
        itsCoeff(coeff)
{
}

const JonesMatrix::View HamakerDipole::evaluateImpl(const Grid &grid,
    const Vector<2>::View &target, const Scalar::View &orientation) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(static_cast<size_t>(target(0).nelements()) == nTime);
    ASSERT(static_cast<size_t>(target(1).nelements()) == nTime);
    ASSERT(static_cast<size_t>(orientation().nelements()) == 1);

    // Get pointers to input and output data.
    const double *theta = target(0).doubleStorage();
    const double *phi = target(1).doubleStorage();
    const double phi0 = orientation().getDouble(0, 0);

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

    // Evaluate beam.
    const unsigned int nHarmonics = itsCoeff.shape(1);
    const unsigned int nPowTheta = itsCoeff.shape(2);
    const unsigned int nPowFreq = itsCoeff.shape(3);

    for(size_t t = 0; t < nTime; ++t)
    {
        // Correct phi for the dipole reference orientation.
        const double theta_t = theta[t];
        const double phi_t = phi[t] - phi0;

        for(size_t f = 0; f < nFreq; ++f)
        {
            // J-jones matrix (2x2 complex matrix)
            dcomplex J[2][2] = {{0.0, 0.0}, {0.0, 0.0}};

            // Only compute the beam response for directions above the horizon.
            if(theta_t < casa::C::pi_2)
            {
                // NB: The model is parameterized in terms of a normalized
                // frequency in the range [-1, 1]. The appropriate conversion is
                // taken care of below.
                const double normFreq = (grid[FREQ]->center(f)
                    - itsCoeff.center()) / itsCoeff.width();

                for(size_t k = 0; k < nHarmonics; ++k)
                {
                    // Compute diagonal projection matrix P for the current
                    // harmonic.
                    dcomplex P[2] = {0.0, 0.0};

                    dcomplex inner[2];
                    for(long i = nPowTheta - 1; i >= 0; --i)
                    {
                        inner[0] = itsCoeff(0, k, i, nPowFreq - 1);
                        inner[1] = itsCoeff(1, k, i, nPowFreq - 1);

                        for(long j = nPowFreq - 2; j >= 0; --j)
                        {
                            inner[0] =
                                inner[0] * normFreq + itsCoeff(0, k, i, j);
                            inner[1] =
                                inner[1] * normFreq + itsCoeff(1, k, i, j);
                        }
                        P[0] = P[0] * theta_t + inner[0];
                        P[1] = P[1] * theta_t + inner[1];
                    }

                    // Compute Jones matrix for this harmonic by rotating P over
                    // kappa * phi_t and add it to the result.
                    const double kappa =
                        ((k & 1) == 0 ? 1.0 : -1.0) * (2.0 * k + 1.0);
                    const double cphi = std::cos(kappa * phi_t);
                    const double sphi = std::sin(kappa * phi_t);

                    J[0][0] += cphi * P[0];
                    J[0][1] += -sphi * P[1];
                    J[1][0] += sphi * P[0];
                    J[1][1] += cphi * P[1];
                }
            }

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

    JonesMatrix::View result;
    result.assign(0, 0, E00);
    result.assign(0, 1, E01);
    result.assign(1, 0, E10);
    result.assign(1, 1, E11);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
