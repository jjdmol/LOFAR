//# EstimateMixed.cc: Estimate Jones matrices for several directions simultaneously. A separate data stream is used for each direction. The mixing coefficients quantify the influence of each direction on each of the other directions (including time and frequency smearing).
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
#include <DPPP/EstimateMixed.h>
#include <Common/OpenMP.h>

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iomanip.h>
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

#include <scimath/Fitting/LSQFit.h>
#include <boost/multi_array.hpp>

namespace LOFAR
{
namespace DPPP
{

void estimate(size_t nDirection, size_t nStation, size_t nBaseline,
    size_t nChannel, vector<const_cursor<fcomplex> > data,
    vector<const_cursor<dcomplex> > model, const_cursor<Baseline> baselines,
    const_cursor<bool> flag, const_cursor<float> weight,
    const_cursor<dcomplex> mix, cursor<double> unknowns)
{
    ASSERT(data.size() == nDirection && model.size() == nDirection);
    LOG_DEBUG_STR("#dr: " << nDirection << " #st: " << nStation << " #bl: "
        << nBaseline << " #ch: " << nChannel << " #cr: " << 4);
    LOG_DEBUG_STR("#unknowns: " << nDirection * nStation * 4 * 2);

    casa::LSQFit solver(nDirection * nStation * 4 * 2);

    // Each visibility provides information about two (complex) unknowns per
    // station per direction. A visibility is measured by a specific
    // interferometer, which is the combination of two stations. Thus, in total
    // each visibility provides information about (no. of directions) x 2 x 2
    // x 2 (scalar) unknowns = (no. of directions) x 8. For each of these
    // unknowns the value of the partial derivative of the model with respect
    // to the unknow has to be computed.
    const unsigned int nPartial = nDirection * 8;

    // Construct partial derivative index template for each correlation.
    boost::multi_array<unsigned int, 2> dIndexTemplate(boost::extents[4]
        [nPartial]);
    for(size_t cr = 0; cr < 4; ++cr)
    {
        size_t idx0 = (cr / 2) * 4 * unknowns.stride(0);
        size_t idx1 = (cr & 1) * 4 * unknowns.stride(0);

        for(size_t dr = 0; dr < nDirection; ++dr)
        {
            dIndexTemplate[cr][dr * 8 + 0] = idx0 + 0 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 1] = idx0 + 1 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 2] = idx0 + 2 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 3] = idx0 + 3 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 4] = idx1 + 0 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 5] = idx1 + 1 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 6] = idx1 + 2 * unknowns.stride(0);
            dIndexTemplate[cr][dr * 8 + 7] = idx1 + 3 * unknowns.stride(0);
            idx0 += unknowns.stride(2);
            idx1 += unknowns.stride(2);
        }
    }

    // Allocate space for intermediate results.
    boost::multi_array<dcomplex, 2> M(boost::extents[nDirection][4]);
    boost::multi_array<dcomplex, 3> dM(boost::extents[nDirection][4][4]);
    boost::multi_array<double, 1> dR(boost::extents[nPartial]);
    boost::multi_array<double, 1> dI(boost::extents[nPartial]);
    boost::multi_array<unsigned int, 2> dIndex(boost::extents[4][nPartial]);

    dcomplex coherence;
    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jp_00_s0, Jp_10_s0, Jp_00_s1, Jp_10_s1, Jp_01_s2, Jp_11_s2,
        Jp_01_s3, Jp_11_s3;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2,
        Jq_01_s3, Jq_11_s3;

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    vector<const_cursor<fcomplex> > c_data(data);
    vector<const_cursor<dcomplex> > c_model(model);
    const_cursor<bool> c_flag = flag;
    const_cursor<float> c_weight = weight;
    const_cursor<dcomplex> c_mix = mix;
    const_cursor<Baseline> c_baselines = baselines;
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

    // Set default iteration axis of the mixing coefficients to 1.
    mix.axis(1);

    // Iterate until convergence.
    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].start();
