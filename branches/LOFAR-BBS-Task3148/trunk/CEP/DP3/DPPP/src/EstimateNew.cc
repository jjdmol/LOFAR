//# EstimateNew.cc: Experimental parameter estimation using a flattened
//# expression tree.
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
#include <DPPP/EstimateNew.h>
#include <BBSKernel/EstimateUtil.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <casa/BasicSL/Constants.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_fstream.h>

namespace LOFAR
{
namespace DPPP
{
using LOFAR::operator<<;

namespace
{
struct vec2
{
    const double &operator[](size_t i) const { return __data[i]; }
    double &operator[](size_t i) { return __data[i]; }
    double __data[2];
};

struct vec3
{
    const double &operator[](size_t i) const { return __data[i]; }
    double &operator[](size_t i) { return __data[i]; }
    double __data[3];
};

struct source
{
    source(double ra, double dec, double I, double Q = 0.0,
        double U = 0.0, double V = 0.0)
    {
        position[0] = ra; position[1] = dec;
        stokes[0] = I; stokes[1] = Q; stokes[2] = U; stokes[3] = V;
    }

    double  stokes[4];
    double  position[2];
};

size_t __NDIR;
vector<vector<source> > __patches;
vector<vector<vec3> > __lmn;

void propagate(EstimateState &state, size_t from, size_t to)
{
    size_t nSol = state.nStat * __NDIR * 8;
    LOG_DEBUG_STR("propagating " << nSol << " coefficients from cell: "
        << from << " to: " << to);

    double *src = &(state.J[from][0][0][0]);
    double *dest = &(state.J[to][0][0][0]);
    for(size_t i = 0; i < nSol; ++i)
    {
        dest[i] = src[i];
    }
}

} //# unnamed namespace


void estimateImpl(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state, size_t ts);

//void estimateImpl3(DPBuffer &target,
//    const vector<DPBuffer> &buffers,
//    const casa::Array<casa::DComplex> &coeff,
//    const casa::Array<casa::DComplex> &coeffSub,
//    EstimateState &state,
//    size_t ts);

void estimate(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &buffers,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffSub,
    EstimateState &state,
    size_t ts)
{
    LOG_DEBUG_STR("sizeof(source): " << sizeof(source));
    LOG_DEBUG_STR("buffers #dir: " << buffers.size() << " #time: "
        << buffers[0].size());
    LOG_DEBUG_STR("coeff #time: " << coeff.size() << " shape: "
        << coeff[0].shape());
//    LOG_DEBUG_STR("grid shape #freq: " << visGrid[0]->size() << " #time:"
//        << visGrid[1]->size());

    ASSERT(buffers.size() == __NDIR + 1);
    const size_t nTime = buffers[0].size();
//    ASSERT(target.size() == nTime);
    ASSERT(buffers[1].size() == nTime);
    ASSERT(coeff.size() == nTime);

    // Transpose axes of input buffer array.
    vector<vector<DPBuffer> > buffersT(nTime);
    for(size_t t = 0; t < nTime; ++t)
    {
        for(size_t dr = 0; dr < buffers.size(); ++dr)
        {
            buffersT[t].push_back(buffers[dr][t]);
        }
    }

    // Propagate solutions from previous block.
    if(ts > 0)
    {
        for(size_t i = 0; i < nTime; ++i)
        {
            propagate(state, ts - 1, ts + i);
        }
    }

#pragma omp parallel for
    for(size_t i = 0; i < nTime; ++i)
    {
        estimateImpl(target[i], buffersT[i], coeff[i], coeffSub[i], state,
            ts + i);

//        // DOES NOT WORK MT, AND IF NT == NTHREADS NOT NEEDED...
//        if(nTimeslot > i + 1)
//        {
//            propagate(state, ts + i, ts + i + 1);
//        }
    }
}

void estimateImpl(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts)
{
    const size_t threadID = OpenMP::threadNum();

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].start();
#endif

    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __NDIR;
    const size_t nTargets = buffers.size();
    const size_t nSt = state.nStat;
    const size_t nCr = 4;

#ifdef ESTIMATE_TIMER
    state.tSim[threadID].start();
#endif

    boost::multi_array<double, 2> uvw_split(boost::extents[nSt][3]);
    boost::multi_array<bool, 1> flag(boost::extents[nSt]);

    const size_t nCh = state.axisDemix->size();
    boost::multi_array<dcomplex, 2> shift(boost::extents[nSt][nCh]);
    boost::multi_array<dcomplex, 4> sim(boost::extents[nModels][nBl][nCh][nCr]);

