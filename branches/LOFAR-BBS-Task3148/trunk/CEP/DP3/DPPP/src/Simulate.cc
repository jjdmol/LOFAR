//# Simulate.cc: Simulate visibilities for a patch of sources.
//#
//# Copyright (C) 2012
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
#include <DPPP/Simulate.h>
#include <Common/LofarLogger.h>
#include <casa/BasicSL/Constants.h>

// Only required for rotateUVW().
#include <DPPP/PhaseShift.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>

namespace LOFAR
{
namespace DPPP
{

namespace
{
// Compute LMN coordinates of \p position relative to \p reference.
//
// \param[in]   reference
// Reference position on the celestial sphere.
// \param[in]   position
// Position of interest on the celestial sphere.
// \param[in]   lmn
// Pointer to a buffer of (at least) length three into which the computed LMN
// coordinates will be written.
inline void radec2lmn(const Position &reference, const Position &position,
    double *lmn);
} // Unnamed namespace.

void splitUVW(size_t nStation, size_t nBaseline,
    const_cursor<Baseline> baselines, const_cursor<double> uvw,
    cursor<double> split)
{
    cursor<double> known(split);
    vector<bool> flag(nStation, false);

    split[0] = 0.0;
    split[1] = 0.0;
    split[2] = 0.0;
    flag[0] = true;

    for(size_t i = 0; i < nBaseline; ++i)
    {
        const size_t p = baselines->first;
        const size_t q = baselines->second;
        if(p != q && flag[p] != flag[q])
        {
            if(flag[p])
            {
                known.forward(1, p);
                split.forward(1, q);
                split[0] = uvw[0] + known[0];
                split[1] = uvw[1] + known[1];
                split[2] = uvw[2] + known[2];
                split.backward(1, q);
                known.backward(1, p);
                flag[q] = true;
            }
            else
            {
                known.forward(1, q);
                split.forward(1, p);
                split[0] = -uvw[0] + known[0];
                split[1] = -uvw[1] + known[1];
                split[2] = -uvw[2] + known[2];
                split.backward(1, p);
                known.backward(1, q);
                flag[p] = true;
            }
        }
        uvw.forward(1);
        ++baselines;
    }

    ASSERTSTR(static_cast<size_t>(std::count(flag.begin(), flag.end(), true))
        == flag.size(), "Unable to split baseline UVW coordinates into station"
        " UVW coordinates.");
}

void rotateUVW(const Position &from, const Position &to, size_t nUVW,
    cursor<double> uvw)
{
    casa::Matrix<double> oldUVW(3,3);
    casa::Matrix<double> newUVW(3,3);
    PhaseShift::fillTransMatrix(oldUVW, from[0], from[1]);
    PhaseShift::fillTransMatrix(newUVW, to[0], to[1]);

    casa::Matrix<double> tmp(casa::product(casa::transpose(newUVW), oldUVW));
    const double *R = tmp.data();

    for(size_t i = 0; i < nUVW; ++i)
    {
        // Compute rotated UVW.
        double u = uvw[0] * R[0] + uvw[1] * R[3] + uvw[2] * R[6];
        double v = uvw[0] * R[1] + uvw[1] * R[4] + uvw[2] * R[7];
        double w = uvw[0] * R[2] + uvw[1] * R[5] + uvw[2] * R[8];

        uvw[0] = u;
        uvw[1] = v;
        uvw[2] = w;

        // Move to next station.
        uvw.forward(1);
    } // Stations.
}

void simulate(const Position &reference, const Patch &patch, size_t nStation,
    size_t nBaseline, size_t nChannel, const_cursor<Baseline> baselines,
    const_cursor<double> freq, const_cursor<double> uvw, cursor<dcomplex> vis)
{
    vector<dcomplex> buf_shift(nStation * nChannel);
    vector<dcomplex> buf_spectrum(nChannel * 4);

    for(Patch::const_iterator it = patch.begin(), end = patch.end(); it != end;
        ++it)
    {
        // Compute LMN coordinates.
        double lmn[3];
        radec2lmn(reference, it->position(), lmn);

        // Compute station phase shifts.
        dcomplex *shift = &(buf_shift[0]);
        for(size_t st = 0; st < nStation; ++st)
        {
            const double phase = casa::C::_2pi * (uvw[0] * lmn[0]
                + uvw[1] * lmn[1] + uvw[2] * (lmn[2] - 1.0));
            uvw.forward(1);

            for(size_t ch = 0; ch < nChannel; ++ch)
            {
                const double chPhase = phase * (*freq) / casa::C::c;
                *shift = dcomplex(cos(chPhase), sin(chPhase));
                ++freq;
                ++shift;
            } // Channels.
            freq -= nChannel;
        } // Stations.
        uvw.backward(1, nStation);

        // Compute source spectrum.
        dcomplex *spectrum = &(buf_spectrum[0]);
        for(size_t ch = 0; ch < nChannel; ++ch)
        {
            Stokes stokes = it->stokes(*freq);
            ++freq;

            *spectrum++ = dcomplex(stokes.I + stokes.Q, 0.0);
            *spectrum++ = dcomplex(stokes.U, stokes.V);
            *spectrum++ = dcomplex(stokes.U, -stokes.V);
            *spectrum++ = dcomplex(stokes.I - stokes.Q, 0.0);
        } // Channels.
        freq -= nChannel;

        for(size_t bl = 0; bl < nBaseline; ++bl)
        {
            const size_t p = baselines->first;
            const size_t q = baselines->second;

            if(p != q)
            {
                const dcomplex *shiftP = &(buf_shift[p * nChannel]);
                const dcomplex *shiftQ = &(buf_shift[q * nChannel]);
                const dcomplex *spectrum = &(buf_spectrum[0]);
                for(size_t ch = 0; ch < nChannel; ++ch)
                {
                    // Compute baseline phase shift.
                    const dcomplex blShift = (*shiftQ) * conj(*shiftP);
                    ++shiftP;
                    ++shiftQ;

                    // Compute visibilities.
                    *vis += blShift * (*spectrum);
                    ++vis;
                    ++spectrum;
                    *vis += blShift * (*spectrum);
                    ++vis;
                    ++spectrum;
                    *vis += blShift * (*spectrum);
                    ++vis;
                    ++spectrum;
                    *vis += blShift * (*spectrum);
                    ++vis;
                    ++spectrum;

                    // Move to next channel.
                    vis -= 4;
                    vis.forward(1);
                } // Channels.
                vis.backward(1, nChannel);
            }

            // Move to next baseline.
            vis.forward(2);
            ++baselines;
        } // Baselines.
        vis.backward(2, nBaseline);
        baselines -= nBaseline;
    } // Components.
}

namespace
{
inline void radec2lmn(const Position &reference, const Position &position,
    double *lmn)
{
    const double dRA = position[0] - reference[0];
    const double pDEC = position[1];
    const double rDEC = reference[1];
    const double cDEC = cos(pDEC);

    const double l = cDEC * sin(dRA);
    const double m = sin(pDEC) * cos(rDEC) - cDEC * sin(rDEC) * cos(dRA);

    lmn[0] = l;
    lmn[1] = m;
    lmn[2] = sqrt(1.0 - l * l - m * m);
}
} // Unnamed namespace.

} //# namespace DPPP
} //# namespace LOFAR