#endif

        for(size_t bl = 0; bl < nBaseline; ++bl)
        {
            const size_t p = baselines->first;
            const size_t q = baselines->second;

            if(p != q)
            {
                // Update partial derivative index for current baseline.
                const size_t offsetP = p * unknowns.stride(1);
                const size_t offsetQ = q * unknowns.stride(1);
                for(size_t cr = 0; cr < 4; ++cr)
                {
                    for(size_t dr = 0; dr < nDirection; ++dr)
                    {
                        dIndex[cr][dr * 8 + 0] = dIndexTemplate[cr][dr * 8 + 0] + offsetP;
                        dIndex[cr][dr * 8 + 1] = dIndexTemplate[cr][dr * 8 + 1] + offsetP;
                        dIndex[cr][dr * 8 + 2] = dIndexTemplate[cr][dr * 8 + 2] + offsetP;
                        dIndex[cr][dr * 8 + 3] = dIndexTemplate[cr][dr * 8 + 3] + offsetP;
                        dIndex[cr][dr * 8 + 4] = dIndexTemplate[cr][dr * 8 + 4] + offsetQ;
                        dIndex[cr][dr * 8 + 5] = dIndexTemplate[cr][dr * 8 + 5] + offsetQ;
                        dIndex[cr][dr * 8 + 6] = dIndexTemplate[cr][dr * 8 + 6] + offsetQ;
                        dIndex[cr][dr * 8 + 7] = dIndexTemplate[cr][dr * 8 + 7] + offsetQ;
                    }
                }

                for(size_t ch = 0; ch < nChannel; ++ch)
                {
                    for(size_t dr = 0; dr < nDirection; ++dr)
                    {
                        unknowns.forward(1, p);
                        Jp_00 = dcomplex(unknowns[0], unknowns[1]);
                        Jp_01 = dcomplex(unknowns[2], unknowns[3]);
                        Jp_10 = dcomplex(unknowns[4], unknowns[5]);
                        Jp_11 = dcomplex(unknowns[6], unknowns[7]);
                        unknowns.backward(1, p);

                        unknowns.forward(1, q);
                        Jq_00 = dcomplex(unknowns[0], -unknowns[1]);
                        Jq_01 = dcomplex(unknowns[2], -unknowns[3]);
                        Jq_10 = dcomplex(unknowns[4], -unknowns[5]);
                        Jq_11 = dcomplex(unknowns[6], -unknowns[7]);
                        unknowns.backward(1, q);

                        coherence = (model[dr][0]);
                        Jp_00_s0 = Jp_00 * coherence;
                        Jp_10_s0 = Jp_10 * coherence;
                        Jq_00_s0 = Jq_00 * coherence;
                        Jq_10_s0 = Jq_10 * coherence;

                        coherence = (model[dr][1]);
                        Jp_00_s1 = Jp_00 * coherence;
                        Jp_10_s1 = Jp_10 * coherence;
                        Jq_01_s1 = Jq_01 * coherence;
                        Jq_11_s1 = Jq_11 * coherence;

                        coherence = (model[dr][2]);
                        Jp_01_s2 = Jp_01 * coherence;
                        Jp_11_s2 = Jp_11 * coherence;
                        Jq_00_s2 = Jq_00 * coherence;
                        Jq_10_s2 = Jq_10 * coherence;

                        coherence = (model[dr][3]);
                        Jp_01_s3 = Jp_01 * coherence;
                        Jp_11_s3 = Jp_11 * coherence;
                        Jq_01_s3 = Jq_01 * coherence;
                        Jq_11_s3 = Jq_11 * coherence;

                        M[dr][0] = Jp_00 * (Jq_00_s0 + Jq_01_s1)
                            + Jp_01 * (Jq_00_s2 + Jq_01_s3);
                        dM[dr][0][0] = Jq_00_s0 + Jq_01_s1;
                        dM[dr][0][1] = Jq_00_s2 + Jq_01_s3;
                        dM[dr][0][2] = Jp_00_s0 + Jp_01_s2;
                        dM[dr][0][3] = Jp_00_s1 + Jp_01_s3;

                        M[dr][1] = Jp_00 * (Jq_10_s0 + Jq_11_s1)
                            + Jp_01 * (Jq_10_s2 + Jq_11_s3);
                        dM[dr][1][0] = Jq_10_s0 + Jq_11_s1;
                        dM[dr][1][1] = Jq_10_s2 + Jq_11_s3;
                        dM[dr][1][2] = Jp_00_s0 + Jp_01_s2; // = dM[dr][0][2];
                        dM[dr][1][3] = Jp_00_s1 + Jp_01_s3; // = dM[dr][0][3];

                        M[dr][2] = Jp_10 * (Jq_00_s0 + Jq_01_s1)
                            + Jp_11 * (Jq_00_s2 + Jq_01_s3);
                        dM[dr][2][0] = Jq_00_s0 + Jq_01_s1; // = dM[dr][0][0];
                        dM[dr][2][1] = Jq_00_s2 + Jq_01_s3; // = dM[dr][0][1];
                        dM[dr][2][2] = Jp_10_s0 + Jp_11_s2;
                        dM[dr][2][3] = Jp_10_s1 + Jp_11_s3;

                        M[dr][3] = Jp_10 * (Jq_10_s0 + Jq_11_s1)
                            + Jp_11 * (Jq_10_s2 + Jq_11_s3);
                        dM[dr][3][0] = Jq_10_s0 + Jq_11_s1; // = dM[dr][1][0];
                        dM[dr][3][1] = Jq_10_s2 + Jq_11_s3; // = dM[dr][1][1];
                        dM[dr][3][2] = Jp_10_s0 + Jp_11_s2; // = dM[dr][2][2];
                        dM[dr][3][3] = Jp_10_s1 + Jp_11_s3; // = dM[dr][2][3];

                        // Move to next direction.
                        unknowns.forward(2);
                    }
                    unknowns.backward(2, nDirection);

                    for(size_t cr = 0; cr < 4; ++cr)
                    {
                        if(!flag[cr])
                        {
                            for(size_t tg = 0; tg < nDirection; ++tg)
                            {
                                dcomplex model = 0.0, partial;
                                for(size_t dr = 0; dr < nDirection; ++dr)
                                {
                                    // Look-up mixing term.
                                    const dcomplex term = *mix;
                                    ++mix;

                                    // Update model visibility.
                                    model += term * M[dr][cr];

                                    // Compute partial derivatives.
                                    partial = term * dM[dr][cr][0];
                                    dR[dr * 8] = real(partial);
                                    dI[dr * 8] = imag(partial);
                                    dR[dr * 8 + 1] = -imag(partial);
                                    dI[dr * 8 + 1] = real(partial);

                                    partial = term * dM[dr][cr][1];
                                    dR[dr * 8 + 2] = real(partial);
                                    dI[dr * 8 + 2] = imag(partial);
                                    dR[dr * 8 + 3] = -imag(partial);
                                    dI[dr * 8 + 3] = real(partial);

                                    partial = term * dM[dr][cr][2];
                                    dR[dr * 8 + 4] = real(partial);
                                    dI[dr * 8 + 4] = imag(partial);
                                    dR[dr * 8 + 5] = imag(partial);
                                    dI[dr * 8 + 5] = -real(partial);

                                    partial = term * dM[dr][cr][3];
                                    dR[dr * 8 + 6] = real(partial);
                                    dI[dr * 8 + 6] = imag(partial);
                                    dR[dr * 8 + 7] = imag(partial);
                                    dI[dr * 8 + 7] = -real(partial);
                                } // Source directions.

                                // Compute the residual.
                                dcomplex residual = static_cast<dcomplex>(data[tg][cr]) - model;

                                // Update the normal equations.
                                solver.makeNorm(nPartial, &(dIndex[cr][0]),
                                    &(dR[0]), static_cast<double>(weight[cr]),
                                    real(residual));
                                solver.makeNorm(nPartial, &(dIndex[cr][0]),
                                    &(dI[0]), static_cast<double>(weight[cr]),
                                    imag(residual));

                                // Move to next target direction.
                                mix -= nDirection;
                                mix.forward(0);
                            } // Target directions.
                            mix.backward(0, nDirection);
                        }

                        // Move to the next correlation.
                        mix.forward(2);
                    } // Correlations.

                    // Move to the next channel.
                    mix.backward(2, 4);
                    mix.forward(3);

                    for(size_t dr = 0; dr < nDirection; ++dr)
                    {
                        model[dr].forward(1);
                        data[dr].forward(1);
                    }
                    flag.forward(1);
                    weight.forward(1);
                } // Channels.

                // Reset cursors to the start of the baseline.
                for(size_t dr = 0; dr < nDirection; ++dr)
                {
                    model[dr].backward(1, nChannel);
                    data[dr].backward(1, nChannel);
                }
                flag.backward(1, nChannel);
                weight.backward(1, nChannel);
                mix.backward(3, nChannel);
            }

            // Move cursors to the next baseline.
            for(size_t dr = 0; dr < nDirection; ++dr)
            {
                model[dr].forward(2);
                data[dr].forward(2);
            }
            flag.forward(2);
            weight.forward(2);
            mix.forward(4);
            ++baselines;
        } // Baselines.

        // Reset all cursors for the next iteration.
        for(size_t dr = 0; dr < nDirection; ++dr)
        {
            model[dr].backward(2, nBaseline);
            data[dr].backward(2, nBaseline);
        }
        flag.backward(2, nBaseline);
        weight.backward(2, nBaseline);
        mix.backward(4, nBaseline);
        baselines -= nBaseline;

#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].start();
#endif
        // Do solve iteration.
        casa::uInt rank;
        bool status = solver.solveLoop(rank, &(*unknowns), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].stop();
#endif

        // Update iteration count.
        ++nIterations;

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
        for(size_t dr = 0; dr < nDirection; ++dr)
        {
            ASSERT(model[dr].address() == c_model[dr].address());
            ASSERT(data[dr].address() == c_data[dr].address());
        }
        ASSERT(flag.address() == c_flag.address());
        ASSERT(weight.address() == c_weight.address());
        ASSERT(mix.address() == c_mix.address());
        ASSERT(baselines.address() == c_baselines.address());
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() //<< " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);
}

} //# namespace DPPP
} //# namespace LOFAR