    // Simulate visibilities for each direction.
    fill(sim.data(), sim.data() + sim.num_elements(), dcomplex(0.0, 0.0));

//    const double _2pi_lambda = casa::C::_2pi * state.freq / casa::C::c;
    for(size_t i = 0; i < nModels; ++i)
    {
        // Split UVW.
        const casa::Matrix<double> &uvw = buffers[i].getUVW();

        size_t found = 1;
        fill(flag.data(), flag.data() + flag.num_elements(), false);
        uvw_split[0][0] = 0.0;
        uvw_split[0][1] = 0.0;
        uvw_split[0][2] = 0.0;
        flag[0] = true;

        for(size_t j = 0; j < nBl; ++j)
        {
            const size_t p = state.baselines[j].first;
            const size_t q = state.baselines[j].second;

            if(p == q || flag[p] == flag[q])
            {
                continue;
            }

            if(flag[p])
            {
                uvw_split[q][0] = uvw(0, j) + uvw_split[p][0];
                uvw_split[q][1] = uvw(1, j) + uvw_split[p][1];
                uvw_split[q][2] = uvw(2, j) + uvw_split[p][2];
                flag[q] = true;
            }
            else
            {
                uvw_split[p][0] = -uvw(0, j) + uvw_split[q][0];
                uvw_split[p][1] = -uvw(1, j) + uvw_split[q][1];
                uvw_split[p][2] = -uvw(2, j) + uvw_split[q][2];
                flag[p] = true;
            }
            ++found;
        }
        ASSERTSTR(found == nSt, "Could not split UVW, found: " << found);

        const vector<source> &sources = __patches[i];
        const size_t nSources = sources.size();
//        LOG_DEBUG_STR("dir: " << i << " #sources: " << nSources);

        const vector<vec3> &lmn = __lmn[i];

        for(size_t k = 0; k < nSources; ++k)
        {
            const dcomplex XX = dcomplex(sources[k].stokes[0] + sources[k].stokes[1], 0.0);
            const dcomplex XY = dcomplex(sources[k].stokes[2], sources[k].stokes[3]);
            const dcomplex YX = dcomplex(sources[k].stokes[2], -sources[k].stokes[3]);
            const dcomplex YY = dcomplex(sources[k].stokes[0] - sources[k].stokes[1], 0.0);

            for(size_t j = 0; j < nSt; ++j)
            {
                const double phase = (uvw_split[j][0] * lmn[k][0]
                    + uvw_split[j][1] * lmn[k][1]
                    + uvw_split[j][2] * (lmn[k][2] - 1.0)) * casa::C::_2pi;

                for(size_t ch = 0; ch < nCh; ++ch)
                {
                    const double phase_f = phase * state.axisDemix->center(ch)
                        / casa::C::c;
                    shift[j][ch] = dcomplex(cos(phase_f), sin(phase_f));
                }
            }

            for(size_t j = 0; j < nBl; ++j)
            {
                const size_t p = state.baselines[j].first;
                const size_t q = state.baselines[j].second;

                if(p == q)
                {
                    continue;
                }

                for(size_t ch = 0; ch < nCh; ++ch)
                {
                    const dcomplex blShift = shift[q][ch] * conj(shift[p][ch]);
                    sim[i][j][ch][0] += blShift * XX;
                    sim[i][j][ch][1] += blShift * XY;
                    sim[i][j][ch][2] += blShift * YX;
                    sim[i][j][ch][3] += blShift * YY;
                }
            }
        }
    }

#ifdef ESTIMATE_TIMER
    state.tSim[threadID].stop();
#endif

    // Estimate parameters.
    const size_t nCoeff = nModels * state.nStat * 4 * 2;
    casa::uInt rank;
    casa::LSQFit solver(static_cast<casa::uInt>(nCoeff));
    configLSQSolver(solver, state.lsqOptions);

    // Only two Jones matrix elements per station per direction are used per
    // correlation.
    const unsigned int nDerivative = nModels * 16 / 2;
    boost::multi_array<dcomplex, 2> M(boost::extents[nModels][4]);
    boost::multi_array<dcomplex, 3> dM(boost::extents[nModels][4][4]);
    boost::multi_array<double, 1> dR(boost::extents[nDerivative]);
    boost::multi_array<double, 1> dI(boost::extents[nDerivative]);
    boost::multi_array<unsigned int, 1> dIndex(boost::extents[nDerivative]);

    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jp_00_s0, Jp_10_s0, Jp_00_s1, Jp_10_s1, Jp_01_s2, Jp_11_s2, Jp_01_s3, Jp_11_s3;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2, Jq_01_s3, Jq_11_s3;

    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
        state.tEq[threadID].start();
#endif

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = state.baselines[bl].first;
            const size_t q = state.baselines[bl].second;

            if(p == q)
            {
                continue;
            }

            for(size_t ch = 0; ch < nCh; ++ch)
            {
                for(size_t dr = 0; dr < nModels; ++dr)
                {
                    Jp_00 = dcomplex(state.J[ts][p][dr][0], state.J[ts][p][dr][1]);
                    Jp_01 = dcomplex(state.J[ts][p][dr][2], state.J[ts][p][dr][3]);
                    Jp_10 = dcomplex(state.J[ts][p][dr][4], state.J[ts][p][dr][5]);
                    Jp_11 = dcomplex(state.J[ts][p][dr][6], state.J[ts][p][dr][7]);

                    Jq_00 = dcomplex(state.J[ts][q][dr][0], -state.J[ts][q][dr][1]);
                    Jq_01 = dcomplex(state.J[ts][q][dr][2], -state.J[ts][q][dr][3]);
                    Jq_10 = dcomplex(state.J[ts][q][dr][4], -state.J[ts][q][dr][5]);
                    Jq_11 = dcomplex(state.J[ts][q][dr][6], -state.J[ts][q][dr][7]);

                    Jp_00_s0 = Jp_00 * sim[dr][bl][ch][0];
                    Jp_10_s0 = Jp_10 * sim[dr][bl][ch][0];
                    Jq_00_s0 = Jq_00 * sim[dr][bl][ch][0];
                    Jq_10_s0 = Jq_10 * sim[dr][bl][ch][0];

                    Jp_00_s1 = Jp_00 * sim[dr][bl][ch][1];
                    Jp_10_s1 = Jp_10 * sim[dr][bl][ch][1];
                    Jq_01_s1 = Jq_01 * sim[dr][bl][ch][1];
                    Jq_11_s1 = Jq_11 * sim[dr][bl][ch][1];

                    Jp_01_s2 = Jp_01 * sim[dr][bl][ch][2];
                    Jp_11_s2 = Jp_11 * sim[dr][bl][ch][2];
                    Jq_00_s2 = Jq_00 * sim[dr][bl][ch][2];
                    Jq_10_s2 = Jq_10 * sim[dr][bl][ch][2];

                    Jp_01_s3 = Jp_01 * sim[dr][bl][ch][3];
                    Jp_11_s3 = Jp_11 * sim[dr][bl][ch][3];
                    Jq_01_s3 = Jq_01 * sim[dr][bl][ch][3];
                    Jq_11_s3 = Jq_11 * sim[dr][bl][ch][3];

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
                }

                for(size_t cr = 0; cr < nCr; ++cr)
                {
                    for(size_t tg = 0; tg < nTargets; ++tg)
                    {
                        if(buffers[tg].getFlags()(cr, ch, bl))
                        {
                            continue;
                        }

                        dcomplex V_sim = 0.0;
                        dcomplex tmp;
                        for(size_t dr = 0; dr < nModels; ++dr)
                        {
                            dcomplex weight = coeff(casa::IPosition(5, tg, dr, cr, ch, bl));
                            V_sim += M[dr][cr] * weight;

                            tmp = dM[dr][cr][0] * weight;
                            dR[dr * 8] = real(tmp);
                            dI[dr * 8] = imag(tmp);
                            dR[dr * 8 + 1] = -imag(tmp);
                            dI[dr * 8 + 1] = real(tmp);

                            tmp = dM[dr][cr][1] * weight;
                            dR[dr * 8 + 2] = real(tmp);
                            dI[dr * 8 + 2] = imag(tmp);
                            dR[dr * 8 + 3] = -imag(tmp);
                            dI[dr * 8 + 3] = real(tmp);

                            tmp = dM[dr][cr][2] * weight;
                            dR[dr * 8 + 4] = real(tmp);
                            dI[dr * 8 + 4] = imag(tmp);
                            dR[dr * 8 + 5] = imag(tmp);
                            dI[dr * 8 + 5] = -real(tmp);

                            tmp = dM[dr][cr][3] * weight;
                            dR[dr * 8 + 6] = real(tmp);
                            dI[dr * 8 + 6] = imag(tmp);
                            dR[dr * 8 + 7] = imag(tmp);
                            dI[dr * 8 + 7] = -real(tmp);
                        }

                        const casa::Cube<casa::Complex> &obsData = buffers[tg].getData();
                        const casa::Cube<float> &obsWeight = buffers[tg].getWeights();

                        casa::Complex V_obs = obsData(cr, ch, bl);
                        double reResidual = real(V_obs) - real(V_sim);
                        double imResidual = imag(V_obs) - imag(V_sim);
                        double obsw = obsWeight(cr, ch, bl);

                        // Update the normal equations.
                        solver.makeNorm(nDerivative, &(state.dIndex[bl][cr][0]), &(dR[0]), obsw, reResidual);
                        solver.makeNorm(nDerivative, &(state.dIndex[bl][cr][0]), &(dI[0]), obsw, imResidual);
                    } // target directions
                } // correlations
            } // channels
        } // baselines

#ifdef ESTIMATE_TIMER
        state.tEq[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
        state.tLM[threadID].start();
#endif
        // Do solve iteration.
        bool status = solver.solveLoop(rank, &(state.J[ts][0][0][0]), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
        state.tLM[threadID].stop();
#endif

        // Update iteration count.
        ++nIterations;
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);

#ifdef ESTIMATE_TIMER
    state.tSub[threadID].start();
#endif

    // Subtract...
    const size_t nChRes = state.axisResidual->size();
    if(nCh != nChRes)
    {
        LOG_DEBUG_STR("Re-simulating for subtract... #freq demix: " << nCh
            << " #freq residual: " << nChRes);

        shift.resize(boost::extents[nSt][nChRes]);
        sim.resize(boost::extents[nModels][nBl][nChRes][nCr]);

        // Simulate visibilities for each direction.
        fill(sim.data(), sim.data() + sim.num_elements(), dcomplex(0.0, 0.0));

        for(size_t i = 0; i < nModels; ++i)
        {
            // Split UVW.
            const casa::Matrix<double> &uvw = buffers[i].getUVW();

            size_t found = 1;
            fill(flag.data(), flag.data() + flag.num_elements(), false);
            uvw_split[0][0] = 0.0;
            uvw_split[0][1] = 0.0;
            uvw_split[0][2] = 0.0;
            flag[0] = true;

            for(size_t j = 0; j < nBl; ++j)
            {
                const size_t p = state.baselines[j].first;
                const size_t q = state.baselines[j].second;

                if(p == q || flag[p] == flag[q])
                {
                    continue;
                }

                if(flag[p])
                {
                    uvw_split[q][0] = uvw(0, j) + uvw_split[p][0];
                    uvw_split[q][1] = uvw(1, j) + uvw_split[p][1];
                    uvw_split[q][2] = uvw(2, j) + uvw_split[p][2];
                    flag[q] = true;
                }
                else
                {
                    uvw_split[p][0] = -uvw(0, j) + uvw_split[q][0];
                    uvw_split[p][1] = -uvw(1, j) + uvw_split[q][1];
                    uvw_split[p][2] = -uvw(2, j) + uvw_split[q][2];
                    flag[p] = true;
                }
                ++found;
            }
            ASSERTSTR(found == nSt, "Could not split UVW, found: " << found);

            const vector<source> &sources = __patches[i];
            const size_t nSources = sources.size();

            const vector<vec3> &lmn = __lmn[i];

            for(size_t k = 0; k < nSources; ++k)
            {
                const dcomplex XX = dcomplex(sources[k].stokes[0] + sources[k].stokes[1], 0.0);
                const dcomplex XY = dcomplex(sources[k].stokes[2], sources[k].stokes[3]);
                const dcomplex YX = dcomplex(sources[k].stokes[2], -sources[k].stokes[3]);
                const dcomplex YY = dcomplex(sources[k].stokes[0] - sources[k].stokes[1], 0.0);

                for(size_t j = 0; j < nSt; ++j)
                {
                    const double phase = (uvw_split[j][0] * lmn[k][0]
                        + uvw_split[j][1] * lmn[k][1]
                        + uvw_split[j][2] * (lmn[k][2] - 1.0)) * casa::C::_2pi;

                    for(size_t ch = 0; ch < nChRes; ++ch)
                    {
                        const double phase_f = phase
                            * state.axisResidual->center(ch) / casa::C::c;
                        shift[j][ch] = dcomplex(cos(phase_f), sin(phase_f));
                    }
                }

                for(size_t j = 0; j < nBl; ++j)
                {
                    const size_t p = state.baselines[j].first;
                    const size_t q = state.baselines[j].second;

                    if(p == q)
                    {
                        continue;
                    }

                    for(size_t ch = 0; ch < nChRes; ++ch)
                    {
                        const dcomplex blShift = shift[q][ch] * conj(shift[p][ch]);
                        sim[i][j][ch][0] += blShift * XX;
                        sim[i][j][ch][1] += blShift * XY;
                        sim[i][j][ch][2] += blShift * YX;
                        sim[i][j][ch][3] += blShift * YY;
                    }
                } // baselines
            } // sources
        } // directions
    }

    casa::Cube<casa::Complex> &tgData = target.getData();
    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = state.baselines[bl].first;
        const size_t q = state.baselines[bl].second;

        if(p == q)
        {
            continue;
        }

        for(size_t ch = 0; ch < nChRes; ++ch)
        {
            dcomplex sim_00 = 0.0, sim_01 = 0.0, sim_10 = 0.0, sim_11 = 0.0;
            for(size_t dr = 0; dr < nModels; ++dr)
            {
                Jp_00 = dcomplex(state.J[ts][p][dr][0], state.J[ts][p][dr][1]);
                Jp_01 = dcomplex(state.J[ts][p][dr][2], state.J[ts][p][dr][3]);
                Jp_10 = dcomplex(state.J[ts][p][dr][4], state.J[ts][p][dr][5]);
                Jp_11 = dcomplex(state.J[ts][p][dr][6], state.J[ts][p][dr][7]);

                Jq_00 = dcomplex(state.J[ts][q][dr][0], state.J[ts][q][dr][1]);
                Jq_01 = dcomplex(state.J[ts][q][dr][2], state.J[ts][q][dr][3]);
                Jq_10 = dcomplex(state.J[ts][q][dr][4], state.J[ts][q][dr][5]);
                Jq_11 = dcomplex(state.J[ts][q][dr][6], state.J[ts][q][dr][7]);

                Jq_00_s0 = Jq_00 * sim[dr][bl][ch][0];
                Jq_10_s0 = Jq_10 * sim[dr][bl][ch][0];
                Jq_01_s1 = Jq_01 * sim[dr][bl][ch][1];
                Jq_11_s1 = Jq_11 * sim[dr][bl][ch][1];
                Jq_00_s2 = Jq_00 * sim[dr][bl][ch][2];
                Jq_10_s2 = Jq_10 * sim[dr][bl][ch][2];
                Jq_01_s3 = Jq_01 * sim[dr][bl][ch][3];
                Jq_11_s3 = Jq_11 * sim[dr][bl][ch][3];

                sim_00 += (Jp_00 * (Jq_00_s0 + Jq_01_s1)
                    + Jp_01 * (Jq_00_s2 + Jq_01_s3))
                    * coeffSub(casa::IPosition(5, nTargets - 1, dr, 0, ch, bl));

                sim_01 += (Jp_00 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_01 * (Jq_10_s2 + Jq_11_s3))
                    * coeffSub(casa::IPosition(5, nTargets - 1, dr, 1, ch, bl));

                sim_10 += (Jp_10 * (Jq_00_s0 + Jq_01_s1)
                    + Jp_11 * (Jq_00_s2 + Jq_01_s3))
                    * coeffSub(casa::IPosition(5, nTargets - 1, dr, 2, ch, bl));

                sim_11 += (Jp_10 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_11 * (Jq_10_s2 + Jq_11_s3))
                    * coeffSub(casa::IPosition(5, nTargets - 1, dr, 3, ch, bl));
            } // directions

            tgData(0, ch, bl) -= sim_00;
            tgData(1, ch, bl) -= sim_01;
            tgData(2, ch, bl) -= sim_10;
            tgData(3, ch, bl) -= sim_11;
        } // channels
    } // baselines

#ifdef ESTIMATE_TIMER
    state.tSub[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].stop();
#endif
}

void __init_source_list(const string &fname)
{
    ifstream inf(fname.c_str());
    LOG_DEBUG_STR("Loading source models from: " << fname);

    if(!(inf >> __NDIR))
    {
        ASSERT(false);
    }

    LOG_DEBUG_STR("NDIR: " << __NDIR);
    __patches.resize(__NDIR);
    __lmn.resize(__NDIR);

    for(size_t i = 0; i < __NDIR; ++i)
    {
        size_t nSources;

        if(!(inf >> nSources))
        {
            ASSERT(false);
        }
        LOG_DEBUG_STR("DIR: " << i << " NSOURCES: " << nSources);

        __patches[i].reserve(nSources);
        double ra, dec, I, Q, U, V;
        for(size_t j = 0; j < nSources; ++j)
        {
            if(!(inf >> ra >> dec >> I >> Q >> U >> V))
            {
                ASSERT(false);
            }

            __patches[i].push_back(source(ra, dec, I, Q, U, V));
        }
        LOG_DEBUG_STR("DIR: " << i << " LAST: " << ra << " " << dec << " " << I << " " << Q << " " << U << " " << V);
    }
}

void __init_lmn(unsigned int dir, double pra, double pdec)
{
    const vector<source> &sources = __patches[dir];
    const size_t nSources = sources.size();

    vector<vec3> &lmn = __lmn[dir];
    lmn.resize(nSources);

    for(size_t j = 0; j < nSources; ++j)
    {
        const double ra = sources[j].position[0];
        const double dec = sources[j].position[1];
        const double dra = ra - pra;
        const double cdec = cos(dec);
        const double l = cdec * sin(dra);
        const double m = sin(dec) * cos(pdec) - cdec * sin(pdec) * cos(dra);

        lmn[j][0] = l;
        lmn[j][1] = m;
        lmn[j][2] = sqrt(1.0 - l * l - m * m);
    }
}

/*
void estimateImpl3(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts)
{
    const size_t threadID = OpenMP::threadNum();

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].start();
#endif

    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __NDIR;
    const size_t nTargets = buffers.size();
    const size_t nSt = state.nStat;

#ifdef ESTIMATE_TIMER
    state.tSim[threadID].start();
#endif

    boost::multi_array<double, 2> uvw_split(boost::extents[nSt][3]);
    boost::multi_array<bool, 1> flag(boost::extents[nSt]);
    boost::multi_array<dcomplex, 1> shift(boost::extents[nSt]);

    // Simulate visibilities for each direction.
    fill(state.sim.data() + threadID * 2 * nBl * 4,
        state.sim.data() + (threadID + 1) * 2 * nBl * 4,
        dcomplex(0.0, 0.0));

    const double _2pi_lambda = casa::C::_2pi * state.freq / casa::C::c;
    for(size_t i = 0; i < nModels; ++i)
    {
        // Split UVW.
        const casa::Matrix<double> &uvw = buffers[i].getUVW();

        size_t found = 1;
        fill(flag.data(), flag.data() + flag.num_elements(), false);
        uvw_split[0][0] = 0.0;
        uvw_split[0][1] = 0.0;
        uvw_split[0][2] = 0.0;
        flag[0] = true;

        for(size_t j = 0; j < nBl; ++j)
        {
            const size_t p = state.baselines[j].first;
            const size_t q = state.baselines[j].second;

            if(p == q || flag[p] == flag[q])
            {
                continue;
            }

            if(flag[p])
            {
                uvw_split[q][0] = uvw(0, j) + uvw_split[p][0];
                uvw_split[q][1] = uvw(1, j) + uvw_split[p][1];
                uvw_split[q][2] = uvw(2, j) + uvw_split[p][2];
                flag[q] = true;
            }
            else
            {
                uvw_split[p][0] = -uvw(0, j) + uvw_split[q][0];
                uvw_split[p][1] = -uvw(1, j) + uvw_split[q][1];
                uvw_split[p][2] = -uvw(2, j) + uvw_split[q][2];
                flag[p] = true;
            }
            ++found;
        }
        ASSERTSTR(found == nSt, "Could not split UVW, found: " << found);

        const vector<source> &sources = __patches[i];
        const size_t nSources = sources.size();
//        LOG_DEBUG_STR("dir: " << i << " #sources: " << nSources);

        const vector<vec3> &lmn = __lmn[i];

        for(size_t k = 0; k < nSources; ++k)
        {
            const dcomplex XX = dcomplex(sources[k].stokes[0] + sources[k].stokes[1], 0.0);
            const dcomplex XY = dcomplex(sources[k].stokes[2], sources[k].stokes[3]);
            const dcomplex YX = dcomplex(sources[k].stokes[2], -sources[k].stokes[3]);
            const dcomplex YY = dcomplex(sources[k].stokes[0] - sources[k].stokes[1], 0.0);

            for(size_t j = 0; j < nSt; ++j)
            {
                const double phase = (uvw_split[j][0] * lmn[k][0] + uvw_split[j][1] * lmn[k][1]
                    + uvw_split[j][2] * (lmn[k][2] - 1.0)) * _2pi_lambda;
                shift[j] = dcomplex(cos(phase), sin(phase));
            }

            for(size_t j = 0; j < nBl; ++j)
            {
                const size_t p = state.baselines[j].first;
                const size_t q = state.baselines[j].second;

                if(p == q)
                {
                    continue;
                }

                const dcomplex blShift = shift[q] * conj(shift[p]);
                state.sim[threadID][i][j][0] += blShift * XX;
                state.sim[threadID][i][j][1] += blShift * XY;
                state.sim[threadID][i][j][2] += blShift * YX;
                state.sim[threadID][i][j][3] += blShift * YY;
            }
        }
    }

#ifdef ESTIMATE_TIMER
    state.tSim[threadID].stop();
#endif

    // Estimate parameters.
    const size_t nCoeff = nModels * state.nStat * 4 * 2;
    casa::uInt rank;
    casa::LSQFit solver(static_cast<casa::uInt>(nCoeff));
    configLSQSolver(solver, state.lsqOptions);

    // Only two Jones matrix elements per station per direction are used per
    // correlation.
    const unsigned int nDerivative = nModels * 16 / 2;
    boost::multi_array<dcomplex, 2> M(boost::extents[nModels][4]);
    boost::multi_array<dcomplex, 3> dM(boost::extents[nModels][4][4]);
    boost::multi_array<double, 1> dR(boost::extents[nDerivative]);
    boost::multi_array<double, 1> dI(boost::extents[nDerivative]);
    boost::multi_array<unsigned int, 1> dIndex(boost::extents[nDerivative]);

    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jp_00_s0, Jp_10_s0, Jp_00_s1, Jp_10_s1, Jp_01_s2, Jp_11_s2, Jp_01_s3, Jp_11_s3;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2, Jq_01_s3, Jq_11_s3;

    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
        state.tEq[threadID].start();
#endif

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = state.baselines[bl].first;
            const size_t q = state.baselines[bl].second;

            if(p == q)
            {
                continue;
            }

            for(size_t dr = 0; dr < nModels; ++dr)
            {
                Jp_00 = dcomplex(state.J[ts][p][dr][0], state.J[ts][p][dr][1]);
                Jp_01 = dcomplex(state.J[ts][p][dr][2], state.J[ts][p][dr][3]);
                Jp_10 = dcomplex(state.J[ts][p][dr][4], state.J[ts][p][dr][5]);
                Jp_11 = dcomplex(state.J[ts][p][dr][6], state.J[ts][p][dr][7]);

                Jq_00 = dcomplex(state.J[ts][q][dr][0], -state.J[ts][q][dr][1]);
                Jq_01 = dcomplex(state.J[ts][q][dr][2], -state.J[ts][q][dr][3]);
                Jq_10 = dcomplex(state.J[ts][q][dr][4], -state.J[ts][q][dr][5]);
                Jq_11 = dcomplex(state.J[ts][q][dr][6], -state.J[ts][q][dr][7]);

                Jp_00_s0 = Jp_00 * state.sim[threadID][dr][bl][0];
                Jp_10_s0 = Jp_10 * state.sim[threadID][dr][bl][0];
                Jq_00_s0 = Jq_00 * state.sim[threadID][dr][bl][0];
                Jq_10_s0 = Jq_10 * state.sim[threadID][dr][bl][0];

                Jp_00_s1 = Jp_00 * state.sim[threadID][dr][bl][1];
                Jp_10_s1 = Jp_10 * state.sim[threadID][dr][bl][1];
                Jq_01_s1 = Jq_01 * state.sim[threadID][dr][bl][1];
                Jq_11_s1 = Jq_11 * state.sim[threadID][dr][bl][1];

                Jp_01_s2 = Jp_01 * state.sim[threadID][dr][bl][2];
                Jp_11_s2 = Jp_11 * state.sim[threadID][dr][bl][2];
                Jq_00_s2 = Jq_00 * state.sim[threadID][dr][bl][2];
                Jq_10_s2 = Jq_10 * state.sim[threadID][dr][bl][2];

                Jp_01_s3 = Jp_01 * state.sim[threadID][dr][bl][3];
                Jp_11_s3 = Jp_11 * state.sim[threadID][dr][bl][3];
                Jq_01_s3 = Jq_01 * state.sim[threadID][dr][bl][3];
                Jq_11_s3 = Jq_11 * state.sim[threadID][dr][bl][3];

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
            }

            for(size_t cr = 0; cr < 4; ++cr)
            {
                for(size_t tg = 0; tg < nTargets; ++tg)
                {
                    if(buffers[tg].getFlags()(cr, 0, bl))
                    {
                        continue;
                    }

                    dcomplex V_sim = 0.0;
                    dcomplex tmp;
                    for(size_t dr = 0; dr < nModels; ++dr)
                    {
                        dcomplex weight = coeff(casa::IPosition(5, tg, dr, cr, 0, bl));
                        V_sim += M[dr][cr] * weight;

                        tmp = dM[dr][cr][0] * weight;
                        dR[dr * 8] = real(tmp);
                        dI[dr * 8] = imag(tmp);
                        dR[dr * 8 + 1] = -imag(tmp);
                        dI[dr * 8 + 1] = real(tmp);

                        tmp = dM[dr][cr][1] * weight;
                        dR[dr * 8 + 2] = real(tmp);
                        dI[dr * 8 + 2] = imag(tmp);
                        dR[dr * 8 + 3] = -imag(tmp);
                        dI[dr * 8 + 3] = real(tmp);

                        tmp = dM[dr][cr][2] * weight;
                        dR[dr * 8 + 4] = real(tmp);
                        dI[dr * 8 + 4] = imag(tmp);
                        dR[dr * 8 + 5] = imag(tmp);
                        dI[dr * 8 + 5] = -real(tmp);

                        tmp = dM[dr][cr][3] * weight;
                        dR[dr * 8 + 6] = real(tmp);
                        dI[dr * 8 + 6] = imag(tmp);
                        dR[dr * 8 + 7] = imag(tmp);
                        dI[dr * 8 + 7] = -real(tmp);
                    }

                    const casa::Cube<casa::Complex> &obsData = buffers[tg].getData();
                    const casa::Cube<float> &obsWeight = buffers[tg].getWeights();

                    casa::Complex V_obs = obsData(cr, 0, bl);
                    double reResidual = real(V_obs) - real(V_sim);
                    double imResidual = imag(V_obs) - imag(V_sim);
                    double obsw = obsWeight(cr, 0, bl);

                    // Update the normal equations.
                    solver.makeNorm(nDerivative, &(state.dIndex[bl][cr][0]), &(dR[0]), obsw, reResidual);
                    solver.makeNorm(nDerivative, &(state.dIndex[bl][cr][0]), &(dI[0]), obsw, imResidual);
                } // target directions
            } // correlations
        } // baselines

#ifdef ESTIMATE_TIMER
        state.tEq[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
        state.tLM[threadID].start();
#endif
        // Do solve iteration.
        bool status = solver.solveLoop(rank, &(state.J[ts][0][0][0]), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
        state.tLM[threadID].stop();
#endif

        // Update iteration count.
        ++nIterations;
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);

#ifdef ESTIMATE_TIMER
    state.tSub[threadID].start();
#endif

    // Subtract...
    casa::Cube<casa::Complex> &tgData = target.getData();

    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = state.baselines[bl].first;
        const size_t q = state.baselines[bl].second;

        if(p == q)
        {
            continue;
        }

        for(size_t dr = 0; dr < nModels; ++dr)
        {
            Jp_00 = dcomplex(state.J[ts][p][dr][0], state.J[ts][p][dr][1]);
            Jp_01 = dcomplex(state.J[ts][p][dr][2], state.J[ts][p][dr][3]);
            Jp_10 = dcomplex(state.J[ts][p][dr][4], state.J[ts][p][dr][5]);
            Jp_11 = dcomplex(state.J[ts][p][dr][6], state.J[ts][p][dr][7]);

            Jq_00 = dcomplex(state.J[ts][q][dr][0], state.J[ts][q][dr][1]);
            Jq_01 = dcomplex(state.J[ts][q][dr][2], state.J[ts][q][dr][3]);
            Jq_10 = dcomplex(state.J[ts][q][dr][4], state.J[ts][q][dr][5]);
            Jq_11 = dcomplex(state.J[ts][q][dr][6], state.J[ts][q][dr][7]);

            Jq_00_s0 = Jq_00 * state.sim[threadID][dr][bl][0];
            Jq_10_s0 = Jq_10 * state.sim[threadID][dr][bl][0];
            Jq_01_s1 = Jq_01 * state.sim[threadID][dr][bl][1];
            Jq_11_s1 = Jq_11 * state.sim[threadID][dr][bl][1];
            Jq_00_s2 = Jq_00 * state.sim[threadID][dr][bl][2];
            Jq_10_s2 = Jq_10 * state.sim[threadID][dr][bl][2];
            Jq_01_s3 = Jq_01 * state.sim[threadID][dr][bl][3];
            Jq_11_s3 = Jq_11 * state.sim[threadID][dr][bl][3];

            M[dr][0] = Jp_00 * (Jq_00_s0 + Jq_01_s1)
                + Jp_01 * (Jq_00_s2 + Jq_01_s3);

            M[dr][1] = Jp_00 * (Jq_10_s0 + Jq_11_s1)
                + Jp_01 * (Jq_10_s2 + Jq_11_s3);

            M[dr][2] = Jp_10 * (Jq_00_s0 + Jq_01_s1)
                + Jp_11 * (Jq_00_s2 + Jq_01_s3);

            M[dr][3] = Jp_10 * (Jq_10_s0 + Jq_11_s1)
                + Jp_11 * (Jq_10_s2 + Jq_11_s3);
        }

        for(size_t cr = 0; cr < 4; ++cr)
        {
            dcomplex V_sim = 0.0;
            for(size_t dr = 0; dr < nModels; ++dr)
            {
                V_sim += M[dr][cr] * coeffSub(casa::IPosition(5, nTargets - 1, dr, cr, 0, bl));
            } // model directions

            tgData(cr, 0, bl) -= V_sim;
        } // correlations
    } // baselines

#ifdef ESTIMATE_TIMER
    state.tSub[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].stop();
#endif
}
*/
} //# namespace DPPP
} //# namespace LOFAR
