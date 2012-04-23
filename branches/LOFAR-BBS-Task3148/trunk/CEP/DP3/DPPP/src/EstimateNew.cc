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

static const size_t __nDir = 2;
static vector<source> __patches[__nDir];
static vector<vec3> __lmn[__nDir];

void propagate(EstimateState &state, size_t from, size_t to)
{
    size_t nSol = state.nStat * __nDir * 8;
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


void estimateImpl(DPBuffer &target, const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state, size_t ts);

void estimateImpl2(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts);

void estimateImpl3(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts);

void estimate(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &buffers,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffSub,
    EstimateState &state,
    size_t ts)
{
    LOG_DEBUG_STR("buffers #dir: " << buffers.size() << " #time: "
        << buffers[0].size());
    LOG_DEBUG_STR("coeff #time: " << coeff.size() << " shape: "
        << coeff[0].shape());
//    LOG_DEBUG_STR("grid shape #freq: " << visGrid[0]->size() << " #time:"
//        << visGrid[1]->size());

    ASSERT(buffers.size() == __nDir + 1);
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
        estimateImpl3(target[i], buffersT[i], coeff[i], coeffSub[i], state, ts + i);

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
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __nDir;
    const size_t nTargets = buffers.size();
    const size_t threadID = OpenMP::threadNum();

    // Simulate visibilities for each direction.
    const double _2pi_lambda = casa::C::_2pi * state.freq / casa::C::c;
    for(size_t i = 0; i < nModels; ++i)
    {
        const casa::Matrix<double> &uvw = buffers[i].getUVW();

        const vector<source> &sources = __patches[i];
        const size_t nSources = sources.size();
//        LOG_DEBUG_STR("dir: " << i << " #sources: " << nSources);

        const vector<vec3> &lmn = __lmn[i];

        for(size_t j = 0; j < nBl; ++j)
        {
            if(state.baselines[j].first == state.baselines[j].second)
            {
                continue;
            }

            const double &u = uvw(0, j);
            const double &v = uvw(1, j);
            const double &w = uvw(2, j);

            dcomplex &PP = state.sim[threadID][i][j][0];
            dcomplex &QQ = state.sim[threadID][i][j][1];

            PP = 0.0;
            QQ = 0.0;

            for(size_t k = 0; k < nSources; ++k)
            {
                const double &I = sources[k].stokes[0];
                const double phase = (u * lmn[k][0] + v * lmn[k][1]
                    + w * (lmn[k][2] - 1.0)) * _2pi_lambda;
                const dcomplex XXYY(I * cos(phase), I * sin(phase));

                PP += XXYY;
                QQ += XXYY;
            }
        }
    }

    for(size_t i = 0; i < nModels; ++i)
    {
        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 1 values: " << setprecision(17) << state.sim[threadID][i][1][0] << " " << state.sim[threadID][i][1][1]);
        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 10 values: " << setprecision(17) << state.sim[threadID][i][10][0] << " " << state.sim[threadID][i][10][1]);
        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 777 values: " << setprecision(17) << state.sim[threadID][i][777][0] << " " << state.sim[threadID][i][777][1]);
    }

    ASSERT(false);

    // Estimate parameters.
    const size_t nCoeff = nModels * state.nStat * 4 * 2;
    casa::uInt rank;
    casa::LSQFit solver(static_cast<casa::uInt>(nCoeff));
    configLSQSolver(solver, state.lsqOptions);

    // Only two Jones matrix elements per station per direction are used per
    // correlation, so divide by two here. (This is because all sources are
    // unpolarized, so XY = YX = 0 for the model.)
    const unsigned int nDerivative = nModels * 16 / 2;
    boost::multi_array<dcomplex, 1> model(boost::extents[nModels]);
    boost::multi_array<dcomplex, 2> partial(boost::extents[nModels][4]);
    vector<double> dR(nDerivative, 0);
    vector<double> dI(nDerivative, 0);
//    boost::multi_array<double, 1> dR(boost::extents[nDerivative]);
//    boost::multi_array<double, 1> dI(boost::extents[nDerivative]);
    boost::multi_array<unsigned int, 1> dIndex(boost::extents[nDerivative]);

    const unsigned int theCR = 5;

    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = state.baselines[bl].first;
            const size_t q = state.baselines[bl].second;

            if(p == q)
            {
                continue;
            }

            if(theCR < 4){ LOG_DEBUG_STR("baseline: " << p << " - " << q); }

            // loop over correlations
            for(size_t cr = 0; cr < 4; ++cr)
            {
//                if(cr == 0)
//                {
//                    for(size_t dr = 0; dr < nModels; ++dr)
//                    {
//                        LOG_DEBUG_STR("SIM NO GAIN dir: " << dr << " value: " << state.sim[dr][bl][0] << " " << state.sim[dr][bl][1]);
//                    }
//                }

                const size_t elp = (cr / 2) * 4;
                const size_t elq = (cr & 1) * 4;

                dcomplex Jp, Jq;
                for(size_t dr = 0; dr < nModels; ++dr)
                {
                    Jp = dcomplex(state.J[ts][p][dr][elp], state.J[ts][p][dr][elp + 1]);
                    Jq = dcomplex(state.J[ts][q][dr][elq], -state.J[ts][q][dr][elq + 1]);
                    if(cr == theCR){ LOG_DEBUG_STR("dir: " << dr << " Jpxx: " << Jp << " Jqxx: " << Jq); }

                    model[dr] = Jp * Jq * state.sim[threadID][dr][bl][0];
                    partial[dr][0] = Jq * state.sim[threadID][dr][bl][0];
                    partial[dr][1] = Jp * state.sim[threadID][dr][bl][0];

                    Jp = dcomplex(state.J[ts][p][dr][elp + 2], state.J[ts][p][dr][elp + 3]);
                    Jq = dcomplex(state.J[ts][q][dr][elq + 2], -state.J[ts][q][dr][elq + 3]);
                    if(cr == theCR){ LOG_DEBUG_STR("dir: " << dr << " Jpxy: " << Jp << " Jqxy: " << Jq); }

                    model[dr] += Jp * Jq * state.sim[threadID][dr][bl][1];
                    partial[dr][2] = Jq * state.sim[threadID][dr][bl][1];
                    partial[dr][3] = Jp * state.sim[threadID][dr][bl][1];

                    // construct coefficient index.
                    dIndex[dr * 8] = p * nModels * 8 + dr * 8 + elp;
                    dIndex[dr * 8 + 1] = p * nModels * 8 + dr * 8 + elp + 1;
                    dIndex[dr * 8 + 2] = q * nModels * 8 + dr * 8 + elq;
                    dIndex[dr * 8 + 3] = q * nModels * 8 + dr * 8 + elq + 1;
                    dIndex[dr * 8 + 4] = p * nModels * 8 + dr * 8 + elp + 2;
                    dIndex[dr * 8 + 5] = p * nModels * 8 + dr * 8 + elp + 3;
                    dIndex[dr * 8 + 6] = q * nModels * 8 + dr * 8 + elq + 2;
                    dIndex[dr * 8 + 7] = q * nModels * 8 + dr * 8 + elq + 3;
                }

//                if(cr == 0)
//                {
//                    for(size_t dr = 0; dr < nModels; ++dr)
//                    {
//                        LOG_DEBUG_STR("SIM GAIN dir: " << dr << " value: " << model[dr]);
//                        vector<double> tmpdR(16, 0);
//                        vector<double> tmpdI(16, 0);


//                    }

//                }

                for(size_t tg = 0; tg < nTargets; ++tg)
                {
                    dcomplex V = 0.0;
                    dcomplex tmp;
                    for(size_t dr = 0; dr < nModels; ++dr)
                    {
                        dcomplex weight = coeff(casa::IPosition(5, tg, dr, cr, 0, bl));
                        V += model[dr] * weight;

                        tmp = partial[dr][0] * weight;
                        dR[dr * 8] = real(tmp);
                        dI[dr * 8] = imag(tmp);
                        dR[dr * 8 + 1] = -imag(tmp);
                        dI[dr * 8 + 1] = real(tmp);

                        tmp = partial[dr][1] * weight;
                        dR[dr * 8 + 2] = real(tmp);
                        dI[dr * 8 + 2] = imag(tmp);
                        dR[dr * 8 + 3] = imag(tmp);
                        dI[dr * 8 + 3] = -real(tmp);

                        tmp = partial[dr][2] * weight;
                        dR[dr * 8 + 4] = real(tmp);
                        dI[dr * 8 + 4] = imag(tmp);
                        dR[dr * 8 + 5] = -imag(tmp);
                        dI[dr * 8 + 5] = real(tmp);

                        tmp = partial[dr][3] * weight;
                        dR[dr * 8 + 6] = real(tmp);
                        dI[dr * 8 + 6] = imag(tmp);
                        dR[dr * 8 + 7] = imag(tmp);
                        dI[dr * 8 + 7] = -real(tmp);
                    }

                    if(cr == theCR)
                    {
                        vector<double> tmpdR(nDerivative * 2, 0);
                        vector<double> tmpdI(nDerivative * 2, 0);

                        for(size_t dr = 0; dr < nModels; ++dr)
                        {
                            tmpdR[dr * 16 + 0] = dR[dr * 8 + 0];
                            tmpdR[dr * 16 + 1] = dR[dr * 8 + 1];
                            tmpdR[dr * 16 + 2] = dR[dr * 8 + 4];
                            tmpdR[dr * 16 + 3] = dR[dr * 8 + 5];

                            tmpdR[dr * 16 + 8] = dR[dr * 8 + 2];
                            tmpdR[dr * 16 + 9] = dR[dr * 8 + 3];
                            tmpdR[dr * 16 + 10] = dR[dr * 8 + 6];
                            tmpdR[dr * 16 + 11] = dR[dr * 8 + 7];

                            tmpdI[dr * 16 + 0] = dI[dr * 8 + 0];
                            tmpdI[dr * 16 + 1] = dI[dr * 8 + 1];
                            tmpdI[dr * 16 + 2] = dI[dr * 8 + 4];
                            tmpdI[dr * 16 + 3] = dI[dr * 8 + 5];

                            tmpdI[dr * 16 + 8] = dI[dr * 8 + 2];
                            tmpdI[dr * 16 + 9] = dI[dr * 8 + 3];
                            tmpdI[dr * 16 + 10] = dI[dr * 8 + 6];
                            tmpdI[dr * 16 + 11] = dI[dr * 8 + 7];
                        }

                        LOG_DEBUG_STR("SIM MIXED target: " << tg << " mixed: " << V);
                        LOG_DEBUG_STR("PARTIALS  target: " << tg << endl << "dR: " << tmpdR << endl << "dI: " << tmpdI);
                    }

                    const casa::Cube<casa::Complex> &obsData = buffers[tg].getData();
                    const casa::Cube<float> &obsWeight = buffers[tg].getWeights();

                    casa::Complex Vobs = obsData(cr, 0, bl);
                    double reResidual = real(Vobs) - real(V);
                    double imResidual = imag(Vobs) - imag(V);
                    double obsw = obsWeight(cr, 0, bl);

                    // Update the normal equations.
                    solver.makeNorm(nDerivative, &(dIndex[0]), &(dR[0]), obsw, reResidual);
                    solver.makeNorm(nDerivative, &(dIndex[0]), &(dI[0]), obsw, imResidual);
//                    solver.makeNorm(&(dR[0]), obsw, reResidual);
//                    solver.makeNorm(&(dI[0]), obsw, imResidual);
                } // target directions
            } // correlations
        } // baselines

        // Do solve iteration.
        bool status = solver.solveLoop(rank, &(state.J[ts][0][0][0]), true);
        ASSERT(status);

        // Update iteration count.
        ++nIterations;
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);


    // Subtract...
    casa::Cube<casa::Complex> &obsData = target.getData();

    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = state.baselines[bl].first;
        const size_t q = state.baselines[bl].second;

        if(p == q)
        {
            continue;
        }

        // loop over correlations
        for(size_t cr = 0; cr < 4; ++cr)
        {
            const size_t elp = (cr / 2) * 4;
            const size_t elq = (cr & 1) * 4;

            dcomplex Jp, Jq, V = 0.0, tmp;
            for(size_t dr = 0; dr < nModels; ++dr)
            {
                Jp = dcomplex(state.J[ts][p][dr][elp], state.J[ts][p][dr][elp + 1]);
                Jq = dcomplex(state.J[ts][q][dr][elq], -state.J[ts][q][dr][elq + 1]);
                tmp = Jp * Jq * state.sim[threadID][dr][bl][0];

                Jp = dcomplex(state.J[ts][p][dr][elp + 2], state.J[ts][p][dr][elp + 3]);
                Jq = dcomplex(state.J[ts][q][dr][elq + 2], -state.J[ts][q][dr][elq + 3]);
                tmp += Jp * Jq * state.sim[threadID][dr][bl][1];

                // index: target, dir, cr, ch, bl
                V += tmp * coeffSub(casa::IPosition(5, nTargets - 1, dr, cr, 0, bl));
            } // model directions

            obsData(cr, 0, bl) -= V;
        } // correlations
    } // baselines
}

void estimateImpl2(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts)
{
#ifdef ESTIMATE_TIMER
    state.tTot.start();
#endif

    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __nDir;
    const size_t nTargets = buffers.size();
    const size_t threadID = OpenMP::threadNum();
    const size_t nSt = state.nStat;

#ifdef ESTIMATE_TIMER
    state.tSim.start();
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

        size_t found = 1, last = 0;
        fill(flag.data(), flag.data() + flag.num_elements(), false);
        uvw_split[0][0] = 0.0;
        uvw_split[0][1] = 0.0;
        uvw_split[0][2] = 0.0;
        flag[0] = true;

        while(found < nSt && found > last)
        {
            last = found;
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
//            LOG_DEBUG_STR("last: " << last << " found: " << found);
        }
        ASSERTSTR(found == nSt, "Could not split UVW, found: " << found);

        const vector<source> &sources = __patches[i];
        const size_t nSources = sources.size();
//        LOG_DEBUG_STR("dir: " << i << " #sources: " << nSources);

        const vector<vec3> &lmn = __lmn[i];

        for(size_t k = 0; k < nSources; ++k)
        {
            const double &I = sources[k].stokes[0];

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

//                const double phase2 = (uvw(0, j) * lmn[k][0] + uvw(1, j) * lmn[k][1]
//                    + uvw(2, j) * (lmn[k][2] - 1.0)) * _2pi_lambda;
//                const dcomplex blShift2(cos(phase2), sin(phase2));
//                ASSERTSTR(casa::near(blShift, blShift2), "phase mismatch: " << setprecision(17) << blShift << " " << blShift2);

                const dcomplex XXYY = I * blShift;
                state.sim[threadID][i][j][0] += XXYY;
                state.sim[threadID][i][j][1] += XXYY;
            }
        }
    }

#ifdef ESTIMATE_TIMER
    state.tSim.stop();
#endif

//    for(size_t i = 0; i < nModels; ++i)
//    {
//        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 1 values: " << setprecision(17) << state.sim[threadID][i][1][0] << " " << state.sim[threadID][i][1][1]);
//        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 10 values: " << setprecision(17) << state.sim[threadID][i][10][0] << " " << state.sim[threadID][i][10][1]);
//        LOG_DEBUG_STR("thread: " << threadID << " model: " << i << " bl: 777 values: " << setprecision(17) << state.sim[threadID][i][777][0] << " " << state.sim[threadID][i][777][1]);
//    }
//
//    ASSERT(false);

    // Estimate parameters.
    const size_t nCoeff = nModels * state.nStat * 4 * 2;
    casa::uInt rank;
    casa::LSQFit solver(static_cast<casa::uInt>(nCoeff));
    configLSQSolver(solver, state.lsqOptions);

    // Only two Jones matrix elements per station per direction are used per
    // correlation, so divide by two here. (This is because all sources are
    // unpolarized, so XY = YX = 0 for the model.)
    const unsigned int nDerivative = nModels * 16 / 2;
    boost::multi_array<dcomplex, 1> model(boost::extents[nModels]);
    boost::multi_array<dcomplex, 2> partial(boost::extents[nModels][4]);
    vector<double> dR(nDerivative, 0);
    vector<double> dI(nDerivative, 0);
//    boost::multi_array<double, 1> dR(boost::extents[nDerivative]);
//    boost::multi_array<double, 1> dI(boost::extents[nDerivative]);
    boost::multi_array<unsigned int, 1> dIndex(boost::extents[nDerivative]);

    const unsigned int theCR = 5;

    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
        state.tEq.start();
#endif

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = state.baselines[bl].first;
            const size_t q = state.baselines[bl].second;

            if(p == q)
            {
                continue;
            }

            if(theCR < 4){ LOG_DEBUG_STR("baseline: " << p << " - " << q); }

            // loop over correlations
            for(size_t cr = 0; cr < 4; ++cr)
            {
//                if(cr == 0)
//                {
//                    for(size_t dr = 0; dr < nModels; ++dr)
//                    {
//                        LOG_DEBUG_STR("SIM NO GAIN dir: " << dr << " value: " << state.sim[dr][bl][0] << " " << state.sim[dr][bl][1]);
//                    }
//                }

                const size_t elp = (cr / 2) * 4;
                const size_t elq = (cr & 1) * 4;

                dcomplex Jp, Jq;
                for(size_t dr = 0; dr < nModels; ++dr)
                {
                    Jp = dcomplex(state.J[ts][p][dr][elp], state.J[ts][p][dr][elp + 1]);
                    Jq = dcomplex(state.J[ts][q][dr][elq], -state.J[ts][q][dr][elq + 1]);
                    if(cr == theCR){ LOG_DEBUG_STR("dir: " << dr << " Jpxx: " << Jp << " Jqxx: " << Jq); }

                    model[dr] = Jp * Jq * state.sim[threadID][dr][bl][0];
                    partial[dr][0] = Jq * state.sim[threadID][dr][bl][0];
                    partial[dr][1] = Jp * state.sim[threadID][dr][bl][0];

                    Jp = dcomplex(state.J[ts][p][dr][elp + 2], state.J[ts][p][dr][elp + 3]);
                    Jq = dcomplex(state.J[ts][q][dr][elq + 2], -state.J[ts][q][dr][elq + 3]);
                    if(cr == theCR){ LOG_DEBUG_STR("dir: " << dr << " Jpxy: " << Jp << " Jqxy: " << Jq); }

                    model[dr] += Jp * Jq * state.sim[threadID][dr][bl][1];
                    partial[dr][2] = Jq * state.sim[threadID][dr][bl][1];
                    partial[dr][3] = Jp * state.sim[threadID][dr][bl][1];

                    // construct coefficient index.
                    dIndex[dr * 8] = p * nModels * 8 + dr * 8 + elp;
                    dIndex[dr * 8 + 1] = p * nModels * 8 + dr * 8 + elp + 1;
                    dIndex[dr * 8 + 2] = q * nModels * 8 + dr * 8 + elq;
                    dIndex[dr * 8 + 3] = q * nModels * 8 + dr * 8 + elq + 1;
                    dIndex[dr * 8 + 4] = p * nModels * 8 + dr * 8 + elp + 2;
                    dIndex[dr * 8 + 5] = p * nModels * 8 + dr * 8 + elp + 3;
                    dIndex[dr * 8 + 6] = q * nModels * 8 + dr * 8 + elq + 2;
                    dIndex[dr * 8 + 7] = q * nModels * 8 + dr * 8 + elq + 3;
                }

//                if(cr == 0)
//                {
//                    for(size_t dr = 0; dr < nModels; ++dr)
//                    {
//                        LOG_DEBUG_STR("SIM GAIN dir: " << dr << " value: " << model[dr]);
//                        vector<double> tmpdR(16, 0);
//                        vector<double> tmpdI(16, 0);


//                    }

//                }

                for(size_t tg = 0; tg < nTargets; ++tg)
                {
                    dcomplex V = 0.0;
                    dcomplex tmp;
                    for(size_t dr = 0; dr < nModels; ++dr)
                    {
                        dcomplex weight = coeff(casa::IPosition(5, tg, dr, cr, 0, bl));
                        V += model[dr] * weight;

                        tmp = partial[dr][0] * weight;
                        dR[dr * 8] = real(tmp);
                        dI[dr * 8] = imag(tmp);
                        dR[dr * 8 + 1] = -imag(tmp);
                        dI[dr * 8 + 1] = real(tmp);

                        tmp = partial[dr][1] * weight;
                        dR[dr * 8 + 2] = real(tmp);
                        dI[dr * 8 + 2] = imag(tmp);
                        dR[dr * 8 + 3] = imag(tmp);
                        dI[dr * 8 + 3] = -real(tmp);

                        tmp = partial[dr][2] * weight;
                        dR[dr * 8 + 4] = real(tmp);
                        dI[dr * 8 + 4] = imag(tmp);
                        dR[dr * 8 + 5] = -imag(tmp);
                        dI[dr * 8 + 5] = real(tmp);

                        tmp = partial[dr][3] * weight;
                        dR[dr * 8 + 6] = real(tmp);
                        dI[dr * 8 + 6] = imag(tmp);
                        dR[dr * 8 + 7] = imag(tmp);
                        dI[dr * 8 + 7] = -real(tmp);
                    }

                    if(cr == theCR)
                    {
                        vector<double> tmpdR(nDerivative * 2, 0);
                        vector<double> tmpdI(nDerivative * 2, 0);

                        for(size_t dr = 0; dr < nModels; ++dr)
                        {
                            tmpdR[dr * 16 + 0] = dR[dr * 8 + 0];
                            tmpdR[dr * 16 + 1] = dR[dr * 8 + 1];
                            tmpdR[dr * 16 + 2] = dR[dr * 8 + 4];
                            tmpdR[dr * 16 + 3] = dR[dr * 8 + 5];

                            tmpdR[dr * 16 + 8] = dR[dr * 8 + 2];
                            tmpdR[dr * 16 + 9] = dR[dr * 8 + 3];
                            tmpdR[dr * 16 + 10] = dR[dr * 8 + 6];
                            tmpdR[dr * 16 + 11] = dR[dr * 8 + 7];

                            tmpdI[dr * 16 + 0] = dI[dr * 8 + 0];
                            tmpdI[dr * 16 + 1] = dI[dr * 8 + 1];
                            tmpdI[dr * 16 + 2] = dI[dr * 8 + 4];
                            tmpdI[dr * 16 + 3] = dI[dr * 8 + 5];

                            tmpdI[dr * 16 + 8] = dI[dr * 8 + 2];
                            tmpdI[dr * 16 + 9] = dI[dr * 8 + 3];
                            tmpdI[dr * 16 + 10] = dI[dr * 8 + 6];
                            tmpdI[dr * 16 + 11] = dI[dr * 8 + 7];
                        }

                        LOG_DEBUG_STR("SIM MIXED target: " << tg << " mixed: " << V);
                        LOG_DEBUG_STR("PARTIALS  target: " << tg << endl << "dR: " << tmpdR << endl << "dI: " << tmpdI);
                    }

                    const casa::Cube<casa::Complex> &obsData = buffers[tg].getData();
                    const casa::Cube<float> &obsWeight = buffers[tg].getWeights();

                    casa::Complex Vobs = obsData(cr, 0, bl);
                    double reResidual = real(Vobs) - real(V);
                    double imResidual = imag(Vobs) - imag(V);
                    double obsw = obsWeight(cr, 0, bl);

                    // Update the normal equations.
                    solver.makeNorm(nDerivative, &(dIndex[0]), &(dR[0]), obsw, reResidual);
                    solver.makeNorm(nDerivative, &(dIndex[0]), &(dI[0]), obsw, imResidual);
//                    solver.makeNorm(&(dR[0]), obsw, reResidual);
//                    solver.makeNorm(&(dI[0]), obsw, imResidual);
                } // target directions
            } // correlations
        } // baselines

#ifdef ESTIMATE_TIMER
        state.tEq.stop();
#endif

#ifdef ESTIMATE_TIMER
        state.tLM.start();
#endif

        // Do solve iteration.
        bool status = solver.solveLoop(rank, &(state.J[ts][0][0][0]), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
        state.tLM.stop();
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
    state.tSub.start();
#endif

    // Subtract...
    casa::Cube<casa::Complex> &obsData = target.getData();

    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = state.baselines[bl].first;
        const size_t q = state.baselines[bl].second;

        if(p == q)
        {
            continue;
        }

        // loop over correlations
        for(size_t cr = 0; cr < 4; ++cr)
        {
            const size_t elp = (cr / 2) * 4;
            const size_t elq = (cr & 1) * 4;

            dcomplex Jp, Jq, V = 0.0, tmp;
            for(size_t dr = 0; dr < nModels; ++dr)
            {
                Jp = dcomplex(state.J[ts][p][dr][elp], state.J[ts][p][dr][elp + 1]);
                Jq = dcomplex(state.J[ts][q][dr][elq], -state.J[ts][q][dr][elq + 1]);
                tmp = Jp * Jq * state.sim[threadID][dr][bl][0];

                Jp = dcomplex(state.J[ts][p][dr][elp + 2], state.J[ts][p][dr][elp + 3]);
                Jq = dcomplex(state.J[ts][q][dr][elq + 2], -state.J[ts][q][dr][elq + 3]);
                tmp += Jp * Jq * state.sim[threadID][dr][bl][1];

                // index: target, dir, cr, ch, bl
                V += tmp * coeffSub(casa::IPosition(5, nTargets - 1, dr, cr, 0, bl));
            } // model directions

            obsData(cr, 0, bl) -= V;
        } // correlations
    } // baselines

#ifdef ESTIMATE_TIMER
    state.tSub.stop();
#endif

#ifdef ESTIMATE_TIMER
    state.tTot.stop();
#endif
}

void estimateImpl3(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts)
{
#ifdef ESTIMATE_TIMER
    state.tTot.start();
#endif

    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __nDir;
    const size_t nTargets = buffers.size();
    const size_t threadID = OpenMP::threadNum();
    const size_t nSt = state.nStat;

#ifdef ESTIMATE_TIMER
    state.tSim.start();
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
    state.tSim.stop();
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
        state.tEq.start();
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
        state.tEq.stop();
#endif

#ifdef ESTIMATE_TIMER
        state.tLM.start();
#endif
        // Do solve iteration.
        bool status = solver.solveLoop(rank, &(state.J[ts][0][0][0]), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
        state.tLM.stop();
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
    state.tSub.start();
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
    state.tSub.stop();
#endif

#ifdef ESTIMATE_TIMER
    state.tTot.stop();
#endif
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
void __init_source_list()
{
    __patches[0].push_back(source(6.122555, 1.026502, 43.210000));
    __patches[0].push_back(source(6.122555, 1.026502, 38.890000));
    __patches[0].push_back(source(6.123351, 1.026963, 38.460000));
    __patches[0].push_back(source(6.122742, 1.026478, 36.980000));
    __patches[0].push_back(source(6.123023, 1.026745, 36.050000));
    __patches[1].push_back(source(5.233814, 0.710917, 612.300000));
    __patches[1].push_back(source(5.233814, 0.710917, 551.100000));
    __patches[1].push_back(source(5.233814, 0.710917, 496.000000));
    __patches[1].push_back(source(5.233270, 0.711062, 467.400000));
    __patches[1].push_back(source(5.233814, 0.710917, 444.600000));
}
*/


void __init_source_list()
{
    __patches[0].push_back(source(6.122555, 1.026502, 43.210000));
    __patches[0].push_back(source(6.122555, 1.026502, 38.890000));
    __patches[0].push_back(source(6.123351, 1.026963, 38.460000));
    __patches[0].push_back(source(6.122742, 1.026478, 36.980000));
    __patches[0].push_back(source(6.123023, 1.026745, 36.050000));
    __patches[0].push_back(source(6.123491, 1.026963, 35.620000));
    __patches[0].push_back(source(6.123070, 1.026915, 35.150000));
    __patches[0].push_back(source(6.122649, 1.026405, 34.710000));
    __patches[0].push_back(source(6.122508, 1.026502, 34.240000));
    __patches[0].push_back(source(6.123257, 1.026987, 34.090000));
    __patches[0].push_back(source(6.123117, 1.026818, 33.530000));
    __patches[0].push_back(source(6.122695, 1.026551, 33.260000));
    __patches[0].push_back(source(6.123351, 1.026866, 32.440000));
    __patches[0].push_back(source(6.122882, 1.026793, 31.760000));
    __patches[0].push_back(source(6.122836, 1.026502, 31.490000));
    __patches[0].push_back(source(6.123632, 1.026963, 31.240000));
    __patches[0].push_back(source(6.122648, 1.026672, 31.070000));
    __patches[0].push_back(source(6.124521, 1.026284, 30.890000));
    __patches[0].push_back(source(6.122602, 1.026405, 30.680000));
    __patches[0].push_back(source(6.122929, 1.026696, 30.610000));
    __patches[0].push_back(source(6.122976, 1.026890, 30.350000));
    __patches[0].push_back(source(6.123444, 1.027012, 30.130000));
    __patches[0].push_back(source(6.122461, 1.026478, 29.880000));
    __patches[0].push_back(source(6.124240, 1.026114, 29.740000));
    __patches[0].push_back(source(6.123163, 1.026963, 29.690000));
    __patches[0].push_back(source(6.123257, 1.026721, 29.450000));
    __patches[0].push_back(source(6.123866, 1.026987, 29.380000));
    __patches[0].push_back(source(6.122648, 1.026551, 28.550000));
    __patches[0].push_back(source(6.122742, 1.026333, 28.330000));
    __patches[0].push_back(source(6.123491, 1.026866, 28.050000));
    __patches[0].push_back(source(6.124427, 1.026236, 27.830000));
    __patches[0].push_back(source(6.122742, 1.026454, 27.720000));
    __patches[0].push_back(source(6.122836, 1.026818, 27.620000));
    __patches[0].push_back(source(6.124053, 1.026939, 27.520000));
    __patches[0].push_back(source(6.124521, 1.026332, 27.510000));
    __patches[0].push_back(source(6.122648, 1.026696, 27.440000));
    __patches[0].push_back(source(6.123163, 1.026793, 27.100000));
    __patches[0].push_back(source(6.123585, 1.027157, 27.130000));
    __patches[0].push_back(source(6.123679, 1.026915, 26.690000));
    __patches[0].push_back(source(6.124193, 1.026114, 26.580000));
    __patches[0].push_back(source(6.122976, 1.026478, 26.370000));
    __patches[0].push_back(source(6.122929, 1.026624, 26.160000));
    __patches[0].push_back(source(6.123070, 1.026963, 26.110000));
    __patches[0].push_back(source(6.123632, 1.027011, 25.930000));
    __patches[0].push_back(source(6.123398, 1.026745, 25.860000));
    __patches[0].push_back(source(6.124334, 1.026842, 25.680000));
    __patches[0].push_back(source(6.122555, 1.026599, 25.440000));
    __patches[0].push_back(source(6.124615, 1.026429, 25.400000));
    __patches[0].push_back(source(6.123351, 1.027036, 25.440000));
    __patches[0].push_back(source(6.122696, 1.026333, 25.290000));
    __patches[0].push_back(source(6.123257, 1.026672, 25.370000));
    __patches[0].push_back(source(6.123819, 1.027036, 25.050000));
    __patches[0].push_back(source(6.124380, 1.026187, 24.910000));
    __patches[0].push_back(source(6.122602, 1.026430, 24.900000));
    __patches[0].push_back(source(6.122789, 1.026745, 24.880000));
    __patches[0].push_back(source(6.122929, 1.026866, 24.640000));
    __patches[0].push_back(source(6.124521, 1.026284, 24.440000));
    __patches[0].push_back(source(6.122930, 1.026357, 24.420000));
    __patches[0].push_back(source(6.122461, 1.026502, 24.300000));
    __patches[0].push_back(source(6.123304, 1.026890, 24.120000));
    __patches[0].push_back(source(6.123585, 1.027157, 23.930000));
    __patches[0].push_back(source(6.123866, 1.026914, 23.790000));
    __patches[0].push_back(source(6.123070, 1.026721, 23.620000));
    __patches[0].push_back(source(6.124474, 1.026381, 23.520000));
    __patches[0].push_back(source(6.124241, 1.026866, 23.500000));
    __patches[0].push_back(source(6.124193, 1.026090, 23.400000));
    __patches[0].push_back(source(6.123070, 1.026478, 23.300000));
    __patches[0].push_back(source(6.122789, 1.026284, 23.070000));
    __patches[0].push_back(source(6.123585, 1.026018, 22.930000));
    __patches[0].push_back(source(6.122601, 1.026696, 22.830000));
    __patches[0].push_back(source(6.124428, 1.026817, 22.810000));
    __patches[0].push_back(source(6.123210, 1.026987, 22.800000));
    __patches[0].push_back(source(6.124007, 1.026987, 22.530000));
    __patches[0].push_back(source(6.124661, 1.026454, 22.550000));
    __patches[0].push_back(source(6.122742, 1.026575, 22.370000));
    __patches[0].push_back(source(6.124193, 1.026211, 22.370000));
    __patches[0].push_back(source(6.123398, 1.027084, 22.180000));
    __patches[0].push_back(source(6.122836, 1.026430, 22.190000));
    __patches[0].push_back(source(6.123444, 1.026769, 22.160000));
    __patches[0].push_back(source(6.122976, 1.026915, 21.960000));
    __patches[0].push_back(source(6.123679, 1.027181, 21.820000));
    __patches[0].push_back(source(6.124849, 1.026405, 21.830000));
    __patches[0].push_back(source(6.123632, 1.026866, 21.700000));
    __patches[0].push_back(source(6.123070, 1.026599, 21.600000));
    __patches[0].push_back(source(6.124521, 1.026236, 21.560000));
    __patches[0].push_back(source(6.122976, 1.026308, 21.500000));
    __patches[0].push_back(source(6.123070, 1.026818, 21.430000));
    __patches[0].push_back(source(6.122415, 1.026502, 21.380000));
    __patches[0].push_back(source(6.124333, 1.026139, 21.140000));
    __patches[0].push_back(source(6.122742, 1.026793, 21.140000));
    __patches[0].push_back(source(6.124661, 1.026332, 21.060000));
    __patches[0].push_back(source(6.124194, 1.026914, 20.920000));
    __patches[0].push_back(source(6.123491, 1.026963, 20.810000));
    __patches[0].push_back(source(6.124053, 1.026066, 20.780000));
    __patches[0].push_back(source(6.122508, 1.026405, 20.780000));
    __patches[0].push_back(source(6.123257, 1.026648, 20.650000));
    __patches[0].push_back(source(6.123491, 1.026042, 20.640000));
    __patches[0].push_back(source(6.123726, 1.027036, 20.560000));
    __patches[0].push_back(source(6.122508, 1.026575, 20.550000));
    __patches[0].push_back(source(6.122836, 1.026236, 20.480000));
    __patches[0].push_back(source(6.123070, 1.026987, 20.370000));
    __patches[0].push_back(source(6.124708, 1.026502, 20.340000));
    __patches[0].push_back(source(6.124475, 1.026842, 20.340000));
    __patches[0].push_back(source(6.123117, 1.026502, 20.150000));
    __patches[0].push_back(source(6.124380, 1.026357, 20.160000));
    __patches[0].push_back(source(6.123070, 1.026381, 20.080000));
    __patches[0].push_back(source(6.123960, 1.026866, 20.020000));
    __patches[0].push_back(source(6.123444, 1.027133, 19.840000));
    __patches[0].push_back(source(6.122836, 1.026866, 19.830000));
    __patches[0].push_back(source(6.122836, 1.026696, 19.630000));
    __patches[0].push_back(source(6.124053, 1.026163, 19.570000));
    __patches[0].push_back(source(6.124007, 1.027036, 19.470000));
    __patches[0].push_back(source(6.122696, 1.026308, 19.500000));
    __patches[0].push_back(source(6.123725, 1.026018, 19.370000));
    __patches[0].push_back(source(6.123023, 1.026115, 19.250000));
    __patches[0].push_back(source(6.124380, 1.026236, 19.220000));
    __patches[0].push_back(source(6.122601, 1.026696, 19.240000));
    __patches[0].push_back(source(6.123632, 1.027181, 19.180000));
    __patches[0].push_back(source(6.123819, 1.026939, 19.110000));
    __patches[0].push_back(source(6.124427, 1.026454, 19.050000));
    __patches[0].push_back(source(6.123538, 1.026696, 18.980000));
    __patches[0].push_back(source(6.124287, 1.026793, 18.920000));
    __patches[0].push_back(source(6.124240, 1.026090, 18.880000));
    __patches[0].push_back(source(6.122695, 1.026478, 18.860000));
    __patches[0].push_back(source(6.123304, 1.026066, 18.760000));
    __patches[0].push_back(source(6.124895, 1.026405, 18.700000));
    __patches[0].push_back(source(6.122602, 1.026599, 18.720000));
    __patches[0].push_back(source(6.123210, 1.026890, 18.680000));
    __patches[0].push_back(source(6.124568, 1.026793, 18.630000));
    __patches[0].push_back(source(6.123070, 1.026260, 18.620000));
    __patches[0].push_back(source(6.124099, 1.026260, 18.460000));
    __patches[0].push_back(source(6.123257, 1.026769, 18.420000));
    __patches[0].push_back(source(6.122929, 1.026551, 18.290000));
    __patches[0].push_back(source(6.123538, 1.027060, 18.260000));
    __patches[0].push_back(source(6.124615, 1.026381, 18.200000));
    __patches[0].push_back(source(6.123959, 1.026042, 18.150000));
    __patches[0].push_back(source(6.124568, 1.026551, 18.130000));
    __patches[0].push_back(source(6.123585, 1.026842, 18.090000));
    __patches[0].push_back(source(6.122836, 1.026187, 18.090000));
    __patches[0].push_back(source(6.123773, 1.027181, 18.060000));
    __patches[0].push_back(source(6.124428, 1.026890, 17.950000));
    __patches[0].push_back(source(6.123210, 1.027036, 17.860000));
    __patches[0].push_back(source(6.122883, 1.026381, 17.810000));
    __patches[0].push_back(source(6.123444, 1.026115, 17.760000));
    __patches[0].push_back(source(6.122695, 1.026793, 17.730000));
    __patches[0].push_back(source(6.122415, 1.026478, 17.680000));
    __patches[0].push_back(source(6.123163, 1.026648, 17.680000));
    __patches[0].push_back(source(6.124661, 1.026284, 17.650000));
    __patches[0].push_back(source(6.123491, 1.025993, 17.590000));
    __patches[0].push_back(source(6.123257, 1.026405, 17.550000));
    __patches[0].push_back(source(6.123819, 1.027084, 17.420000));
    __patches[0].push_back(source(6.124475, 1.026720, 17.380000));
    __patches[0].push_back(source(6.122976, 1.026963, 17.270000));
    __patches[0].push_back(source(6.124755, 1.026502, 17.150000));
    __patches[0].push_back(source(6.124147, 1.026963, 17.090000));
    __patches[0].push_back(source(6.122555, 1.026381, 17.050000));
    __patches[0].push_back(source(6.124427, 1.026139, 17.010000));
    __patches[0].push_back(source(6.123398, 1.026818, 16.980000));
    __patches[0].push_back(source(6.123772, 1.026212, 17.010000));
    __patches[0].push_back(source(6.124380, 1.026333, 16.900000));
    __patches[0].push_back(source(6.122601, 1.026720, 16.860000));
    __patches[0].push_back(source(6.124006, 1.026817, 16.820000));
    __patches[0].push_back(source(6.123398, 1.026672, 16.730000));
    __patches[0].push_back(source(6.123398, 1.027157, 16.760000));
    __patches[0].push_back(source(6.122929, 1.026745, 16.690000));
    __patches[0].push_back(source(6.122977, 1.026115, 16.700000));
    __patches[0].push_back(source(6.123117, 1.026478, 16.650000));
    __patches[0].push_back(source(6.124146, 1.026042, 16.530000));
    __patches[0].push_back(source(6.123351, 1.026939, 16.530000));
    __patches[0].push_back(source(6.123117, 1.026357, 16.480000));
    __patches[0].push_back(source(6.124568, 1.026623, 16.470000));
    __patches[0].push_back(source(6.124054, 1.027084, 16.380000));
    __patches[0].push_back(source(6.123351, 1.026018, 16.360000));
    __patches[0].push_back(source(6.124193, 1.026163, 16.320000));
    __patches[0].push_back(source(6.122649, 1.026308, 16.230000));
    __patches[0].push_back(source(6.124942, 1.026381, 16.240000));
    __patches[0].push_back(source(6.124569, 1.026817, 16.170000));
    __patches[0].push_back(source(6.123304, 1.026527, 16.180000));
    __patches[0].push_back(source(6.122835, 1.026890, 16.150000));
    __patches[0].push_back(source(6.124334, 1.026454, 16.120000));
    __patches[0].push_back(source(6.123585, 1.027205, 16.070000));
    __patches[0].push_back(source(6.123819, 1.026818, 16.070000));
    __patches[0].push_back(source(6.122742, 1.026624, 16.040000));
    __patches[0].push_back(source(6.123164, 1.026163, 16.050000));
    __patches[0].push_back(source(6.124100, 1.026308, 15.870000));
    __patches[0].push_back(source(6.123631, 1.025993, 15.840000));
    __patches[0].push_back(source(6.124241, 1.026842, 15.770000));
    __patches[0].push_back(source(6.122461, 1.026575, 15.730000));
    __patches[0].push_back(source(6.124708, 1.026381, 15.730000));
    __patches[0].push_back(source(6.123632, 1.026672, 15.680000));
    __patches[0].push_back(source(6.123257, 1.027108, 15.720000));
    __patches[0].push_back(source(6.123819, 1.027181, 15.560000));
    __patches[0].push_back(source(6.124475, 1.026914, 15.470000));
    __patches[0].push_back(source(6.123585, 1.026139, 15.460000));
    __patches[0].push_back(source(6.124286, 1.026066, 15.390000));
    __patches[0].push_back(source(6.123023, 1.026236, 15.400000));
    __patches[0].push_back(source(6.124474, 1.026526, 15.410000));
    __patches[0].push_back(source(6.123070, 1.027011, 15.330000));
    __patches[0].push_back(source(6.124334, 1.026745, 15.300000));
    __patches[0].push_back(source(6.123070, 1.026866, 15.220000));
    __patches[0].push_back(source(6.122649, 1.026502, 15.240000));
    __patches[0].push_back(source(6.123866, 1.026963, 15.170000));
    __patches[0].push_back(source(6.124567, 1.026211, 15.120000));
    __patches[0].push_back(source(6.123257, 1.026599, 15.070000));
    __patches[0].push_back(source(6.123912, 1.026018, 15.020000));
    __patches[0].push_back(source(6.123304, 1.026430, 15.000000));
    __patches[0].push_back(source(6.124615, 1.026696, 15.020000));
    __patches[0].push_back(source(6.122883, 1.026139, 14.930000));
    __patches[0].push_back(source(6.123398, 1.027012, 14.920000));
    __patches[0].push_back(source(6.123959, 1.026163, 14.920000));
    __patches[0].push_back(source(6.122415, 1.026454, 14.850000));
    __patches[0].push_back(source(6.124568, 1.026429, 14.800000));
    __patches[0].push_back(source(6.122742, 1.026793, 14.820000));
    __patches[0].push_back(source(6.122789, 1.026236, 14.660000));
    __patches[0].push_back(source(6.124708, 1.026284, 14.520000));
    __patches[0].push_back(source(6.123679, 1.026939, 14.490000));
    __patches[0].push_back(source(6.123257, 1.026042, 14.480000));
    __patches[0].push_back(source(6.123725, 1.026284, 14.400000));
    __patches[0].push_back(source(6.122836, 1.026478, 14.430000));
    __patches[0].push_back(source(6.123632, 1.027060, 14.380000));
    __patches[0].push_back(source(6.123163, 1.026721, 14.370000));
    __patches[0].push_back(source(6.124334, 1.026623, 14.310000));
    __patches[0].push_back(source(6.124147, 1.026963, 14.280000));
    __patches[0].push_back(source(6.123210, 1.026309, 14.250000));
    __patches[0].push_back(source(6.124287, 1.026211, 14.260000));
    __patches[0].push_back(source(6.123632, 1.026575, 14.210000));
    __patches[0].push_back(source(6.122555, 1.026648, 14.110000));
    __patches[0].push_back(source(6.124942, 1.026405, 14.090000));
    __patches[0].push_back(source(6.122930, 1.026357, 14.080000));
    __patches[0].push_back(source(6.123726, 1.027205, 14.080000));
    __patches[0].push_back(source(6.123444, 1.026769, 14.150000));
    __patches[0].push_back(source(6.123538, 1.025993, 13.970000));
    __patches[0].push_back(source(6.124054, 1.027084, 13.940000));
    __patches[0].push_back(source(6.124755, 1.026526, 13.910000));
    __patches[0].push_back(source(6.124099, 1.026042, 13.910000));
    __patches[0].push_back(source(6.124615, 1.026841, 13.830000));
    __patches[0].push_back(source(6.122789, 1.026890, 13.850000));
    __patches[0].push_back(source(6.123491, 1.027157, 13.810000));
    __patches[0].push_back(source(6.122555, 1.026381, 13.780000));
    __patches[0].push_back(source(6.123772, 1.026696, 13.800000));
    __patches[0].push_back(source(6.124334, 1.026333, 13.750000));
    __patches[0].push_back(source(6.123070, 1.026090, 13.750000));
    __patches[0].push_back(source(6.123163, 1.027084, 13.660000));
    __patches[0].push_back(source(6.122789, 1.026648, 13.700000));
    __patches[0].push_back(source(6.124474, 1.026114, 13.640000));
    __patches[0].push_back(source(6.124053, 1.026769, 13.610000));
    __patches[0].push_back(source(6.123351, 1.026527, 13.620000));
    __patches[0].push_back(source(6.124240, 1.026454, 13.590000));
    __patches[0].push_back(source(6.123351, 1.026139, 13.530000));
    __patches[0].push_back(source(6.123913, 1.027157, 13.490000));
    __patches[0].push_back(source(6.122601, 1.026745, 13.420000));
    __patches[0].push_back(source(6.124475, 1.026939, 13.440000));
    __patches[0].push_back(source(6.123725, 1.026793, 13.370000));
    __patches[0].push_back(source(6.123398, 1.026357, 13.370000));
    __patches[0].push_back(source(6.124053, 1.026308, 13.330000));
    __patches[0].push_back(source(6.123772, 1.025993, 13.200000));
    __patches[0].push_back(source(6.124615, 1.026720, 13.210000));
    __patches[0].push_back(source(6.122929, 1.026769, 13.190000));
    __patches[0].push_back(source(6.124802, 1.026454, 13.170000));
    __patches[0].push_back(source(6.122414, 1.026526, 13.130000));
    __patches[0].push_back(source(6.124147, 1.026890, 13.090000));
    __patches[0].push_back(source(6.124333, 1.026042, 13.070000));
    __patches[0].push_back(source(6.122976, 1.026987, 13.030000));
    __patches[0].push_back(source(6.123023, 1.026527, 13.070000));
    __patches[0].push_back(source(6.124662, 1.026599, 12.980000));
    __patches[0].push_back(source(6.122649, 1.026284, 12.990000));
    __patches[0].push_back(source(6.123632, 1.026139, 12.940000));
    __patches[0].push_back(source(6.123819, 1.026599, 12.900000));
    __patches[0].push_back(source(6.123257, 1.026987, 12.850000));
    __patches[0].push_back(source(6.122883, 1.026090, 12.830000));
    __patches[0].push_back(source(6.123351, 1.026842, 12.740000));
    __patches[0].push_back(source(6.124381, 1.026817, 12.720000));
    __patches[0].push_back(source(6.123678, 1.026502, 12.690000));
    __patches[0].push_back(source(6.123913, 1.026890, 12.650000));
    __patches[0].push_back(source(6.123865, 1.026090, 12.600000));
    __patches[0].push_back(source(6.123913, 1.027011, 12.620000));
    __patches[0].push_back(source(6.123070, 1.026212, 12.570000));
    __patches[0].push_back(source(6.124287, 1.026696, 12.590000));
    __patches[0].push_back(source(6.124848, 1.026308, 12.560000));
    __patches[0].push_back(source(6.122461, 1.026599, 12.500000));
    __patches[0].push_back(source(6.123538, 1.026648, 12.470000));
    __patches[0].push_back(source(6.123865, 1.026236, 12.410000));
    __patches[0].push_back(source(6.123304, 1.027133, 12.400000));
    __patches[0].push_back(source(6.124849, 1.026575, 12.400000));
    __patches[0].push_back(source(6.123351, 1.025993, 12.340000));
    __patches[0].push_back(source(6.124474, 1.026260, 12.340000));
    __patches[0].push_back(source(6.124194, 1.027036, 12.270000));
    __patches[0].push_back(source(6.123164, 1.026357, 12.270000));
    __patches[0].push_back(source(6.123070, 1.026599, 12.250000));
    __patches[0].push_back(source(6.124334, 1.026526, 12.200000));
    __patches[0].push_back(source(6.122835, 1.026914, 12.200000));
    __patches[0].push_back(source(6.122415, 1.026454, 12.180000));
    __patches[0].push_back(source(6.123538, 1.027181, 12.140000));
    __patches[0].push_back(source(6.124520, 1.026139, 12.150000));
    __patches[0].push_back(source(6.122742, 1.026187, 12.110000));
    __patches[0].push_back(source(6.123772, 1.026357, 12.080000));
    __patches[0].push_back(source(6.122883, 1.026308, 11.990000));
    __patches[0].push_back(source(6.124616, 1.026890, 11.940000));
    __patches[0].push_back(source(6.123538, 1.026915, 11.960000));
    __patches[0].push_back(source(6.124052, 1.026018, 11.950000));
    __patches[0].push_back(source(6.122508, 1.026696, 11.910000));
    __patches[0].push_back(source(6.123164, 1.026042, 11.880000));
    __patches[0].push_back(source(6.123773, 1.027205, 11.850000));
    __patches[0].push_back(source(6.124287, 1.026405, 11.840000));
    __patches[0].push_back(source(6.124989, 1.026381, 11.790000));
    __patches[0].push_back(source(6.123070, 1.026478, 11.750000));
    __patches[0].push_back(source(6.123116, 1.027084, 11.730000));
    __patches[0].push_back(source(6.124381, 1.026963, 11.740000));
    __patches[0].push_back(source(6.122695, 1.026793, 11.680000));
    __patches[0].push_back(source(6.123632, 1.026260, 11.660000));
    __patches[0].push_back(source(6.124193, 1.026187, 11.650000));
    __patches[0].push_back(source(6.123632, 1.026769, 11.660000));
    __patches[0].push_back(source(6.124147, 1.026551, 11.600000));
    __patches[0].push_back(source(6.123772, 1.025993, 11.580000));
    __patches[0].push_back(source(6.124615, 1.026405, 11.520000));
    __patches[0].push_back(source(6.123210, 1.026139, 11.530000));
    __patches[0].push_back(source(6.122649, 1.026478, 11.490000));
    __patches[0].push_back(source(6.123960, 1.027157, 11.470000));
    __patches[0].push_back(source(6.123351, 1.026478, 11.480000));
    __patches[0].push_back(source(6.123163, 1.026890, 11.390000));
    __patches[0].push_back(source(6.122602, 1.026357, 11.330000));
    __patches[0].push_back(source(6.124849, 1.026478, 11.330000));
    __patches[0].push_back(source(6.124380, 1.026066, 11.300000));
    __patches[0].push_back(source(6.122976, 1.026793, 11.320000));
    __patches[0].push_back(source(6.124288, 1.026866, 11.300000));
    __patches[0].push_back(source(6.123491, 1.025969, 11.300000));
    __patches[0].push_back(source(6.123304, 1.026599, 11.200000));
    __patches[0].push_back(source(6.124334, 1.026672, 11.170000));
    __patches[0].push_back(source(6.124708, 1.026235, 11.180000));
    __patches[0].push_back(source(6.123538, 1.026357, 11.130000));
    __patches[0].push_back(source(6.123398, 1.026987, 11.090000));
    __patches[0].push_back(source(6.124662, 1.026817, 11.060000));
    __patches[0].push_back(source(6.122695, 1.026599, 11.050000));
    __patches[0].push_back(source(6.122789, 1.026115, 11.030000));
    __patches[0].push_back(source(6.123679, 1.026551, 10.990000));
    __patches[0].push_back(source(6.123538, 1.026066, 10.960000));
    __patches[0].push_back(source(6.123772, 1.026696, 10.940000));
    __patches[0].push_back(source(6.123023, 1.027011, 10.910000));
    __patches[0].push_back(source(6.123866, 1.026818, 10.890000));
    __patches[0].push_back(source(6.122836, 1.026696, 10.880000));
    __patches[0].push_back(source(6.124334, 1.026284, 10.890000));
    __patches[0].push_back(source(6.124709, 1.026696, 10.880000));
    __patches[0].push_back(source(6.123632, 1.027060, 10.830000));
    __patches[0].push_back(source(6.123163, 1.026721, 10.750000));
    __patches[0].push_back(source(6.123398, 1.026236, 10.750000));
    __patches[0].push_back(source(6.124568, 1.026502, 10.700000));
    __patches[0].push_back(source(6.123913, 1.027060, 10.700000));
    __patches[0].push_back(source(6.122696, 1.026260, 10.680000));
    __patches[0].push_back(source(6.122508, 1.026575, 10.660000));
    __patches[0].push_back(source(6.124428, 1.026769, 10.600000));
    __patches[0].push_back(source(6.122930, 1.026042, 10.570000));
    __patches[0].push_back(source(6.123398, 1.027157, 10.590000));
    __patches[0].push_back(source(6.124989, 1.026356, 10.550000));
    __patches[0].push_back(source(6.122742, 1.026866, 10.530000));
    __patches[0].push_back(source(6.122368, 1.026454, 10.580000));
    __patches[0].push_back(source(6.124896, 1.026575, 10.520000));
    __patches[0].push_back(source(6.124099, 1.026090, 10.520000));
    __patches[0].push_back(source(6.123538, 1.026454, 10.460000));
    __patches[0].push_back(source(6.124241, 1.027036, 10.460000));
    __patches[0].push_back(source(6.124100, 1.026381, 10.450000));
    __patches[0].push_back(source(6.124708, 1.026332, 10.420000));
    __patches[0].push_back(source(6.123679, 1.026939, 10.380000));
    __patches[0].push_back(source(6.123023, 1.026405, 10.370000));
    __patches[0].push_back(source(6.123210, 1.025993, 10.350000));
    __patches[0].push_back(source(6.125083, 1.026720, 10.360000));
    __patches[0].push_back(source(6.124099, 1.025993, 10.340000));
    __patches[0].push_back(source(6.123913, 1.026599, 10.300000));
    __patches[0].push_back(source(6.123912, 1.026163, 10.210000));
    __patches[0].push_back(source(6.124101, 1.027108, 10.200000));
    __patches[0].push_back(source(6.122601, 1.026745, 10.180000));
    __patches[0].push_back(source(6.123163, 1.027108, 10.170000));
    __patches[0].push_back(source(6.124567, 1.026163, 10.140000));
    __patches[0].push_back(source(6.123117, 1.026236, 10.110000));
    __patches[0].push_back(source(6.124428, 1.026963, 10.090000));
    __patches[0].push_back(source(6.123070, 1.026648, 10.090000));
    __patches[0].push_back(source(6.123632, 1.026163, 10.070000));
    __patches[0].push_back(source(6.124053, 1.026745, 10.040000));
    __patches[0].push_back(source(6.123585, 1.027205, 10.000000));
    __patches[0].push_back(source(6.124895, 1.026260, 10.020000));
    __patches[0].push_back(source(6.123538, 1.026599, 9.977000));
    __patches[0].push_back(source(6.124053, 1.026963, 9.970000));
    __patches[0].push_back(source(6.122976, 1.026890, 9.954000));
    __patches[0].push_back(source(6.122602, 1.026478, 9.979000));
    __patches[0].push_back(source(6.124381, 1.026478, 9.884000));
    __patches[0].push_back(source(6.124662, 1.026890, 9.883000));
    __patches[0].push_back(source(6.123631, 1.025969, 9.875000));
    __patches[0].push_back(source(6.123725, 1.026405, 9.854000));
    __patches[0].push_back(source(6.122555, 1.026357, 9.813000));
    __patches[0].push_back(source(6.123164, 1.025824, 9.839000));
    __patches[0].push_back(source(6.123304, 1.026818, 9.768000));
    __patches[0].push_back(source(6.124286, 1.026114, 9.742000));
    __patches[0].push_back(source(6.124896, 1.026453, 9.782000));
    __patches[0].push_back(source(6.123117, 1.026115, 9.706000));
    __patches[0].push_back(source(6.123819, 1.027205, 9.703000));
    __patches[0].push_back(source(6.122508, 1.026672, 9.698000));
    __patches[0].push_back(source(6.123257, 1.026454, 9.676000));
    __patches[0].push_back(source(6.124428, 1.026575, 9.655000));
    __patches[0].push_back(source(6.123491, 1.026745, 9.620000));
    __patches[0].push_back(source(6.124801, 1.026187, 9.609000));
    __patches[0].push_back(source(6.125036, 1.026647, 9.607000));
    __patches[0].push_back(source(6.123491, 1.026866, 9.556000));
    __patches[0].push_back(source(6.122976, 1.027011, 9.533000));
    __patches[0].push_back(source(6.122976, 1.026236, 9.498000));
    __patches[0].push_back(source(6.124380, 1.026017, 9.474000));
    __patches[0].push_back(source(6.123398, 1.025921, 9.451000));
    __patches[0].push_back(source(6.122883, 1.026502, 9.464000));
    __patches[0].push_back(source(6.123819, 1.026308, 9.473000));
    __patches[0].push_back(source(6.124194, 1.026793, 9.440000));
    __patches[0].push_back(source(6.124474, 1.026381, 9.401000));
    __patches[0].push_back(source(6.123726, 1.027108, 9.328000));
    __patches[0].push_back(source(6.125130, 1.026720, 9.341000));
    __patches[0].push_back(source(6.123164, 1.026333, 9.309000));
    __patches[0].push_back(source(6.122461, 1.026575, 9.273000));
    __patches[0].push_back(source(6.123398, 1.026115, 9.260000));
    __patches[0].push_back(source(6.124193, 1.026284, 9.247000));
    __patches[0].push_back(source(6.122835, 1.026939, 9.236000));
    __patches[0].push_back(source(6.122743, 1.026163, 9.226000));
    __patches[0].push_back(source(6.123819, 1.026696, 9.220000));
    __patches[0].push_back(source(6.123444, 1.027060, 9.211000));
    __patches[0].push_back(source(6.124521, 1.026720, 9.171000));
    __patches[0].push_back(source(6.123912, 1.025969, 9.171000));
    __patches[0].push_back(source(6.123398, 1.026357, 9.164000));
    __patches[0].push_back(source(6.124194, 1.026914, 9.129000));
    __patches[0].push_back(source(6.122461, 1.026405, 9.105000));
    __patches[0].push_back(source(6.122977, 1.025993, 9.128000));
    __patches[0].push_back(source(6.122695, 1.026842, 9.103000));
    __patches[0].push_back(source(6.124567, 1.026066, 9.081000));
    __patches[0].push_back(source(6.123772, 1.026842, 9.065000));
    __patches[0].push_back(source(6.124755, 1.026405, 9.049000));
    __patches[0].push_back(source(6.123678, 1.026527, 9.020000));
    __patches[0].push_back(source(6.123819, 1.026066, 9.008000));
    __patches[0].push_back(source(6.124709, 1.026793, 8.948000));
    __patches[0].push_back(source(6.124054, 1.027157, 8.927000));
    __patches[0].push_back(source(6.123632, 1.026284, 8.885000));
    __patches[0].push_back(source(6.124193, 1.026551, 8.866000));
    __patches[0].push_back(source(6.124942, 1.026308, 8.869000));
    __patches[0].push_back(source(6.122789, 1.026721, 8.863000));
    __patches[0].push_back(source(6.124616, 1.026938, 8.854000));
    __patches[0].push_back(source(6.122649, 1.026260, 8.837000));
    __patches[0].push_back(source(6.123491, 1.026042, 8.836000));
    __patches[0].push_back(source(6.122789, 1.026599, 8.845000));
    __patches[0].push_back(source(6.123351, 1.027157, 8.803000));
    __patches[0].push_back(source(6.124708, 1.026551, 8.781000));
    __patches[0].push_back(source(6.123304, 1.026915, 8.740000));
    __patches[0].push_back(source(6.122368, 1.026502, 8.706000));
    __patches[0].push_back(source(6.123585, 1.025921, 8.674000));
    __patches[0].push_back(source(6.124239, 1.025993, 8.663000));
    __patches[0].push_back(source(6.124006, 1.026866, 8.654000));
    __patches[0].push_back(source(6.123772, 1.026405, 8.685000));
    __patches[0].push_back(source(6.124802, 1.026672, 8.624000));
    __patches[0].push_back(source(6.122836, 1.026381, 8.610000));
    __patches[0].push_back(source(6.123632, 1.027230, 8.622000));
    __patches[0].push_back(source(6.124661, 1.026187, 8.618000));
    __patches[0].push_back(source(6.123351, 1.026236, 8.603000));
    __patches[0].push_back(source(6.124240, 1.026648, 8.573000));
    __patches[0].push_back(source(6.123164, 1.025824, 8.587000));
    __patches[0].push_back(source(6.123585, 1.026648, 8.562000));
    __patches[0].push_back(source(6.123210, 1.025969, 8.497000));
    __patches[0].push_back(source(6.123116, 1.027108, 8.469000));
    __patches[0].push_back(source(6.124335, 1.027011, 8.480000));
    __patches[0].push_back(source(6.124146, 1.026430, 8.430000));
    __patches[0].push_back(source(6.122976, 1.026769, 8.421000));
    __patches[0].push_back(source(6.124943, 1.026575, 8.425000));
    __patches[0].push_back(source(6.123163, 1.026502, 8.407000));
    __patches[0].push_back(source(6.122836, 1.026090, 8.376000));
    __patches[0].push_back(source(6.124614, 1.026284, 8.389000));
    __patches[0].push_back(source(6.124007, 1.027036, 8.355000));
    __patches[0].push_back(source(6.123866, 1.026599, 8.348000));
    __patches[0].push_back(source(6.123959, 1.026260, 8.308000));
    __patches[0].push_back(source(6.123866, 1.026769, 8.250000));
    __patches[0].push_back(source(6.123023, 1.026890, 8.236000));
    __patches[0].push_back(source(6.123257, 1.026599, 8.177000));
    __patches[0].push_back(source(6.123679, 1.026987, 8.186000));
    __patches[0].push_back(source(6.124522, 1.026817, 8.186000));
    __patches[0].push_back(source(6.122883, 1.026284, 8.171000));
    __patches[0].push_back(source(6.124193, 1.026163, 8.161000));
    __patches[0].push_back(source(6.122508, 1.026720, 8.167000));
    __patches[0].push_back(source(6.125130, 1.026744, 8.150000));
    __patches[0].push_back(source(6.123070, 1.026115, 8.084000));
    __patches[0].push_back(source(6.124989, 1.026405, 8.094000));
    __patches[0].push_back(source(6.122602, 1.026478, 8.092000));
    __patches[0].push_back(source(6.123491, 1.027205, 8.062000));
    __patches[0].push_back(source(6.124474, 1.026163, 8.049000));
    __patches[0].push_back(source(6.123257, 1.026721, 8.047000));
    __patches[0].push_back(source(6.123491, 1.026357, 8.044000));
    __patches[0].push_back(source(6.124099, 1.025993, 8.003000));
    __patches[0].push_back(source(6.123210, 1.027011, 7.995000));
    __patches[0].push_back(source(6.124521, 1.026623, 7.981000));
    __patches[0].push_back(source(6.124756, 1.026890, 7.944000));
    __patches[0].push_back(source(6.124473, 1.026017, 7.942000));
    __patches[0].push_back(source(6.122321, 1.026478, 7.936000));
    __patches[0].push_back(source(6.123632, 1.026187, 7.924000));
    __patches[0].push_back(source(6.122695, 1.026866, 7.935000));
    __patches[0].push_back(source(6.124053, 1.026575, 7.915000));
    __patches[0].push_back(source(6.124895, 1.026211, 7.905000));
    __patches[0].push_back(source(6.125130, 1.026623, 7.914000));
    __patches[0].push_back(source(6.123866, 1.027205, 7.915000));
    __patches[0].push_back(source(6.123632, 1.026769, 7.893000));
    __patches[0].push_back(source(6.122976, 1.026575, 7.885000));
    __patches[0].push_back(source(6.122977, 1.025969, 7.891000));
    __patches[0].push_back(source(6.123818, 1.025945, 7.862000));
    __patches[0].push_back(source(6.122649, 1.026357, 7.820000));
    __patches[0].push_back(source(6.124006, 1.026381, 7.788000));
    __patches[0].push_back(source(6.124522, 1.026963, 7.785000));
    __patches[0].push_back(source(6.122461, 1.026623, 7.803000));
    __patches[0].push_back(source(6.123679, 1.027108, 7.790000));
    __patches[0].push_back(source(6.123398, 1.026478, 7.797000));
    __patches[0].push_back(source(6.122976, 1.027011, 7.742000));
    __patches[0].push_back(source(6.123164, 1.025848, 7.750000));
    __patches[0].push_back(source(6.124896, 1.026502, 7.710000));
    __patches[0].push_back(source(6.123866, 1.026914, 7.682000));
    __patches[0].push_back(source(6.124380, 1.026308, 7.666000));
    __patches[0].push_back(source(6.124896, 1.026672, 7.647000));
    __patches[0].push_back(source(6.122929, 1.026454, 7.620000));
    __patches[0].push_back(source(6.123304, 1.026139, 7.585000));
    __patches[0].push_back(source(6.123210, 1.027157, 7.593000));
    __patches[0].push_back(source(6.124006, 1.026696, 7.584000));
    __patches[0].push_back(source(6.124288, 1.027060, 7.577000));
    __patches[0].push_back(source(6.122742, 1.026187, 7.576000));
    __patches[0].push_back(source(6.123444, 1.025896, 7.576000));
    __patches[0].push_back(source(6.123819, 1.026115, 7.543000));
    __patches[0].push_back(source(6.123678, 1.026527, 7.521000));
    __patches[0].push_back(source(6.124661, 1.026454, 7.524000));
    __patches[0].push_back(source(6.122648, 1.026793, 7.493000));
    __patches[0].push_back(source(6.124099, 1.026090, 7.475000));
    __patches[0].push_back(source(6.124661, 1.026090, 7.430000));
    __patches[0].push_back(source(6.123351, 1.027060, 7.415000));
    __patches[0].push_back(source(6.124101, 1.027157, 7.384000));
    __patches[0].push_back(source(6.124942, 1.026284, 7.384000));
    __patches[0].push_back(source(6.123070, 1.026696, 7.371000));
    __patches[0].push_back(source(6.122415, 1.026381, 7.354000));
    __patches[0].push_back(source(6.124334, 1.026720, 7.341000));
    __patches[0].push_back(source(6.123491, 1.026939, 7.327000));
    __patches[0].push_back(source(6.123351, 1.026284, 7.328000));
    __patches[0].push_back(source(6.124381, 1.026866, 7.328000));
    __patches[0].push_back(source(6.124474, 1.026429, 7.304000));
    __patches[0].push_back(source(6.122836, 1.026066, 7.257000));
    __patches[0].push_back(source(6.122835, 1.026963, 7.233000));
    __patches[0].push_back(source(6.123678, 1.025921, 7.222000));
    __patches[0].push_back(source(6.124239, 1.025969, 7.181000));
    __patches[0].push_back(source(6.124849, 1.026769, 7.162000));
    __patches[0].push_back(source(6.123772, 1.026696, 7.166000));
    __patches[0].push_back(source(6.123585, 1.026284, 7.166000));
    __patches[0].push_back(source(6.124240, 1.026260, 7.114000));
    __patches[0].push_back(source(6.123726, 1.027230, 7.116000));
    __patches[0].push_back(source(6.122695, 1.026648, 7.115000));
    __patches[0].push_back(source(6.123210, 1.026042, 7.119000));
    __patches[0].push_back(source(6.124333, 1.026090, 7.105000));
    __patches[0].push_back(source(6.125036, 1.026381, 7.080000));
    __patches[0].push_back(source(6.123304, 1.026793, 7.073000));
    __patches[0].push_back(source(6.124053, 1.026502, 7.069000));
    __patches[0].push_back(source(6.123117, 1.025848, 7.049000));
    __patches[0].push_back(source(6.122461, 1.026599, 7.023000));
    __patches[0].push_back(source(6.123491, 1.026551, 7.035000));
    __patches[0].push_back(source(6.124895, 1.026187, 7.021000));
    __patches[0].push_back(source(6.124100, 1.026890, 7.011000));
    __patches[0].push_back(source(6.124802, 1.026357, 6.969000));
    __patches[0].push_back(source(6.125177, 1.026744, 6.992000));
    __patches[0].push_back(source(6.123491, 1.026430, 6.969000));
    __patches[0].push_back(source(6.123678, 1.026042, 6.959000));
    __patches[0].push_back(source(6.122321, 1.026478, 6.918000));
    __patches[0].push_back(source(6.124662, 1.026841, 6.910000));
    __patches[0].push_back(source(6.122789, 1.026308, 6.913000));
    __patches[0].push_back(source(6.123912, 1.026236, 6.922000));
    __patches[0].push_back(source(6.123070, 1.027084, 6.912000));
    __patches[0].push_back(source(6.122555, 1.026745, 6.855000));
    __patches[0].push_back(source(6.124756, 1.026963, 6.848000));
    __patches[0].push_back(source(6.122789, 1.026430, 6.838000));
    __patches[0].push_back(source(6.124755, 1.026599, 6.830000));
    __patches[0].push_back(source(6.123491, 1.027205, 6.806000));
    __patches[0].push_back(source(6.123023, 1.026599, 6.797000));
    __patches[0].push_back(source(6.124100, 1.026624, 6.797000));
    __patches[0].push_back(source(6.122836, 1.026163, 6.814000));
    __patches[0].push_back(source(6.125130, 1.026623, 6.800000));
    __patches[0].push_back(source(6.124241, 1.027108, 6.777000));
    __patches[0].push_back(source(6.123585, 1.026818, 6.747000));
    __patches[0].push_back(source(6.122929, 1.026818, 6.734000));
    __patches[0].push_back(source(6.123117, 1.026405, 6.735000));
    __patches[0].push_back(source(6.124053, 1.026769, 6.696000));
    __patches[0].push_back(source(6.122977, 1.025969, 6.702000));
    __patches[0].push_back(source(6.123959, 1.025945, 6.701000));
    __patches[0].push_back(source(6.123585, 1.027084, 6.697000));
    __patches[0].push_back(source(6.123304, 1.026187, 6.667000));
    __patches[0].push_back(source(6.123491, 1.026672, 6.666000));
    __patches[0].push_back(source(6.124474, 1.026333, 6.655000));
    __patches[0].push_back(source(6.124147, 1.026987, 6.629000));
    __patches[0].push_back(source(6.124989, 1.026502, 6.630000));
    __patches[0].push_back(source(6.123163, 1.026939, 6.603000));
    __patches[0].push_back(source(6.123257, 1.025945, 6.598000));
    __patches[0].push_back(source(6.124334, 1.026551, 6.580000));
    __patches[0].push_back(source(6.124615, 1.026745, 6.548000));
    __patches[0].push_back(source(6.122649, 1.026236, 6.553000));
    __patches[0].push_back(source(6.123725, 1.026357, 6.542000));
    __patches[0].push_back(source(6.123632, 1.026963, 6.548000));
    __patches[0].push_back(source(6.124520, 1.026017, 6.526000));
    __patches[0].push_back(source(6.123444, 1.026042, 6.510000));
    __patches[0].push_back(source(6.124569, 1.026987, 6.475000));
    __patches[0].push_back(source(6.123210, 1.027157, 6.452000));
    __patches[0].push_back(source(6.122461, 1.026672, 6.455000));
    __patches[0].push_back(source(6.124427, 1.026163, 6.447000));
    __patches[0].push_back(source(6.123959, 1.026333, 6.404000));
    __patches[0].push_back(source(6.122742, 1.026890, 6.404000));
    __patches[0].push_back(source(6.123398, 1.026357, 6.409000));
    __patches[0].push_back(source(6.123772, 1.027108, 6.373000));
    __patches[0].push_back(source(6.122461, 1.026381, 6.376000));
    __patches[0].push_back(source(6.124754, 1.026138, 6.380000));
    __patches[0].push_back(source(6.123772, 1.026599, 6.384000));
    __patches[0].push_back(source(6.124662, 1.026502, 6.356000));
    __patches[0].push_back(source(6.122836, 1.026721, 6.355000));
    __patches[0].push_back(source(6.122555, 1.026502, 6.324000));
    __patches[0].push_back(source(6.124850, 1.026938, 6.306000));
    __patches[0].push_back(source(6.123398, 1.025872, 6.319000));
    __patches[0].push_back(source(6.123773, 1.027230, 6.310000));
    __patches[0].push_back(source(6.123912, 1.026066, 6.295000));
    __patches[0].push_back(source(6.123070, 1.026284, 6.300000));
    __patches[0].push_back(source(6.123960, 1.027011, 6.298000));
    __patches[0].push_back(source(6.124942, 1.026284, 6.251000));
    __patches[0].push_back(source(6.123164, 1.025824, 6.228000));
    __patches[0].push_back(source(6.124990, 1.026793, 6.202000));
    __patches[0].push_back(source(6.123819, 1.026842, 6.194000));
    __patches[0].push_back(source(6.122976, 1.027011, 6.213000));
    __patches[0].push_back(source(6.123772, 1.026187, 6.204000));
    __patches[0].push_back(source(6.123257, 1.026527, 6.164000));
    __patches[0].push_back(source(6.123772, 1.025921, 6.160000));
    __patches[0].push_back(source(6.125083, 1.026623, 6.136000));
    __patches[0].push_back(source(6.124661, 1.026235, 6.135000));
    __patches[0].push_back(source(6.124521, 1.026623, 6.129000));
    __patches[0].push_back(source(6.124333, 1.025969, 6.124000));
    __patches[0].push_back(source(6.123070, 1.026115, 6.115000));
    __patches[0].push_back(source(6.124193, 1.026405, 6.093000));
    __patches[0].push_back(source(6.123866, 1.026721, 6.067000));
    __patches[0].push_back(source(6.122601, 1.026817, 6.087000));
    __patches[0].push_back(source(6.122555, 1.026284, 6.050000));
    __patches[0].push_back(source(6.123491, 1.026187, 6.042000));
    __patches[0].push_back(source(6.124007, 1.027181, 6.026000));
    __patches[0].push_back(source(6.123117, 1.026721, 6.002000));
    __patches[0].push_back(source(6.125083, 1.026381, 6.027000));
    __patches[0].push_back(source(6.125177, 1.026744, 5.984000));
    __patches[0].push_back(source(6.124335, 1.026963, 5.995000));
    __patches[0].push_back(source(6.122602, 1.026405, 5.981000));
    __patches[0].push_back(source(6.123678, 1.026478, 5.948000));
    __patches[0].push_back(source(6.122836, 1.026042, 5.946000));
    __patches[0].push_back(source(6.124802, 1.026357, 5.937000));
    __patches[0].push_back(source(6.123351, 1.027036, 5.936000));
    __patches[0].push_back(source(6.124100, 1.026842, 5.927000));
    __patches[0].push_back(source(6.124943, 1.026696, 5.890000));
    __patches[0].push_back(source(6.124052, 1.025945, 5.902000));
    __patches[0].push_back(source(6.123444, 1.027181, 5.898000));
    __patches[0].push_back(source(6.122789, 1.026551, 5.877000));
    __patches[0].push_back(source(6.123444, 1.026890, 5.856000));
    __patches[0].push_back(source(6.123210, 1.026430, 5.828000));
    __patches[0].push_back(source(6.124006, 1.026187, 5.825000));
    __patches[0].push_back(source(6.122976, 1.026212, 5.803000));
    __patches[0].push_back(source(6.122414, 1.026599, 5.817000));
    __patches[0].push_back(source(6.122835, 1.026963, 5.808000));
    __patches[0].push_back(source(6.124614, 1.026066, 5.780000));
    __patches[0].push_back(source(6.123585, 1.025896, 5.775000));
    __patches[0].push_back(source(6.123632, 1.027254, 5.770000));
    __patches[0].push_back(source(6.124194, 1.026696, 5.778000));
    __patches[0].push_back(source(6.123585, 1.026769, 5.775000));
    __patches[0].push_back(source(6.122321, 1.026502, 5.742000));
    __patches[0].push_back(source(6.122977, 1.025945, 5.720000));
    __patches[0].push_back(source(6.124989, 1.026502, 5.721000));
    __patches[0].push_back(source(6.124382, 1.027036, 5.722000));
    __patches[0].push_back(source(6.124381, 1.026430, 5.718000));
    __patches[0].push_back(source(6.124942, 1.026187, 5.696000));
    __patches[0].push_back(source(6.123116, 1.027133, 5.696000));
    __patches[0].push_back(source(6.123632, 1.026284, 5.699000));
    __patches[0].push_back(source(6.122696, 1.026139, 5.666000));
    __patches[0].push_back(source(6.123257, 1.026818, 5.656000));
    __patches[0].push_back(source(6.124287, 1.026793, 5.658000));
    __patches[0].push_back(source(6.123725, 1.026405, 5.643000));
    __patches[0].push_back(source(6.122976, 1.026502, 5.623000));
    __patches[0].push_back(source(6.124334, 1.026551, 5.634000));
    __patches[0].push_back(source(6.124943, 1.026866, 5.632000));
    __patches[0].push_back(source(6.124194, 1.027133, 5.630000));
    __patches[0].push_back(source(6.123117, 1.025824, 5.571000));
    __patches[0].push_back(source(6.122929, 1.026672, 5.570000));
    __patches[0].push_back(source(6.124099, 1.026090, 5.550000));
    __patches[0].push_back(source(6.123491, 1.026624, 5.552000));
    __patches[0].push_back(source(6.124709, 1.027011, 5.522000));
    __patches[0].push_back(source(6.123257, 1.026284, 5.528000));
    __patches[0].push_back(source(6.125130, 1.026599, 5.528000));
    __patches[0].push_back(source(6.123866, 1.026914, 5.518000));
    __patches[0].push_back(source(6.124380, 1.026114, 5.517000));
    __patches[0].push_back(source(6.122976, 1.026842, 5.501000));
    __patches[0].push_back(source(6.124006, 1.026478, 5.495000));
    __patches[0].push_back(source(6.123819, 1.026042, 5.467000));
    __patches[0].push_back(source(6.123164, 1.026042, 5.445000));
    __patches[0].push_back(source(6.124662, 1.026769, 5.425000));
    __patches[0].push_back(source(6.124521, 1.026332, 5.425000));
    __patches[0].push_back(source(6.123491, 1.027084, 5.404000));
    __patches[0].push_back(source(6.124333, 1.025969, 5.393000));
    __patches[0].push_back(source(6.124006, 1.026599, 5.384000));
    __patches[0].push_back(source(6.122508, 1.026720, 5.366000));
    __patches[0].push_back(source(6.123023, 1.026308, 5.371000));
    __patches[0].push_back(source(6.123398, 1.025848, 5.368000));
    __patches[0].push_back(source(6.123163, 1.026963, 5.369000));
    __patches[0].push_back(source(6.123913, 1.027084, 5.330000));
    __patches[0].push_back(source(6.122508, 1.026333, 5.327000));
    __patches[0].push_back(source(6.124287, 1.026260, 5.319000));
    __patches[0].push_back(source(6.124802, 1.026623, 5.314000));
    __patches[0].push_back(source(6.124709, 1.026890, 5.289000));
    __patches[0].push_back(source(6.122648, 1.026842, 5.272000));
    __patches[0].push_back(source(6.124801, 1.026138, 5.270000));
    __patches[0].push_back(source(6.123959, 1.025945, 5.248000));
    __patches[0].push_back(source(6.123491, 1.026502, 5.251000));
    __patches[0].push_back(source(6.122368, 1.026429, 5.221000));
    __patches[0].push_back(source(6.123725, 1.026818, 5.210000));
    __patches[0].push_back(source(6.124568, 1.026526, 5.205000));
    __patches[0].push_back(source(6.122649, 1.026211, 5.206000));
    __patches[0].push_back(source(6.124334, 1.026914, 5.203000));
    __patches[0].push_back(source(6.123444, 1.026042, 5.204000));
    __patches[0].push_back(source(6.123679, 1.026987, 5.189000));
    __patches[0].push_back(source(6.125083, 1.026381, 5.180000));
    __patches[0].push_back(source(6.124520, 1.026017, 5.170000));
    __patches[0].push_back(source(6.123304, 1.027181, 5.178000));
    __patches[0].push_back(source(6.123866, 1.026308, 5.177000));
    __patches[0].push_back(source(6.122789, 1.026357, 5.177000));
    __patches[0].push_back(source(6.125177, 1.026744, 5.179000));
    __patches[0].push_back(source(6.122695, 1.026648, 5.167000));
    __patches[0].push_back(source(6.124850, 1.026987, 5.162000));
    __patches[0].push_back(source(6.122789, 1.026042, 5.163000));
    __patches[0].push_back(source(6.123444, 1.026696, 5.150000));
    __patches[0].push_back(source(6.123164, 1.025799, 5.154000));
    __patches[0].push_back(source(6.122976, 1.027036, 5.110000));
    __patches[0].push_back(source(6.124241, 1.027108, 5.101000));
    __patches[0].push_back(source(6.124989, 1.026284, 5.082000));
    __patches[0].push_back(source(6.122368, 1.026575, 5.061000));
    __patches[0].push_back(source(6.123679, 1.027254, 5.062000));
    __patches[0].push_back(source(6.123257, 1.026139, 5.063000));
    __patches[0].push_back(source(6.123023, 1.026599, 5.069000));
    __patches[0].push_back(source(6.123398, 1.026405, 5.037000));
    __patches[0].push_back(source(6.124100, 1.026672, 5.028000));
    __patches[0].push_back(source(6.123304, 1.025945, 5.025000));
    __patches[0].push_back(source(6.123070, 1.026721, 5.009000));
    __patches[0].push_back(source(6.124053, 1.026405, 4.987000));
    __patches[0].push_back(source(6.124146, 1.025945, 4.979000));
    __patches[0].push_back(source(6.124100, 1.026866, 4.975000));
    __patches[0].push_back(source(6.122789, 1.026939, 4.987000));
    __patches[0].push_back(source(6.125130, 1.026623, 4.955000));
    __patches[0].push_back(source(6.122929, 1.026430, 4.946000));
    __patches[0].push_back(source(6.123725, 1.026187, 4.930000));
    __patches[0].push_back(source(6.123116, 1.027133, 4.929000));
    __patches[0].push_back(source(6.123726, 1.027084, 4.928000));
    __patches[0].push_back(source(6.124521, 1.026236, 4.924000));
    __patches[0].push_back(source(6.123632, 1.026745, 4.892000));
    __patches[0].push_back(source(6.124615, 1.026672, 4.878000));
    __patches[0].push_back(source(6.123772, 1.025921, 4.889000));
    __patches[0].push_back(source(6.124521, 1.026405, 4.866000));
    __patches[0].push_back(source(6.123679, 1.026575, 4.874000));
    __patches[0].push_back(source(6.123913, 1.027230, 4.847000));
    __patches[0].push_back(source(6.124896, 1.026817, 4.837000));
    __patches[0].push_back(source(6.124988, 1.026187, 4.836000));
    __patches[0].push_back(source(6.123070, 1.025896, 4.842000));
    __patches[0].push_back(source(6.122789, 1.026793, 4.832000));
    __patches[0].push_back(source(6.123351, 1.026260, 4.834000));
    __patches[0].push_back(source(6.124006, 1.026769, 4.806000));
    __patches[0].push_back(source(6.125036, 1.026478, 4.797000));
    __patches[0].push_back(source(6.124616, 1.027011, 4.778000));
    __patches[0].push_back(source(6.122930, 1.026163, 4.789000));
    __patches[0].push_back(source(6.124333, 1.026187, 4.756000));
    __patches[0].push_back(source(6.122321, 1.026502, 4.752000));
    __patches[0].push_back(source(6.123538, 1.026915, 4.756000));
    __patches[0].push_back(source(6.124569, 1.026890, 4.737000));
    __patches[0].push_back(source(6.124053, 1.026527, 4.724000));
    __patches[0].push_back(source(6.123819, 1.026066, 4.701000));
    __patches[0].push_back(source(6.122649, 1.026114, 4.699000));
    __patches[0].push_back(source(6.122929, 1.026527, 4.700000));
    __patches[0].push_back(source(6.123491, 1.027205, 4.700000));
    __patches[0].push_back(source(6.122555, 1.026769, 4.680000));
    __patches[0].push_back(source(6.124943, 1.026672, 4.663000));
    __patches[0].push_back(source(6.125082, 1.026356, 4.657000));
    __patches[0].push_back(source(6.122461, 1.026648, 4.653000));
    __patches[0].push_back(source(6.122789, 1.026308, 4.644000));
    __patches[0].push_back(source(6.124147, 1.027011, 4.641000));
    __patches[0].push_back(source(6.123164, 1.025799, 4.625000));
    __patches[0].push_back(source(6.123912, 1.026308, 4.623000));
    __patches[0].push_back(source(6.124381, 1.026696, 4.642000));
    __patches[0].push_back(source(6.123117, 1.026915, 4.620000));
    __patches[0].push_back(source(6.124333, 1.026090, 4.611000));
    __patches[0].push_back(source(6.122368, 1.026381, 4.609000));
    __patches[0].push_back(source(6.123491, 1.026551, 4.596000));
    __patches[0].push_back(source(6.124147, 1.027157, 4.573000));
    __patches[0].push_back(source(6.124802, 1.026381, 4.563000));
    __patches[0].push_back(source(6.125224, 1.026768, 4.568000));
    __patches[0].push_back(source(6.123959, 1.025896, 4.560000));
    __patches[0].push_back(source(6.123257, 1.027011, 4.559000));
    __patches[0].push_back(source(6.123210, 1.026115, 4.523000));
    __patches[0].push_back(source(6.123819, 1.026672, 4.525000));
    __patches[0].push_back(source(6.122602, 1.026527, 4.508000));
    __patches[0].push_back(source(6.124520, 1.025993, 4.513000));
    __patches[0].push_back(source(6.124897, 1.026963, 4.509000));
    __patches[0].push_back(source(6.124755, 1.026502, 4.512000));
    __patches[0].push_back(source(6.123210, 1.025945, 4.493000));
    __patches[0].push_back(source(6.123257, 1.026575, 4.497000));
    __patches[0].push_back(source(6.123585, 1.026284, 4.483000));
    __patches[0].push_back(source(6.124801, 1.026260, 4.481000));
    __patches[0].push_back(source(6.124287, 1.026599, 4.482000));
    __patches[0].push_back(source(6.122695, 1.026890, 4.477000));
    __patches[0].push_back(source(6.123631, 1.025872, 4.444000));
    __patches[0].push_back(source(6.123164, 1.026430, 4.447000));
    __patches[0].push_back(source(6.123726, 1.027254, 4.442000));
    __patches[0].push_back(source(6.122743, 1.026042, 4.423000));
    __patches[0].push_back(source(6.124334, 1.026308, 4.407000));
    __patches[0].push_back(source(6.124662, 1.026769, 4.407000));
    __patches[0].push_back(source(6.123866, 1.026939, 4.406000));
    __patches[0].push_back(source(6.123070, 1.027133, 4.396000));
    __patches[0].push_back(source(6.124428, 1.027011, 4.383000));
    __patches[0].push_back(source(6.122508, 1.026284, 4.385000));
    __patches[0].push_back(source(6.124286, 1.025969, 4.362000));
    __patches[0].push_back(source(6.122930, 1.025945, 4.365000));
    __patches[0].push_back(source(6.122742, 1.026721, 4.349000));
    __patches[0].push_back(source(6.123866, 1.026454, 4.337000));
    __patches[0].push_back(source(6.123304, 1.026818, 4.337000));
    __patches[0].push_back(source(6.123351, 1.025824, 4.327000));
    __patches[0].push_back(source(6.124568, 1.026575, 4.323000));
    __patches[0].push_back(source(6.124801, 1.026114, 4.330000));
    __patches[0].push_back(source(6.123959, 1.026211, 4.325000));
    __patches[0].push_back(source(6.125177, 1.026671, 4.317000));
    __patches[0].push_back(source(6.123398, 1.026333, 4.304000));
    __patches[0].push_back(source(6.123257, 1.027181, 4.283000));
    __patches[0].push_back(source(6.122742, 1.026211, 4.271000));
    __patches[0].push_back(source(6.125083, 1.026550, 4.264000));
    __patches[0].push_back(source(6.123444, 1.026163, 4.263000));
    __patches[0].push_back(source(6.123866, 1.026793, 4.249000));
    __patches[0].push_back(source(6.124053, 1.026090, 4.223000));
    __patches[0].push_back(source(6.124288, 1.027084, 4.209000));
    __patches[0].push_back(source(6.124006, 1.026624, 4.210000));
    __patches[0].push_back(source(6.123585, 1.027011, 4.199000));
    __patches[0].push_back(source(6.122555, 1.026454, 4.186000));
    __patches[0].push_back(source(6.123023, 1.027011, 4.172000));
    __patches[0].push_back(source(6.124334, 1.026866, 4.182000));
    __patches[0].push_back(source(6.124521, 1.026454, 4.173000));
    __patches[0].push_back(source(6.122461, 1.026720, 4.166000));
    __patches[0].push_back(source(6.123725, 1.025993, 4.142000));
    __patches[0].push_back(source(6.123679, 1.027133, 4.153000));
    __patches[0].push_back(source(6.123257, 1.026721, 4.147000));
    __patches[0].push_back(source(6.125037, 1.026865, 4.138000));
    __patches[0].push_back(source(6.123772, 1.026139, 4.129000));
    __patches[0].push_back(source(6.124287, 1.026745, 4.114000));
    __patches[0].push_back(source(6.124801, 1.026235, 4.107000));
    __patches[0].push_back(source(6.123023, 1.026042, 4.104000));
    __patches[0].push_back(source(6.122976, 1.026672, 4.106000));
    __patches[0].push_back(source(6.123725, 1.026357, 4.109000));
    __patches[0].push_back(source(6.122461, 1.026599, 4.076000));
    __patches[0].push_back(source(6.124707, 1.026090, 4.072000));
    __patches[0].push_back(source(6.123772, 1.026551, 4.078000));
    __patches[0].push_back(source(6.123117, 1.026284, 4.083000));
    __patches[0].push_back(source(6.123117, 1.025799, 4.052000));
    __patches[0].push_back(source(6.124007, 1.027205, 4.041000));
    __patches[0].push_back(source(6.123585, 1.026818, 4.042000));
    __patches[0].push_back(source(6.122321, 1.026502, 4.035000));
    __patches[0].push_back(source(6.122789, 1.026939, 4.018000));
    __patches[0].push_back(source(6.124240, 1.026478, 4.019000));
    __patches[0].push_back(source(6.124897, 1.027011, 4.022000));
    __patches[0].push_back(source(6.124942, 1.026526, 4.017000));
    __patches[0].push_back(source(6.123491, 1.027230, 3.997000));
    __patches[0].push_back(source(6.122742, 1.026381, 4.000000));
    __patches[0].push_back(source(6.123491, 1.025848, 3.995000));
    __patches[0].push_back(source(6.122976, 1.026842, 3.991000));
    __patches[0].push_back(source(6.123398, 1.026430, 3.996000));
    __patches[0].push_back(source(6.124146, 1.025921, 3.975000));
    __patches[0].push_back(source(6.125035, 1.026308, 3.975000));
    __patches[0].push_back(source(6.124943, 1.026793, 3.965000));
    __patches[0].push_back(source(6.124007, 1.027084, 3.953000));
    __patches[0].push_back(source(6.123210, 1.026187, 3.966000));
    __patches[0].push_back(source(6.124240, 1.026211, 3.953000));
    __patches[0].push_back(source(6.125036, 1.026405, 3.939000));
    __patches[0].push_back(source(6.124053, 1.026866, 3.930000));
    __patches[0].push_back(source(6.123304, 1.026042, 3.916000));
    __patches[0].push_back(source(6.125224, 1.026793, 3.926000));
    __patches[0].push_back(source(6.124988, 1.026187, 3.914000));
    __patches[0].push_back(source(6.122601, 1.026817, 3.897000));
    __patches[0].push_back(source(6.124006, 1.026381, 3.903000));
    __patches[0].push_back(source(6.122602, 1.026163, 3.891000));
    __patches[0].push_back(source(6.123912, 1.025921, 3.882000));
    __patches[0].push_back(source(6.124663, 1.027035, 3.886000));
    __patches[0].push_back(source(6.123210, 1.026478, 3.876000));
    __patches[0].push_back(source(6.124614, 1.026017, 3.880000));
    __patches[0].push_back(source(6.123679, 1.027278, 3.866000));
    __patches[0].push_back(source(6.123023, 1.027133, 3.853000));
    __patches[0].push_back(source(6.122462, 1.026333, 3.849000));
    __patches[0].push_back(source(6.123772, 1.026648, 3.846000));
    __patches[0].push_back(source(6.124802, 1.026623, 3.831000));
    __patches[0].push_back(source(6.124616, 1.026914, 3.822000));
    __patches[0].push_back(source(6.123491, 1.026648, 3.811000));
    __patches[0].push_back(source(6.123398, 1.027060, 3.816000));
    __patches[0].push_back(source(6.123023, 1.025896, 3.818000));
    __patches[0].push_back(source(6.124240, 1.026357, 3.825000));
    __patches[0].push_back(source(6.122695, 1.026551, 3.810000));
    __patches[0].push_back(source(6.124802, 1.026381, 3.774000));
    __patches[0].push_back(source(6.123585, 1.026066, 3.780000));
    __patches[0].push_back(source(6.123023, 1.026939, 3.771000));
    __patches[0].push_back(source(6.125177, 1.026623, 3.764000));
    __patches[0].push_back(source(6.124147, 1.026672, 3.778000));
    __patches[0].push_back(source(6.122976, 1.026212, 3.748000));
    __patches[0].push_back(source(6.124240, 1.026042, 3.741000));
    __patches[0].push_back(source(6.124147, 1.026963, 3.737000));
    __patches[0].push_back(source(6.122976, 1.026575, 3.728000));
    __patches[0].push_back(source(6.123538, 1.026915, 3.736000));
    __patches[0].push_back(source(6.123585, 1.026236, 3.710000));
    __patches[0].push_back(source(6.124333, 1.025945, 3.703000));
    __patches[0].push_back(source(6.123632, 1.026478, 3.701000));
    __patches[0].push_back(source(6.122929, 1.027036, 3.706000));
    __patches[0].push_back(source(6.124194, 1.027133, 3.702000));
    __patches[0].push_back(source(6.122508, 1.026236, 3.706000));
    __patches[0].push_back(source(6.123632, 1.026745, 3.688000));
    __patches[0].push_back(source(6.123117, 1.025799, 3.666000));
    __patches[0].push_back(source(6.125037, 1.026938, 3.668000));
    __patches[0].push_back(source(6.124521, 1.026308, 3.659000));
    __patches[0].push_back(source(6.122649, 1.026066, 3.661000));
    __patches[0].push_back(source(6.122321, 1.026405, 3.658000));
    __patches[0].push_back(source(6.124662, 1.026769, 3.656000));
    __patches[0].push_back(source(6.124567, 1.026187, 3.643000));
    __patches[0].push_back(source(6.122695, 1.026672, 3.648000));
    __patches[0].push_back(source(6.123304, 1.026260, 3.640000));
    __patches[0].push_back(source(6.123351, 1.027181, 3.644000));
    __patches[0].push_back(source(6.123210, 1.025921, 3.628000));
    __patches[0].push_back(source(6.124474, 1.026599, 3.623000));
    __patches[0].push_back(source(6.122836, 1.026793, 3.603000));
    __patches[0].push_back(source(6.123772, 1.026866, 3.599000));
    __patches[0].push_back(source(6.122929, 1.026430, 3.601000));
    __patches[0].push_back(source(6.123866, 1.026284, 3.597000));
    __patches[0].push_back(source(6.125224, 1.026720, 3.588000));
    __patches[0].push_back(source(6.123678, 1.025872, 3.586000));
    __patches[0].push_back(source(6.123959, 1.026502, 3.588000));
    __patches[0].push_back(source(6.124380, 1.026114, 3.590000));
    __patches[0].push_back(source(6.124054, 1.027205, 3.560000));
    __patches[0].push_back(source(6.123913, 1.026721, 3.549000));
    __patches[0].push_back(source(6.123351, 1.025799, 3.538000));
    __patches[0].push_back(source(6.124989, 1.026478, 3.530000));
    __patches[0].push_back(source(6.124756, 1.026914, 3.528000));
    __patches[0].push_back(source(6.124895, 1.026138, 3.516000));
    __patches[0].push_back(source(6.122321, 1.026526, 3.514000));
    __patches[0].push_back(source(6.122695, 1.026914, 3.501000));
    __patches[0].push_back(source(6.123164, 1.026042, 3.498000));
    __patches[0].push_back(source(6.124052, 1.025872, 3.497000));
    __patches[0].push_back(source(6.124943, 1.026672, 3.479000));
    __patches[0].push_back(source(6.122836, 1.025993, 3.470000));
    __patches[0].push_back(source(6.123117, 1.026381, 3.469000));
    __patches[0].push_back(source(6.123772, 1.027036, 3.470000));
    __patches[0].push_back(source(6.123819, 1.027254, 3.450000));
    __patches[0].push_back(source(6.124616, 1.027035, 3.447000));
    __patches[0].push_back(source(6.122508, 1.026745, 3.447000));
    __patches[0].push_back(source(6.124381, 1.026745, 3.430000));
    __patches[0].push_back(source(6.123070, 1.026696, 3.430000));
    __patches[0].push_back(source(6.123912, 1.026042, 3.431000));
    __patches[0].push_back(source(6.124427, 1.026381, 3.421000));
    __patches[0].push_back(source(6.125224, 1.026817, 3.403000));
    __patches[0].push_back(source(6.123210, 1.027181, 3.416000));
    __patches[0].push_back(source(6.124335, 1.027011, 3.406000));
    __patches[0].push_back(source(6.124568, 1.026502, 3.389000));
    __patches[0].push_back(source(6.123210, 1.026575, 3.387000));
    __patches[0].push_back(source(6.123257, 1.026939, 3.378000));
    __patches[0].push_back(source(6.124288, 1.026890, 3.380000));
    __patches[0].push_back(source(6.125177, 1.026574, 3.372000));
    __patches[0].push_back(source(6.122836, 1.026333, 3.365000));
    __patches[0].push_back(source(6.123725, 1.026163, 3.371000));
    __patches[0].push_back(source(6.124567, 1.025993, 3.377000));
    __patches[0].push_back(source(6.122508, 1.026308, 3.351000));
    __patches[0].push_back(source(6.123679, 1.027157, 3.355000));
    __patches[0].push_back(source(6.124897, 1.027035, 3.352000));
    __patches[0].push_back(source(6.122461, 1.026623, 3.352000));
    __patches[0].push_back(source(6.124895, 1.026308, 3.334000));
    __patches[0].push_back(source(6.123164, 1.025775, 3.333000));
    __patches[0].push_back(source(6.123725, 1.026381, 3.326000));
    __patches[0].push_back(source(6.124147, 1.026793, 3.345000));
    __patches[0].push_back(source(6.122930, 1.026139, 3.305000));
    __patches[0].push_back(source(6.123070, 1.026818, 3.289000));
    __patches[0].push_back(source(6.124288, 1.027108, 3.294000));
    __patches[0].push_back(source(6.123632, 1.026769, 3.279000));
    __patches[0].push_back(source(6.123538, 1.027230, 3.286000));
    __patches[0].push_back(source(6.124427, 1.026260, 3.283000));
    __patches[0].push_back(source(6.125083, 1.026405, 3.267000));
    __patches[0].push_back(source(6.123818, 1.025872, 3.263000));
    __patches[0].push_back(source(6.124474, 1.026090, 3.263000));
    __patches[0].push_back(source(6.122508, 1.026454, 3.268000));
    __patches[0].push_back(source(6.123210, 1.026163, 3.272000));
    __patches[0].push_back(source(6.123585, 1.027011, 3.267000));
    __patches[0].push_back(source(6.124240, 1.026502, 3.267000));
    __patches[0].push_back(source(6.123070, 1.027133, 3.256000));
    __patches[0].push_back(source(6.123632, 1.026599, 3.249000));
    __patches[0].push_back(source(6.122601, 1.026842, 3.222000));
    __patches[0].push_back(source(6.122930, 1.025921, 3.213000));
    __patches[0].push_back(source(6.124194, 1.026672, 3.213000));
    __patches[0].push_back(source(6.124006, 1.026187, 3.210000));
    __patches[0].push_back(source(6.123444, 1.026527, 3.209000));
    __patches[0].push_back(source(6.124988, 1.026187, 3.202000));
    __patches[0].push_back(source(6.124333, 1.025920, 3.194000));
    __patches[0].push_back(source(6.123398, 1.026066, 3.187000));
    __patches[0].push_back(source(6.125224, 1.026696, 3.177000));
    __patches[0].push_back(source(6.122929, 1.026648, 3.178000));
    __patches[0].push_back(source(6.125082, 1.026308, 3.180000));
    __patches[0].push_back(source(6.123959, 1.026381, 3.176000));
    __patches[0].push_back(source(6.122929, 1.026502, 3.163000));
    __patches[0].push_back(source(6.122602, 1.026163, 3.162000));
    __patches[0].push_back(source(6.125084, 1.026914, 3.160000));
    __patches[0].push_back(source(6.122321, 1.026429, 3.151000));
    __patches[0].push_back(source(6.123210, 1.027036, 3.144000));
    __patches[0].push_back(source(6.123398, 1.026357, 3.134000));
    __patches[0].push_back(source(6.124615, 1.026623, 3.127000));
    __patches[0].push_back(source(6.123772, 1.026042, 3.119000));
    __patches[0].push_back(source(6.122696, 1.026042, 3.124000));
    __patches[0].push_back(source(6.124147, 1.027036, 3.120000));
    __patches[0].push_back(source(6.123070, 1.026915, 3.104000));
    __patches[0].push_back(source(6.124053, 1.026866, 3.097000));
    __patches[0].push_back(source(6.123351, 1.025799, 3.095000));
    __patches[0].push_back(source(6.124707, 1.026066, 3.104000));
    __patches[0].push_back(source(6.124053, 1.026575, 3.092000));
    __patches[0].push_back(source(6.123491, 1.026236, 3.083000));
    __patches[0].push_back(source(6.124756, 1.026769, 3.077000));
    __patches[0].push_back(source(6.123679, 1.027278, 3.064000));
    __patches[0].push_back(source(6.122368, 1.026575, 3.069000));
    __patches[0].push_back(source(6.123210, 1.025921, 3.070000));
    __patches[0].push_back(source(6.122461, 1.026720, 3.052000));
    __patches[0].push_back(source(6.124193, 1.025872, 3.042000));
    __patches[0].push_back(source(6.122929, 1.027060, 3.041000));
    __patches[0].push_back(source(6.125224, 1.026817, 3.043000));
    __patches[0].push_back(source(6.124615, 1.026502, 3.049000));
    __patches[0].push_back(source(6.123444, 1.026648, 3.031000));
    __patches[0].push_back(source(6.123866, 1.026987, 3.023000));
    __patches[0].push_back(source(6.122789, 1.026405, 3.023000));
    __patches[0].push_back(source(6.122789, 1.026260, 3.015000));
    __patches[0].push_back(source(6.124333, 1.026211, 3.007000));
    __patches[0].push_back(source(6.124756, 1.026914, 3.004000));
    __patches[0].push_back(source(6.124147, 1.027181, 3.000000));
    __patches[0].push_back(source(6.123866, 1.026308, 3.001000));
    __patches[0].push_back(source(6.123117, 1.025775, 2.986000));
    __patches[0].push_back(source(6.124943, 1.026550, 2.985000));
    __patches[0].push_back(source(6.122789, 1.026963, 2.988000));
    __patches[0].push_back(source(6.123866, 1.026502, 2.968000));
    __patches[0].push_back(source(6.123725, 1.026866, 2.964000));
    __patches[0].push_back(source(6.123959, 1.025945, 2.964000));
    __patches[0].push_back(source(6.124661, 1.026187, 2.963000));
    __patches[0].push_back(source(6.122648, 1.026575, 2.964000));
    __patches[0].push_back(source(6.125177, 1.026599, 2.947000));
    __patches[0].push_back(source(6.124803, 1.027035, 2.948000));
    __patches[0].push_back(source(6.124099, 1.026090, 2.934000));
    __patches[0].push_back(source(6.123538, 1.026478, 2.938000));
    __patches[0].push_back(source(6.122883, 1.025993, 2.927000));
    __patches[0].push_back(source(6.123444, 1.027133, 2.931000));
    __patches[0].push_back(source(6.124520, 1.025945, 2.925000));
    __patches[0].push_back(source(6.124381, 1.026914, 2.915000));
    __patches[0].push_back(source(6.122462, 1.026308, 2.913000));
    __patches[0].push_back(source(6.123023, 1.026672, 2.905000));
    __patches[0].push_back(source(6.123304, 1.026260, 2.901000));
    __patches[0].push_back(source(6.123538, 1.025872, 2.899000));
    __patches[0].push_back(source(6.124568, 1.026381, 2.890000));
    __patches[0].push_back(source(6.123585, 1.026745, 2.882000));
    __patches[0].push_back(source(6.125037, 1.026987, 2.872000));
    __patches[0].push_back(source(6.123023, 1.027157, 2.867000));
    __patches[0].push_back(source(6.123070, 1.026042, 2.853000));
    __patches[0].push_back(source(6.123679, 1.027133, 2.860000));
    __patches[0].push_back(source(6.124194, 1.026769, 2.851000));
    __patches[0].push_back(source(6.125036, 1.026453, 2.849000));
    __patches[0].push_back(source(6.123210, 1.026793, 2.853000));
    __patches[0].push_back(source(6.125082, 1.026308, 2.838000));
    __patches[0].push_back(source(6.123818, 1.025848, 2.833000));
    __patches[0].push_back(source(6.124335, 1.027108, 2.832000));
    __patches[0].push_back(source(6.124990, 1.026817, 2.832000));
    __patches[0].push_back(source(6.122976, 1.026793, 2.823000));
    __patches[0].push_back(source(6.124848, 1.026114, 2.817000));
    __patches[0].push_back(source(6.123772, 1.026187, 2.818000));
    __patches[0].push_back(source(6.124100, 1.026672, 2.808000));
    __patches[0].push_back(source(6.123257, 1.027181, 2.811000));
    __patches[0].push_back(source(6.122321, 1.026526, 2.805000));
    __patches[0].push_back(source(6.122976, 1.026551, 2.802000));
    __patches[0].push_back(source(6.123257, 1.026163, 2.795000));
    __patches[0].push_back(source(6.122649, 1.026066, 2.789000));
    __patches[0].push_back(source(6.124568, 1.026623, 2.777000));
    __patches[0].push_back(source(6.124193, 1.025993, 2.770000));
    __patches[0].push_back(source(6.124802, 1.026332, 2.776000));
    __patches[0].push_back(source(6.125224, 1.026720, 2.776000));
    __patches[0].push_back(source(6.123913, 1.027230, 2.767000));
    __patches[0].push_back(source(6.122368, 1.026381, 2.752000));
    __patches[0].push_back(source(6.123491, 1.026866, 2.751000));
    __patches[0].push_back(source(6.123398, 1.025993, 2.747000));
    __patches[0].push_back(source(6.124334, 1.026405, 2.741000));
    __patches[0].push_back(source(6.123210, 1.026430, 2.740000));
    __patches[0].push_back(source(6.123538, 1.027254, 2.741000));
    __patches[0].push_back(source(6.122555, 1.026793, 2.736000));
    __patches[0].push_back(source(6.124475, 1.026842, 2.732000));
    __patches[0].push_back(source(6.123585, 1.027011, 2.731000));
    __patches[0].push_back(source(6.123772, 1.026624, 2.732000));
    __patches[0].push_back(source(6.122555, 1.026187, 2.720000));
    __patches[0].push_back(source(6.124146, 1.026284, 2.702000));
    __patches[0].push_back(source(6.123023, 1.025872, 2.702000));
    __patches[0].push_back(source(6.123210, 1.026309, 2.706000));
    __patches[0].push_back(source(6.124569, 1.027060, 2.703000));
    __patches[0].push_back(source(6.123960, 1.027084, 2.695000));
    __patches[0].push_back(source(6.124521, 1.026720, 2.700000));
    __patches[0].push_back(source(6.122461, 1.026648, 2.683000));
    __patches[0].push_back(source(6.122929, 1.027060, 2.682000));
    __patches[0].push_back(source(6.124099, 1.025872, 2.686000));
    __patches[0].push_back(source(6.124943, 1.026599, 2.668000));
    __patches[0].push_back(source(6.123632, 1.026284, 2.662000));
    __patches[0].push_back(source(6.122649, 1.026357, 2.650000));
    __patches[0].push_back(source(6.123960, 1.026793, 2.653000));
    __patches[0].push_back(source(6.123444, 1.025824, 2.653000));
    __patches[0].push_back(source(6.122648, 1.026890, 2.652000));
    __patches[0].push_back(source(6.124427, 1.026090, 2.644000));
    __patches[0].push_back(source(6.124944, 1.027035, 2.641000));
    __patches[0].push_back(source(6.123913, 1.026478, 2.633000));
    __patches[0].push_back(source(6.124988, 1.026163, 2.628000));
    __patches[0].push_back(source(6.123444, 1.026115, 2.625000));
    __patches[0].push_back(source(6.123444, 1.026599, 2.622000));
    __patches[0].push_back(source(6.124896, 1.026720, 2.611000));
    __patches[0].push_back(source(6.123351, 1.026939, 2.611000));
    __patches[0].push_back(source(6.124054, 1.027205, 2.612000));
    __patches[0].push_back(source(6.124380, 1.025945, 2.610000));
    __patches[0].push_back(source(6.123117, 1.025775, 2.603000));
    __patches[0].push_back(source(6.122555, 1.026478, 2.584000));
    __patches[0].push_back(source(6.122976, 1.026915, 2.585000));
    __patches[0].push_back(source(6.123960, 1.026914, 2.589000));
    __patches[0].push_back(source(6.125224, 1.026817, 2.585000));
    __patches[0].push_back(source(6.124474, 1.026284, 2.590000));
    __patches[0].push_back(source(6.123912, 1.026284, 2.576000));
    __patches[0].push_back(source(6.125130, 1.026550, 2.572000));
    __patches[0].push_back(source(6.122976, 1.026236, 2.562000));
    __patches[0].push_back(source(6.123772, 1.026042, 2.561000));
    __patches[0].push_back(source(6.124521, 1.026551, 2.563000));
    __patches[0].push_back(source(6.124335, 1.026987, 2.566000));
    __patches[0].push_back(source(6.123023, 1.027157, 2.548000));
    __patches[0].push_back(source(6.122695, 1.026745, 2.560000));
    __patches[0].push_back(source(6.124660, 1.025993, 2.556000));
    __patches[0].push_back(source(6.125036, 1.026429, 2.537000));
    __patches[0].push_back(source(6.123632, 1.026769, 2.533000));
    __patches[0].push_back(source(6.122274, 1.026454, 2.534000));
    __patches[0].push_back(source(6.123444, 1.026405, 2.537000));
    __patches[0].push_back(source(6.122883, 1.026139, 2.527000));
    __patches[0].push_back(source(6.124990, 1.026914, 2.517000));
    __patches[0].push_back(source(6.123491, 1.027230, 2.510000));
    __patches[0].push_back(source(6.124801, 1.026235, 2.512000));
    __patches[0].push_back(source(6.123725, 1.025824, 2.507000));
    __patches[0].push_back(source(6.122836, 1.026527, 2.509000));
    __patches[0].push_back(source(6.124754, 1.026090, 2.499000));
    __patches[0].push_back(source(6.124053, 1.026405, 2.501000));
    __patches[0].push_back(source(6.122883, 1.025969, 2.498000));
    __patches[0].push_back(source(6.122836, 1.026405, 2.496000));
    __patches[0].push_back(source(6.123773, 1.027278, 2.491000));
    __patches[0].push_back(source(6.123679, 1.026866, 2.484000));
    __patches[0].push_back(source(6.122789, 1.026963, 2.468000));
    __patches[0].push_back(source(6.124146, 1.026139, 2.456000));
    __patches[0].push_back(source(6.124569, 1.026963, 2.457000));
    __patches[0].push_back(source(6.124193, 1.026527, 2.465000));
    __patches[0].push_back(source(6.123304, 1.026696, 2.465000));
    __patches[0].push_back(source(6.125224, 1.026647, 2.456000));
    __patches[0].push_back(source(6.123164, 1.025921, 2.442000));
    __patches[0].push_back(source(6.124052, 1.025848, 2.437000));
    __patches[0].push_back(source(6.122836, 1.026818, 2.436000));
    __patches[0].push_back(source(6.123491, 1.027084, 2.438000));
    __patches[0].push_back(source(6.123491, 1.026478, 2.427000));
    __patches[0].push_back(source(6.125082, 1.026332, 2.411000));
    __patches[0].push_back(source(6.124053, 1.026648, 2.408000));
    __patches[0].push_back(source(6.122508, 1.026236, 2.403000));
    __patches[0].push_back(source(6.122368, 1.026575, 2.400000));
    __patches[0].push_back(source(6.124568, 1.026478, 2.403000));
    __patches[0].push_back(source(6.122976, 1.026672, 2.400000));
    __patches[0].push_back(source(6.124288, 1.027133, 2.400000));
    __patches[0].push_back(source(6.124380, 1.026187, 2.391000));
    __patches[0].push_back(source(6.123491, 1.026260, 2.389000));
    __patches[0].push_back(source(6.124756, 1.026793, 2.390000));
    __patches[0].push_back(source(6.123257, 1.025775, 2.388000));
    __patches[0].push_back(source(6.124239, 1.025872, 2.379000));
    __patches[0].push_back(source(6.122415, 1.026333, 2.384000));
    __patches[0].push_back(source(6.123866, 1.026721, 2.373000));
    __patches[0].push_back(source(6.123912, 1.026115, 2.370000));
    __patches[0].push_back(source(6.124755, 1.026575, 2.368000));
    __patches[0].push_back(source(6.123725, 1.026454, 2.368000));
    __patches[0].push_back(source(6.123304, 1.026066, 2.361000));
    __patches[0].push_back(source(6.123210, 1.026890, 2.357000));
    __patches[0].push_back(source(6.122461, 1.026720, 2.350000));
    __patches[0].push_back(source(6.123023, 1.026333, 2.345000));
    __patches[0].push_back(source(6.123257, 1.027181, 2.344000));
    __patches[0].push_back(source(6.122696, 1.026042, 2.345000));
    __patches[0].push_back(source(6.124147, 1.026842, 2.334000));
    __patches[0].push_back(source(6.124756, 1.026914, 2.327000));
    __patches[0].push_back(source(6.123210, 1.027036, 2.319000));
    __patches[0].push_back(source(6.123117, 1.026575, 2.319000));
    __patches[0].push_back(source(6.123818, 1.025945, 2.302000));
    __patches[0].push_back(source(6.124100, 1.027084, 2.303000));
    __patches[0].push_back(source(6.124427, 1.026017, 2.294000));
    __patches[0].push_back(source(6.125224, 1.026793, 2.292000));
    __patches[0].push_back(source(6.123210, 1.026187, 2.285000));
    __patches[0].push_back(source(6.124755, 1.026381, 2.279000));
    __patches[0].push_back(source(6.124991, 1.027035, 2.281000));
    __patches[0].push_back(source(6.123538, 1.025921, 2.285000));
    __patches[0].push_back(source(6.124241, 1.026745, 2.277000));
    __patches[0].push_back(source(6.123679, 1.027302, 2.277000));
    __patches[0].push_back(source(6.123819, 1.027060, 2.268000));
    __patches[0].push_back(source(6.122602, 1.026624, 2.272000));
    __patches[0].push_back(source(6.123632, 1.026648, 2.272000));
    __patches[0].push_back(source(6.125084, 1.026914, 2.271000));
    __patches[0].push_back(source(6.124567, 1.025945, 2.269000));
    __patches[0].push_back(source(6.123023, 1.025848, 2.264000));
    __patches[0].push_back(source(6.123070, 1.027181, 2.266000));
    __patches[0].push_back(source(6.122554, 1.026842, 2.261000));
    __patches[0].push_back(source(6.124428, 1.027060, 2.260000));
    __patches[0].push_back(source(6.122742, 1.026187, 2.262000));
    __patches[0].push_back(source(6.124988, 1.026163, 2.253000));
    __patches[0].push_back(source(6.124006, 1.026211, 2.250000));
    __patches[0].push_back(source(6.124381, 1.026599, 2.259000));
    __patches[0].push_back(source(6.125130, 1.026550, 2.241000));
    __patches[0].push_back(source(6.123117, 1.026745, 2.231000));
    __patches[0].push_back(source(6.123398, 1.026357, 2.230000));
    __patches[0].push_back(source(6.123679, 1.027157, 2.219000));
    __patches[0].push_back(source(6.122836, 1.026284, 2.219000));
    __patches[0].push_back(source(6.122274, 1.026478, 2.207000));
    __patches[0].push_back(source(6.123117, 1.026018, 2.210000));
    __patches[0].push_back(source(6.124099, 1.025993, 2.215000));
    __patches[0].push_back(source(6.124474, 1.026357, 2.210000));
    __patches[0].push_back(source(6.123866, 1.026308, 2.198000));
    __patches[0].push_back(source(6.122882, 1.027036, 2.196000));
    __patches[0].push_back(source(6.124241, 1.026987, 2.195000));
    __patches[0].push_back(source(6.124990, 1.026672, 2.193000));
    __patches[0].push_back(source(6.123538, 1.026963, 2.179000));
    __patches[0].push_back(source(6.123398, 1.026527, 2.183000));
    __patches[0].push_back(source(6.124147, 1.027205, 2.175000));
    __patches[0].push_back(source(6.123117, 1.025775, 2.169000));
    __patches[0].push_back(source(6.123725, 1.026090, 2.173000));
    __patches[0].push_back(source(6.122602, 1.026114, 2.173000));
    __patches[0].push_back(source(6.123772, 1.025824, 2.160000));
    __patches[0].push_back(source(6.125035, 1.026284, 2.157000));
    __patches[0].push_back(source(6.122368, 1.026381, 2.150000));
    __patches[0].push_back(source(6.123959, 1.026405, 2.147000));
    __patches[0].push_back(source(6.125224, 1.026671, 2.144000));
    __patches[0].push_back(source(6.124428, 1.026842, 2.147000));
    __patches[0].push_back(source(6.122695, 1.026914, 2.143000));
    __patches[0].push_back(source(6.124801, 1.026090, 2.133000));
    __patches[0].push_back(source(6.122883, 1.026430, 2.139000));
    __patches[0].push_back(source(6.123959, 1.026551, 2.134000));
    __patches[0].push_back(source(6.125083, 1.026429, 2.131000));
    __patches[0].push_back(source(6.123725, 1.026890, 2.132000));
    __patches[0].push_back(source(6.124333, 1.025920, 2.123000));
    __patches[0].push_back(source(6.124663, 1.027060, 2.124000));
    __patches[0].push_back(source(6.122695, 1.026745, 2.116000));
    __patches[0].push_back(source(6.123491, 1.026139, 2.112000));
    __patches[0].push_back(source(6.123257, 1.026793, 2.106000));
    __patches[0].push_back(source(6.123491, 1.027230, 2.106000));
    __patches[0].push_back(source(6.123211, 1.025896, 2.105000));
    __patches[0].push_back(source(6.124848, 1.026260, 2.095000));
    __patches[0].push_back(source(6.124709, 1.026672, 2.091000));
    __patches[0].push_back(source(6.124193, 1.026308, 2.092000));
    __patches[0].push_back(source(6.122555, 1.026478, 2.084000));
    __patches[0].push_back(source(6.123913, 1.026769, 2.092000));
    __patches[0].push_back(source(6.123491, 1.025824, 2.072000));
    __patches[0].push_back(source(6.124193, 1.026454, 2.069000));
    __patches[0].push_back(source(6.123210, 1.026454, 2.070000));
    __patches[0].push_back(source(6.123866, 1.026963, 2.068000));
    __patches[0].push_back(source(6.123866, 1.027254, 2.055000));
    __patches[0].push_back(source(6.122883, 1.025969, 2.059000));
    __patches[0].push_back(source(6.123116, 1.027060, 2.052000));
    __patches[0].push_back(source(6.124707, 1.026017, 2.045000));
    __patches[0].push_back(source(6.123491, 1.026769, 2.044000));
    __patches[0].push_back(source(6.124897, 1.027035, 2.046000));
    __patches[0].push_back(source(6.122602, 1.026308, 2.038000));
    __patches[0].push_back(source(6.123023, 1.027181, 2.035000));
    __patches[0].push_back(source(6.123304, 1.026284, 2.034000));
    __patches[0].push_back(source(6.125271, 1.026817, 2.036000));
    __patches[0].push_back(source(6.124614, 1.026187, 2.032000));
    __patches[0].push_back(source(6.124382, 1.027132, 2.033000));
    __patches[0].push_back(source(6.124428, 1.026696, 2.033000));
    __patches[0].push_back(source(6.124099, 1.025824, 2.031000));
    __patches[0].push_back(source(6.122977, 1.025872, 2.035000));
    __patches[0].push_back(source(6.122367, 1.026648, 2.033000));
    __patches[0].push_back(source(6.124943, 1.026550, 2.029000));
    __patches[0].push_back(source(6.123070, 1.026915, 2.021000));
    __patches[0].push_back(source(6.123772, 1.026599, 2.022000));
    __patches[0].push_back(source(6.124333, 1.026187, 2.016000));
    __patches[0].push_back(source(6.123725, 1.026187, 2.014000));
    __patches[0].push_back(source(6.124615, 1.026769, 1.997000));
    __patches[0].push_back(source(6.122883, 1.026551, 1.994000));
    __patches[0].push_back(source(6.122696, 1.026042, 1.983000));
    __patches[0].push_back(source(6.124006, 1.025945, 1.979000));
    __patches[0].push_back(source(6.123117, 1.026357, 1.970000));
    __patches[0].push_back(source(6.125084, 1.026890, 1.970000));
    __patches[0].push_back(source(6.122929, 1.026818, 1.970000));
    __patches[0].push_back(source(6.122274, 1.026526, 1.970000));
    __patches[0].push_back(source(6.123304, 1.027205, 1.973000));
    __patches[0].push_back(source(6.124568, 1.026454, 1.963000));
    __patches[0].push_back(source(6.122555, 1.026187, 1.966000));
    __patches[0].push_back(source(6.125271, 1.026696, 1.957000));
    __patches[0].push_back(source(6.122929, 1.026672, 1.952000));
    __patches[0].push_back(source(6.123678, 1.026333, 1.952000));
    __patches[0].push_back(source(6.124381, 1.026914, 1.950000));
    __patches[0].push_back(source(6.123679, 1.027302, 1.945000));
    __patches[0].push_back(source(6.123351, 1.026939, 1.942000));
    __patches[0].push_back(source(6.125223, 1.026574, 1.943000));
    __patches[0].push_back(source(6.123304, 1.025775, 1.940000));
    __patches[0].push_back(source(6.124474, 1.026090, 1.937000));
    __patches[0].push_back(source(6.123398, 1.027108, 1.934000));
    __patches[0].push_back(source(6.123585, 1.025945, 1.924000));
    __patches[0].push_back(source(6.122977, 1.026042, 1.920000));
    __patches[0].push_back(source(6.124615, 1.026575, 1.919000));
    __patches[0].push_back(source(6.123351, 1.026672, 1.925000));
    __patches[0].push_back(source(6.125035, 1.026187, 1.916000));
    __patches[0].push_back(source(6.124006, 1.026866, 1.916000));
    __patches[0].push_back(source(6.122742, 1.026381, 1.914000));
    __patches[0].push_back(source(6.122508, 1.026793, 1.910000));
    __patches[0].push_back(source(6.125037, 1.027035, 1.908000));
    __patches[0].push_back(source(6.124567, 1.025920, 1.912000));
    __patches[0].push_back(source(6.124241, 1.027181, 1.912000));
    __patches[0].push_back(source(6.123913, 1.026478, 1.909000));
    __patches[0].push_back(source(6.123678, 1.026042, 1.906000));
    __patches[0].push_back(source(6.123210, 1.026187, 1.903000));
    __patches[0].push_back(source(6.122789, 1.026987, 1.892000));
    __patches[0].push_back(source(6.125082, 1.026356, 1.894000));
    __patches[0].push_back(source(6.122648, 1.026575, 1.888000));
    __patches[0].push_back(source(6.124100, 1.026672, 1.883000));
    __patches[0].push_back(source(6.124521, 1.026260, 1.887000));
    __patches[0].push_back(source(6.125084, 1.026793, 1.879000));
    __patches[0].push_back(source(6.123726, 1.027133, 1.878000));
    __patches[0].push_back(source(6.124569, 1.026963, 1.875000));
    __patches[0].push_back(source(6.122415, 1.026284, 1.868000));
    __patches[0].push_back(source(6.123725, 1.025799, 1.863000));
    __patches[0].push_back(source(6.124522, 1.027084, 1.860000));
    __patches[0].push_back(source(6.124239, 1.025993, 1.854000));
    __patches[0].push_back(source(6.123679, 1.026769, 1.851000));
    __patches[0].push_back(source(6.123912, 1.026284, 1.840000));
    __patches[0].push_back(source(6.123117, 1.025775, 1.838000));
    __patches[0].push_back(source(6.123585, 1.027011, 1.834000));
    __patches[0].push_back(source(6.122882, 1.027060, 1.828000));
    __patches[0].push_back(source(6.124849, 1.026381, 1.831000));
    __patches[0].push_back(source(6.123070, 1.026599, 1.831000));
    __patches[0].push_back(source(6.124100, 1.026551, 1.818000));
    __patches[0].push_back(source(6.123491, 1.026236, 1.820000));
    __patches[0].push_back(source(6.124005, 1.025824, 1.812000));
    __patches[0].push_back(source(6.122321, 1.026405, 1.816000));
    __patches[0].push_back(source(6.124007, 1.027230, 1.805000));
    __patches[0].push_back(source(6.124193, 1.026090, 1.807000));
    __patches[0].push_back(source(6.123538, 1.026648, 1.807000));
    __patches[0].push_back(source(6.122930, 1.025896, 1.803000));
    __patches[0].push_back(source(6.122601, 1.026866, 1.799000));
    __patches[0].push_back(source(6.125083, 1.026478, 1.799000));
    __patches[0].push_back(source(6.123398, 1.026381, 1.797000));
    __patches[0].push_back(source(6.124754, 1.026017, 1.793000));
    __patches[0].push_back(source(6.123070, 1.027181, 1.788000));
    __patches[0].push_back(source(6.124335, 1.027036, 1.793000));
    __patches[0].push_back(source(6.124239, 1.025872, 1.790000));
    __patches[0].push_back(source(6.124756, 1.026817, 1.783000));
    __patches[0].push_back(source(6.122602, 1.026648, 1.785000));
    __patches[0].push_back(source(6.122789, 1.026187, 1.781000));
    __patches[0].push_back(source(6.124895, 1.026114, 1.774000));
    __patches[0].push_back(source(6.123304, 1.026042, 1.771000));
    __patches[0].push_back(source(6.123679, 1.026866, 1.774000));
    __patches[0].push_back(source(6.123444, 1.026527, 1.768000));
    __patches[0].push_back(source(6.123491, 1.027254, 1.762000));
    __patches[0].push_back(source(6.125271, 1.026768, 1.763000));
    __patches[0].push_back(source(6.124007, 1.027108, 1.761000));
    __patches[0].push_back(source(6.124427, 1.026357, 1.756000));
    __patches[0].push_back(source(6.124241, 1.026745, 1.756000));
    __patches[0].push_back(source(6.123912, 1.026115, 1.750000));
    __patches[0].push_back(source(6.122367, 1.026696, 1.744000));
    __patches[0].push_back(source(6.124194, 1.026914, 1.742000));
    __patches[0].push_back(source(6.122930, 1.026260, 1.738000));
    __patches[0].push_back(source(6.125084, 1.026938, 1.738000));
    __patches[0].push_back(source(6.123351, 1.025799, 1.739000));
    __patches[0].push_back(source(6.123678, 1.026454, 1.728000));
    __patches[0].push_back(source(6.122368, 1.026575, 1.732000));
    __patches[0].push_back(source(6.124568, 1.026478, 1.726000));
    __patches[0].push_back(source(6.123023, 1.026721, 1.721000));
    __patches[0].push_back(source(6.125270, 1.026623, 1.724000));
    __patches[0].push_back(source(6.122509, 1.026163, 1.719000));
    __patches[0].push_back(source(6.123491, 1.026115, 1.718000));
    __patches[0].push_back(source(6.123819, 1.026987, 1.718000));
    __patches[0].push_back(source(6.124193, 1.026308, 1.713000));
    __patches[0].push_back(source(6.124709, 1.026648, 1.707000));
    __patches[0].push_back(source(6.123679, 1.027327, 1.701000));
    __patches[0].push_back(source(6.122976, 1.027108, 1.702000));
    __patches[0].push_back(source(6.123117, 1.026478, 1.700000));
    __patches[0].push_back(source(6.123210, 1.025921, 1.697000));
    __patches[0].push_back(source(6.123866, 1.026745, 1.694000));
    __patches[0].push_back(source(6.124614, 1.025945, 1.682000));
    __patches[0].push_back(source(6.124991, 1.027059, 1.683000));
    __patches[0].push_back(source(6.122742, 1.026963, 1.679000));
    __patches[0].push_back(source(6.122649, 1.026066, 1.677000));
    __patches[0].push_back(source(6.123865, 1.025921, 1.683000));
    __patches[0].push_back(source(6.125083, 1.026405, 1.677000));
    __patches[0].push_back(source(6.124428, 1.026623, 1.668000));
    __patches[0].push_back(source(6.124287, 1.026187, 1.673000));
    __patches[0].push_back(source(6.122227, 1.026502, 1.673000));
    __patches[0].push_back(source(6.124848, 1.026235, 1.666000));
    __patches[0].push_back(source(6.124990, 1.026672, 1.667000));
    __patches[0].push_back(source(6.124756, 1.027060, 1.661000));
    __patches[0].push_back(source(6.122836, 1.026842, 1.664000));
    __patches[0].push_back(source(6.123772, 1.025799, 1.663000));
    __patches[0].push_back(source(6.123257, 1.026212, 1.662000));
    __patches[0].push_back(source(6.123491, 1.026818, 1.654000));
    __patches[0].push_back(source(6.123959, 1.026430, 1.654000));
    __patches[0].push_back(source(6.122836, 1.025969, 1.651000));
    __patches[0].push_back(source(6.122883, 1.026430, 1.656000));
    __patches[0].push_back(source(6.123444, 1.027108, 1.641000));
    __patches[0].push_back(source(6.124943, 1.026890, 1.642000));
    __patches[0].push_back(source(6.124380, 1.025896, 1.635000));
    __patches[0].push_back(source(6.124147, 1.027205, 1.637000));
    __patches[0].push_back(source(6.122368, 1.026381, 1.631000));
    __patches[0].push_back(source(6.123444, 1.026939, 1.629000));
    __patches[0].push_back(source(6.123819, 1.026624, 1.626000));
    __patches[0].push_back(source(6.124006, 1.026211, 1.625000));
    __patches[0].push_back(source(6.125082, 1.026308, 1.627000));
    __patches[0].push_back(source(6.124989, 1.026575, 1.618000));
    __patches[0].push_back(source(6.123538, 1.025824, 1.620000));
    __patches[0].push_back(source(6.125082, 1.026162, 1.612000));
    __patches[0].push_back(source(6.122508, 1.026769, 1.607000));
    __patches[0].push_back(source(6.124240, 1.026430, 1.608000));
    __patches[0].push_back(source(6.122977, 1.026139, 1.612000));
    __patches[0].push_back(source(6.124428, 1.026842, 1.604000));
    __patches[0].push_back(source(6.122415, 1.026260, 1.606000));
    __patches[0].push_back(source(6.123959, 1.026018, 1.602000));
    __patches[0].push_back(source(6.123491, 1.027230, 1.596000));
    __patches[0].push_back(source(6.122929, 1.026551, 1.596000));
    __patches[0].push_back(source(6.123211, 1.025751, 1.591000));
    __patches[0].push_back(source(6.124147, 1.026648, 1.589000));
    __patches[0].push_back(source(6.124475, 1.027108, 1.590000));
    __patches[0].push_back(source(6.123164, 1.026357, 1.588000));
    __patches[0].push_back(source(6.123257, 1.026793, 1.584000));
    __patches[0].push_back(source(6.124896, 1.026744, 1.579000));
    __patches[0].push_back(source(6.123023, 1.026018, 1.579000));
    __patches[0].push_back(source(6.123210, 1.027205, 1.575000));
    __patches[0].push_back(source(6.124427, 1.026042, 1.577000));
    __patches[0].push_back(source(6.122602, 1.026502, 1.571000));
    __patches[0].push_back(source(6.123819, 1.027278, 1.569000));
    __patches[0].push_back(source(6.125177, 1.026550, 1.564000));
    __patches[0].push_back(source(6.123772, 1.026308, 1.562000));
    __patches[0].push_back(source(6.124801, 1.026114, 1.561000));
    __patches[0].push_back(source(6.124007, 1.026939, 1.559000));
    __patches[0].push_back(source(6.122835, 1.027036, 1.556000));
    __patches[0].push_back(source(6.123585, 1.026721, 1.553000));
    __patches[0].push_back(source(6.124147, 1.027060, 1.552000));
    __patches[0].push_back(source(6.125084, 1.027035, 1.552000));
    __patches[0].push_back(source(6.122742, 1.026745, 1.545000));
    __patches[0].push_back(source(6.122929, 1.027181, 1.552000));
    __patches[0].push_back(source(6.122602, 1.026333, 1.549000));
    __patches[0].push_back(source(6.123351, 1.026018, 1.547000));
    __patches[0].push_back(source(6.125318, 1.026817, 1.544000));
    __patches[0].push_back(source(6.123726, 1.027157, 1.537000));
    __patches[0].push_back(source(6.122977, 1.025848, 1.535000));
    __patches[0].push_back(source(6.124755, 1.026551, 1.533000));
    __patches[0].push_back(source(6.123491, 1.026284, 1.528000));
    __patches[0].push_back(source(6.124053, 1.026817, 1.523000));
    __patches[0].push_back(source(6.123070, 1.026939, 1.521000));
    __patches[0].push_back(source(6.124474, 1.026236, 1.522000));
    __patches[0].push_back(source(6.124193, 1.025848, 1.518000));
    __patches[0].push_back(source(6.125271, 1.026696, 1.519000));
    __patches[0].push_back(source(6.123585, 1.026575, 1.518000));
    __patches[0].push_back(source(6.123585, 1.026381, 1.502000));
    __patches[0].push_back(source(6.122274, 1.026502, 1.505000));
    __patches[0].push_back(source(6.122648, 1.026890, 1.506000));
    __patches[0].push_back(source(6.123585, 1.025921, 1.500000));
    __patches[0].push_back(source(6.124568, 1.026381, 1.493000));
    __patches[0].push_back(source(6.123304, 1.026599, 1.487000));
    __patches[0].push_back(source(6.124475, 1.027011, 1.486000));
    __patches[0].push_back(source(6.123491, 1.026139, 1.487000));
    __patches[0].push_back(source(6.124240, 1.026599, 1.487000));
    __patches[0].push_back(source(6.123116, 1.027036, 1.481000));
    __patches[0].push_back(source(6.124193, 1.025993, 1.480000));
    __patches[0].push_back(source(6.122555, 1.026236, 1.474000));
    __patches[0].push_back(source(6.123725, 1.026187, 1.472000));
    __patches[0].push_back(source(6.122367, 1.026648, 1.467000));
    __patches[0].push_back(source(6.124335, 1.027181, 1.466000));
    __patches[0].push_back(source(6.124801, 1.026041, 1.465000));
    __patches[0].push_back(source(6.124287, 1.026163, 1.458000));
    __patches[0].push_back(source(6.124568, 1.026551, 1.460000));
    __patches[0].push_back(source(6.123257, 1.026309, 1.457000));
    __patches[0].push_back(source(6.125084, 1.026914, 1.454000));
    __patches[0].push_back(source(6.122509, 1.026114, 1.453000));
    __patches[0].push_back(source(6.123351, 1.026696, 1.451000));
    __patches[0].push_back(source(6.123257, 1.025896, 1.451000));
    __patches[0].push_back(source(6.123772, 1.027036, 1.453000));
    __patches[0].push_back(source(6.124802, 1.026308, 1.449000));
    __patches[0].push_back(source(6.123725, 1.025775, 1.449000));
    __patches[0].push_back(source(6.123679, 1.027327, 1.451000));
    __patches[0].push_back(source(6.124426, 1.025920, 1.448000));
    __patches[0].push_back(source(6.122976, 1.026284, 1.447000));
    __patches[0].push_back(source(6.123304, 1.026915, 1.447000));
    __patches[0].push_back(source(6.122602, 1.026648, 1.442000));
    __patches[0].push_back(source(6.123913, 1.026551, 1.448000));
    __patches[0].push_back(source(6.124568, 1.026696, 1.437000));
    __patches[0].push_back(source(6.125270, 1.026623, 1.427000));
    __patches[0].push_back(source(6.124006, 1.026357, 1.430000));
    __patches[0].push_back(source(6.122696, 1.026018, 1.429000));
    __patches[0].push_back(source(6.125082, 1.026284, 1.426000));
    __patches[0].push_back(source(6.122929, 1.026672, 1.424000));
    __patches[0].push_back(source(6.123725, 1.026866, 1.422000));
    __patches[0].push_back(source(6.123164, 1.025751, 1.423000));
    __patches[0].push_back(source(6.123210, 1.027205, 1.414000));
    __patches[0].push_back(source(6.123678, 1.026066, 1.416000));
    __patches[0].push_back(source(6.124288, 1.026890, 1.415000));
    __patches[0].push_back(source(6.124287, 1.026745, 1.409000));
    __patches[0].push_back(source(6.125036, 1.026429, 1.407000));
    __patches[0].push_back(source(6.124005, 1.025824, 1.406000));
    __patches[0].push_back(source(6.122789, 1.026381, 1.404000));
    __patches[0].push_back(source(6.125037, 1.026817, 1.402000));
    __patches[0].push_back(source(6.125035, 1.026162, 1.398000));
    __patches[0].push_back(source(6.124193, 1.026478, 1.397000));
    __patches[0].push_back(source(6.123163, 1.026502, 1.393000));
    __patches[0].push_back(source(6.123538, 1.026987, 1.390000));
    __patches[0].push_back(source(6.124569, 1.026938, 1.391000));
    __patches[0].push_back(source(6.122321, 1.026381, 1.389000));
    __patches[0].push_back(source(6.122976, 1.027157, 1.384000));
    __patches[0].push_back(source(6.124660, 1.025969, 1.387000));
    __patches[0].push_back(source(6.123164, 1.026187, 1.389000));
    __patches[0].push_back(source(6.124007, 1.027230, 1.391000));
    __patches[0].push_back(source(6.124850, 1.027035, 1.378000));
    __patches[0].push_back(source(6.122929, 1.026793, 1.378000));
    __patches[0].push_back(source(6.123866, 1.026745, 1.372000));
    __patches[0].push_back(source(6.122508, 1.026454, 1.371000));
    __patches[0].push_back(source(6.124801, 1.026187, 1.371000));
    __patches[0].push_back(source(6.123772, 1.026478, 1.362000));
    __patches[0].push_back(source(6.124802, 1.026623, 1.359000));
    __patches[0].push_back(source(6.124193, 1.026308, 1.360000));
    __patches[0].push_back(source(6.122977, 1.025848, 1.361000));
    __patches[0].push_back(source(6.125131, 1.027035, 1.359000));
    __patches[0].push_back(source(6.122461, 1.026745, 1.364000));
    __patches[0].push_back(source(6.122977, 1.026018, 1.354000));
    __patches[0].push_back(source(6.122742, 1.026987, 1.351000));
    __patches[0].push_back(source(6.124241, 1.027011, 1.347000));
    __patches[0].push_back(source(6.124567, 1.026090, 1.344000));
    __patches[0].push_back(source(6.123818, 1.025896, 1.344000));
    __patches[0].push_back(source(6.124849, 1.026429, 1.342000));
    __patches[0].push_back(source(6.123398, 1.027108, 1.338000));
    __patches[0].push_back(source(6.122368, 1.026236, 1.339000));
    __patches[0].push_back(source(6.123959, 1.026139, 1.334000));
    __patches[0].push_back(source(6.124054, 1.027108, 1.335000));
    __patches[0].push_back(source(6.125318, 1.026841, 1.334000));
    __patches[0].push_back(source(6.123725, 1.026648, 1.332000));
    __patches[0].push_back(source(6.124616, 1.027084, 1.333000));
    __patches[0].push_back(source(6.123351, 1.025799, 1.329000));
    __patches[0].push_back(source(6.122695, 1.026527, 1.327000));
    __patches[0].push_back(source(6.124709, 1.026793, 1.326000));
    __patches[0].push_back(source(6.123491, 1.027230, 1.320000));
    __patches[0].push_back(source(6.122836, 1.025945, 1.322000));
    __patches[0].push_back(source(6.122601, 1.026866, 1.319000));
    __patches[0].push_back(source(6.123163, 1.026793, 1.315000));
    __patches[0].push_back(source(6.123491, 1.026381, 1.311000));
    __patches[0].push_back(source(6.125130, 1.026502, 1.309000));
    __patches[0].push_back(source(6.124475, 1.026793, 1.309000));
    __patches[0].push_back(source(6.123538, 1.026793, 1.305000));
    __patches[0].push_back(source(6.122274, 1.026526, 1.301000));
    __patches[0].push_back(source(6.123538, 1.026236, 1.301000));
    __patches[0].push_back(source(6.123117, 1.026381, 1.296000));
    __patches[0].push_back(source(6.123819, 1.027278, 1.297000));
    __patches[0].push_back(source(6.123491, 1.025945, 1.297000));
    __patches[0].push_back(source(6.124567, 1.025920, 1.298000));
    __patches[0].push_back(source(6.125177, 1.026744, 1.292000));
    __patches[0].push_back(source(6.123117, 1.026648, 1.290000));
    __patches[0].push_back(source(6.124006, 1.026236, 1.287000));
    __patches[0].push_back(source(6.124053, 1.026696, 1.290000));
    __patches[0].push_back(source(6.122882, 1.027084, 1.286000));
    __patches[0].push_back(source(6.124288, 1.027157, 1.285000));
    __patches[0].push_back(source(6.122883, 1.026115, 1.280000));
    __patches[0].push_back(source(6.123772, 1.025775, 1.276000));
    __patches[0].push_back(source(6.124521, 1.026454, 1.272000));
    __patches[0].push_back(source(6.122695, 1.026745, 1.272000));
    __patches[0].push_back(source(6.124848, 1.026041, 1.271000));
    __patches[0].push_back(source(6.125270, 1.026623, 1.268000));
    __patches[0].push_back(source(6.122602, 1.026308, 1.266000));
    __patches[0].push_back(source(6.125178, 1.026938, 1.264000));
    __patches[0].push_back(source(6.124286, 1.025872, 1.262000));
    __patches[0].push_back(source(6.123726, 1.027157, 1.260000));
    __patches[0].push_back(source(6.124006, 1.026817, 1.260000));
    __patches[0].push_back(source(6.123070, 1.025775, 1.256000));
    __patches[0].push_back(source(6.123444, 1.026527, 1.252000));
    __patches[0].push_back(source(6.122836, 1.026260, 1.249000));
    __patches[0].push_back(source(6.124521, 1.026260, 1.249000));
    __patches[0].push_back(source(6.124943, 1.026550, 1.243000));
    __patches[0].push_back(source(6.125082, 1.026162, 1.242000));
    __patches[0].push_back(source(6.122976, 1.027181, 1.245000));
    __patches[0].push_back(source(6.123257, 1.026042, 1.244000));
    __patches[0].push_back(source(6.123164, 1.025921, 1.233000));
    __patches[0].push_back(source(6.122929, 1.026551, 1.231000));
    __patches[0].push_back(source(6.124100, 1.026527, 1.231000));
    __patches[0].push_back(source(6.122602, 1.026066, 1.228000));
    __patches[0].push_back(source(6.123725, 1.026042, 1.229000));
    __patches[0].push_back(source(6.123210, 1.026915, 1.228000));
    __patches[0].push_back(source(6.123679, 1.027327, 1.228000));
    __patches[0].push_back(source(6.123866, 1.026963, 1.227000));
    __patches[0].push_back(source(6.124897, 1.026914, 1.222000));
    __patches[0].push_back(source(6.122227, 1.026429, 1.222000));
    __patches[0].push_back(source(6.122367, 1.026672, 1.215000));
    __patches[0].push_back(source(6.122462, 1.026187, 1.217000));
    __patches[0].push_back(source(6.123959, 1.026430, 1.217000));
    __patches[0].push_back(source(6.124052, 1.025824, 1.217000));
    __patches[0].push_back(source(6.124756, 1.027084, 1.215000));
    __patches[0].push_back(source(6.124240, 1.026139, 1.214000));
    __patches[0].push_back(source(6.123210, 1.027205, 1.215000));
    __patches[0].push_back(source(6.125082, 1.026284, 1.208000));
    __patches[0].push_back(source(6.124380, 1.026017, 1.208000));
    __patches[0].push_back(source(6.124475, 1.026648, 1.209000));
    __patches[0].push_back(source(6.123257, 1.026187, 1.208000));
    __patches[0].push_back(source(6.123585, 1.026648, 1.210000));
    __patches[0].push_back(source(6.122321, 1.026333, 1.206000));
    __patches[0].push_back(source(6.125459, 1.026914, 1.204000));
    __patches[0].push_back(source(6.123210, 1.027060, 1.203000));
    __patches[0].push_back(source(6.123866, 1.026333, 1.200000));
    __patches[0].push_back(source(6.124194, 1.027205, 1.204000));
    __patches[0].push_back(source(6.125037, 1.026672, 1.201000));
    __patches[0].push_back(source(6.122883, 1.026430, 1.194000));
    __patches[0].push_back(source(6.122882, 1.026890, 1.194000));
    __patches[0].push_back(source(6.123538, 1.025824, 1.193000));
    __patches[0].push_back(source(6.124614, 1.026357, 1.189000));
    __patches[0].push_back(source(6.124194, 1.026769, 1.186000));
    __patches[0].push_back(source(6.122321, 1.026575, 1.180000));
    __patches[0].push_back(source(6.124991, 1.027059, 1.176000));
    __patches[0].push_back(source(6.124709, 1.026648, 1.180000));
    __patches[0].push_back(source(6.123772, 1.026187, 1.177000));
    __patches[0].push_back(source(6.122929, 1.026672, 1.177000));
    __patches[0].push_back(source(6.124801, 1.026187, 1.175000));
    __patches[0].push_back(source(6.124473, 1.025896, 1.172000));
    __patches[0].push_back(source(6.123491, 1.026963, 1.174000));
    __patches[0].push_back(source(6.122508, 1.026817, 1.166000));
    __patches[0].push_back(source(6.124522, 1.027132, 1.169000));
    __patches[0].push_back(source(6.123304, 1.026309, 1.170000));
    __patches[0].push_back(source(6.125083, 1.026405, 1.160000));
    __patches[0].push_back(source(6.122883, 1.025896, 1.157000));
    __patches[0].push_back(source(6.124382, 1.027036, 1.157000));
    __patches[0].push_back(source(6.123679, 1.026745, 1.158000));
    __patches[0].push_back(source(6.122742, 1.027011, 1.151000));
    __patches[0].push_back(source(6.124193, 1.026333, 1.152000));
    __patches[0].push_back(source(6.123959, 1.025993, 1.151000));
    __patches[0].push_back(source(6.124941, 1.026090, 1.152000));
    __patches[0].push_back(source(6.125177, 1.026526, 1.157000));
    __patches[0].push_back(source(6.123491, 1.026139, 1.149000));
    __patches[0].push_back(source(6.125318, 1.026768, 1.145000));
    __patches[0].push_back(source(6.123444, 1.027230, 1.144000));
    __patches[0].push_back(source(6.124568, 1.026502, 1.140000));
    __patches[0].push_back(source(6.123304, 1.026624, 1.141000));
    __patches[0].push_back(source(6.124334, 1.026890, 1.140000));
    __patches[0].push_back(source(6.123960, 1.027133, 1.136000));
    __patches[0].push_back(source(6.123725, 1.025775, 1.135000));
    __patches[0].push_back(source(6.122648, 1.026890, 1.133000));
    __patches[0].push_back(source(6.123725, 1.026890, 1.135000));
    __patches[0].push_back(source(6.122509, 1.026090, 1.133000));
    __patches[0].push_back(source(6.123959, 1.026599, 1.127000));
    __patches[0].push_back(source(6.123398, 1.027108, 1.123000));
    __patches[0].push_back(source(6.124006, 1.026115, 1.120000));
    __patches[0].push_back(source(6.123726, 1.027351, 1.118000));
    __patches[0].push_back(source(6.122180, 1.026502, 1.115000));
    __patches[0].push_back(source(6.124567, 1.026090, 1.115000));
    __patches[0].push_back(source(6.124616, 1.026914, 1.114000));
    __patches[0].push_back(source(6.123351, 1.026454, 1.116000));
    __patches[0].push_back(source(6.123351, 1.025799, 1.113000));
    __patches[0].push_back(source(6.122743, 1.026163, 1.115000));
    __patches[0].push_back(source(6.125178, 1.027035, 1.111000));
    __patches[0].push_back(source(6.124756, 1.026720, 1.109000));
    __patches[0].push_back(source(6.125270, 1.026623, 1.104000));
    __patches[0].push_back(source(6.124007, 1.027230, 1.101000));
    __patches[0].push_back(source(6.123538, 1.026818, 1.101000));
    __patches[0].push_back(source(6.125082, 1.026138, 1.103000));
    __patches[0].push_back(source(6.122649, 1.026381, 1.101000));
    __patches[0].push_back(source(6.123865, 1.025872, 1.097000));
    __patches[0].push_back(source(6.122882, 1.027084, 1.096000));
    __patches[0].push_back(source(6.123117, 1.026163, 1.096000));
    __patches[0].push_back(source(6.122790, 1.025945, 1.093000));
    __patches[0].push_back(source(6.123117, 1.026769, 1.091000));
    __patches[0].push_back(source(6.124802, 1.026478, 1.089000));
    __patches[0].push_back(source(6.123164, 1.025751, 1.085000));
    __patches[0].push_back(source(6.125224, 1.026841, 1.085000));
    __patches[0].push_back(source(6.122883, 1.026575, 1.080000));
    __patches[0].push_back(source(6.123819, 1.026987, 1.079000));
    __patches[0].push_back(source(6.122929, 1.027181, 1.081000));
    __patches[0].push_back(source(6.124240, 1.026599, 1.082000));
    __patches[0].push_back(source(6.123585, 1.026381, 1.077000));
    __patches[0].push_back(source(6.124193, 1.025824, 1.080000));
    __patches[0].push_back(source(6.124848, 1.026308, 1.073000));
    __patches[0].push_back(source(6.122461, 1.026769, 1.075000));
    __patches[0].push_back(source(6.122321, 1.026357, 1.073000));
    __patches[0].push_back(source(6.124707, 1.025969, 1.069000));
    __patches[0].push_back(source(6.124194, 1.026987, 1.068000));
    __patches[0].push_back(source(6.124146, 1.025993, 1.067000));
    __patches[0].push_back(source(6.124240, 1.026454, 1.069000));
    __patches[0].push_back(source(6.125412, 1.026889, 1.068000));
    __patches[0].push_back(source(6.122977, 1.026018, 1.069000));
    __patches[0].push_back(source(6.122461, 1.026454, 1.063000));
    __patches[0].push_back(source(6.123585, 1.026575, 1.054000));
    __patches[0].push_back(source(6.124006, 1.026211, 1.054000));
    __patches[0].push_back(source(6.124803, 1.026841, 1.054000));
    __patches[0].push_back(source(6.122555, 1.026623, 1.051000));
    __patches[0].push_back(source(6.124567, 1.026211, 1.048000));
    __patches[0].push_back(source(6.124569, 1.027108, 1.048000));
    __patches[0].push_back(source(6.122930, 1.026260, 1.050000));
    __patches[0].push_back(source(6.124100, 1.026696, 1.048000));
    __patches[0].push_back(source(6.123491, 1.027254, 1.046000));
    __patches[0].push_back(source(6.123304, 1.026915, 1.044000));
    __patches[0].push_back(source(6.122368, 1.026236, 1.044000));
    __patches[0].push_back(source(6.124989, 1.026575, 1.040000));
    __patches[0].push_back(source(6.123444, 1.026187, 1.035000));
    __patches[0].push_back(source(6.123959, 1.025799, 1.035000));
    __patches[0].push_back(source(6.123117, 1.026454, 1.035000));
    __patches[0].push_back(source(6.122977, 1.025848, 1.035000));
    __patches[0].push_back(source(6.122321, 1.026672, 1.032000));
    __patches[0].push_back(source(6.122788, 1.027011, 1.031000));
    __patches[0].push_back(source(6.125129, 1.026235, 1.025000));
    __patches[0].push_back(source(6.123725, 1.026502, 1.023000));
    __patches[0].push_back(source(6.124428, 1.027036, 1.022000));
    __patches[0].push_back(source(6.124473, 1.025896, 1.017000));
    __patches[0].push_back(source(6.125271, 1.026720, 1.018000));
    __patches[0].push_back(source(6.122742, 1.026769, 1.015000));
    __patches[0].push_back(source(6.124287, 1.026187, 1.016000));
    __patches[0].push_back(source(6.123163, 1.027205, 1.016000));
    __patches[0].push_back(source(6.123678, 1.026066, 1.015000));
    __patches[0].push_back(source(6.124709, 1.026623, 1.014000));
    __patches[0].push_back(source(6.124991, 1.027059, 1.014000));
    __patches[0].push_back(source(6.123398, 1.026721, 1.013000));
    __patches[0].push_back(source(6.123211, 1.025896, 1.011000));
    __patches[0].push_back(source(6.124429, 1.027181, 1.011000));
    __patches[0].push_back(source(6.124801, 1.026041, 1.012000));
    __patches[0].push_back(source(6.123444, 1.026309, 1.007000));
    __patches[0].push_back(source(6.122415, 1.026042, 1.006000));
    __patches[0].push_back(source(6.123913, 1.026769, 1.005000));
    __patches[0].push_back(source(6.124287, 1.026357, 1.006000));
    __patches[0].push_back(source(6.123679, 1.027181, 1.006000));
    __patches[0].push_back(source(6.122695, 1.026914, 0.999200));
    __patches[0].push_back(source(6.124897, 1.026963, 0.994900));
    __patches[0].push_back(source(6.125177, 1.026502, 0.994400));
    __patches[0].push_back(source(6.122883, 1.026430, 0.994200));
    __patches[0].push_back(source(6.123257, 1.026042, 0.993700));
    __patches[0].push_back(source(6.124007, 1.026914, 0.992800));
    __patches[0].push_back(source(6.123679, 1.027327, 0.993800));
    __patches[0].push_back(source(6.123725, 1.025775, 0.987000));
    __patches[0].push_back(source(6.123163, 1.027060, 0.985200));
    __patches[0].push_back(source(6.123772, 1.026284, 0.984200));
    __patches[0].push_back(source(6.125129, 1.026356, 0.985100));
    __patches[0].push_back(source(6.122181, 1.026454, 0.982300));
    __patches[0].push_back(source(6.123117, 1.025751, 0.982100));
    __patches[0].push_back(source(6.123210, 1.026575, 0.982800));
    __patches[0].push_back(source(6.124475, 1.026817, 0.980900));
    __patches[0].push_back(source(6.124006, 1.026454, 0.981700));
    __patches[0].push_back(source(6.122696, 1.025993, 0.979700));
    __patches[0].push_back(source(6.125459, 1.026914, 0.982100));
    __patches[0].push_back(source(6.123865, 1.025993, 0.978400));
    __patches[0].push_back(source(6.123023, 1.026915, 0.977800));
    __patches[0].push_back(source(6.125365, 1.026792, 0.976400));
    __patches[0].push_back(source(6.124101, 1.027230, 0.974200));
    __patches[0].push_back(source(6.124380, 1.026017, 0.972500));
    __patches[0].push_back(source(6.124241, 1.026745, 0.970700));
    __patches[0].push_back(source(6.123117, 1.026333, 0.969000));
    __patches[0].push_back(source(6.122274, 1.026575, 0.969800));
    __patches[0].push_back(source(6.125082, 1.026138, 0.965000));
    __patches[0].push_back(source(6.125270, 1.026599, 0.965600));
    __patches[0].push_back(source(6.123725, 1.026866, 0.962400));
    __patches[0].push_back(source(6.124100, 1.026308, 0.959800));
    __patches[0].push_back(source(6.122788, 1.027230, 0.958500));
    __patches[0].push_back(source(6.122602, 1.026236, 0.959000));
    __patches[0].push_back(source(6.123631, 1.025921, 0.956400));
    __patches[0].push_back(source(6.123538, 1.027084, 0.955100));
    __patches[0].push_back(source(6.122508, 1.026842, 0.954200));
    __patches[0].push_back(source(6.124568, 1.026526, 0.950000));
    __patches[0].push_back(source(6.124007, 1.027108, 0.950900));
    __patches[0].push_back(source(6.123679, 1.026696, 0.955300));
    __patches[0].push_back(source(6.122368, 1.026211, 0.945500));
    __patches[0].push_back(source(6.125225, 1.026987, 0.945000));
    __patches[0].push_back(source(6.123023, 1.026696, 0.944200));
    __patches[0].push_back(source(6.123491, 1.026527, 0.941500));
    __patches[0].push_back(source(6.124757, 1.027108, 0.941200));
    __patches[0].push_back(source(6.123866, 1.027278, 0.939800));
    __patches[0].push_back(source(6.123117, 1.025921, 0.935100));
    __patches[0].push_back(source(6.124895, 1.026090, 0.934200));
    __patches[0].push_back(source(6.122929, 1.027108, 0.934900));
    __patches[0].push_back(source(6.122649, 1.026502, 0.935000));
    __patches[0].push_back(source(6.124616, 1.026987, 0.932100));
    __patches[0].push_back(source(6.123912, 1.026090, 0.931100));
    __patches[0].push_back(source(6.124428, 1.026672, 0.929000));
    __patches[0].push_back(source(6.124286, 1.025872, 0.928700));
    __patches[0].push_back(source(6.124614, 1.026332, 0.930700));
    __patches[0].push_back(source(6.125037, 1.026744, 0.934300));
    __patches[0].push_back(source(6.123491, 1.026963, 0.925900));
    __patches[0].push_back(source(6.122367, 1.026696, 0.921700));
    __patches[0].push_back(source(6.123117, 1.026818, 0.916800));
    __patches[0].push_back(source(6.123257, 1.026236, 0.914300));
    __patches[0].push_back(source(6.123444, 1.025945, 0.913000));
    __patches[0].push_back(source(6.124567, 1.026090, 0.913300));
    __patches[0].push_back(source(6.122321, 1.026357, 0.911100));
    __patches[0].push_back(source(6.124709, 1.026744, 0.910600));
    __patches[0].push_back(source(6.124288, 1.027181, 0.911500));
    __patches[0].push_back(source(6.122462, 1.026090, 0.909700));
    __patches[0].push_back(source(6.123398, 1.026381, 0.906700));
    __patches[0].push_back(source(6.124614, 1.025920, 0.904500));
    __patches[0].push_back(source(6.125129, 1.026259, 0.906600));
    __patches[0].push_back(source(6.123585, 1.026648, 0.902900));
    __patches[0].push_back(source(6.124427, 1.026429, 0.901800));
    __patches[0].push_back(source(6.124288, 1.026914, 0.903400));
    __patches[0].push_back(source(6.125130, 1.026453, 0.898800));
    __patches[0].push_back(source(6.122415, 1.025969, 0.898400));
    __patches[0].push_back(source(6.122929, 1.027181, 0.898300));
    __patches[0].push_back(source(6.125131, 1.026890, 0.895200));
    __patches[0].push_back(source(6.124427, 1.026308, 0.895200));
    __patches[0].push_back(source(6.123585, 1.025824, 0.894400));
    __patches[0].push_back(source(6.122836, 1.026818, 0.892300));
    __patches[0].push_back(source(6.123725, 1.026212, 0.890800));
    __patches[0].push_back(source(6.123023, 1.026018, 0.891600));
    __patches[0].push_back(source(6.124100, 1.026624, 0.890300));
    __patches[0].push_back(source(6.123023, 1.026551, 0.891400));
    __patches[0].push_back(source(6.123772, 1.027108, 0.888200));
    __patches[0].push_back(source(6.123211, 1.025727, 0.888700));
    __patches[0].push_back(source(6.122461, 1.026817, 0.884700));
    __patches[0].push_back(source(6.122602, 1.026333, 0.886100));
    __patches[0].push_back(source(6.123538, 1.026793, 0.885700));
    __patches[0].push_back(source(6.123070, 1.026163, 0.882000));
    __patches[0].push_back(source(6.124193, 1.026090, 0.881300));
    __patches[0].push_back(source(6.125131, 1.027059, 0.879900));
    __patches[0].push_back(source(6.124709, 1.026623, 0.879200));
    __patches[0].push_back(source(6.123163, 1.027205, 0.879400));
    __patches[0].push_back(source(6.124147, 1.026478, 0.876500));
    __patches[0].push_back(source(6.122882, 1.026648, 0.874000));
    __patches[0].push_back(source(6.124801, 1.026187, 0.874900));
    __patches[0].push_back(source(6.124241, 1.027084, 0.875200));
    __patches[0].push_back(source(6.123959, 1.025775, 0.872100));
    __patches[0].push_back(source(6.122180, 1.026526, 0.870400));
    __patches[0].push_back(source(6.123023, 1.025799, 0.868000));
    __patches[0].push_back(source(6.123444, 1.026139, 0.868700));
    __patches[0].push_back(source(6.123398, 1.027230, 0.864400));
    __patches[0].push_back(source(6.125271, 1.026696, 0.862200));
    __patches[0].push_back(source(6.124801, 1.025993, 0.858000));
    __patches[0].push_back(source(6.122742, 1.027011, 0.860200));
    __patches[0].push_back(source(6.123351, 1.026672, 0.860800));
    __patches[0].push_back(source(6.124849, 1.026454, 0.861500));
    __patches[0].push_back(source(6.124803, 1.027108, 0.856600));
    __patches[0].push_back(source(6.124006, 1.026236, 0.856600));
    __patches[0].push_back(source(6.122181, 1.026429, 0.856100));
    __patches[0].push_back(source(6.122743, 1.026115, 0.855800));
    __patches[0].push_back(source(6.123818, 1.025896, 0.854400));
    __patches[0].push_back(source(6.124053, 1.026866, 0.853600));
    __patches[0].push_back(source(6.123679, 1.027351, 0.851500));
    __patches[0].push_back(source(6.123398, 1.027108, 0.848100));
    __patches[0].push_back(source(6.122930, 1.026284, 0.847000));
    __patches[0].push_back(source(6.125412, 1.026914, 0.845000));
    __patches[0].push_back(source(6.122461, 1.026502, 0.845500));
    __patches[0].push_back(source(6.124333, 1.026211, 0.843500));
    __patches[0].push_back(source(6.122836, 1.025896, 0.839700));
    __patches[0].push_back(source(6.123585, 1.026405, 0.841500));
    __patches[0].push_back(source(6.124194, 1.026769, 0.838500));
    __patches[0].push_back(source(6.123351, 1.025799, 0.836500));
    __patches[0].push_back(source(6.123351, 1.026915, 0.836700));
    __patches[0].push_back(source(6.125082, 1.026138, 0.835700));
    __patches[0].push_back(source(6.125270, 1.026599, 0.836200));
    __patches[0].push_back(source(6.124803, 1.027011, 0.834700));
    __patches[0].push_back(source(6.122741, 1.027230, 0.832100));
    __patches[0].push_back(source(6.123726, 1.027230, 0.830600));
    __patches[0].push_back(source(6.122648, 1.026914, 0.830700));
    __patches[0].push_back(source(6.123912, 1.026357, 0.831400));
    __patches[0].push_back(source(6.123304, 1.026042, 0.834100));
    __patches[0].push_back(source(6.122836, 1.026381, 0.833000));
    __patches[0].push_back(source(6.124897, 1.026866, 0.829800));
    __patches[0].push_back(source(6.124426, 1.025896, 0.829900));
    __patches[0].push_back(source(6.124989, 1.026599, 0.823800));
    __patches[0].push_back(source(6.123913, 1.026599, 0.825000));
    __patches[0].push_back(source(6.122696, 1.025993, 0.824400));
    __patches[0].push_back(source(6.124147, 1.027011, 0.824000));
    __patches[0].push_back(source(6.122601, 1.026672, 0.821300));
    __patches[0].push_back(source(6.125083, 1.026381, 0.820800));
    __patches[0].push_back(source(6.123725, 1.026818, 0.818800));
    __patches[0].push_back(source(6.124194, 1.027230, 0.816400));
    __patches[0].push_back(source(6.123257, 1.026333, 0.817100));
    __patches[0].push_back(source(6.124193, 1.025824, 0.815400));
    __patches[0].push_back(source(6.124287, 1.026599, 0.811500));
    __patches[0].push_back(source(6.125412, 1.026792, 0.809900));
    __patches[0].push_back(source(6.122321, 1.026260, 0.810100));
    __patches[0].push_back(source(6.123772, 1.026987, 0.810500));
    __patches[0].push_back(source(6.124146, 1.025969, 0.806800));
    __patches[0].push_back(source(6.124193, 1.026333, 0.805500));
    __patches[0].push_back(source(6.123023, 1.026769, 0.802100));
    __patches[0].push_back(source(6.124894, 1.026066, 0.801400));
    __patches[0].push_back(source(6.124428, 1.027036, 0.800200));
    __patches[0].push_back(source(6.125177, 1.026526, 0.799500));
    __patches[0].push_back(source(6.122368, 1.026017, 0.801000));
    __patches[0].push_back(source(6.123678, 1.025775, 0.797500));
    __patches[0].push_back(source(6.122882, 1.027084, 0.796500));
    __patches[0].push_back(source(6.122930, 1.026090, 0.797100));
    __patches[0].push_back(source(6.123398, 1.026502, 0.798400));
    __patches[0].push_back(source(6.122321, 1.026575, 0.794900));
    __patches[0].push_back(source(6.124660, 1.025945, 0.794500));
    __patches[0].push_back(source(6.124521, 1.026211, 0.792400));
    __patches[0].push_back(source(6.124381, 1.026842, 0.792700));
    __patches[0].push_back(source(6.123631, 1.026090, 0.790100));
    __patches[0].push_back(source(6.123538, 1.027254, 0.790400));
    __patches[0].push_back(source(6.124475, 1.027181, 0.790200));
    __patches[0].push_back(source(6.125038, 1.027084, 0.787900));
    __patches[0].push_back(source(6.123959, 1.026454, 0.787500));
    __patches[0].push_back(source(6.123211, 1.025896, 0.787700));
    __patches[0].push_back(source(6.122367, 1.026696, 0.785100));
    __patches[0].push_back(source(6.122836, 1.026527, 0.784800));
    __patches[0].push_back(source(6.122462, 1.026187, 0.783900));
    __patches[0].push_back(source(6.123210, 1.026842, 0.781200));
    __patches[0].push_back(source(6.124709, 1.026623, 0.781900));
    __patches[0].push_back(source(6.125082, 1.026259, 0.779000));
    __patches[0].push_back(source(6.124380, 1.026017, 0.780100));
    __patches[0].push_back(source(6.122461, 1.026817, 0.776600));
    __patches[0].push_back(source(6.124568, 1.026478, 0.776900));
    __patches[0].push_back(source(6.123913, 1.027254, 0.776400));
    __patches[0].push_back(source(6.123117, 1.026478, 0.773500));
    __patches[0].push_back(source(6.124569, 1.026914, 0.772400));
    __patches[0].push_back(source(6.123491, 1.026212, 0.769100));
    __patches[0].push_back(source(6.123959, 1.025799, 0.770800));
    __patches[0].push_back(source(6.122321, 1.026357, 0.772600));
    __patches[0].push_back(source(6.123070, 1.025751, 0.767700));
    __patches[0].push_back(source(6.123913, 1.026769, 0.767300));
    __patches[0].push_back(source(6.123163, 1.027060, 0.765200));
    __patches[0].push_back(source(6.123585, 1.026599, 0.763400));
    __patches[0].push_back(source(6.125272, 1.026962, 0.761300));
    __patches[0].push_back(source(6.123023, 1.026939, 0.761000));
    __patches[0].push_back(source(6.125318, 1.026696, 0.761900));
    __patches[0].push_back(source(6.124848, 1.026284, 0.761800));
    __patches[0].push_back(source(6.124053, 1.026211, 0.757600));
    __patches[0].push_back(source(6.123538, 1.026333, 0.755300));
    __patches[0].push_back(source(6.123444, 1.026769, 0.756400));
    __patches[0].push_back(source(6.124007, 1.027108, 0.752900));
    __patches[0].push_back(source(6.123726, 1.027351, 0.750500));
    __patches[0].push_back(source(6.123257, 1.025727, 0.751900));
    __patches[0].push_back(source(6.122602, 1.026381, 0.746000));
    __patches[0].push_back(source(6.122929, 1.027181, 0.747200));
    __patches[0].push_back(source(6.122695, 1.026769, 0.747300));
    __patches[0].push_back(source(6.124475, 1.026720, 0.746200));
    __patches[0].push_back(source(6.124757, 1.027132, 0.746900));
    __patches[0].push_back(source(6.122415, 1.025945, 0.746000));
    __patches[0].push_back(source(6.125037, 1.026720, 0.741600));
    __patches[0].push_back(source(6.123818, 1.025921, 0.741200));
    __patches[0].push_back(source(6.125129, 1.026138, 0.739500));
    __patches[0].push_back(source(6.123257, 1.026624, 0.735600));
    __patches[0].push_back(source(6.123210, 1.026187, 0.735700));
    __patches[0].push_back(source(6.122742, 1.027011, 0.732800));
    __patches[0].push_back(source(6.122367, 1.027011, -0.737700));
    __patches[0].push_back(source(6.124006, 1.026066, 0.737200));
    __patches[0].push_back(source(6.124990, 1.026914, 0.733300));
    __patches[0].push_back(source(6.122462, 1.026090, 0.732800));
    __patches[0].push_back(source(6.124287, 1.026454, 0.732200));
    __patches[0].push_back(source(6.123585, 1.026915, 0.730600));
    __patches[0].push_back(source(6.123725, 1.026478, 0.730300));
    __patches[0].push_back(source(6.123304, 1.025872, 0.729700));
    __patches[0].push_back(source(6.122741, 1.027230, 0.729500));
    __patches[0].push_back(source(6.125083, 1.026381, 0.729500));
    __patches[0].push_back(source(6.125412, 1.026792, 0.729800));
    __patches[0].push_back(source(6.124567, 1.026066, 0.727500));
    __patches[0].push_back(source(6.122134, 1.026526, 0.728100));
    __patches[0].push_back(source(6.123912, 1.025727, 0.728200));
    __patches[0].push_back(source(6.123398, 1.027230, 0.725300));
    __patches[0].push_back(source(6.125459, 1.026914, 0.726800));
    __patches[0].push_back(source(6.122790, 1.025945, 0.720700));
    __patches[0].push_back(source(6.124756, 1.026720, 0.721500));
    __patches[0].push_back(source(6.123070, 1.026333, 0.722000));
    __patches[0].push_back(source(6.124288, 1.027205, 0.720800));
    __patches[0].push_back(source(6.123491, 1.026139, 0.720000));
    __patches[0].push_back(source(6.122181, 1.026405, 0.719100));
    __patches[0].push_back(source(6.125130, 1.026478, 0.716500));
    __patches[0].push_back(source(6.123679, 1.026672, 0.714500));
    __patches[0].push_back(source(6.122836, 1.026648, 0.712800));
    __patches[0].push_back(source(6.124756, 1.026841, 0.712900));
    __patches[0].push_back(source(6.124661, 1.026332, 0.712300));
    __patches[0].push_back(source(6.124426, 1.025896, 0.712200));
    __patches[0].push_back(source(6.124241, 1.026769, 0.711400));
    __patches[0].push_back(source(6.123304, 1.026042, 0.709900));
    __patches[0].push_back(source(6.124241, 1.026914, 0.711000));
    __patches[0].push_back(source(6.123163, 1.027205, 0.708900));
    __patches[0].push_back(source(6.122415, 1.026187, 0.707500));
    __patches[0].push_back(source(6.123304, 1.026939, 0.709000));
    __patches[0].push_back(source(6.124147, 1.026527, 0.706800));
    __patches[0].push_back(source(6.123444, 1.025945, 0.703600));
    __patches[0].push_back(source(6.122648, 1.026914, 0.702500));
    __patches[0].push_back(source(6.124287, 1.026163, 0.701100));
    __patches[0].push_back(source(6.124007, 1.027230, 0.699900));
    __patches[0].push_back(source(6.124569, 1.027157, 0.699100));
    __patches[0].push_back(source(6.122836, 1.026430, 0.698400));
    __patches[0].push_back(source(6.123725, 1.026212, 0.699300));
    __patches[0].push_back(source(6.123819, 1.026987, 0.698100));
    __patches[0].push_back(source(6.125129, 1.026235, 0.698100));
    __patches[0].push_back(source(6.122461, 1.026817, 0.696300));
    __patches[0].push_back(source(6.125317, 1.026623, 0.695200));
    __patches[0].push_back(source(6.125178, 1.027059, 0.696300));
    __patches[0].push_back(source(6.124754, 1.025969, 0.694500));
    __patches[0].push_back(source(6.124193, 1.026333, 0.692700));
    __patches[0].push_back(source(6.122367, 1.027011, -0.693900));
    __patches[0].push_back(source(6.122649, 1.026260, 0.691800));
    __patches[0].push_back(source(6.122930, 1.025848, 0.690900));
    __patches[0].push_back(source(6.123772, 1.027108, 0.692700));
    __patches[0].push_back(source(6.124053, 1.026672, 0.691300));
    __patches[0].push_back(source(6.124193, 1.025824, 0.690500));
    __patches[0].push_back(source(6.124755, 1.026454, 0.686800));
    __patches[0].push_back(source(6.125177, 1.026841, 0.686900));
    __patches[0].push_back(source(6.124428, 1.027060, 0.686700));
    __patches[0].push_back(source(6.122883, 1.026236, 0.685700));
    __patches[0].push_back(source(6.123725, 1.025775, 0.684900));
    __patches[0].push_back(source(6.124894, 1.026066, 0.684600));
    __patches[0].push_back(source(6.122275, 1.026042, 0.681900));
    __patches[0].push_back(source(6.122930, 1.026066, 0.680100));
    __patches[0].push_back(source(6.123726, 1.027351, 0.679500));
    __patches[0].push_back(source(6.123725, 1.026842, 0.678600));
    __patches[0].push_back(source(6.123351, 1.026430, 0.678800));
    __patches[0].push_back(source(6.122788, 1.027036, 0.678500));
    __patches[0].push_back(source(6.122321, 1.026623, 0.680800));
    __patches[0].push_back(source(6.123912, 1.025993, 0.673700));
    __patches[0].push_back(source(6.123117, 1.026818, 0.673600));
    __patches[0].push_back(source(6.124943, 1.026550, 0.672400));
    __patches[0].push_back(source(6.123398, 1.027084, 0.672200));
    __patches[0].push_back(source(6.123070, 1.025921, 0.669600));
    __patches[0].push_back(source(6.123866, 1.026575, 0.668800));
    __patches[0].push_back(source(6.124991, 1.027084, 0.666800));
    __patches[0].push_back(source(6.122415, 1.026454, 0.668000));
    __patches[0].push_back(source(6.122226, 1.027084, -0.666000));
    __patches[0].push_back(source(6.122695, 1.027254, 0.667500));
    __patches[0].push_back(source(6.122742, 1.026793, 0.667000));
    __patches[0].push_back(source(6.124053, 1.026211, 0.666700));
    __patches[0].push_back(source(6.125082, 1.026114, 0.666200));
    __patches[0].push_back(source(6.124287, 1.026623, 0.664900));
    __patches[0].push_back(source(6.123257, 1.026309, 0.664100));
    __patches[0].push_back(source(6.123959, 1.025848, 0.662600));
    __patches[0].push_back(source(6.124567, 1.026187, 0.665100));
    __patches[0].push_back(source(6.123257, 1.025703, 0.659200));
    __patches[0].push_back(source(6.123117, 1.026696, 0.657700));
    __patches[0].push_back(source(6.124944, 1.026962, 0.657900));
    __patches[0].push_back(source(6.123960, 1.027133, 0.658200));
    __patches[0].push_back(source(6.122555, 1.026551, 0.652800));
    __patches[0].push_back(source(6.124755, 1.026623, 0.651900));
    __patches[0].push_back(source(6.123632, 1.026357, 0.649500));
    __patches[0].push_back(source(6.122321, 1.026333, 0.649500));
    __patches[0].push_back(source(6.123538, 1.026793, 0.650800));
    __patches[0].push_back(source(6.123163, 1.027060, 0.647000));
    __patches[0].push_back(source(6.125459, 1.026914, 0.645900));
    __patches[0].push_back(source(6.122602, 1.026066, 0.644000));
    __patches[0].push_back(source(6.124522, 1.026817, 0.643300));
    __patches[0].push_back(source(6.123959, 1.025702, 0.644200));
    __patches[0].push_back(source(6.123023, 1.025775, 0.644700));
    __patches[0].push_back(source(6.122648, 1.026914, 0.643800));
    __patches[0].push_back(source(6.124614, 1.025920, 0.643000));
    __patches[0].push_back(source(6.123398, 1.026696, 0.643900));
    __patches[0].push_back(source(6.123304, 1.027205, 0.640600));
    __patches[0].push_back(source(6.122601, 1.026672, 0.640600));
    __patches[0].push_back(source(6.124053, 1.026963, 0.639900));
    __patches[0].push_back(source(6.123912, 1.026139, 0.640200));
    __patches[0].push_back(source(6.122367, 1.027011, -0.636900));
    __patches[0].push_back(source(6.125458, 1.026792, 0.635900));
    __patches[0].push_back(source(6.125176, 1.026332, 0.636800));
    __patches[0].push_back(source(6.123444, 1.026187, 0.635600));
    __patches[0].push_back(source(6.123491, 1.026575, 0.635500));
    __patches[0].push_back(source(6.124521, 1.026526, 0.633300));
    __patches[0].push_back(source(6.124427, 1.026042, 0.632400));
    __patches[0].push_back(source(6.123819, 1.027254, 0.632200));
    __patches[0].push_back(source(6.122321, 1.026211, 0.632000));
    __patches[0].push_back(source(6.122929, 1.027108, 0.628400));
    __patches[0].push_back(source(6.124569, 1.026938, 0.627400));
    __patches[0].push_back(source(6.122134, 1.026526, 0.626900));
    __patches[0].push_back(source(6.124146, 1.026333, 0.627200));
    __patches[0].push_back(source(6.125317, 1.026623, 0.627300));
    __patches[0].push_back(source(6.123070, 1.026430, 0.627200));
    __patches[0].push_back(source(6.122415, 1.025945, 0.626500));
    __patches[0].push_back(source(6.125129, 1.026211, 0.623200));
    __patches[0].push_back(source(6.123117, 1.026551, 0.626000));
    __patches[0].push_back(source(6.124661, 1.026308, 0.621700));
    __patches[0].push_back(source(6.122695, 1.027011, 0.622400));
    __patches[0].push_back(source(6.123678, 1.025945, 0.620500));
    __patches[0].push_back(source(6.124476, 1.027205, 0.621400));
    __patches[0].push_back(source(6.124053, 1.026817, 0.623200));
    __patches[0].push_back(source(6.123866, 1.026357, 0.620300));
    __patches[0].push_back(source(6.124473, 1.025872, 0.618000));
    __patches[0].push_back(source(6.122461, 1.026842, 0.617300));
    __patches[0].push_back(source(6.122929, 1.027205, 0.618700));
    __patches[0].push_back(source(6.122226, 1.027084, -0.617600));
    __patches[0].push_back(source(6.122930, 1.026260, 0.615800));
    __patches[0].push_back(source(6.125318, 1.026720, 0.615600));
    __patches[0].push_back(source(6.123304, 1.025824, 0.613400));
    __patches[0].push_back(source(6.124147, 1.027060, 0.614400));
    __patches[0].push_back(source(6.125225, 1.027011, 0.613100));
    __patches[0].push_back(source(6.122649, 1.026430, 0.608500));
    __patches[0].push_back(source(6.122790, 1.025945, 0.610000));
    __patches[0].push_back(source(6.124848, 1.026187, 0.608300));
    __patches[0].push_back(source(6.124896, 1.026817, 0.609100));
    __patches[0].push_back(source(6.123538, 1.027254, 0.607800));
    __patches[0].push_back(source(6.124241, 1.026745, 0.609700));
    __patches[0].push_back(source(6.122367, 1.026720, 0.607400));
    __patches[0].push_back(source(6.122460, 1.027181, 0.609700));
    __patches[0].push_back(source(6.124757, 1.027132, 0.605700));
    __patches[0].push_back(source(6.125177, 1.026526, 0.605400));
    __patches[0].push_back(source(6.123772, 1.026987, 0.605800));
    __patches[0].push_back(source(6.124099, 1.026042, 0.605600));
    __patches[0].push_back(source(6.122134, 1.026429, 0.604300));
    __patches[0].push_back(source(6.124756, 1.026720, 0.603900));
    __patches[0].push_back(source(6.122228, 1.026066, 0.602300));
    __patches[0].push_back(source(6.123491, 1.026309, 0.600000));
    __patches[0].push_back(source(6.122929, 1.026914, 0.599900));
    __patches[0].push_back(source(6.123444, 1.025945, 0.599900));
    __patches[0].push_back(source(6.124801, 1.025969, 0.599400));
    __patches[0].push_back(source(6.123304, 1.026624, 0.598100));
    __patches[0].push_back(source(6.124661, 1.026454, 0.598800));
    __patches[0].push_back(source(6.124428, 1.027060, 0.594800));
    __patches[0].push_back(source(6.123679, 1.027351, 0.595900));
    __patches[0].push_back(source(6.123304, 1.026915, 0.593800));
    __patches[0].push_back(source(6.122977, 1.026042, 0.594500));
    __patches[0].push_back(source(6.122695, 1.027254, 0.594400));
    __patches[0].push_back(source(6.124193, 1.026551, 0.594400));
    __patches[0].push_back(source(6.124941, 1.026090, 0.592700));
    __patches[0].push_back(source(6.122836, 1.026551, 0.591700));
    __patches[0].push_back(source(6.123585, 1.025799, 0.589200));
    __patches[0].push_back(source(6.123726, 1.027157, 0.589600));
    __patches[0].push_back(source(6.123725, 1.026696, 0.588700));
    __patches[0].push_back(source(6.123772, 1.026236, 0.586800));
    __patches[0].push_back(source(6.122367, 1.027011, -0.585400));
    __patches[0].push_back(source(6.125225, 1.026890, 0.582900));
    __patches[0].push_back(source(6.124239, 1.025848, 0.583100));
    __patches[0].push_back(source(6.123117, 1.025921, 0.584000));
    __patches[0].push_back(source(6.122602, 1.026308, 0.584600));
    __patches[0].push_back(source(6.124427, 1.026260, 0.582900));
    __patches[0].push_back(source(6.125129, 1.026405, 0.580900));
    __patches[0].push_back(source(6.124288, 1.027205, 0.580100));
    __patches[0].push_back(source(6.124053, 1.026454, 0.577100));
    __patches[0].push_back(source(6.122601, 1.026890, 0.577500));
    __patches[0].push_back(source(6.123211, 1.025727, 0.578300));
    __patches[0].push_back(source(6.122226, 1.027108, -0.575900));
    __patches[0].push_back(source(6.123444, 1.026478, 0.575900));
    __patches[0].push_back(source(6.123678, 1.026066, 0.576500));
    __patches[0].push_back(source(6.122789, 1.026139, 0.575700));
    __patches[0].push_back(source(6.124614, 1.026066, 0.574000));
    __patches[0].push_back(source(6.122227, 1.026551, 0.574200));
    __patches[0].push_back(source(6.123960, 1.026672, 0.572400));
    __patches[0].push_back(source(6.124990, 1.026623, 0.570600));
    __patches[0].push_back(source(6.123163, 1.027230, 0.568900));
    __patches[0].push_back(source(6.123023, 1.026769, 0.569400));
    __patches[0].push_back(source(6.123959, 1.025775, 0.568900));
    __patches[0].push_back(source(6.124991, 1.027084, 0.568800));
    __patches[0].push_back(source(6.122274, 1.026332, 0.568200));
    __patches[0].push_back(source(6.123725, 1.026866, 0.565700));
    __patches[0].push_back(source(6.124288, 1.026914, 0.563900));
    __patches[0].push_back(source(6.123257, 1.026212, 0.562600));
    __patches[0].push_back(source(6.122882, 1.027569, -0.562500));
    __patches[0].push_back(source(6.124709, 1.026599, 0.560000));
    __patches[0].push_back(source(6.124287, 1.026163, 0.561100));
    __patches[0].push_back(source(6.122882, 1.026648, 0.560000));
    __patches[0].push_back(source(6.125412, 1.026938, 0.560400));
    __patches[0].push_back(source(6.122414, 1.026817, 0.561700));
    __patches[0].push_back(source(6.123304, 1.026357, 0.558200));
    __patches[0].push_back(source(6.123491, 1.026818, 0.558400));
    __patches[0].push_back(source(6.122039, 1.026987, -0.557600));
    __patches[0].push_back(source(6.125129, 1.026235, 0.556900));
    __patches[0].push_back(source(6.125458, 1.026817, 0.556300));
    __patches[0].push_back(source(6.123023, 1.025775, 0.555900));
    __patches[0].push_back(source(6.123912, 1.025702, 0.555100));
    __patches[0].push_back(source(6.125082, 1.026114, 0.555500));
    __patches[0].push_back(source(6.122835, 1.027060, 0.554200));
    __patches[0].push_back(source(6.125270, 1.026599, 0.555800));
    __patches[0].push_back(source(6.123773, 1.027351, 0.553100));
    __patches[0].push_back(source(6.122415, 1.025945, 0.550700));
    __patches[0].push_back(source(6.123960, 1.027133, 0.549200));
    __patches[0].push_back(source(6.122695, 1.027254, 0.551600));
    __patches[0].push_back(source(6.124286, 1.025993, 0.550400));
    __patches[0].push_back(source(6.124381, 1.026454, 0.551000));
    __patches[0].push_back(source(6.122883, 1.026333, 0.548100));
    __patches[0].push_back(source(6.124522, 1.027181, 0.548800));
    __patches[0].push_back(source(6.123585, 1.026599, 0.546600));
    __patches[0].push_back(source(6.124053, 1.026211, 0.545500));
    __patches[0].push_back(source(6.122415, 1.026187, 0.545400));
    __patches[0].push_back(source(6.123444, 1.026139, 0.546300));
    __patches[0].push_back(source(6.124475, 1.026842, 0.543500));
    __patches[0].push_back(source(6.122462, 1.026066, 0.543800));
    __patches[0].push_back(source(6.123210, 1.027060, 0.542900));
    __patches[0].push_back(source(6.122321, 1.026672, 0.542500));
    __patches[0].push_back(source(6.122367, 1.027011, -0.541900));
    __patches[0].push_back(source(6.124660, 1.025945, 0.541700));
    __patches[0].push_back(source(6.124895, 1.026405, 0.542500));
    __patches[0].push_back(source(6.122883, 1.025872, 0.541500));
    __patches[0].push_back(source(6.122460, 1.027181, 0.540400));
    __patches[0].push_back(source(6.124944, 1.026962, 0.540700));
    __patches[0].push_back(source(6.123398, 1.026721, 0.539400));
    __patches[0].push_back(source(6.124054, 1.027230, 0.537400));
    __patches[0].push_back(source(6.123959, 1.026066, 0.537700));
    __patches[0].push_back(source(6.122368, 1.026429, 0.536400));
    __patches[0].push_back(source(6.123070, 1.026163, 0.534800));
    __patches[0].push_back(source(6.122742, 1.026793, 0.534500));
    __patches[0].push_back(source(6.123725, 1.025775, 0.534600));
    __patches[0].push_back(source(6.122929, 1.027205, 0.532600));
    __patches[0].push_back(source(6.125084, 1.026793, 0.533200));
    __patches[0].push_back(source(6.125130, 1.026502, 0.530700));
    __patches[0].push_back(source(6.124475, 1.027035, 0.532300));
    __patches[0].push_back(source(6.123491, 1.027084, 0.531500));
    __patches[0].push_back(source(6.124194, 1.026624, 0.531100));
    __patches[0].push_back(source(6.124614, 1.026187, 0.531500));
    __patches[0].push_back(source(6.122461, 1.026551, 0.530500));
    __patches[0].push_back(source(6.123538, 1.026963, 0.528600));
    __patches[0].push_back(source(6.122648, 1.026914, 0.526400));
    __patches[0].push_back(source(6.123585, 1.026381, 0.526200));
    __patches[0].push_back(source(6.124897, 1.027108, 0.525500));
    __patches[0].push_back(source(6.123257, 1.025703, 0.524800));
    __patches[0].push_back(source(6.125318, 1.026696, 0.524700));
    __patches[0].push_back(source(6.123726, 1.027230, 0.524800));
    __patches[0].push_back(source(6.124848, 1.026017, 0.523400));
    __patches[0].push_back(source(6.124193, 1.026357, 0.521300));
    __patches[0].push_back(source(6.124194, 1.026793, 0.521700));
    __patches[0].push_back(source(6.122181, 1.026066, 0.521600));
    __patches[0].push_back(source(6.124709, 1.026672, 0.521100));
    __patches[0].push_back(source(6.123912, 1.025945, 0.520000));
    __patches[0].push_back(source(6.122134, 1.026454, 0.519100));
    __patches[0].push_back(source(6.123210, 1.026599, 0.519200));
    __patches[0].push_back(source(6.124288, 1.027108, 0.517800));
    __patches[0].push_back(source(6.122743, 1.025969, 0.517900));
    __patches[0].push_back(source(6.125178, 1.027059, 0.517200));
    __patches[0].push_back(source(6.122695, 1.027011, 0.516700));
    __patches[0].push_back(source(6.122414, 1.027011, -0.516900));
    __patches[0].push_back(source(6.123959, 1.025824, 0.516400));
    __patches[0].push_back(source(6.124850, 1.026866, 0.516100));
    __patches[0].push_back(source(6.123444, 1.027254, 0.514200));
    __patches[0].push_back(source(6.123866, 1.026793, 0.514500));
    __patches[0].push_back(source(6.125270, 1.026308, 0.513900));
    __patches[0].push_back(source(6.122274, 1.026308, 0.513900));
    __patches[0].push_back(source(6.123257, 1.026042, 0.512000));
    __patches[0].push_back(source(6.123678, 1.026478, 0.510500));
    __patches[0].push_back(source(6.122742, 1.026502, 0.510700));
    __patches[0].push_back(source(6.122039, 1.026987, -0.507900));
    __patches[0].push_back(source(6.122461, 1.026817, 0.508400));
    __patches[0].push_back(source(6.124146, 1.026066, 0.507600));
    __patches[0].push_back(source(6.123164, 1.026309, 0.507000));
    __patches[0].push_back(source(6.123117, 1.026818, 0.507400));
    __patches[0].push_back(source(6.122226, 1.027108, -0.504000));
    __patches[0].push_back(source(6.124895, 1.026308, 0.504200));
    __patches[0].push_back(source(6.124473, 1.025872, 0.505300));
    __patches[0].push_back(source(6.122179, 1.027302, 0.505800));
    __patches[0].push_back(source(6.125412, 1.026938, 0.504000));
    __patches[0].push_back(source(6.123819, 1.026987, 0.504000));
    __patches[0].push_back(source(6.123304, 1.025848, 0.505200));
    __patches[0].push_back(source(6.122649, 1.026381, 0.505000));
    __patches[0].push_back(source(6.124568, 1.026502, 0.502600));
    __patches[0].push_back(source(6.124756, 1.027011, 0.502100));
    __patches[0].push_back(source(6.124006, 1.026454, 0.502700));
    __patches[0].push_back(source(6.125129, 1.026114, 0.500200));
    __patches[0].push_back(source(6.122976, 1.027133, 0.498600));
    __patches[0].push_back(source(6.124381, 1.026672, 0.496300));
    __patches[0].push_back(source(6.122882, 1.027569, -0.495800));
    __patches[0].push_back(source(6.123959, 1.026187, 0.496200));
    __patches[0].push_back(source(6.123866, 1.027254, 0.494200));
    __patches[0].push_back(source(6.122649, 1.025751, 0.494200));
    __patches[0].push_back(source(6.124710, 1.027132, 0.493500));
    __patches[0].push_back(source(6.122930, 1.026381, 0.493600));
    __patches[0].push_back(source(6.125037, 1.026696, 0.492800));
    __patches[0].push_back(source(6.124239, 1.025799, 0.493000));
    __patches[0].push_back(source(6.123070, 1.025921, 0.493800));
    __patches[0].push_back(source(6.122741, 1.027230, 0.491100));
    __patches[0].push_back(source(6.123772, 1.026212, 0.490800));
    __patches[0].push_back(source(6.124007, 1.026939, 0.488800));
    __patches[0].push_back(source(6.122695, 1.026648, 0.489600));
    __patches[0].push_back(source(6.122321, 1.026211, 0.490500));
    __patches[0].push_back(source(6.124568, 1.026357, 0.490500));
    __patches[0].push_back(source(6.125411, 1.026768, 0.486700));
    __patches[0].push_back(source(6.123726, 1.027375, 0.486100));
    __patches[0].push_back(source(6.123959, 1.025678, 0.485900));
    __patches[0].push_back(source(6.122882, 1.026939, 0.485400));
    __patches[0].push_back(source(6.124989, 1.026575, 0.485000));
    __patches[0].push_back(source(6.123679, 1.026745, 0.486100));
    __patches[0].push_back(source(6.122977, 1.026066, 0.483700));
    __patches[0].push_back(source(6.122368, 1.026042, 0.481800));
    __patches[0].push_back(source(6.125129, 1.026211, 0.482100));
    __patches[0].push_back(source(6.123912, 1.026333, 0.483700));
    __patches[0].push_back(source(6.122367, 1.026696, 0.481600));
    __patches[0].push_back(source(6.123726, 1.027133, 0.480700));
    __patches[0].push_back(source(6.122133, 1.026914, -0.480400));
    __patches[0].push_back(source(6.123163, 1.027230, 0.479600));
    __patches[0].push_back(source(6.123585, 1.026599, 0.480000));
    __patches[0].push_back(source(6.122181, 1.026381, 0.478500));
    __patches[0].push_back(source(6.124941, 1.026090, 0.477600));
    __patches[0].push_back(source(6.123304, 1.026915, 0.477000));
    __patches[0].push_back(source(6.123304, 1.026430, 0.476800));
    __patches[0].push_back(source(6.122460, 1.027181, 0.475500));
    __patches[0].push_back(source(6.124288, 1.026987, 0.475200));
    __patches[0].push_back(source(6.123444, 1.026139, 0.476200));
    __patches[0].push_back(source(6.122930, 1.025824, 0.475200));
    __patches[0].push_back(source(6.122226, 1.027108, -0.474400));
    __patches[0].push_back(source(6.123818, 1.025872, 0.474000));
    __patches[0].push_back(source(6.122930, 1.026260, 0.473800));
    __patches[0].push_back(source(6.122601, 1.026890, 0.474400));
    __patches[0].push_back(source(6.122087, 1.026526, 0.474900));
    __patches[0].push_back(source(6.124146, 1.026308, 0.473800));
    __patches[0].push_back(source(6.125129, 1.026405, 0.473700));
    __patches[0].push_back(source(6.124801, 1.025969, 0.475200));
    __patches[0].push_back(source(6.124288, 1.027229, 0.471700));
    __patches[0].push_back(source(6.123398, 1.026648, 0.469600));
    __patches[0].push_back(source(6.124569, 1.026938, 0.468000));
    __patches[0].push_back(source(6.125317, 1.026623, 0.468400));
    __patches[0].push_back(source(6.122367, 1.027011, -0.467400));
    __patches[0].push_back(source(6.123069, 1.027521, -0.468000));
    __patches[0].push_back(source(6.125178, 1.026938, 0.467600));
    __patches[0].push_back(source(6.123912, 1.026090, 0.466100));
    __patches[0].push_back(source(6.122602, 1.026236, 0.468100));
    __patches[0].push_back(source(6.124567, 1.026066, 0.465800));
    __patches[0].push_back(source(6.122414, 1.026817, 0.465100));
    __patches[0].push_back(source(6.123491, 1.026818, 0.464800));
    __patches[0].push_back(source(6.124522, 1.026769, 0.464100));
    __patches[0].push_back(source(6.122274, 1.026575, 0.462300));
    __patches[0].push_back(source(6.122134, 1.026114, 0.462400));
    __patches[0].push_back(source(6.123257, 1.025703, 0.460200));
    __patches[0].push_back(source(6.124006, 1.026551, 0.460100));
    __patches[0].push_back(source(6.123398, 1.026527, 0.459200));
    __patches[0].push_back(source(6.122695, 1.027011, 0.459400));
    __patches[0].push_back(source(6.125177, 1.026526, 0.458300));
    __patches[0].push_back(source(6.122882, 1.026672, 0.456300));
    __patches[0].push_back(source(6.122649, 1.026042, 0.457100));
    __patches[0].push_back(source(6.124660, 1.025920, 0.455600));
    __patches[0].push_back(source(6.124006, 1.026672, 0.455600));
    __patches[0].push_back(source(6.123538, 1.027254, 0.455800));
    __patches[0].push_back(source(6.124476, 1.027229, 0.455700));
    __patches[0].push_back(source(6.122695, 1.027254, 0.455200));
    __patches[0].push_back(source(6.123725, 1.025969, 0.455600));
    __patches[0].push_back(source(6.123538, 1.026333, 0.455000));
    __patches[0].push_back(source(6.125599, 1.026889, 0.454200));
    __patches[0].push_back(source(6.122179, 1.027302, 0.453400));
    __patches[0].push_back(source(6.123585, 1.025581, 0.453600));
    __patches[0].push_back(source(6.124803, 1.026744, 0.453000));
    __patches[0].push_back(source(6.125223, 1.026308, 0.452200));
    __patches[0].push_back(source(6.124427, 1.026454, 0.450800));
    __patches[0].push_back(source(6.122039, 1.027011, -0.450000));
    __patches[0].push_back(source(6.123070, 1.025751, 0.449200));
    __patches[0].push_back(source(6.123678, 1.026090, 0.450400));
    __patches[0].push_back(source(6.123117, 1.026454, 0.449800));
    __patches[0].push_back(source(6.123772, 1.026987, 0.448900));
    __patches[0].push_back(source(6.124803, 1.027132, 0.446900));
    __patches[0].push_back(source(6.124194, 1.026769, 0.446000));
    __patches[0].push_back(source(6.124005, 1.025799, 0.445200));
    __patches[0].push_back(source(6.124709, 1.026648, 0.445400));
    __patches[0].push_back(source(6.124427, 1.026236, 0.445700));
    __patches[0].push_back(source(6.125225, 1.027035, 0.444500));
    __patches[0].push_back(source(6.122415, 1.025945, 0.444500));
    __patches[0].push_back(source(6.123070, 1.026163, 0.443300));
    __patches[0].push_back(source(6.124380, 1.026017, 0.443500));
    __patches[0].push_back(source(6.123070, 1.026818, 0.442700));
    __patches[0].push_back(source(6.122742, 1.026430, 0.441800));
    __patches[0].push_back(source(6.123725, 1.026866, 0.441000));
    __patches[0].push_back(source(6.123304, 1.027230, 0.441200));
    __patches[0].push_back(source(6.123444, 1.026212, 0.439500));
    __patches[0].push_back(source(6.124755, 1.026478, 0.438900));
    __patches[0].push_back(source(6.122789, 1.026817, 0.440500));
    __patches[0].push_back(source(6.123444, 1.025945, 0.437700));
    __patches[0].push_back(source(6.125318, 1.026720, 0.437700));
    __patches[0].push_back(source(6.122929, 1.027205, 0.437000));
    __patches[0].push_back(source(6.124007, 1.027108, 0.438100));
    __patches[0].push_back(source(6.122133, 1.026914, -0.436800));
    __patches[0].push_back(source(6.124473, 1.025872, 0.437000));
    __patches[0].push_back(source(6.122790, 1.025921, 0.435600));
    __patches[0].push_back(source(6.122835, 1.027036, 0.435800));
    __patches[0].push_back(source(6.122883, 1.026551, 0.435500));
    __patches[0].push_back(source(6.124850, 1.026866, 0.436300));
    __patches[0].push_back(source(6.124848, 1.026211, 0.436700));
    __patches[0].push_back(source(6.123866, 1.027254, 0.434400));
    __patches[0].push_back(source(6.122181, 1.026381, 0.434000));
    __patches[0].push_back(source(6.122882, 1.027569, -0.434100));
    __patches[0].push_back(source(6.122366, 1.027496, 0.433900));
    __patches[0].push_back(source(6.124287, 1.026163, 0.432100));
    __patches[0].push_back(source(6.122367, 1.027011, -0.431300));
    __patches[0].push_back(source(6.123398, 1.025799, 0.430900));
    __patches[0].push_back(source(6.123210, 1.027036, 0.430200));
    __patches[0].push_back(source(6.122415, 1.026163, 0.430200));
    __patches[0].push_back(source(6.125412, 1.026938, 0.429900));
    __patches[0].push_back(source(6.124569, 1.027157, 0.430000));
    __patches[0].push_back(source(6.123726, 1.027375, 0.428500));
    __patches[0].push_back(source(6.124192, 1.025751, 0.426800));
    __patches[0].push_back(source(6.124894, 1.026066, 0.427200));
    __patches[0].push_back(source(6.122601, 1.026890, 0.427500));
    __patches[0].push_back(source(6.121804, 1.027350, 0.427000));
    __patches[0].push_back(source(6.122603, 1.025751, 0.428400));
    __patches[0].push_back(source(6.122229, 1.025605, -0.429400));
    __patches[0].push_back(source(6.123069, 1.027521, -0.428000));
    __patches[0].push_back(source(6.125458, 1.026817, 0.425600));
    __patches[0].push_back(source(6.124288, 1.027108, 0.425000));
    __patches[0].push_back(source(6.124053, 1.026454, 0.424700));
    __patches[0].push_back(source(6.122226, 1.027108, -0.423800));
    __patches[0].push_back(source(6.124288, 1.026866, 0.422800));
    __patches[0].push_back(source(6.122460, 1.027205, 0.422900));
    __patches[0].push_back(source(6.122461, 1.026526, 0.421500));
    __patches[0].push_back(source(6.124099, 1.025921, 0.420800));
    __patches[0].push_back(source(6.123257, 1.026309, 0.419800));
    __patches[0].push_back(source(6.123444, 1.026745, 0.421300));
    __patches[0].push_back(source(6.122367, 1.026696, 0.419200));
    __patches[0].push_back(source(6.125129, 1.026211, 0.419800));
    __patches[0].push_back(source(6.124568, 1.026551, 0.420400));
    __patches[0].push_back(source(6.124053, 1.026236, 0.418800));
    __patches[0].push_back(source(6.122414, 1.026817, 0.418600));
    __patches[0].push_back(source(6.125085, 1.027108, 0.419900));
    __patches[0].push_back(source(6.123772, 1.027108, 0.419200));
    __patches[0].push_back(source(6.123163, 1.026575, 0.419000));
    __patches[0].push_back(source(6.123772, 1.025751, 0.417600));
    __patches[0].push_back(source(6.124475, 1.026842, 0.418900));
    __patches[0].push_back(source(6.125130, 1.026478, 0.417000));
    __patches[0].push_back(source(6.123070, 1.026018, 0.416300));
    __patches[0].push_back(source(6.125455, 1.025871, -0.417300));
    __patches[0].push_back(source(6.123913, 1.026793, 0.415200));
    __patches[0].push_back(source(6.122930, 1.026381, 0.414700));
    __patches[0].push_back(source(6.122181, 1.026066, 0.413900));
    __patches[0].push_back(source(6.122415, 1.026405, 0.412300));
    __patches[0].push_back(source(6.124194, 1.026575, 0.412800));
    __patches[0].push_back(source(6.123959, 1.025678, 0.412200));
    __patches[0].push_back(source(6.123117, 1.025921, 0.412600));
    __patches[0].push_back(source(6.124848, 1.025993, 0.410100));
    __patches[0].push_back(source(6.123398, 1.027108, 0.410200));
    __patches[0].push_back(source(6.125317, 1.026623, 0.410400));
    __patches[0].push_back(source(6.124614, 1.026332, 0.410400));
    __patches[0].push_back(source(6.122274, 1.026308, 0.408800));
    __patches[0].push_back(source(6.125602, 1.027544, -0.407400));
    __patches[0].push_back(source(6.123725, 1.026236, 0.407300));
    __patches[0].push_back(source(6.124944, 1.026962, 0.405500));
    __patches[0].push_back(source(6.121944, 1.027544, 0.406000));
    __patches[0].push_back(source(6.122883, 1.025848, 0.404300));
    __patches[0].push_back(source(6.122695, 1.027011, 0.405500));
    __patches[0].push_back(source(6.124288, 1.026987, 0.404400));
    __patches[0].push_back(source(6.122695, 1.027254, 0.404300));
    __patches[0].push_back(source(6.123959, 1.025993, 0.404200));
    __patches[0].push_back(source(6.123538, 1.025581, 0.404700));
    __patches[0].push_back(source(6.123304, 1.026042, 0.403700));
    __patches[0].push_back(source(6.122789, 1.026139, 0.403400));
    __patches[0].push_back(source(6.122179, 1.027326, 0.403300));
    __patches[0].push_back(source(6.123398, 1.026454, 0.402100));
    __patches[0].push_back(source(6.122367, 1.027011, -0.403100));
    __patches[0].push_back(source(6.125129, 1.026114, 0.402900));
    __patches[0].push_back(source(6.124193, 1.026333, 0.401300));
    __patches[0].push_back(source(6.122648, 1.026914, 0.399700));
    __patches[0].push_back(source(6.122134, 1.026526, 0.399000));
    __patches[0].push_back(source(6.123069, 1.027254, 0.400000));
    __patches[0].push_back(source(6.124381, 1.026648, 0.399900));
    __patches[0].push_back(source(6.125787, 1.026889, 0.399600));
    __patches[0].push_back(source(6.123210, 1.026818, 0.399500));
    __patches[0].push_back(source(6.124475, 1.027084, 0.399300));
    __patches[0].push_back(source(6.122602, 1.026308, 0.398300));
    __patches[0].push_back(source(6.124709, 1.026623, 0.398700));
    __patches[0].push_back(source(6.124614, 1.026187, 0.399600));
    __patches[0].push_back(source(6.122039, 1.027011, -0.397600));
    __patches[0].push_back(source(6.125270, 1.026308, 0.398500));
    __patches[0].push_back(source(6.122229, 1.025605, -0.397900));
    __patches[0].push_back(source(6.124054, 1.027230, 0.396700));
    __patches[0].push_back(source(6.122603, 1.025727, 0.395400));
    __patches[0].push_back(source(6.123398, 1.026648, 0.394600));
    __patches[0].push_back(source(6.122226, 1.027108, -0.395300));
    __patches[0].push_back(source(6.123023, 1.027108, 0.394400));
    __patches[0].push_back(source(6.124707, 1.025920, 0.394700));
    __patches[0].push_back(source(6.122321, 1.026211, 0.394400));
    __patches[0].push_back(source(6.123351, 1.026212, 0.393800));
    __patches[0].push_back(source(6.123585, 1.025799, 0.393200));
    __patches[0].push_back(source(6.123912, 1.026139, 0.392700));
    __patches[0].push_back(source(6.124850, 1.027060, 0.393300));
    __patches[0].push_back(source(6.125271, 1.026865, 0.392700));
    __patches[0].push_back(source(6.123632, 1.027205, 0.391100));
    __patches[0].push_back(source(6.124053, 1.026866, 0.389500));
    __patches[0].push_back(source(6.122133, 1.026914, -0.388000));
    __patches[0].push_back(source(6.122366, 1.027472, 0.389200));
    __patches[0].push_back(source(6.123632, 1.027351, 0.388700));
    __patches[0].push_back(source(6.122883, 1.026284, 0.389700));
    __patches[0].push_back(source(6.123069, 1.027521, -0.389300));
    __patches[0].push_back(source(6.123023, 1.026745, 0.388400));
    __patches[0].push_back(source(6.122414, 1.026720, 0.387900));
    __patches[0].push_back(source(6.122134, 1.026114, 0.386700));
    __patches[0].push_back(source(6.123959, 1.026381, 0.386800));
    __patches[0].push_back(source(6.123678, 1.025921, 0.386900));
    __patches[0].push_back(source(6.121804, 1.027350, 0.386300));
    __patches[0].push_back(source(6.123538, 1.026818, 0.384500));
    __patches[0].push_back(source(6.122461, 1.026842, 0.385400));
    __patches[0].push_back(source(6.124520, 1.025848, 0.385200));
    __patches[0].push_back(source(6.123491, 1.026939, 0.384600));
    __patches[0].push_back(source(6.124989, 1.026575, 0.384600));
    __patches[0].push_back(source(6.121945, 1.027181, 0.385400));
    __patches[0].push_back(source(6.124287, 1.026454, 0.382900));
    __patches[0].push_back(source(6.125459, 1.026938, 0.382100));
    __patches[0].push_back(source(6.123023, 1.025775, 0.381100));
    __patches[0].push_back(source(6.123444, 1.026527, 0.379900));
    __patches[0].push_back(source(6.122415, 1.025945, 0.378900));
    __patches[0].push_back(source(6.124147, 1.027036, 0.377900));
    __patches[0].push_back(source(6.123257, 1.027205, 0.378000));
    __patches[0].push_back(source(6.122274, 1.026575, 0.377700));
    __patches[0].push_back(source(6.125455, 1.025871, -0.377600));
    __patches[0].push_back(source(6.124335, 1.027278, 0.378100));
    __patches[0].push_back(source(6.124894, 1.026066, 0.377900));
    __patches[0].push_back(source(6.124100, 1.026696, 0.377400));
    __patches[0].push_back(source(6.122134, 1.026381, 0.376900));
    __patches[0].push_back(source(6.124146, 1.026066, 0.377100));
    __patches[0].push_back(source(6.124803, 1.026744, 0.375800));
    __patches[0].push_back(source(6.124848, 1.026332, 0.375800));
    __patches[0].push_back(source(6.125602, 1.027544, -0.376800));
    __patches[0].push_back(source(6.124053, 1.026211, 0.376100));
    __patches[0].push_back(source(6.125318, 1.026720, 0.376100));
    __patches[0].push_back(source(6.122743, 1.025969, 0.375900));
    __patches[0].push_back(source(6.123585, 1.027084, 0.374800));
    __patches[0].push_back(source(6.124239, 1.025824, 0.374200));
    __patches[0].push_back(source(6.123538, 1.026333, 0.374500));
    __patches[0].push_back(source(6.123257, 1.025703, 0.373800));
    __patches[0].push_back(source(6.122647, 1.027520, -0.373200));
    __patches[0].push_back(source(6.123632, 1.026672, 0.373400));
    __patches[0].push_back(source(6.122695, 1.026430, 0.370800));
    __patches[0].push_back(source(6.123257, 1.025872, 0.370400));
    __patches[0].push_back(source(6.123023, 1.026163, 0.369800));
    __patches[0].push_back(source(6.122460, 1.027181, 0.369900));
    __patches[0].push_back(source(6.123538, 1.027278, 0.370400));
    __patches[0].push_back(source(6.122226, 1.027108, -0.370100));
    __patches[0].push_back(source(6.124709, 1.027011, 0.369900));
    __patches[0].push_back(source(6.122182, 1.025605, -0.369100));
    __patches[0].push_back(source(6.125082, 1.026211, 0.369600));
    __patches[0].push_back(source(6.124893, 1.025605, 0.368500));
    __patches[0].push_back(source(6.122836, 1.026648, 0.369400));
    __patches[0].push_back(source(6.124522, 1.027205, 0.367600));
    __patches[0].push_back(source(6.125741, 1.027228, 0.367000));
    __patches[0].push_back(source(6.122929, 1.026939, 0.365600));
    __patches[0].push_back(source(6.124007, 1.026939, 0.365600));
    __patches[0].push_back(source(6.122368, 1.026042, 0.365100));
    __patches[0].push_back(source(6.125084, 1.026793, 0.366000));
    __patches[0].push_back(source(6.124192, 1.025727, 0.366600));
    __patches[0].push_back(source(6.123069, 1.027278, 0.364300));
    __patches[0].push_back(source(6.122133, 1.026914, -0.364800));
    __patches[0].push_back(source(6.122601, 1.026890, 0.365100));
    __patches[0].push_back(source(6.122367, 1.027011, -0.364600));
    __patches[0].push_back(source(6.122882, 1.027569, -0.362800));
    __patches[0].push_back(source(6.125270, 1.026574, 0.362700));
    __patches[0].push_back(source(6.124803, 1.027132, 0.362800));
    __patches[0].push_back(source(6.124380, 1.025993, 0.362800));
    __patches[0].push_back(source(6.123491, 1.026115, 0.361500));
    __patches[0].push_back(source(6.122929, 1.027205, 0.361400));
    __patches[0].push_back(source(6.123538, 1.025581, 0.361800));
    __patches[0].push_back(source(6.125782, 1.025847, -0.361100));
    __patches[0].push_back(source(6.122414, 1.026817, 0.362200));
    __patches[0].push_back(source(6.122555, 1.026551, 0.361500));
    __patches[0].push_back(source(6.125363, 1.026283, 0.360300));
    __patches[0].push_back(source(6.124333, 1.026139, 0.360800));
    __patches[0].push_back(source(6.124474, 1.026502, 0.360400));
    __patches[0].push_back(source(6.124288, 1.027157, 0.360000));
    __patches[0].push_back(source(6.122649, 1.025751, 0.360400));
    __patches[0].push_back(source(6.121944, 1.027544, 0.359600));
    __patches[0].push_back(source(6.123304, 1.026357, 0.358400));
    __patches[0].push_back(source(6.123772, 1.026963, 0.358400));
    __patches[0].push_back(source(6.123772, 1.025872, 0.358200));
    __patches[0].push_back(source(6.125225, 1.026962, 0.358600));
    __patches[0].push_back(source(6.122134, 1.026454, 0.358200));
    __patches[0].push_back(source(6.125130, 1.026502, 0.357400));
    __patches[0].push_back(source(6.123913, 1.026599, 0.357500));
    __patches[0].push_back(source(6.124569, 1.026914, 0.357800));
    __patches[0].push_back(source(6.122649, 1.026236, 0.356700));
    __patches[0].push_back(source(6.122976, 1.027108, 0.357700));
    __patches[0].push_back(source(6.122179, 1.027302, 0.356300));
    __patches[0].push_back(source(6.124848, 1.025969, 0.356500));
    __patches[0].push_back(source(6.124427, 1.026405, 0.355400));
    __patches[0].push_back(source(6.125646, 1.026865, 0.356200));
    __patches[0].push_back(source(6.124522, 1.026817, 0.355100));
    __patches[0].push_back(source(6.123257, 1.026624, 0.354000));
    __patches[0].push_back(source(6.122039, 1.027011, -0.354700));
    __patches[0].push_back(source(6.123773, 1.027375, 0.352800));
    __patches[0].push_back(source(6.123678, 1.026042, 0.352300));
    __patches[0].push_back(source(6.122696, 1.026066, 0.352200));
    __patches[0].push_back(source(6.124988, 1.026114, 0.350300));
    __patches[0].push_back(source(6.125131, 1.027084, 0.350700));
    __patches[0].push_back(source(6.123866, 1.027254, 0.350400));
    __patches[0].push_back(source(6.123257, 1.026963, 0.350700));
    __patches[0].push_back(source(6.125602, 1.027544, -0.350000));
    __patches[0].push_back(source(6.122648, 1.026648, 0.349800));
    __patches[0].push_back(source(6.123959, 1.025678, 0.350100));
    __patches[0].push_back(source(6.124006, 1.026502, 0.349300));
    __patches[0].push_back(source(6.121945, 1.027181, 0.349600));
    __patches[0].push_back(source(6.123069, 1.027496, -0.347900));
    __patches[0].push_back(source(6.122182, 1.025605, -0.347900));
    __patches[0].push_back(source(6.122134, 1.026357, 0.349000));
    __patches[0].push_back(source(6.124709, 1.026599, 0.348300));
    __patches[0].push_back(source(6.123398, 1.026721, 0.349200));
    __patches[0].push_back(source(6.124006, 1.026066, 0.347800));
    __patches[0].push_back(source(6.122366, 1.027472, 0.347400));
    __patches[0].push_back(source(6.121804, 1.027326, 0.346100));
    __patches[0].push_back(source(6.123257, 1.026260, 0.345700));
    __patches[0].push_back(source(6.122883, 1.025848, 0.346700));
    __patches[0].push_back(source(6.122695, 1.027230, 0.347000));
    __patches[0].push_back(source(6.123725, 1.026866, 0.344900));
    __patches[0].push_back(source(6.122695, 1.027011, 0.344700));
    __patches[0].push_back(source(6.124005, 1.025799, 0.344500));
    __patches[0].push_back(source(6.125458, 1.026792, 0.345000));
    __patches[0].push_back(source(6.123070, 1.025945, 0.343500));
    __patches[0].push_back(source(6.122648, 1.026769, 0.342300));
    __patches[0].push_back(source(6.124893, 1.025605, 0.342200));
    __patches[0].push_back(source(6.125455, 1.025871, -0.341100));
    __patches[0].push_back(source(6.122226, 1.027108, -0.341600));
    __patches[0].push_back(source(6.122647, 1.027520, -0.342600));
    __patches[0].push_back(source(6.124193, 1.026308, 0.341800));
    __patches[0].push_back(source(6.124660, 1.025945, 0.340800));
    __patches[0].push_back(source(6.122321, 1.026211, 0.340900));
    __patches[0].push_back(source(6.125129, 1.026356, 0.340500));
    __patches[0].push_back(source(6.123070, 1.025751, 0.339500));
    __patches[0].push_back(source(6.125786, 1.026865, 0.339100));
    __patches[0].push_back(source(6.123585, 1.026575, 0.339300));
    __patches[0].push_back(source(6.122367, 1.026987, -0.339500));
    __patches[0].push_back(source(6.122929, 1.026527, 0.337500));
    __patches[0].push_back(source(6.124241, 1.026769, 0.337600));
    __patches[0].push_back(source(6.124194, 1.027254, 0.337800));
    __patches[0].push_back(source(6.123444, 1.026187, 0.336700));
    __patches[0].push_back(source(6.122601, 1.026890, 0.336500));
    __patches[0].push_back(source(6.123632, 1.027157, 0.336800));
    __patches[0].push_back(source(6.122134, 1.026114, 0.336600));
    __patches[0].push_back(source(6.122133, 1.026914, -0.336300));
    __patches[0].push_back(source(6.123070, 1.026042, 0.335700));
    __patches[0].push_back(source(6.124849, 1.026454, 0.335300));
    __patches[0].push_back(source(6.124803, 1.026866, 0.335500));
    __patches[0].push_back(source(6.123351, 1.025799, 0.334600));
    __patches[0].push_back(source(6.125741, 1.027228, 0.333500));
    __patches[0].push_back(source(6.124614, 1.026066, 0.332600));
    __patches[0].push_back(source(6.123913, 1.027133, 0.332800));
    __patches[0].push_back(source(6.122835, 1.027036, 0.332100));
    __patches[0].push_back(source(6.122368, 1.026405, 0.331500));
    __patches[0].push_back(source(6.124520, 1.025872, 0.331800));
    __patches[0].push_back(source(6.123725, 1.026745, 0.330700));
    __patches[0].push_back(source(6.122414, 1.026817, 0.331000));
    __patches[0].push_back(source(6.125783, 1.025871, -0.330300));
    __patches[0].push_back(source(6.125129, 1.026114, 0.330200));
    __patches[0].push_back(source(6.123819, 1.026284, 0.330700));
    __patches[0].push_back(source(6.124991, 1.027108, 0.331700));
    __patches[0].push_back(source(6.122883, 1.026260, 0.330700));
    __patches[0].push_back(source(6.124240, 1.026599, 0.330600));
    __patches[0].push_back(source(6.122274, 1.026308, 0.329100));
    __patches[0].push_back(source(6.124709, 1.026720, 0.328600));
    __patches[0].push_back(source(6.122460, 1.027205, 0.328900));
    __patches[0].push_back(source(6.121948, 1.025872, 0.327300));
    __patches[0].push_back(source(6.123116, 1.027545, -0.328200));
    __patches[0].push_back(source(6.124288, 1.026963, 0.327100));
    __patches[0].push_back(source(6.125459, 1.026938, 0.327300));
    __patches[0].push_back(source(6.123726, 1.027375, 0.326500));
    __patches[0].push_back(source(6.125784, 1.026307, 0.326300));
    __patches[0].push_back(source(6.125318, 1.026696, 0.326800));
    __patches[0].push_back(source(6.121945, 1.027205, 0.325500));
    __patches[0].push_back(source(6.123398, 1.025945, 0.325500));
    __patches[0].push_back(source(6.123304, 1.026430, 0.325300));
    __patches[0].push_back(source(6.124521, 1.026357, 0.325200));
    __patches[0].push_back(source(6.123538, 1.025581, 0.324800));
    __patches[0].push_back(source(6.125602, 1.027544, -0.325100));
    __patches[0].push_back(source(6.124006, 1.025993, 0.325000));
    __patches[0].push_back(source(6.123163, 1.027060, 0.324400));
    __patches[0].push_back(source(6.122229, 1.025581, -0.323700));
    __patches[0].push_back(source(6.122603, 1.025727, 0.325200));
    __patches[0].push_back(source(6.123069, 1.027302, 0.323700));
    __patches[0].push_back(source(6.124239, 1.025751, 0.324500));
    __patches[0].push_back(source(6.122835, 1.027375, -0.324400));
    __patches[0].push_back(source(6.125597, 1.026210, 0.324300));
    __patches[0].push_back(source(6.124053, 1.026866, 0.324600));
    __patches[0].push_back(source(6.122414, 1.026720, 0.323200));
    __patches[0].push_back(source(6.124614, 1.026211, 0.323400));
    __patches[0].push_back(source(6.122414, 1.026987, -0.322900));
    __patches[0].push_back(source(6.124989, 1.026575, 0.322900));
    __patches[0].push_back(source(6.121944, 1.027544, 0.322200));
    __patches[0].push_back(source(6.123023, 1.026793, 0.322500));
    __patches[0].push_back(source(6.124990, 1.026938, 0.321000));
    __patches[0].push_back(source(6.123163, 1.026575, 0.320400));
    __patches[0].push_back(source(6.123070, 1.026139, 0.319100));
    __patches[0].push_back(source(6.123632, 1.026454, 0.319300));
    __patches[0].push_back(source(6.122976, 1.027205, 0.318000));
    __patches[0].push_back(source(6.124006, 1.026187, 0.318500));
    __patches[0].push_back(source(6.122087, 1.026526, 0.317800));
    __patches[0].push_back(source(6.125037, 1.026672, 0.318600));
    __patches[0].push_back(source(6.122462, 1.026090, 0.317500));
    __patches[0].push_back(source(6.123491, 1.026793, 0.317600));
    __patches[0].push_back(source(6.124476, 1.027229, 0.316400));
    __patches[0].push_back(source(6.123772, 1.026187, 0.315800));
    __patches[0].push_back(source(6.122226, 1.027108, -0.316800));
    __patches[0].push_back(source(6.125082, 1.026211, 0.316500));
    __patches[0].push_back(source(6.122648, 1.027254, 0.315500));
    __patches[0].push_back(source(6.124522, 1.026963, 0.314800));
    __patches[0].push_back(source(6.124522, 1.027084, 0.314600));
    __patches[0].push_back(source(6.124006, 1.026672, 0.314200));
    __patches[0].push_back(source(6.123912, 1.025702, 0.314600));
    __patches[0].push_back(source(6.124893, 1.025605, 0.313200));
    __patches[0].push_back(source(6.123023, 1.026381, 0.313100));
    __patches[0].push_back(source(6.122179, 1.027302, 0.312900));
    __patches[0].push_back(source(6.124848, 1.025993, 0.313800));
    __patches[0].push_back(source(6.125455, 1.025871, -0.312900));
    __patches[0].push_back(source(6.123491, 1.027278, 0.312700));
    __patches[0].push_back(source(6.122789, 1.026793, 0.312300));
    __patches[0].push_back(source(6.122133, 1.026914, -0.311700));
    __patches[0].push_back(source(6.125177, 1.026502, 0.311900));
    __patches[0].push_back(source(6.125785, 1.026550, -0.311600));
    __patches[0].push_back(source(6.124333, 1.026066, 0.311700));
    __patches[0].push_back(source(6.124662, 1.026502, 0.310700));
    __patches[0].push_back(source(6.123304, 1.026042, 0.310800));
    __patches[0].push_back(source(6.124335, 1.027326, 0.311100));
    __patches[0].push_back(source(6.122415, 1.025945, 0.309900));
    __patches[0].push_back(source(6.123210, 1.026842, 0.308500));
    __patches[0].push_back(source(6.123211, 1.025703, 0.309300));
    __patches[0].push_back(source(6.123163, 1.027230, 0.308900));
    __patches[0].push_back(source(6.122601, 1.026890, 0.308800));
    __patches[0].push_back(source(6.122695, 1.026430, 0.309300));
    __patches[0].push_back(source(6.123913, 1.026793, 0.309000));
    __patches[0].push_back(source(6.122647, 1.027520, -0.307300));
    __patches[0].push_back(source(6.122413, 1.027496, 0.309900));
    __patches[0].push_back(source(6.122229, 1.025605, -0.307800));
    __patches[0].push_back(source(6.123538, 1.026357, 0.306600));
    __patches[0].push_back(source(6.123772, 1.026987, 0.306600));
    __patches[0].push_back(source(6.123678, 1.025945, 0.305800));
    __patches[0].push_back(source(6.124333, 1.026211, 0.306400));
    __patches[0].push_back(source(6.122274, 1.026575, 0.306100));
    __patches[0].push_back(source(6.125225, 1.026987, 0.306300));
    __patches[0].push_back(source(6.123444, 1.027521, -0.305800));
    __patches[0].push_back(source(6.122743, 1.026115, 0.305700));
    __patches[0].push_back(source(6.122039, 1.027011, -0.304800));
    __patches[0].push_back(source(6.125363, 1.026283, 0.304300));
    __patches[0].push_back(source(6.124286, 1.025848, 0.303200));
    __patches[0].push_back(source(6.125787, 1.026889, 0.303900));
    __patches[0].push_back(source(6.121804, 1.027350, 0.304000));
    __patches[0].push_back(source(6.124288, 1.027133, 0.303900));
    __patches[0].push_back(source(6.122603, 1.025727, 0.304600));
    __patches[0].push_back(source(6.122134, 1.026114, 0.304600));
    __patches[0].push_back(source(6.123398, 1.026624, 0.303500));
    __patches[0].push_back(source(6.125742, 1.027568, -0.303800));
    __patches[0].push_back(source(6.122882, 1.027569, -0.302800));
    __patches[0].push_back(source(6.123631, 1.025799, 0.303000));
    __patches[0].push_back(source(6.124709, 1.026623, 0.302900));
    __patches[0].push_back(source(6.124803, 1.027132, 0.302200));
    __patches[0].push_back(source(6.123257, 1.026987, 0.301900));
    __patches[0].push_back(source(6.122134, 1.026429, 0.300600));
    __patches[0].push_back(source(6.122695, 1.026987, 0.300900));
    __patches[0].push_back(source(6.123116, 1.027521, -0.300600));
    __patches[0].push_back(source(6.123631, 1.026066, 0.300700));
    __patches[0].push_back(source(6.125741, 1.027228, 0.300200));
    __patches[0].push_back(source(6.124895, 1.026332, 0.300300));
    __patches[0].push_back(source(6.124381, 1.026454, 0.299400));
    __patches[0].push_back(source(6.124241, 1.026866, 0.299500));
    __patches[0].push_back(source(6.122602, 1.026308, 0.298300));
    __patches[0].push_back(source(6.122367, 1.027011, -0.299200));
    __patches[0].push_back(source(6.122414, 1.026817, 0.299000));
    __patches[0].push_back(source(6.122835, 1.027375, -0.299000));
    __patches[0].push_back(source(6.122601, 1.027326, 0.301000));
    __patches[0].push_back(source(6.125412, 1.026792, 0.299000));
    __patches[0].push_back(source(6.124942, 1.026163, 0.298400));
    __patches[0].push_back(source(6.122930, 1.025872, 0.298000));
    __patches[0].push_back(source(6.125783, 1.025871, -0.297600));
    __patches[0].push_back(source(6.121945, 1.027205, 0.298000));
    __patches[0].push_back(source(6.123632, 1.027351, 0.296400));
    __patches[0].push_back(source(6.122368, 1.026187, 0.295400));
    __patches[0].push_back(source(6.123772, 1.027108, 0.294400));
    __patches[0].push_back(source(6.123398, 1.026502, 0.294400));
    __patches[0].push_back(source(6.123351, 1.025799, 0.293600));
    __patches[0].push_back(source(6.124146, 1.026308, 0.293200));
    __patches[0].push_back(source(6.122226, 1.027108, -0.293500));
    __patches[0].push_back(source(6.124849, 1.026793, 0.293100));
    __patches[0].push_back(source(6.122460, 1.027181, 0.293200));
    __patches[0].push_back(source(6.125555, 1.027544, -0.293500));
    __patches[0].push_back(source(6.124147, 1.026696, 0.293500));
    __patches[0].push_back(source(6.123538, 1.025581, 0.293900));
    __patches[0].push_back(source(6.125317, 1.026599, 0.292700));
    __patches[0].push_back(source(6.125785, 1.026550, -0.293600));
    __patches[0].push_back(source(6.124239, 1.025727, 0.292600));
    __patches[0].push_back(source(6.122836, 1.026672, 0.293400));
    __patches[0].push_back(source(6.124894, 1.026041, 0.292600));
    __patches[0].push_back(source(6.125597, 1.026210, 0.291200));
    __patches[0].push_back(source(6.122087, 1.026357, 0.290900));
    __patches[0].push_back(source(6.123773, 1.027230, 0.290400));
    __patches[0].push_back(source(6.121948, 1.025872, 0.290000));
    __patches[0].push_back(source(6.123538, 1.026915, 0.290000));
    __patches[0].push_back(source(6.122695, 1.027230, 0.289700));
    __patches[0].push_back(source(6.123912, 1.026042, 0.290400));
    __patches[0].push_back(source(6.124053, 1.026502, 0.289900));
    __patches[0].push_back(source(6.122883, 1.026333, 0.289300));
    __patches[0].push_back(source(6.124893, 1.025605, 0.289000));
    __patches[0].push_back(source(6.123163, 1.026696, 0.289100));
    __patches[0].push_back(source(6.122977, 1.026066, 0.289400));
    __patches[0].push_back(source(6.123773, 1.027375, 0.288700));
    __patches[0].push_back(source(6.122182, 1.025581, -0.288600));
    __patches[0].push_back(source(6.125131, 1.026817, 0.286800));
    __patches[0].push_back(source(6.122882, 1.026939, 0.286100));
    __patches[0].push_back(source(6.124007, 1.026939, 0.286200));
    __patches[0].push_back(source(6.124660, 1.025920, 0.285700));
    __patches[0].push_back(source(6.125176, 1.026332, 0.286200));
    __patches[0].push_back(source(6.124662, 1.026938, 0.285900));
    __patches[0].push_back(source(6.123069, 1.027302, 0.285200));
    __patches[0].push_back(source(6.122133, 1.026914, -0.284700));
    __patches[0].push_back(source(6.122929, 1.026527, 0.285300));
    __patches[0].push_back(source(6.123070, 1.025945, 0.285000));
    __patches[0].push_back(source(6.123959, 1.025799, 0.284100));
    __patches[0].push_back(source(6.123444, 1.026212, 0.283500));
    __patches[0].push_back(source(6.123585, 1.026575, 0.283600));
    __patches[0].push_back(source(6.124428, 1.026672, 0.282300));
    __patches[0].push_back(source(6.124053, 1.026236, 0.283100));
    __patches[0].push_back(source(6.124663, 1.027084, 0.283300));
    __patches[0].push_back(source(6.125315, 1.026041, 0.282300));
    __patches[0].push_back(source(6.125455, 1.025871, -0.283400));
    __patches[0].push_back(source(6.122508, 1.026551, 0.281500));
    __patches[0].push_back(source(6.121804, 1.027350, 0.281700));
    __patches[0].push_back(source(6.122414, 1.026817, 0.281700));
    __patches[0].push_back(source(6.122647, 1.027520, -0.282200));
    __patches[0].push_back(source(6.125599, 1.026913, 0.281400));
    __patches[0].push_back(source(6.123023, 1.027133, 0.281000));
    __patches[0].push_back(source(6.122413, 1.027496, 0.280100));
    __patches[0].push_back(source(6.124520, 1.025848, 0.280100));
    __patches[0].push_back(source(6.124614, 1.026332, 0.280600));
    __patches[0].push_back(source(6.122181, 1.026066, 0.280600));
    __patches[0].push_back(source(6.124335, 1.027326, 0.279400));
    __patches[0].push_back(source(6.122274, 1.026308, 0.279300));
    __patches[0].push_back(source(6.122226, 1.027108, -0.279600));
    __patches[0].push_back(source(6.122835, 1.027375, -0.277900));
    __patches[0].push_back(source(6.121945, 1.027181, 0.279000));
    __patches[0].push_back(source(6.123444, 1.026721, 0.277100));
    __patches[0].push_back(source(6.122601, 1.026890, 0.277900));
    __patches[0].push_back(source(6.122367, 1.026987, -0.282000));
    __patches[0].push_back(source(6.123023, 1.025775, 0.277400));
    __patches[0].push_back(source(6.124522, 1.026842, 0.277800));
    __patches[0].push_back(source(6.125129, 1.026114, 0.277000));
    __patches[0].push_back(source(6.123959, 1.025654, 0.277000));
    __patches[0].push_back(source(6.125036, 1.026526, 0.277200));
    __patches[0].push_back(source(6.124053, 1.026090, 0.277100));
    __patches[0].push_back(source(6.123210, 1.026284, 0.277400));
    __patches[0].push_back(source(6.124194, 1.027036, 0.277400));
    __patches[0].push_back(source(6.123772, 1.025872, 0.275800));
    __patches[0].push_back(source(6.124054, 1.027230, 0.275800));
    __patches[0].push_back(source(6.122743, 1.025945, 0.276100));
    __patches[0].push_back(source(6.121944, 1.027544, 0.277000));
    __patches[0].push_back(source(6.123444, 1.027521, -0.277000));
    __patches[0].push_back(source(6.125412, 1.026938, 0.275300));
    __patches[0].push_back(source(6.124240, 1.026551, 0.274400));
    __patches[0].push_back(source(6.122415, 1.026381, 0.273900));
    __patches[0].push_back(source(6.122649, 1.025751, 0.274000));
    __patches[0].push_back(source(6.123632, 1.027181, 0.274100));
    __patches[0].push_back(source(6.125784, 1.026307, 0.273300));
    __patches[0].push_back(source(6.122182, 1.025605, -0.273200));
    __patches[0].push_back(source(6.122367, 1.026696, 0.274100));
    __patches[0].push_back(source(6.122835, 1.027036, 0.273100));
    __patches[0].push_back(source(6.122039, 1.027011, -0.273200));
    __patches[0].push_back(source(6.125785, 1.026550, -0.273100));
    __patches[0].push_back(source(6.122179, 1.027351, 0.272800));
    __patches[0].push_back(source(6.123116, 1.027545, -0.273800));
    __patches[0].push_back(source(6.125783, 1.025871, -0.272600));
    __patches[0].push_back(source(6.123772, 1.026818, 0.272600));
    __patches[0].push_back(source(6.122696, 1.026066, 0.272100));
    __patches[0].push_back(source(6.125789, 1.027568, -0.271200));
    __patches[0].push_back(source(6.124521, 1.026526, 0.272300));
    __patches[0].push_back(source(6.124380, 1.026090, 0.272500));
    __patches[0].push_back(source(6.122976, 1.027205, 0.272100));
    __patches[0].push_back(source(6.125458, 1.026817, 0.271700));
    __patches[0].push_back(source(6.125085, 1.027108, 0.270700));
    __patches[0].push_back(source(6.125035, 1.026211, 0.270800));
    __patches[0].push_back(source(6.123631, 1.026066, 0.269500));
    __patches[0].push_back(source(6.123211, 1.025678, 0.270300));
    __patches[0].push_back(source(6.125318, 1.026696, 0.269100));
    __patches[0].push_back(source(6.124053, 1.026842, 0.269500));
    __patches[0].push_back(source(6.123023, 1.026866, 0.268700));
    __patches[0].push_back(source(6.123304, 1.026405, 0.268400));
    __patches[0].push_back(source(6.125741, 1.027204, 0.268600));
    __patches[0].push_back(source(6.124893, 1.025605, 0.267500));
    __patches[0].push_back(source(6.125555, 1.027544, -0.267800));
    __patches[0].push_back(source(6.123117, 1.026163, 0.267900));
    __patches[0].push_back(source(6.124193, 1.026357, 0.267200));
    __patches[0].push_back(source(6.123772, 1.025751, 0.266800));
    __patches[0].push_back(source(6.123164, 1.026042, 0.266600));
    __patches[0].push_back(source(6.122274, 1.026575, 0.265800));
    __patches[0].push_back(source(6.124380, 1.025993, 0.265300));
    __patches[0].push_back(source(6.124990, 1.026938, 0.265600));
    __patches[0].push_back(source(6.123069, 1.027302, 0.264800));
    __patches[0].push_back(source(6.123726, 1.027375, 0.263500));
    __patches[0].push_back(source(6.124709, 1.026696, 0.263400));
    __patches[0].push_back(source(6.124803, 1.027132, 0.263800));
    __patches[0].push_back(source(6.123819, 1.026987, 0.262700));
    __patches[0].push_back(source(6.123585, 1.025581, 0.263000));
    __patches[0].push_back(source(6.122460, 1.027181, 0.263000));
    __patches[0].push_back(source(6.122275, 1.025726, 0.263400));
    __patches[0].push_back(source(6.124848, 1.025993, 0.263100));
    __patches[0].push_back(source(6.122133, 1.026914, -0.263500));
    __patches[0].push_back(source(6.122414, 1.026817, 0.263000));
    __patches[0].push_back(source(6.122647, 1.027520, -0.262100));
    __patches[0].push_back(source(6.122226, 1.027108, -0.262600));
    __patches[0].push_back(source(6.121945, 1.027181, 0.262800));
    __patches[0].push_back(source(6.123912, 1.026187, 0.262400));
    __patches[0].push_back(source(6.124522, 1.026939, 0.260800));
    __patches[0].push_back(source(6.121948, 1.025872, 0.260700));
    __patches[0].push_back(source(6.123913, 1.026648, 0.260400));
    __patches[0].push_back(source(6.125783, 1.025992, -0.260500));
    __patches[0].push_back(source(6.125786, 1.026865, 0.260600));
    __patches[0].push_back(source(6.122601, 1.027326, 0.261000));
    __patches[0].push_back(source(6.124239, 1.025581, -0.261200));
    __patches[0].push_back(source(6.122321, 1.026211, 0.260500));
    __patches[0].push_back(source(6.122415, 1.025920, 0.258900));
    __patches[0].push_back(source(6.124192, 1.025727, 0.258900));
    __patches[0].push_back(source(6.123304, 1.026624, 0.259300));
    __patches[0].push_back(source(6.123960, 1.027133, 0.259700));
    __patches[0].push_back(source(6.123351, 1.027108, 0.258500));
    __patches[0].push_back(source(6.125317, 1.026599, 0.258400));
    __patches[0].push_back(source(6.123585, 1.026260, 0.258700));
    __patches[0].push_back(source(6.123117, 1.026430, 0.258300));
    __patches[0].push_back(source(6.122087, 1.026526, 0.258300));
    __patches[0].push_back(source(6.125789, 1.027374, 0.258500));
    __patches[0].push_back(source(6.125315, 1.026041, 0.257800));
    __patches[0].push_back(source(6.123491, 1.026793, 0.258000));
    __patches[0].push_back(source(6.122367, 1.026987, -0.258400));
    __patches[0].push_back(source(6.122601, 1.026890, 0.259400));
    __patches[0].push_back(source(6.124709, 1.026599, 0.257800));
    __patches[0].push_back(source(6.122883, 1.026260, 0.257800));
    __patches[0].push_back(source(6.123116, 1.027521, -0.257400));
    __patches[0].push_back(source(6.122229, 1.025605, -0.257600));
    __patches[0].push_back(source(6.125785, 1.026550, -0.256400));
    __patches[0].push_back(source(6.125363, 1.026283, 0.256900));
    __patches[0].push_back(source(6.122413, 1.027520, 0.256500));
    __patches[0].push_back(source(6.122087, 1.026357, 0.255700));
    __patches[0].push_back(source(6.125787, 1.026962, 0.255800));
    __patches[0].push_back(source(6.124522, 1.027108, 0.256800));
    __patches[0].push_back(source(6.123163, 1.027230, 0.256100));
    __patches[0].push_back(source(6.123725, 1.025993, 0.255800));
    __patches[0].push_back(source(6.122835, 1.027375, -0.255000));
    __patches[0].push_back(source(6.124708, 1.026502, 0.255200));
    __patches[0].push_back(source(6.123351, 1.025799, 0.255200));
    __patches[0].push_back(source(6.122648, 1.027230, 0.255200));
    __patches[0].push_back(source(6.123444, 1.027521, -0.255100));
    __patches[0].push_back(source(6.124380, 1.026139, 0.254700));
    __patches[0].push_back(source(6.125455, 1.025847, -0.254500));
    __patches[0].push_back(source(6.123819, 1.026551, 0.253700));
    __patches[0].push_back(source(6.124522, 1.027229, 0.253600));
    __patches[0].push_back(source(6.122695, 1.026624, 0.252900));
    __patches[0].push_back(source(6.121804, 1.027326, 0.252300));
    __patches[0].push_back(source(6.123210, 1.027011, 0.251800));
    __patches[0].push_back(source(6.122883, 1.025872, 0.251900));
    __patches[0].push_back(source(6.124052, 1.025969, 0.251500));
    __patches[0].push_back(source(6.125597, 1.026210, 0.251200));
    __patches[0].push_back(source(6.123491, 1.027278, 0.251300));
    __patches[0].push_back(source(6.121947, 1.026041, -0.251200));
    __patches[0].push_back(source(6.122179, 1.027278, 0.250400));
    __patches[0].push_back(source(6.122134, 1.026138, 0.250400));
    __patches[0].push_back(source(6.122603, 1.025727, 0.250000));
    __patches[0].push_back(source(6.124288, 1.027133, 0.250100));
    __patches[0].push_back(source(6.125221, 1.025750, -0.250500));
    __patches[0].push_back(source(6.125225, 1.026987, 0.250800));
    __patches[0].push_back(source(6.124567, 1.025872, 0.250800));
    __patches[0].push_back(source(6.125602, 1.027544, -0.250100));
    __patches[0].push_back(source(6.125177, 1.026502, 0.250500));
    __patches[0].push_back(source(6.124893, 1.025605, 0.249200));
    __patches[0].push_back(source(6.123257, 1.026212, 0.249700));
    __patches[0].push_back(source(6.123444, 1.026648, 0.249500));
    __patches[0].push_back(source(6.124427, 1.026429, 0.249100));
    __patches[0].push_back(source(6.125783, 1.025871, -0.248900));
    __patches[0].push_back(source(6.124661, 1.026066, 0.249000));
    __patches[0].push_back(source(6.122601, 1.026745, 0.249500));
    __patches[0].push_back(source(6.122321, 1.026333, 0.249500));
    __patches[0].push_back(source(6.122882, 1.027569, -0.248500));
    __patches[0].push_back(source(6.124053, 1.026866, 0.248700));
    __patches[0].push_back(source(6.122695, 1.027011, 0.247700));
    __patches[0].push_back(source(6.125741, 1.027253, 0.246800));
    __patches[0].push_back(source(6.124006, 1.026430, 0.246600));
    __patches[0].push_back(source(6.122226, 1.027108, -0.246900));
    __patches[0].push_back(source(6.124894, 1.026066, 0.245500));
    __patches[0].push_back(source(6.125083, 1.026696, 0.245700));
    __patches[0].push_back(source(6.122462, 1.026090, 0.245200));
    __patches[0].push_back(source(6.124522, 1.026769, 0.244400));
    __patches[0].push_back(source(6.123959, 1.025654, 0.245200));
    __patches[0].push_back(source(6.123351, 1.025969, 0.245000));
    __patches[0].push_back(source(6.124335, 1.027351, 0.244700));
    __patches[0].push_back(source(6.122086, 1.026744, 0.244800));
    __patches[0].push_back(source(6.124848, 1.026332, 0.243900));
    __patches[0].push_back(source(6.123772, 1.026187, 0.244000));
    __patches[0].push_back(source(6.122649, 1.026260, 0.243900));
    __patches[0].push_back(source(6.122882, 1.026696, 0.244900));
    __patches[0].push_back(source(6.123023, 1.027108, 0.244100));
    __patches[0].push_back(source(6.125131, 1.026817, 0.243200));
    __patches[0].push_back(source(6.122039, 1.027011, -0.243700));
    __patches[0].push_back(source(6.124288, 1.026987, 0.243700));
    __patches[0].push_back(source(6.122182, 1.025581, -0.242900));
    __patches[0].push_back(source(6.123070, 1.025727, 0.243400));
    __patches[0].push_back(source(6.123444, 1.026502, 0.243100));
    __patches[0].push_back(source(6.125789, 1.027568, -0.242400));
    __patches[0].push_back(source(6.125320, 1.027496, 0.242300));
    __patches[0].push_back(source(6.124287, 1.026236, 0.242000));
    __patches[0].push_back(source(6.125789, 1.027374, 0.242300));
    __patches[0].push_back(source(6.125175, 1.026090, 0.241100));
    __patches[0].push_back(source(6.122367, 1.026987, -0.241500));
    __patches[0].push_back(source(6.122976, 1.027205, 0.240800));
    __patches[0].push_back(source(6.123773, 1.027375, 0.240700));
    __patches[0].push_back(source(6.123070, 1.026842, 0.240700));
    __patches[0].push_back(source(6.122835, 1.027375, -0.240700));
    __patches[0].push_back(source(6.121945, 1.027181, 0.241000));
    __patches[0].push_back(source(6.124989, 1.026575, 0.239800));
    __patches[0].push_back(source(6.122742, 1.026430, 0.240100));
    __patches[0].push_back(source(6.124006, 1.026696, 0.240200));
    __patches[0].push_back(source(6.124707, 1.025920, 0.240400));
    __patches[0].push_back(source(6.123069, 1.027327, 0.240600));
    __patches[0].push_back(source(6.123538, 1.025581, 0.239900));
    __patches[0].push_back(source(6.121947, 1.026041, -0.239500));
    __patches[0].push_back(source(6.125459, 1.026938, 0.238700));
    __patches[0].push_back(source(6.121948, 1.025872, 0.238400));
    __patches[0].push_back(source(6.122742, 1.026817, 0.238200));
    __patches[0].push_back(source(6.125784, 1.026307, 0.238700));
    __patches[0].push_back(source(6.125785, 1.026550, -0.237800));
    __patches[0].push_back(source(6.122133, 1.026914, -0.237800));
    __patches[0].push_back(source(6.122414, 1.026817, 0.238000));
    __patches[0].push_back(source(6.123772, 1.025872, 0.237800));
    __patches[0].push_back(source(6.123585, 1.027108, 0.237500));
    __patches[0].push_back(source(6.122977, 1.026163, 0.236600));
    __patches[0].push_back(source(6.124896, 1.026453, 0.236500));
    __patches[0].push_back(source(6.122647, 1.027520, -0.236500));
    __patches[0].push_back(source(6.122413, 1.027496, 0.239400));
    __patches[0].push_back(source(6.122181, 1.026066, 0.236200));
    __patches[0].push_back(source(6.123538, 1.026357, 0.236500));
    __patches[0].push_back(source(6.122695, 1.027230, 0.236500));
    __patches[0].push_back(source(6.124569, 1.026938, 0.236500));
    __patches[0].push_back(source(6.125783, 1.025992, -0.236100));
    __patches[0].push_back(source(6.123538, 1.026745, 0.236000));
    __patches[0].push_back(source(6.125786, 1.026865, 0.234500));
    __patches[0].push_back(source(6.124286, 1.025848, 0.235000));
    __patches[0].push_back(source(6.123070, 1.025969, 0.235100));
    __patches[0].push_back(source(6.124991, 1.027108, 0.235000));
    __patches[0].push_back(source(6.122134, 1.026405, 0.235400));
    __patches[0].push_back(source(6.124239, 1.025581, -0.235300));
    __patches[0].push_back(source(6.122179, 1.027375, 0.235100));
    __patches[0].push_back(source(6.123116, 1.027521, -0.234400));
    __patches[0].push_back(source(6.125788, 1.027156, 0.234100));
    __patches[0].push_back(source(6.124192, 1.025727, 0.234200));
    __patches[0].push_back(source(6.125555, 1.027544, -0.234300));
    __patches[0].push_back(source(6.125223, 1.026308, 0.234100));
    __patches[0].push_back(source(6.123725, 1.026866, 0.233500));
    __patches[0].push_back(source(6.124006, 1.026211, 0.232700));
    __patches[0].push_back(source(6.122367, 1.026720, 0.233000));
    __patches[0].push_back(source(6.125318, 1.026720, 0.233200));
    __patches[0].push_back(source(6.123211, 1.025678, 0.232900));
    __patches[0].push_back(source(6.123585, 1.027327, 0.233600));
    __patches[0].push_back(source(6.122883, 1.026308, 0.232200));
    __patches[0].push_back(source(6.122835, 1.026963, 0.232200));
    __patches[0].push_back(source(6.122226, 1.027108, -0.232400));
    __patches[0].push_back(source(6.125082, 1.026211, 0.232500));
    __patches[0].push_back(source(6.124194, 1.027230, 0.232200));
    __patches[0].push_back(source(6.125455, 1.025871, -0.232200));
    __patches[0].push_back(source(6.122415, 1.025920, 0.231600));
    __patches[0].push_back(source(6.124893, 1.025605, 0.230900));
    __patches[0].push_back(source(6.121804, 1.027278, 0.231200));
    __patches[0].push_back(source(6.124381, 1.026502, 0.231300));
    __patches[0].push_back(source(6.124099, 1.026090, 0.231500));
    __patches[0].push_back(source(6.123163, 1.026575, 0.230600));
    __patches[0].push_back(source(6.124147, 1.027060, 0.230500));
    __patches[0].push_back(source(6.121944, 1.027544, 0.229900));
    __patches[0].push_back(source(6.124944, 1.026962, 0.230300));
    __patches[0].push_back(source(6.124848, 1.025993, 0.229900));
    __patches[0].push_back(source(6.121805, 1.026769, -0.229400));
    __patches[0].push_back(source(6.122696, 1.026066, 0.229000));
    __patches[0].push_back(source(6.123585, 1.025799, 0.228400));
    __patches[0].push_back(source(6.124753, 1.025726, 0.229000));
    __patches[0].push_back(source(6.122182, 1.025605, -0.227800));
    __patches[0].push_back(source(6.123163, 1.026721, 0.228200));
    __patches[0].push_back(source(6.124428, 1.026648, 0.228200));
    __patches[0].push_back(source(6.122460, 1.027157, 0.227900));
    __patches[0].push_back(source(6.122275, 1.025726, 0.228700));
    __patches[0].push_back(source(6.125221, 1.025726, -0.227600));
    __patches[0].push_back(source(6.122508, 1.026551, 0.227000));
    __patches[0].push_back(source(6.123444, 1.027521, -0.226800));
    __patches[0].push_back(source(6.124240, 1.026333, 0.226700));
    __patches[0].push_back(source(6.123725, 1.025969, 0.226700));
    __patches[0].push_back(source(6.125315, 1.026041, 0.226400));
    __patches[0].push_back(source(6.123491, 1.026939, 0.226000));
    __patches[0].push_back(source(6.125787, 1.026962, 0.225900));
    __patches[0].push_back(source(6.122368, 1.026187, 0.225100));
    __patches[0].push_back(source(6.121947, 1.026041, -0.225600));
    __patches[0].push_back(source(6.125783, 1.025871, -0.225100));
    __patches[0].push_back(source(6.124241, 1.026866, 0.225200));
    __patches[0].push_back(source(6.123866, 1.027254, 0.224500));
    __patches[0].push_back(source(6.122601, 1.026890, 0.224200));
    __patches[0].push_back(source(6.122367, 1.026987, -0.227100));
    __patches[0].push_back(source(6.122601, 1.027326, 0.224800));
    __patches[0].push_back(source(6.124989, 1.026502, 0.225000));
    __patches[0].push_back(source(6.124709, 1.027011, 0.223900));
    __patches[0].push_back(source(6.124333, 1.026090, 0.223500));
    __patches[0].push_back(source(6.125320, 1.027496, 0.223600));
    __patches[0].push_back(source(6.122603, 1.025727, 0.223300));
    __patches[0].push_back(source(6.125458, 1.026792, 0.223200));
    __patches[0].push_back(source(6.125785, 1.026550, -0.223300));
    __patches[0].push_back(source(6.125789, 1.027374, 0.223700));
    __patches[0].push_back(source(6.123257, 1.026212, 0.223700));
    __patches[0].push_back(source(6.125789, 1.027568, -0.223700));
    __patches[0].push_back(source(6.124147, 1.026672, 0.222300));
    __patches[0].push_back(source(6.122647, 1.027520, -0.222500));
    __patches[0].push_back(source(6.122087, 1.026526, 0.221900));
    __patches[0].push_back(source(6.121804, 1.027350, 0.221600));
    __patches[0].push_back(source(6.123678, 1.026066, 0.221600));
    __patches[0].push_back(source(6.123116, 1.027230, 0.221700));
    __patches[0].push_back(source(6.123959, 1.025630, 0.222200));
    __patches[0].push_back(source(6.123023, 1.025775, 0.222300));
    __patches[0].push_back(source(6.124709, 1.026648, 0.222500));
    __patches[0].push_back(source(6.123257, 1.026405, 0.221500));
    __patches[0].push_back(source(6.124239, 1.025581, -0.221500));
    __patches[0].push_back(source(6.125741, 1.027253, 0.221200));
    __patches[0].push_back(source(6.125555, 1.027544, -0.221200));
    __patches[0].push_back(source(6.123632, 1.026624, 0.221000));
    __patches[0].push_back(source(6.124803, 1.027132, 0.220600));
    __patches[0].push_back(source(6.122087, 1.026357, 0.220300));
    __patches[0].push_back(source(6.123632, 1.027181, 0.220200));
    __patches[0].push_back(source(6.125597, 1.026210, 0.220200));
    __patches[0].push_back(source(6.125317, 1.026599, 0.220100));
    __patches[0].push_back(source(6.123960, 1.027569, 0.219800));
    __patches[0].push_back(source(6.122835, 1.027375, -0.219600));
    __patches[0].push_back(source(6.122226, 1.027108, -0.219200));
    __patches[0].push_back(source(6.124474, 1.026381, 0.219100));
    __patches[0].push_back(source(6.124567, 1.025848, 0.219000));
    __patches[0].push_back(source(6.122179, 1.027278, 0.219100));
    __patches[0].push_back(source(6.124006, 1.026018, 0.218200));
    __patches[0].push_back(source(6.122133, 1.026914, -0.218600));
    __patches[0].push_back(source(6.122414, 1.026817, 0.220000));
    __patches[0].push_back(source(6.122368, 1.026357, 0.219200));
    __patches[0].push_back(source(6.123210, 1.026284, 0.218500));
    __patches[0].push_back(source(6.122648, 1.027230, 0.218500));
    __patches[0].push_back(source(6.123116, 1.027521, -0.218800));
    __patches[0].push_back(source(6.124005, 1.025751, 0.217600));
    __patches[0].push_back(source(6.122977, 1.025896, 0.217300));
    __patches[0].push_back(source(6.123819, 1.026963, 0.217300));
    __patches[0].push_back(source(6.122134, 1.026138, 0.217600));
    __patches[0].push_back(source(6.125598, 1.026526, -0.217500));
    __patches[0].push_back(source(6.124382, 1.027375, 0.216700));
    __patches[0].push_back(source(6.124803, 1.026769, 0.216000));
    __patches[0].push_back(source(6.122413, 1.027545, 0.216100));
    __patches[0].push_back(source(6.121945, 1.027181, 0.216800));
    __patches[0].push_back(source(6.121948, 1.025872, 0.216000));
    __patches[0].push_back(source(6.123163, 1.027060, 0.216200));
    __patches[0].push_back(source(6.123773, 1.027375, 0.216000));
    __patches[0].push_back(source(6.124893, 1.025605, 0.215400));
    __patches[0].push_back(source(6.123351, 1.025799, 0.215700));
    __patches[0].push_back(source(6.122182, 1.025605, -0.215300));
    __patches[0].push_back(source(6.123398, 1.026648, 0.215700));
    __patches[0].push_back(source(6.123772, 1.027084, 0.216200));
    __patches[0].push_back(source(6.122086, 1.026744, 0.215600));
    __patches[0].push_back(source(6.124287, 1.026211, 0.215300));
    __patches[0].push_back(source(6.124568, 1.026526, 0.214100));
    __patches[0].push_back(source(6.125783, 1.025992, -0.214200));
    __patches[0].push_back(source(6.123819, 1.026793, 0.214700));
    __patches[0].push_back(source(6.123585, 1.025581, 0.214100));
    __patches[0].push_back(source(6.123117, 1.026042, 0.213600));
    __patches[0].push_back(source(6.123491, 1.027496, -0.213900));
    __patches[0].push_back(source(6.125318, 1.026889, 0.213800));
    __patches[0].push_back(source(6.125784, 1.026307, 0.214000));
    __patches[0].push_back(source(6.122835, 1.027569, -0.213200));
    __patches[0].push_back(source(6.123866, 1.026624, 0.213500));
    __patches[0].push_back(source(6.121947, 1.026041, -0.212600));
    __patches[0].push_back(source(6.122039, 1.027011, -0.212400));
    __patches[0].push_back(source(6.121805, 1.026962, 0.212800));
    __patches[0].push_back(source(6.122976, 1.027205, 0.212200));
    __patches[0].push_back(source(6.124942, 1.026187, 0.211700));
    __patches[0].push_back(source(6.123819, 1.026260, 0.211800));
    __patches[0].push_back(source(6.125225, 1.027035, 0.211200));
    __patches[0].push_back(source(6.122649, 1.026333, 0.210900));
    __patches[0].push_back(source(6.125320, 1.027496, 0.211100));
    __patches[0].push_back(source(6.122882, 1.026963, 0.210000));
    __patches[0].push_back(source(6.121805, 1.026769, -0.209500));
    __patches[0].push_back(source(6.125223, 1.026526, 0.209600));
    __patches[0].push_back(source(6.124569, 1.027108, 0.209400));
    __patches[0].push_back(source(6.125129, 1.026114, 0.209700));
    __patches[0].push_back(source(6.125555, 1.027544, -0.209100));
    __patches[0].push_back(source(6.122228, 1.026042, 0.208600));
    __patches[0].push_back(source(6.123069, 1.027327, 0.209000));
    __patches[0].push_back(source(6.124192, 1.025727, 0.209000));
    __patches[0].push_back(source(6.125783, 1.025871, -0.208700));
    __patches[0].push_back(source(6.123351, 1.025969, 0.208600));
    __patches[0].push_back(source(6.124007, 1.026939, 0.208500));
    __patches[0].push_back(source(6.124614, 1.026187, 0.208400));
    __patches[0].push_back(source(6.122929, 1.026527, 0.208400));
    __patches[0].push_back(source(6.125084, 1.026817, 0.208600));
    __patches[0].push_back(source(6.122601, 1.026890, 0.208400));
    __patches[0].push_back(source(6.122367, 1.026987, -0.211800));
    __patches[0].push_back(source(6.125789, 1.027374, 0.208200));
    __patches[0].push_back(source(6.123913, 1.027157, 0.207800));
    __patches[0].push_back(source(6.125787, 1.026962, 0.207200));
    __patches[0].push_back(source(6.125410, 1.026283, 0.206700));
    __patches[0].push_back(source(6.123444, 1.026454, 0.206900));
    __patches[0].push_back(source(6.122415, 1.026163, 0.206900));
    __patches[0].push_back(source(6.124753, 1.025726, 0.207100));
    __patches[0].push_back(source(6.122274, 1.026575, 0.206100));
    __patches[0].push_back(source(6.125785, 1.026574, -0.206500));
    __patches[0].push_back(source(6.124522, 1.027229, 0.206400));
    __patches[0].push_back(source(6.122133, 1.026914, -0.206600));
    __patches[0].push_back(source(6.122835, 1.027375, -0.206000));
    __patches[0].push_back(source(6.122648, 1.027011, 0.205500));
    __patches[0].push_back(source(6.125221, 1.025750, -0.205600));
    __patches[0].push_back(source(6.122226, 1.027108, -0.205100));
    __patches[0].push_back(source(6.122460, 1.027157, 0.205400));
    __patches[0].push_back(source(6.124848, 1.026357, 0.205000));
    __patches[0].push_back(source(6.125692, 1.026671, -0.205700));
    __patches[0].push_back(source(6.122603, 1.025727, 0.205500));
    __patches[0].push_back(source(6.123070, 1.026842, 0.205200));
    __patches[0].push_back(source(6.123164, 1.025678, 0.205000));
    __patches[0].push_back(source(6.125315, 1.026017, 0.205300));
    __patches[0].push_back(source(6.125599, 1.026865, 0.205400));
    __patches[0].push_back(source(6.123070, 1.026139, 0.204800));
    __patches[0].push_back(source(6.123538, 1.027302, 0.205000));
    __patches[0].push_back(source(6.122367, 1.026696, 0.205100));
    __patches[0].push_back(source(6.125788, 1.027156, 0.204600));
    __patches[0].push_back(source(6.123959, 1.026478, 0.204400));
    __patches[0].push_back(source(6.125455, 1.025847, -0.204200));
    __patches[0].push_back(source(6.124660, 1.025920, 0.204300));
    __patches[0].push_back(source(6.125454, 1.025605, 0.204400));
    __patches[0].push_back(source(6.124052, 1.025993, 0.204600));
    __patches[0].push_back(source(6.122647, 1.027520, -0.204500));
    __patches[0].push_back(source(6.122182, 1.025581, -0.204900));
    __patches[0].push_back(source(6.122366, 1.027448, 0.204400));
    __patches[0].push_back(source(6.125789, 1.027568, -0.204500));
    __patches[0].push_back(source(6.122836, 1.026333, 0.204500));
    __patches[0].push_back(source(6.124053, 1.026842, 0.203300));
    __patches[0].push_back(source(6.125318, 1.026696, 0.203100));
    __patches[0].push_back(source(6.124006, 1.026211, 0.203000));
    __patches[0].push_back(source(6.122275, 1.025726, 0.202800));
    __patches[0].push_back(source(6.125034, 1.025823, -0.202400));
    __patches[0].push_back(source(6.124382, 1.027375, 0.202100));
    __patches[0].push_back(source(6.123257, 1.026987, 0.201900));
    __patches[0].push_back(source(6.124569, 1.026938, 0.201900));
    __patches[0].push_back(source(6.121804, 1.027350, 0.201300));
    __patches[0].push_back(source(6.123491, 1.026769, 0.201300));
    __patches[0].push_back(source(6.122696, 1.026090, 0.201500));
    __patches[0].push_back(source(6.124522, 1.026817, 0.201800));
    __patches[0].push_back(source(6.124239, 1.025581, -0.201900));
    __patches[0].push_back(source(6.124848, 1.026017, 0.202000));
    __patches[0].push_back(source(6.122182, 1.025920, -0.201200));
    __patches[0].push_back(source(6.123960, 1.027569, 0.201200));
    __patches[0].push_back(source(6.122415, 1.025920, 0.201500));
    __patches[0].push_back(source(6.124427, 1.026429, 0.201500));
    __patches[0].push_back(source(6.122413, 1.027545, 0.201000));
    __patches[0].push_back(source(6.123163, 1.027545, -0.200400));
    __patches[0].push_back(source(6.122134, 1.026405, 0.201400));
    __patches[0].push_back(source(6.123772, 1.025872, 0.200900));
    __patches[0].push_back(source(6.122414, 1.026817, 0.201600));
    __patches[0].push_back(source(6.125597, 1.026210, 0.199800));
    __patches[0].push_back(source(6.123304, 1.026648, 0.199700));
    __patches[0].push_back(source(6.121945, 1.027181, 0.199100));
    __patches[0].push_back(source(6.123959, 1.026115, 0.199400));
    __patches[0].push_back(source(6.123959, 1.025630, 0.199700));
    __patches[0].push_back(source(6.122601, 1.027326, 0.199100));
    __patches[0].push_back(source(6.124709, 1.026648, 0.199400));
    __patches[0].push_back(source(6.122930, 1.025824, 0.198400));
    __patches[0].push_back(source(6.123210, 1.027230, 0.198600));
    __patches[0].push_back(source(6.122695, 1.026430, 0.198500));
    __patches[0].push_back(source(6.124893, 1.025605, 0.198100));
    __patches[0].push_back(source(6.125786, 1.026865, 0.198000));
    __patches[0].push_back(source(6.123726, 1.027399, 0.198000));
    __patches[0].push_back(source(6.125598, 1.026526, -0.198500));
    __patches[0].push_back(source(6.125741, 1.027253, 0.197600));
    __patches[0].push_back(source(6.122882, 1.026696, 0.197000));
    __patches[0].push_back(source(6.122179, 1.027108, -0.197400));
    __patches[0].push_back(source(6.121805, 1.026962, 0.197400));
    __patches[0].push_back(source(6.122179, 1.027375, 0.196400));
    __patches[0].push_back(source(6.122415, 1.026042, 0.196700));
    __patches[0].push_back(source(6.121947, 1.026041, -0.197000));
    __patches[0].push_back(source(6.123491, 1.027496, -0.196700));
    __patches[0].push_back(source(6.125783, 1.025992, -0.196700));
    __patches[0].push_back(source(6.123023, 1.027108, 0.196100));
    __patches[0].push_back(source(6.124288, 1.026963, 0.196900));
    __patches[0].push_back(source(6.125317, 1.026599, 0.196000));
    __patches[0].push_back(source(6.124662, 1.026526, 0.195800));
    __patches[0].push_back(source(6.124944, 1.026938, 0.195700));
    __patches[0].push_back(source(6.124614, 1.026066, 0.195300));
    __patches[0].push_back(source(6.123631, 1.026066, 0.195200));
    __patches[0].push_back(source(6.125320, 1.027496, 0.195400));
    __patches[0].push_back(source(6.123538, 1.026551, 0.195200));
    __patches[0].push_back(source(6.122367, 1.026987, -0.195000));
    __patches[0].push_back(source(6.122742, 1.026817, 0.195600));
    __patches[0].push_back(source(6.124147, 1.027254, 0.194800));
    __patches[0].push_back(source(6.121804, 1.027253, 0.194100));
    __patches[0].push_back(source(6.122696, 1.026211, 0.194300));
    __patches[0].push_back(source(6.125411, 1.026768, 0.193900));
    __patches[0].push_back(source(6.124803, 1.027132, 0.194300));
    __patches[0].push_back(source(6.125555, 1.027544, -0.194200));
    __patches[0].push_back(source(6.121805, 1.026769, -0.194300));
    __patches[0].push_back(source(6.122039, 1.027011, -0.193800));
    __patches[0].push_back(source(6.122648, 1.027230, 0.194000));
    __patches[0].push_back(source(6.122134, 1.026138, 0.193500));
    __patches[0].push_back(source(6.125692, 1.026671, -0.193500));
    __patches[0].push_back(source(6.125082, 1.026211, 0.193900));
    __patches[0].push_back(source(6.123257, 1.026187, 0.193600));
    __patches[0].push_back(source(6.124147, 1.027060, 0.193800));
    __patches[0].push_back(source(6.122086, 1.026744, 0.192600));
    __patches[0].push_back(source(6.124333, 1.026042, 0.192500));
    __patches[0].push_back(source(6.125132, 1.027350, 0.192700));
    __patches[0].push_back(source(6.121806, 1.026526, -0.192700));
    __patches[0].push_back(source(6.122040, 1.026357, 0.192500));
    __patches[0].push_back(source(6.122229, 1.025605, -0.192300));
    __patches[0].push_back(source(6.123585, 1.025775, 0.192600));
    __patches[0].push_back(source(6.122647, 1.027520, -0.191700));
    __patches[0].push_back(source(6.124240, 1.026599, 0.192100));
    __patches[0].push_back(source(6.124192, 1.025727, 0.191800));
    __patches[0].push_back(source(6.123117, 1.025678, 0.191800));
    __patches[0].push_back(source(6.125789, 1.027374, 0.191300));
    __patches[0].push_back(source(6.125459, 1.026938, 0.191100));
    __patches[0].push_back(source(6.121948, 1.025872, 0.191200));
    __patches[0].push_back(source(6.121944, 1.027544, 0.191200));
    __patches[0].push_back(source(6.124989, 1.026575, 0.190900));
    __patches[0].push_back(source(6.123304, 1.027133, 0.191100));
    __patches[0].push_back(source(6.123959, 1.026405, 0.191000));
    __patches[0].push_back(source(6.125785, 1.026574, -0.190400));
    __patches[0].push_back(source(6.124239, 1.025581, -0.190600));
    __patches[0].push_back(source(6.122413, 1.027569, 0.190000));
    __patches[0].push_back(source(6.123585, 1.026236, 0.189500));
    __patches[0].push_back(source(6.122368, 1.026357, 0.190200));
    __patches[0].push_back(source(6.123679, 1.026696, 0.189400));
    __patches[0].push_back(source(6.124333, 1.026139, 0.189400));
    __patches[0].push_back(source(6.124753, 1.025702, 0.189500));
    __patches[0].push_back(source(6.123632, 1.027205, 0.189700));
    __patches[0].push_back(source(6.122460, 1.027157, 0.188700));
    __patches[0].push_back(source(6.123538, 1.025581, 0.188900));
    __patches[0].push_back(source(6.122178, 1.027520, -0.188900));
    __patches[0].push_back(source(6.124895, 1.026332, 0.188700));
    __patches[0].push_back(source(6.121947, 1.026041, -0.188900));
    __patches[0].push_back(source(6.122883, 1.026284, 0.188500));
    __patches[0].push_back(source(6.123163, 1.026721, 0.188600));
    __patches[0].push_back(source(6.125783, 1.025871, -0.188400));
    __patches[0].push_back(source(6.123163, 1.027545, -0.188300));
    __patches[0].push_back(source(6.122508, 1.026551, 0.187800));
    __patches[0].push_back(source(6.122415, 1.025920, 0.187600));
    __patches[0].push_back(source(6.122133, 1.026914, -0.187200));
    __patches[0].push_back(source(6.125315, 1.026017, 0.187200));
    __patches[0].push_back(source(6.124475, 1.027084, 0.187200));
    __patches[0].push_back(source(6.122182, 1.025920, -0.186900));
    __patches[0].push_back(source(6.124381, 1.026696, 0.187300));
    __patches[0].push_back(source(6.123070, 1.025945, 0.187300));
    __patches[0].push_back(source(6.122321, 1.026187, 0.186900));
    __patches[0].push_back(source(6.124193, 1.026308, 0.186600));
    __patches[0].push_back(source(6.124286, 1.025848, 0.187100));
    __patches[0].push_back(source(6.123678, 1.025945, 0.186800));
    __patches[0].push_back(source(6.125085, 1.027108, 0.186600));
    __patches[0].push_back(source(6.124851, 1.027326, -0.187300));
    __patches[0].push_back(source(6.124848, 1.025969, 0.186100));
    __patches[0].push_back(source(6.122414, 1.026817, 0.187000));
    __patches[0].push_back(source(6.123491, 1.026939, 0.187100));
    __patches[0].push_back(source(6.123585, 1.027302, 0.186100));
    __patches[0].push_back(source(6.125221, 1.025726, -0.185700));
    __patches[0].push_back(source(6.124893, 1.025605, 0.187100));
    __patches[0].push_back(source(6.125454, 1.025605, 0.186500));
    __patches[0].push_back(source(6.125784, 1.026307, 0.185600));
    __patches[0].push_back(source(6.125789, 1.027568, -0.185900));
    __patches[0].push_back(source(6.123117, 1.026551, 0.185400));
    __patches[0].push_back(source(6.123538, 1.026381, 0.185100));
    __patches[0].push_back(source(6.122835, 1.027375, -0.185200));
    __patches[0].push_back(source(6.122977, 1.025606, 0.185600));
    __patches[0].push_back(source(6.122179, 1.027278, 0.185300));
    __patches[0].push_back(source(6.122226, 1.027108, -0.185900));
    __patches[0].push_back(source(6.125551, 1.026501, -0.185100));
    __patches[0].push_back(source(6.123117, 1.026018, 0.185000));
    __patches[0].push_back(source(6.122648, 1.026914, 0.184400));
    __patches[0].push_back(source(6.123960, 1.026769, 0.184600));
    __patches[0].push_back(source(6.125502, 1.025871, -0.184900));
    __patches[0].push_back(source(6.124894, 1.026066, 0.185300));
    __patches[0].push_back(source(6.123069, 1.027327, 0.184200));
    __patches[0].push_back(source(6.125178, 1.026987, 0.183700));
    __patches[0].push_back(source(6.125177, 1.026502, 0.183700));
    __patches[0].push_back(source(6.124660, 1.025896, 0.183700));
    __patches[0].push_back(source(6.125034, 1.025823, -0.184700));
    __patches[0].push_back(source(6.121805, 1.026962, 0.183500));
    __patches[0].push_back(source(6.123023, 1.027205, 0.183700));
    __patches[0].push_back(source(6.124476, 1.027254, 0.183100));
    __patches[0].push_back(source(6.125222, 1.025944, 0.182700));
    __patches[0].push_back(source(6.123210, 1.026284, 0.183300));
    __patches[0].push_back(source(6.125317, 1.026599, 0.182900));
    __patches[0].push_back(source(6.123772, 1.025751, 0.182400));
    __patches[0].push_back(source(6.122603, 1.025727, 0.182000));
    __patches[0].push_back(source(6.123819, 1.026236, 0.182100));
    __patches[0].push_back(source(6.122182, 1.025581, -0.182200));
    __patches[0].push_back(source(6.123960, 1.027569, 0.181700));
    __patches[0].push_back(source(6.122835, 1.026963, 0.181000));
    __patches[0].push_back(source(6.121805, 1.026769, -0.180900));
    __patches[0].push_back(source(6.125555, 1.027544, -0.180800));
    __patches[0].push_back(source(6.124616, 1.026963, 0.180700));
    __patches[0].push_back(source(6.125692, 1.026671, -0.180600));
    __patches[0].push_back(source(6.124007, 1.026939, 0.180900));
    __patches[0].push_back(source(6.125318, 1.026696, 0.180200));
    __patches[0].push_back(source(6.124758, 1.027569, 0.180300));
    __patches[0].push_back(source(6.123304, 1.026818, 0.180500));
    __patches[0].push_back(source(6.123491, 1.027496, -0.180200));
    __patches[0].push_back(source(6.123726, 1.027399, 0.183100));
    __patches[0].push_back(source(6.121804, 1.027350, 0.180000));
    __patches[0].push_back(source(6.124710, 1.027060, 0.179700));
    __patches[0].push_back(source(6.125413, 1.027180, -0.179800));
    __patches[0].push_back(source(6.125741, 1.027253, 0.179900));
    __patches[0].push_back(source(6.123351, 1.025799, 0.180000));
    __patches[0].push_back(source(6.125174, 1.025678, -0.180200));
    __patches[0].push_back(source(6.122274, 1.026551, 0.180400));
    __patches[0].push_back(source(6.122275, 1.025726, 0.179100));
    __patches[0].push_back(source(6.125597, 1.026210, 0.178700));
    __patches[0].push_back(source(6.123304, 1.026405, 0.178800));
    __patches[0].push_back(source(6.122274, 1.026308, 0.178500));
    __patches[0].push_back(source(6.122367, 1.026696, 0.178700));
    __patches[0].push_back(source(6.121806, 1.026526, -0.179000));
    __patches[0].push_back(source(6.122039, 1.027011, -0.179100));
    __patches[0].push_back(source(6.121945, 1.027181, 0.179300));
    __patches[0].push_back(source(6.125412, 1.026792, 0.178600));
    __patches[0].push_back(source(6.123585, 1.027133, 0.178400));
    __patches[0].push_back(source(6.124381, 1.026478, 0.178500));
    __patches[0].push_back(source(6.125176, 1.026332, 0.178400));
    __patches[0].push_back(source(6.125320, 1.027496, 0.178800));
    __patches[0].push_back(source(6.122790, 1.025921, 0.177800));
    __patches[0].push_back(source(6.125551, 1.026501, -0.177700));
    __patches[0].push_back(source(6.122181, 1.026429, 0.178000));
    __patches[0].push_back(source(6.125736, 1.026016, -0.178100));
    __patches[0].push_back(source(6.122413, 1.027496, 0.177400));
    __patches[0].push_back(source(6.122647, 1.027520, -0.178700));
    __patches[0].push_back(source(6.122696, 1.026090, 0.177700));
    __patches[0].push_back(source(6.124567, 1.025848, 0.177600));
    __patches[0].push_back(source(6.123304, 1.026599, 0.177300));
    __patches[0].push_back(source(6.122226, 1.027108, -0.177700));
    __patches[0].push_back(source(6.124382, 1.027375, 0.177500));
    __patches[0].push_back(source(6.124006, 1.026090, 0.176900));
    __patches[0].push_back(source(6.125788, 1.027156, 0.176500));
    __patches[0].push_back(source(6.122181, 1.026066, 0.176500));
    __patches[0].push_back(source(6.125785, 1.026574, -0.176600));
    __patches[0].push_back(source(6.122133, 1.026914, -0.176600));
    __patches[0].push_back(source(6.123351, 1.025969, 0.176700));
    __patches[0].push_back(source(6.123866, 1.026624, 0.176400));
    __patches[0].push_back(source(6.123959, 1.025630, 0.176400));
    __patches[0].push_back(source(6.124239, 1.025581, -0.178200));
    __patches[0].push_back(source(6.124147, 1.027060, 0.176700));
    __patches[0].push_back(source(6.125789, 1.027374, 0.175700));
    __patches[0].push_back(source(6.124989, 1.026502, 0.176200));
    __patches[0].push_back(source(6.122178, 1.027520, -0.175900));
    __patches[0].push_back(source(6.121947, 1.026041, -0.175800));
    __patches[0].push_back(source(6.122648, 1.027230, 0.175500));
    __patches[0].push_back(source(6.122367, 1.026817, 0.175500));
    __patches[0].push_back(source(6.123491, 1.026745, 0.175900));
    __patches[0].push_back(source(6.121948, 1.025872, 0.175600));
    __patches[0].push_back(source(6.123023, 1.025775, 0.176000));
    __patches[0].push_back(source(6.124522, 1.026866, 0.175600));
    __patches[0].push_back(source(6.124053, 1.026236, 0.175500));
    __patches[0].push_back(source(6.125131, 1.026817, 0.175400));
    __patches[0].push_back(source(6.123070, 1.026139, 0.175100));
    __patches[0].push_back(source(6.122366, 1.027569, 0.174700));
    __patches[0].push_back(source(6.125555, 1.027520, -0.174900));
    __patches[0].push_back(source(6.121804, 1.027253, 0.175100));
    __patches[0].push_back(source(6.122367, 1.026987, -0.175000));
    __patches[0].push_back(source(6.125082, 1.026211, 0.175000));
    __patches[0].push_back(source(6.122601, 1.026866, 0.175100));
    __patches[0].push_back(source(6.124005, 1.025751, 0.175000));
    __patches[0].push_back(source(6.124893, 1.025605, 0.174600));
    __patches[0].push_back(source(6.123210, 1.027011, 0.174600));
    __patches[0].push_back(source(6.124147, 1.026672, 0.174500));
    __patches[0].push_back(source(6.122835, 1.027569, -0.174300));
    __patches[0].push_back(source(6.122182, 1.025920, -0.173200));
    __patches[0].push_back(source(6.123913, 1.027157, 0.173300));
    __patches[0].push_back(source(6.122601, 1.027326, 0.173200));
    __patches[0].push_back(source(6.123538, 1.025581, 0.173100));
    __patches[0].push_back(source(6.123257, 1.025581, -0.173600));
    __patches[0].push_back(source(6.125787, 1.026962, 0.173500));
    __patches[0].push_back(source(6.124850, 1.027132, 0.172600));
    __patches[0].push_back(source(6.125786, 1.026841, 0.172100));
    __patches[0].push_back(source(6.124006, 1.025993, 0.171800));
    __patches[0].push_back(source(6.125783, 1.025871, -0.171900));
    __patches[0].push_back(source(6.124521, 1.026357, 0.172100));
    __patches[0].push_back(source(6.121805, 1.026987, 0.171800));
    __patches[0].push_back(source(6.125413, 1.027180, -0.172000));
    __patches[0].push_back(source(6.124851, 1.027326, -0.171500));
    __patches[0].push_back(source(6.125132, 1.027374, 0.173200));
    __patches[0].push_back(source(6.125315, 1.025993, 0.171500));
    __patches[0].push_back(source(6.123866, 1.027254, 0.171200));
    __patches[0].push_back(source(6.123491, 1.027496, -0.171600));
    __patches[0].push_back(source(6.124100, 1.026866, 0.171100));
    __patches[0].push_back(source(6.122835, 1.027375, -0.170900));
    __patches[0].push_back(source(6.123024, 1.025630, 0.171000));
    __patches[0].push_back(source(6.123585, 1.026575, 0.170300));
    __patches[0].push_back(source(6.122602, 1.026575, 0.170200));
    __patches[0].push_back(source(6.122415, 1.026163, 0.170200));
    __patches[0].push_back(source(6.122182, 1.025605, -0.170600));
    __patches[0].push_back(source(6.122648, 1.027036, 0.170000));
    __patches[0].push_back(source(6.124615, 1.026526, 0.169800));
    __patches[0].push_back(source(6.125178, 1.027084, 0.169500));
    __patches[0].push_back(source(6.122086, 1.026890, -0.169300));
    __patches[0].push_back(source(6.125034, 1.025823, -0.169500));
    __patches[0].push_back(source(6.124753, 1.025726, 0.171100));
    __patches[0].push_back(source(6.122039, 1.026744, 0.169500));
    __patches[0].push_back(source(6.123117, 1.026430, 0.169400));
    __patches[0].push_back(source(6.124192, 1.025727, 0.169400));
    __patches[0].push_back(source(6.123116, 1.027521, -0.169300));
    __patches[0].push_back(source(6.125454, 1.025580, 0.169000));
    __patches[0].push_back(source(6.125174, 1.025678, -0.169600));
    __patches[0].push_back(source(6.122460, 1.027157, 0.168900));
    __patches[0].push_back(source(6.123773, 1.027399, 0.168400));
    __patches[0].push_back(source(6.122087, 1.026526, 0.168400));
    __patches[0].push_back(source(6.125784, 1.026307, 0.168800));
    __patches[0].push_back(source(6.125789, 1.027568, -0.168600));
    __patches[0].push_back(source(6.124707, 1.025920, 0.168600));
    __patches[0].push_back(source(6.121806, 1.026526, -0.168600));
    __patches[0].push_back(source(6.122040, 1.026357, 0.169500));
    __patches[0].push_back(source(6.121897, 1.027544, 0.168500));
    __patches[0].push_back(source(6.124944, 1.026938, 0.168200));
    __patches[0].push_back(source(6.121805, 1.026769, -0.168200));
    __patches[0].push_back(source(6.122883, 1.025824, 0.168400));
    __patches[0].push_back(source(6.124286, 1.026090, 0.168200));
    __patches[0].push_back(source(6.123116, 1.027254, 0.168600));
    __patches[0].push_back(source(6.122695, 1.026793, 0.167700));
    __patches[0].push_back(source(6.122178, 1.027520, -0.167800));
    __patches[0].push_back(source(6.121808, 1.025750, 0.168300));
    __patches[0].push_back(source(6.122883, 1.026284, 0.167700));
    __patches[0].push_back(source(6.122415, 1.025920, 0.168400));
    __patches[0].push_back(source(6.125083, 1.026672, 0.167800));
    __patches[0].push_back(source(6.125692, 1.026671, -0.167700));
    __patches[0].push_back(source(6.125320, 1.027496, 0.167200));
    __patches[0].push_back(source(6.122226, 1.027108, -0.167000));
    __patches[0].push_back(source(6.122179, 1.027375, 0.166900));
    __patches[0].push_back(source(6.124193, 1.026357, 0.167000));
    __patches[0].push_back(source(6.123678, 1.025993, 0.167600));
    __patches[0].push_back(source(6.125455, 1.025847, -0.167000));
    __patches[0].push_back(source(6.124239, 1.025581, -0.166800));
    __patches[0].push_back(source(6.123772, 1.026793, 0.166100));
    __patches[0].push_back(source(6.125175, 1.026090, 0.165800));
    __patches[0].push_back(source(6.124333, 1.026236, 0.165700));
    __patches[0].push_back(source(6.124756, 1.026672, 0.166000));
    __patches[0].push_back(source(6.123772, 1.025872, 0.165600));
    __patches[0].push_back(source(6.125459, 1.026938, 0.165500));
    __patches[0].push_back(source(6.125783, 1.025992, -0.165100));
    __patches[0].push_back(source(6.124569, 1.027108, 0.165000));
    __patches[0].push_back(source(6.124757, 1.027544, 0.165000));
    __patches[0].push_back(source(6.124428, 1.026696, 0.165000));
    __patches[0].push_back(source(6.125317, 1.026599, 0.164900));
    __patches[0].push_back(source(6.122647, 1.027520, -0.165000));
    __patches[0].push_back(source(6.122366, 1.027448, 0.165200));
    __patches[0].push_back(source(6.125789, 1.027374, 0.165000));
    __patches[0].push_back(source(6.125555, 1.027520, -0.166400));
    __patches[0].push_back(source(6.125785, 1.026574, -0.165700));
    __patches[0].push_back(source(6.125410, 1.026283, 0.164600));
    __patches[0].push_back(source(6.124848, 1.026357, 0.164600));
    __patches[0].push_back(source(6.123023, 1.027108, 0.164600));
    __patches[0].push_back(source(6.123210, 1.026696, 0.164300));
    __patches[0].push_back(source(6.122649, 1.026284, 0.164200));
    __patches[0].push_back(source(6.122366, 1.027569, 0.164500));
    __patches[0].push_back(source(6.122367, 1.026987, -0.164200));
    __patches[0].push_back(source(6.123772, 1.026139, 0.164400));
    __patches[0].push_back(source(6.123960, 1.027569, 0.164500));
    __patches[0].push_back(source(6.125222, 1.025944, 0.164000));
    __patches[0].push_back(source(6.121805, 1.026962, 0.163900));
    __patches[0].push_back(source(6.125460, 1.027156, -0.164300));
    __patches[0].push_back(source(6.123117, 1.026866, 0.164300));
    __patches[0].push_back(source(6.123210, 1.026284, 0.163800));
    __patches[1].push_back(source(5.233814, 0.710917, 612.300000));
    __patches[1].push_back(source(5.233814, 0.710917, 551.100000));
    __patches[1].push_back(source(5.233814, 0.710917, 496.000000));
    __patches[1].push_back(source(5.233270, 0.711062, 467.400000));
    __patches[1].push_back(source(5.233814, 0.710917, 444.600000));
    __patches[1].push_back(source(5.233270, 0.711062, 419.000000));
    __patches[1].push_back(source(5.233814, 0.710917, 398.500000));
    __patches[1].push_back(source(5.233334, 0.711038, 380.300000));
    __patches[1].push_back(source(5.233782, 0.710917, 358.600000));
    __patches[1].push_back(source(5.233270, 0.711062, 344.200000));
    __patches[1].push_back(source(5.233718, 0.710941, 324.500000));
    __patches[1].push_back(source(5.233398, 0.711014, 314.200000));
    __patches[1].push_back(source(5.233846, 0.710917, 305.500000));
    __patches[1].push_back(source(5.233238, 0.711062, 298.500000));
    __patches[1].push_back(source(5.233686, 0.710941, 276.400000));
    __patches[1].push_back(source(5.233430, 0.711014, 270.500000));
    __patches[1].push_back(source(5.233878, 0.710892, 265.900000));
    __patches[1].push_back(source(5.233238, 0.711062, 259.500000));
    __patches[1].push_back(source(5.233590, 0.710965, 240.200000));
    __patches[1].push_back(source(5.233846, 0.710917, 235.600000));
    __patches[1].push_back(source(5.233398, 0.711014, 230.400000));
    __patches[1].push_back(source(5.233238, 0.711062, 223.000000));
    __patches[1].push_back(source(5.233686, 0.710941, 212.100000));
    __patches[1].push_back(source(5.233878, 0.710892, 206.600000));
    __patches[1].push_back(source(5.233207, 0.711062, 198.800000));
    __patches[1].push_back(source(5.233430, 0.711014, 194.800000));
    __patches[1].push_back(source(5.233878, 0.710917, 185.600000));
    __patches[1].push_back(source(5.233654, 0.710941, 180.900000));
    __patches[1].push_back(source(5.233398, 0.711038, 173.600000));
    __patches[1].push_back(source(5.233207, 0.711062, 169.800000));
    __patches[1].push_back(source(5.233750, 0.710965, 163.800000));
    __patches[1].push_back(source(5.233558, 0.710965, 158.800000));
    __patches[1].push_back(source(5.233878, 0.710917, 154.300000));
    __patches[1].push_back(source(5.233207, 0.711086, 150.600000));
    __patches[1].push_back(source(5.233430, 0.711038, 144.100000));
    __patches[1].push_back(source(5.233686, 0.710892, 138.600000));
    __patches[1].push_back(source(5.233878, 0.710917, 135.300000));
    __patches[1].push_back(source(5.233334, 0.711014, 133.900000));
    __patches[1].push_back(source(5.233750, 0.710989, 127.600000));
    __patches[1].push_back(source(5.233207, 0.711062, 126.600000));
    __patches[1].push_back(source(5.233558, 0.710941, 125.200000));
    __patches[1].push_back(source(5.233814, 0.710844, 122.700000));
    __patches[1].push_back(source(5.233526, 0.711038, 121.200000));
    __patches[1].push_back(source(5.233910, 0.710917, 114.100000));
    __patches[1].push_back(source(5.233207, 0.711086, 111.500000));
    __patches[1].push_back(source(5.233750, 0.710989, 108.600000));
    __patches[1].push_back(source(5.233814, 0.710844, 107.700000));
    __patches[1].push_back(source(5.233366, 0.710989, 106.200000));
    __patches[1].push_back(source(5.233526, 0.711038, 104.200000));
    __patches[1].push_back(source(5.233558, 0.710917, 101.000000));
    __patches[1].push_back(source(5.233207, 0.711111, 98.150000));
    __patches[1].push_back(source(5.233718, 0.710868, 96.200000));
    __patches[1].push_back(source(5.233846, 0.710965, 95.950000));
    __patches[1].push_back(source(5.233430, 0.711062, 92.820000));
    __patches[1].push_back(source(5.233302, 0.710989, 89.990000));
    __patches[1].push_back(source(5.233846, 0.710844, 88.300000));
    __patches[1].push_back(source(5.233910, 0.710941, 87.080000));
    __patches[1].push_back(source(5.233494, 0.710941, 85.490000));
    __patches[1].push_back(source(5.233238, 0.711135, 84.230000));
    __patches[1].push_back(source(5.233750, 0.710989, 82.620000));
    __patches[1].push_back(source(5.233175, 0.711038, 81.710000));
    __patches[1].push_back(source(5.233686, 0.710868, 79.630000));
    __patches[1].push_back(source(5.233494, 0.711062, 79.500000));
    __patches[1].push_back(source(5.233366, 0.711111, 74.610000));
    __patches[1].push_back(source(5.233942, 0.710892, 73.880000));
    __patches[1].push_back(source(5.233302, 0.710989, 73.560000));
    __patches[1].push_back(source(5.233654, 0.710989, 71.910000));
    __patches[1].push_back(source(5.233494, 0.710917, 71.290000));
    __patches[1].push_back(source(5.233846, 0.710844, 69.700000));
    __patches[1].push_back(source(5.233175, 0.711086, 69.100000));
    __patches[1].push_back(source(5.233814, 0.710989, 68.280000));
    __patches[1].push_back(source(5.233686, 0.710868, 65.710000));
    __patches[1].push_back(source(5.233238, 0.711135, 64.720000));
    __patches[1].push_back(source(5.233558, 0.711038, 63.630000));
    __patches[1].push_back(source(5.233910, 0.710965, 62.090000));
    __patches[1].push_back(source(5.233430, 0.710941, 61.820000));
    __patches[1].push_back(source(5.233175, 0.711038, 60.560000));
    __patches[1].push_back(source(5.233430, 0.711086, 59.530000));
    __patches[1].push_back(source(5.233846, 0.710844, 59.020000));
    __patches[1].push_back(source(5.233334, 0.710965, 56.300000));
    __patches[1].push_back(source(5.233814, 0.710989, 55.680000));
    __patches[1].push_back(source(5.233686, 0.710868, 54.690000));
    __patches[1].push_back(source(5.233366, 0.711111, 54.330000));
    __patches[1].push_back(source(5.233622, 0.710989, 52.900000));
    __patches[1].push_back(source(5.233143, 0.711062, 52.300000));
    __patches[1].push_back(source(5.233942, 0.710892, 51.390000));
    __patches[1].push_back(source(5.233494, 0.710917, 50.890000));
    __patches[1].push_back(source(5.233238, 0.711135, 50.200000));
    __patches[1].push_back(source(5.233910, 0.710965, 48.480000));
    __patches[1].push_back(source(5.233846, 0.710844, 48.030000));
    __patches[1].push_back(source(5.233270, 0.710989, 47.780000));
    __patches[1].push_back(source(5.233558, 0.711038, 46.370000));
    __patches[1].push_back(source(5.233686, 0.710868, 44.490000));
    __patches[1].push_back(source(5.233366, 0.711111, 44.170000));
    __patches[1].push_back(source(5.233814, 0.710989, 43.440000));
    __patches[1].push_back(source(5.233143, 0.711038, 43.120000));
    __patches[1].push_back(source(5.233494, 0.710917, 42.640000));
    __patches[1].push_back(source(5.233206, 0.711135, 41.720000));
    __patches[1].push_back(source(5.233878, 0.710844, 41.000000));
    __patches[1].push_back(source(5.233302, 0.710965, 40.610000));
    __patches[1].push_back(source(5.233654, 0.710989, 40.310000));
    __patches[1].push_back(source(5.233910, 0.710965, 39.140000));
    __patches[1].push_back(source(5.233430, 0.711086, 38.210000));
    __patches[1].push_back(source(5.233686, 0.710868, 37.210000));
    __patches[1].push_back(source(5.233175, 0.711014, 36.250000));
    __patches[1].push_back(source(5.233462, 0.710917, 35.620000));
    __patches[1].push_back(source(5.233206, 0.711135, 35.580000));
    __patches[1].push_back(source(5.233878, 0.710844, 35.080000));
    __patches[1].push_back(source(5.233942, 0.710965, 34.710000));
    __patches[1].push_back(source(5.233526, 0.711062, 34.540000));
    __patches[1].push_back(source(5.233302, 0.710965, 33.410000));
    __patches[1].push_back(source(5.233686, 0.710989, 33.050000));
    __patches[1].push_back(source(5.233143, 0.711086, 31.650000));
    __patches[1].push_back(source(5.233814, 0.711014, 31.370000));
    __patches[1].push_back(source(5.233366, 0.711111, 31.060000));
    __patches[1].push_back(source(5.233686, 0.710868, 31.010000));
    __patches[1].push_back(source(5.233942, 0.710868, 30.460000));
    __patches[1].push_back(source(5.233494, 0.710917, 30.000000));
    __patches[1].push_back(source(5.233302, 0.710965, 28.560000));
    __patches[1].push_back(source(5.233942, 0.710965, 28.600000));
    __patches[1].push_back(source(5.233494, 0.711062, 28.110000));
    __patches[1].push_back(source(5.233846, 0.710820, 27.920000));
    __patches[1].push_back(source(5.233143, 0.711014, 27.230000));
    __patches[1].push_back(source(5.233270, 0.711135, 27.160000));
    __patches[1].push_back(source(5.233654, 0.710989, 26.880000));
    __patches[1].push_back(source(5.233143, 0.711111, 25.810000));
    __patches[1].push_back(source(5.233462, 0.710917, 25.590000));
    __patches[1].push_back(source(5.233814, 0.711014, 25.530000));
    __patches[1].push_back(source(5.233974, 0.710892, 25.010000));
    __patches[1].push_back(source(5.233718, 0.710844, 24.890000));
    __patches[1].push_back(source(5.233334, 0.711135, 23.900000));
    __patches[1].push_back(source(5.233270, 0.710965, 23.460000));
    __patches[1].push_back(source(5.233590, 0.711038, 23.060000));
    __patches[1].push_back(source(5.233878, 0.710844, 23.120000));
    __patches[1].push_back(source(5.233111, 0.711086, 22.560000));
    __patches[1].push_back(source(5.233942, 0.710965, 22.220000));
    __patches[1].push_back(source(5.233494, 0.710917, 21.810000));
    __patches[1].push_back(source(5.233430, 0.711086, 21.210000));
    __patches[1].push_back(source(5.233686, 0.710868, 21.220000));
    __patches[1].push_back(source(5.233143, 0.711014, 20.930000));
    __patches[1].push_back(source(5.233782, 0.711014, 20.980000));
    __patches[1].push_back(source(5.233654, 0.710989, 20.010000));
    __patches[1].push_back(source(5.233206, 0.711135, 19.780000));
    __patches[1].push_back(source(5.233366, 0.710941, 19.540000));
    __patches[1].push_back(source(5.233910, 0.710844, 19.560000));
    __patches[1].push_back(source(5.233942, 0.710965, 18.890000));
    __patches[1].push_back(source(5.233143, 0.711014, 18.380000));
    __patches[1].push_back(source(5.233334, 0.711135, 18.290000));
    __patches[1].push_back(source(5.233686, 0.710868, 18.130000));
    __patches[1].push_back(source(5.233558, 0.711062, 17.700000));
    __patches[1].push_back(source(5.233143, 0.711111, 17.340000));
    __patches[1].push_back(source(5.233814, 0.711014, 17.270000));
    __patches[1].push_back(source(5.233462, 0.710917, 17.090000));
    __patches[1].push_back(source(5.233846, 0.710820, 16.940000));
    __patches[1].push_back(source(5.233974, 0.710892, 16.740000));
    __patches[1].push_back(source(5.233270, 0.710965, 16.520000));
    __patches[1].push_back(source(5.233430, 0.711038, 16.170000));
    __patches[1].push_back(source(5.233622, 0.710965, 15.700000));
    __patches[1].push_back(source(5.233334, 0.711135, 15.310000));
    __patches[1].push_back(source(5.233718, 0.710844, 14.980000));
    __patches[1].push_back(source(5.233942, 0.710965, 14.940000));
    __patches[1].push_back(source(5.233143, 0.711014, 14.830000));
    __patches[1].push_back(source(5.233111, 0.711110, 14.470000));
    __patches[1].push_back(source(5.233814, 0.711014, 14.300000));
    __patches[1].push_back(source(5.233334, 0.710941, 14.200000));
    __patches[1].push_back(source(5.233910, 0.710844, 13.910000));
    __patches[1].push_back(source(5.233558, 0.711062, 13.850000));
    __patches[1].push_back(source(5.233526, 0.710892, 13.670000));
    __patches[1].push_back(source(5.233334, 0.711135, 13.160000));
    __patches[1].push_back(source(5.233654, 0.710989, 13.040000));
    __patches[1].push_back(source(5.233974, 0.710892, 12.780000));
    __patches[1].push_back(source(5.233111, 0.711110, 12.720000));
    __patches[1].push_back(source(5.233398, 0.711014, 12.560000));
    __patches[1].push_back(source(5.233846, 0.710820, 12.400000));
    __patches[1].push_back(source(5.233302, 0.710941, 12.250000));
    __patches[1].push_back(source(5.233814, 0.711014, 11.880000));
    __patches[1].push_back(source(5.233686, 0.710868, 11.750000));
    __patches[1].push_back(source(5.233143, 0.711014, 11.640000));
    __patches[1].push_back(source(5.233398, 0.711111, 11.400000));
    __patches[1].push_back(source(5.233494, 0.710892, 11.220000));
    __patches[1].push_back(source(5.233942, 0.710965, 11.180000));
    __patches[1].push_back(source(5.233654, 0.710989, 10.830000));
    __patches[1].push_back(source(5.233174, 0.711135, 10.750000));
    __patches[1].push_back(source(5.233974, 0.710868, 10.490000));
    __patches[1].push_back(source(5.233302, 0.710941, 10.280000));
    __patches[1].push_back(source(5.233590, 0.711062, 10.230000));
    __patches[1].push_back(source(5.233846, 0.710820, 10.220000));
    __patches[1].push_back(source(5.233111, 0.711110, 9.895000));
    __patches[1].push_back(source(5.233398, 0.711014, 9.738000));
    __patches[1].push_back(source(5.233718, 0.710844, 9.636000));
    __patches[1].push_back(source(5.233334, 0.711135, 9.506000));
    __patches[1].push_back(source(5.233143, 0.711014, 9.493000));
    __patches[1].push_back(source(5.233814, 0.711014, 9.489000));
    __patches[1].push_back(source(5.233494, 0.710892, 9.289000));
    __patches[1].push_back(source(5.233974, 0.710892, 9.008000));
    __patches[1].push_back(source(5.233814, 0.710917, -8.942000));
    __patches[1].push_back(source(5.233622, 0.710965, 8.953000));
    __patches[1].push_back(source(5.233942, 0.710965, 8.721000));
    __patches[1].push_back(source(5.233270, 0.710941, 8.602000));
    __patches[1].push_back(source(5.233334, 0.711135, 8.439000));
    __patches[1].push_back(source(5.233814, 0.710917, -8.468000));
    __patches[1].push_back(source(5.233846, 0.710820, 8.544000));
    __patches[1].push_back(source(5.233590, 0.711062, 8.350000));
    __patches[1].push_back(source(5.233111, 0.711110, 8.282000));
    __patches[1].push_back(source(5.233686, 0.710868, 8.183000));
    __patches[1].push_back(source(5.233814, 0.711014, 8.060000));
    __patches[1].push_back(source(5.233814, 0.710917, -8.214000));
    __patches[1].push_back(source(5.233974, 0.710868, 8.206000));
    __patches[1].push_back(source(5.233526, 0.710868, 7.923000));
    __patches[1].push_back(source(5.233143, 0.711014, 7.837000));
    __patches[1].push_back(source(5.233654, 0.710965, 7.793000));
    __patches[1].push_back(source(5.233814, 0.710917, -7.926000));
    __patches[1].push_back(source(5.233398, 0.711014, 7.662000));
    __patches[1].push_back(source(5.233942, 0.710965, 7.535000));
    __patches[1].push_back(source(5.233910, 0.710844, 7.405000));
    __patches[1].push_back(source(5.233814, 0.710917, -7.592000));
    __patches[1].push_back(source(5.233398, 0.711111, 7.425000));
    __patches[1].push_back(source(5.233654, 0.710965, 7.249000));
    __patches[1].push_back(source(5.233430, 0.710917, 7.151000));
    __patches[1].push_back(source(5.233814, 0.710917, -7.136000));
    __patches[1].push_back(source(5.233750, 0.710844, 7.210000));
    __patches[1].push_back(source(5.233111, 0.711110, 7.169000));
    __patches[1].push_back(source(5.233814, 0.711014, 7.111000));
    __patches[1].push_back(source(5.233974, 0.710892, 6.978000));
    __patches[1].push_back(source(5.233814, 0.710917, -7.012000));
    __patches[1].push_back(source(5.233238, 0.711159, 6.908000));
    __patches[1].push_back(source(5.233878, 0.710844, 6.874000));
    __patches[1].push_back(source(5.233270, 0.710941, 6.764000));
    __patches[1].push_back(source(5.233814, 0.710917, -6.611000));
    __patches[1].push_back(source(5.233654, 0.710965, 6.685000));
    __patches[1].push_back(source(5.233526, 0.710844, 6.546000));
    __patches[1].push_back(source(5.233942, 0.710965, 6.454000));
    __patches[1].push_back(source(5.233143, 0.711014, 6.381000));
    __patches[1].push_back(source(5.233398, 0.711111, 6.354000));
    __patches[1].push_back(source(5.233814, 0.710917, -6.365000));
    __patches[1].push_back(source(5.233878, 0.710844, 6.531000));
    __patches[1].push_back(source(5.233718, 0.710868, 6.266000));
    __patches[1].push_back(source(5.233814, 0.710917, -6.200000));
    __patches[1].push_back(source(5.233782, 0.711014, 6.327000));
    __patches[1].push_back(source(5.233974, 0.710892, 6.107000));
    __patches[1].push_back(source(5.233111, 0.711110, 6.061000));
    __patches[1].push_back(source(5.233814, 0.710917, -5.953000));
    __patches[1].push_back(source(5.233654, 0.710965, 6.078000));
    __patches[1].push_back(source(5.233878, 0.710844, 5.867000));
    __patches[1].push_back(source(5.233590, 0.711062, 5.783000));
    __patches[1].push_back(source(5.233814, 0.710917, -5.800000));
    __patches[1].push_back(source(5.233718, 0.710868, 5.787000));
    __patches[1].push_back(source(5.233334, 0.711135, 5.728000));
    __patches[1].push_back(source(5.233430, 0.710917, 5.673000));
    __patches[1].push_back(source(5.233814, 0.711014, 5.575000));
    __patches[1].push_back(source(5.233814, 0.710917, -5.577000));
    __patches[1].push_back(source(5.233974, 0.710892, 5.580000));
    __patches[1].push_back(source(5.233270, 0.710941, 5.489000));
    __patches[1].push_back(source(5.233430, 0.711014, 5.452000));
    __patches[1].push_back(source(5.233111, 0.711110, 5.351000));
    __patches[1].push_back(source(5.233814, 0.710917, -5.274000));
    __patches[1].push_back(source(5.233878, 0.710844, 5.477000));
    __patches[1].push_back(source(5.233654, 0.710965, 5.315000));
    __patches[1].push_back(source(5.233526, 0.710844, 5.230000));
    __patches[1].push_back(source(5.233942, 0.710965, 5.223000));
    __patches[1].push_back(source(5.233814, 0.710917, -5.248000));
    __patches[1].push_back(source(5.233718, 0.710868, 5.211000));
    __patches[1].push_back(source(5.233143, 0.710989, 5.023000));
    __patches[1].push_back(source(5.233398, 0.711111, 4.998000));
    __patches[1].push_back(source(5.233814, 0.710917, -4.931000));
    __patches[1].push_back(source(5.233878, 0.710844, 5.036000));
    __patches[1].push_back(source(5.233782, 0.711014, 4.936000));
    __patches[1].push_back(source(5.233654, 0.710965, 4.771000));
    __patches[1].push_back(source(5.233814, 0.710917, -4.895000));
    __patches[1].push_back(source(5.233974, 0.710892, 4.825000));
    __patches[1].push_back(source(5.233111, 0.711110, 4.722000));
    __patches[1].push_back(source(5.233718, 0.710868, 4.695000));
    __patches[1].push_back(source(5.233814, 0.710917, -4.743000));
    __patches[1].push_back(source(5.233942, 0.710965, 4.627000));
    __patches[1].push_back(source(5.233238, 0.711159, 4.591000));
    __patches[1].push_back(source(5.233526, 0.710820, 4.548000));
    __patches[1].push_back(source(5.233878, 0.710844, 4.486000));
    __patches[1].push_back(source(5.233846, 0.710917, -4.618000));
    __patches[1].push_back(source(5.233750, 0.710989, 4.469000));
    __patches[1].push_back(source(5.233270, 0.710941, 4.463000));
    __patches[1].push_back(source(5.233238, 0.711062, -4.493000));
    __patches[1].push_back(source(5.233430, 0.711014, 4.423000));
    __patches[1].push_back(source(5.233143, 0.711014, 4.348000));
    __patches[1].push_back(source(5.233814, 0.710917, -4.345000));
    __patches[1].push_back(source(5.233718, 0.710868, 4.402000));
    __patches[1].push_back(source(5.233974, 0.710892, 4.359000));
    __patches[1].push_back(source(5.233238, 0.711062, -4.348000));
    __patches[1].push_back(source(5.233366, 0.711111, 4.370000));
    __patches[1].push_back(source(5.233111, 0.711086, 4.295000));
    __patches[1].push_back(source(5.233846, 0.710917, -4.266000));
    __patches[1].push_back(source(5.233782, 0.710989, 4.219000));
    __patches[1].push_back(source(5.233238, 0.711062, -4.166000));
    __patches[1].push_back(source(5.233878, 0.710844, 4.189000));
    __patches[1].push_back(source(5.233814, 0.710917, -4.184000));
    __patches[1].push_back(source(5.233942, 0.710965, 4.152000));
    __patches[1].push_back(source(5.233430, 0.710917, 4.110000));
    __patches[1].push_back(source(5.233654, 0.710965, 4.089000));
    __patches[1].push_back(source(5.233111, 0.711086, 4.072000));
    __patches[1].push_back(source(5.233846, 0.710917, -4.042000));
    __patches[1].push_back(source(5.233878, 0.710844, 4.046000));
    __patches[1].push_back(source(5.233238, 0.711062, -4.003000));
    __patches[1].push_back(source(5.233366, 0.711111, 4.063000));
    __patches[1].push_back(source(5.233206, 0.711135, 3.927000));
    __patches[1].push_back(source(5.233718, 0.710868, 3.908000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.980000));
    __patches[1].push_back(source(5.233974, 0.710892, 3.979000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.910000));
    __patches[1].push_back(source(5.233143, 0.711014, 3.943000));
    __patches[1].push_back(source(5.233782, 0.710989, 3.822000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.887000));
    __patches[1].push_back(source(5.233430, 0.711014, 3.778000));
    __patches[1].push_back(source(5.233974, 0.710892, 3.708000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.737000));
    __patches[1].push_back(source(5.233111, 0.711086, 3.845000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.699000));
    __patches[1].push_back(source(5.233718, 0.710868, 3.765000));
    __patches[1].push_back(source(5.233206, 0.711135, 3.698000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.715000));
    __patches[1].push_back(source(5.233302, 0.710941, 3.686000));
    __patches[1].push_back(source(5.233526, 0.710820, 3.636000));
    __patches[1].push_back(source(5.233942, 0.710965, 3.627000));
    __patches[1].push_back(source(5.233366, 0.711111, 3.577000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.567000));
    __patches[1].push_back(source(5.233878, 0.710844, 3.692000));
    __patches[1].push_back(source(5.233782, 0.710989, 3.564000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.522000));
    __patches[1].push_back(source(5.233686, 0.710965, 3.526000));
    __patches[1].push_back(source(5.233111, 0.711086, 3.490000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.614000));
    __patches[1].push_back(source(5.233143, 0.711014, 3.451000));
    __patches[1].push_back(source(5.233942, 0.710868, 3.417000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.488000));
    __patches[1].push_back(source(5.233622, 0.711062, 3.412000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.386000));
    __patches[1].push_back(source(5.233206, 0.711135, 3.467000));
    __patches[1].push_back(source(5.233718, 0.710868, 3.398000));
    __patches[1].push_back(source(5.233942, 0.711062, -3.407000));
    __patches[1].push_back(source(5.233398, 0.711014, 3.351000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.340000));
    __patches[1].push_back(source(5.233942, 0.710965, 3.341000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.345000));
    __patches[1].push_back(source(5.233942, 0.710868, 3.340000));
    __patches[1].push_back(source(5.233718, 0.710795, -3.340000));
    __patches[1].push_back(source(5.233143, 0.711111, 3.311000));
    __patches[1].push_back(source(5.233718, 0.710868, 3.286000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.271000));
    __patches[1].push_back(source(5.233814, 0.710989, 3.286000));
    __patches[1].push_back(source(5.233398, 0.710917, 3.240000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.241000));
    __patches[1].push_back(source(5.233143, 0.711014, 3.268000));
    __patches[1].push_back(source(5.233366, 0.711111, 3.254000));
    __patches[1].push_back(source(5.233750, 0.710795, -3.200000));
    __patches[1].push_back(source(5.233846, 0.710844, 3.201000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.238000));
    __patches[1].push_back(source(5.233942, 0.711062, -3.197000));
    __patches[1].push_back(source(5.233526, 0.710820, 3.143000));
    __patches[1].push_back(source(5.233686, 0.710965, 3.109000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.086000));
    __patches[1].push_back(source(5.233143, 0.711111, 3.220000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.071000));
    __patches[1].push_back(source(5.233942, 0.710868, 3.177000));
    __patches[1].push_back(source(5.233718, 0.710868, 3.122000));
    __patches[1].push_back(source(5.233750, 0.710795, -3.170000));
    __patches[1].push_back(source(5.233942, 0.710965, 3.055000));
    __patches[1].push_back(source(5.233846, 0.710917, -3.044000));
    __patches[1].push_back(source(5.233814, 0.710989, 3.041000));
    __patches[1].push_back(source(5.233846, 0.710844, 3.019000));
    __patches[1].push_back(source(5.233942, 0.711062, -3.020000));
    __patches[1].push_back(source(5.233302, 0.710941, 3.013000));
    __patches[1].push_back(source(5.233238, 0.711062, -3.039000));
    __patches[1].push_back(source(5.233366, 0.711111, 3.048000));
    __patches[1].push_back(source(5.233814, 0.710917, -3.017000));
    __patches[1].push_back(source(5.233111, 0.711086, 3.012000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.992000));
    __patches[1].push_back(source(5.233750, 0.710795, -3.102000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.942000));
    __patches[1].push_back(source(5.233206, 0.711135, 2.986000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.930000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.885000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.961000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.942000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.944000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.878000));
    __patches[1].push_back(source(5.233398, 0.711014, 2.909000));
    __patches[1].push_back(source(5.233974, 0.710892, 2.890000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.934000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.893000));
    __patches[1].push_back(source(5.233494, 0.710820, 2.804000));
    __patches[1].push_back(source(5.233942, 0.711062, -2.795000));
    __patches[1].push_back(source(5.233206, 0.711135, 2.794000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.841000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.783000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.793000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.792000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.827000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.804000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.871000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.830000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.758000));
    __patches[1].push_back(source(5.233686, 0.710965, 2.813000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.736000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.744000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.714000));
    __patches[1].push_back(source(5.233910, 0.710965, 2.788000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.722000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.730000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.807000));
    __patches[1].push_back(source(5.233302, 0.710941, 2.710000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.713000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.700000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.687000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.696000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.792000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.640000));
    __patches[1].push_back(source(5.233942, 0.711062, -2.632000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.606000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.667000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.643000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.679000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.596000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.605000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.605000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.601000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.643000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.611000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.607000));
    __patches[1].push_back(source(5.233494, 0.710795, 2.579000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.595000));
    __patches[1].push_back(source(5.233910, 0.710965, 2.539000));
    __patches[1].push_back(source(5.234038, 0.710965, -2.553000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.539000));
    __patches[1].push_back(source(5.233686, 0.710965, 2.615000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.512000));
    __patches[1].push_back(source(5.233270, 0.710965, 2.504000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.544000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.556000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.507000));
    __patches[1].push_back(source(5.233910, 0.710965, 2.511000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.485000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.549000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.492000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.566000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.488000));
    __patches[1].push_back(source(5.233942, 0.711062, -2.471000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.451000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.495000));
    __patches[1].push_back(source(5.233622, 0.711062, 2.446000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.419000));
    __patches[1].push_back(source(5.233942, 0.710795, -2.449000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.414000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.417000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.443000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.394000));
    __patches[1].push_back(source(5.233398, 0.711014, 2.438000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.400000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.407000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.428000));
    __patches[1].push_back(source(5.233143, 0.710917, -2.403000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.363000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.420000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.415000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.383000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.373000));
    __patches[1].push_back(source(5.233270, 0.710965, 2.422000));
    __patches[1].push_back(source(5.233494, 0.710795, 2.364000));
    __patches[1].push_back(source(5.233206, 0.711135, 2.368000));
    __patches[1].push_back(source(5.234038, 0.710965, -2.353000));
    __patches[1].push_back(source(5.233942, 0.710965, 2.385000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.334000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.344000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.361000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.316000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.325000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.334000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.292000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.297000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.375000));
    __patches[1].push_back(source(5.233942, 0.710795, -2.355000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.288000));
    __patches[1].push_back(source(5.233942, 0.711062, -2.270000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.269000));
    __patches[1].push_back(source(5.233398, 0.710917, 2.258000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.262000));
    __patches[1].push_back(source(5.233143, 0.710917, -2.250000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.247000));
    __patches[1].push_back(source(5.233910, 0.710965, 2.301000));
    __patches[1].push_back(source(5.234038, 0.710965, -2.281000));
    __patches[1].push_back(source(5.233686, 0.710965, 2.279000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.248000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.305000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.236000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.275000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.230000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.299000));
    __patches[1].push_back(source(5.233942, 0.710941, 2.250000));
    __patches[1].push_back(source(5.233270, 0.710965, 2.210000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.196000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.216000));
    __patches[1].push_back(source(5.233942, 0.710795, -2.174000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.239000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.246000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.212000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.194000));
    __patches[1].push_back(source(5.233494, 0.710795, 2.175000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.148000));
    __patches[1].push_back(source(5.233814, 0.710917, -2.189000));
    __patches[1].push_back(source(5.233814, 0.710989, 2.206000));
    __patches[1].push_back(source(5.234038, 0.710965, -2.171000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.146000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.217000));
    __patches[1].push_back(source(5.233143, 0.710917, -2.150000));
    __patches[1].push_back(source(5.233942, 0.710965, 2.138000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.145000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.129000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.163000));
    __patches[1].push_back(source(5.233238, 0.711135, 2.139000));
    __patches[1].push_back(source(5.233718, 0.711183, 2.123000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.121000));
    __patches[1].push_back(source(5.233942, 0.710795, -2.164000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.105000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.148000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.098000));
    __patches[1].push_back(source(5.233270, 0.710965, 2.133000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.108000));
    __patches[1].push_back(source(5.233366, 0.711111, 2.087000));
    __patches[1].push_back(source(5.233047, 0.710989, -2.092000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.087000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.149000));
    __patches[1].push_back(source(5.233942, 0.711062, -2.090000));
    __patches[1].push_back(source(5.233910, 0.710965, 2.116000));
    __patches[1].push_back(source(5.234038, 0.710965, -2.097000));
    __patches[1].push_back(source(5.233143, 0.711111, 2.054000));
    __patches[1].push_back(source(5.233238, 0.711062, -2.080000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.051000));
    __patches[1].push_back(source(5.233942, 0.710941, 2.099000));
    __patches[1].push_back(source(5.233143, 0.711014, 2.047000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.036000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.092000));
    __patches[1].push_back(source(5.233494, 0.710795, 2.037000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.024000));
    __patches[1].push_back(source(5.233782, 0.710989, 2.071000));
    __patches[1].push_back(source(5.233942, 0.710868, 2.026000));
    __patches[1].push_back(source(5.233942, 0.710795, -2.068000));
    __patches[1].push_back(source(5.233366, 0.711183, -2.008000));
    __patches[1].push_back(source(5.233302, 0.711135, 2.047000));
    __patches[1].push_back(source(5.233846, 0.710844, 2.010000));
    __patches[1].push_back(source(5.233846, 0.710917, -2.071000));
    __patches[1].push_back(source(5.233047, 0.710989, -2.014000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.999000));
    __patches[1].push_back(source(5.233270, 0.710965, 2.020000));
    __patches[1].push_back(source(5.233143, 0.711086, 1.980000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.979000));
    __patches[1].push_back(source(5.233942, 0.710941, 2.032000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.980000));
    __patches[1].push_back(source(5.233334, 0.711111, 2.013000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.972000));
    __patches[1].push_back(source(5.233143, 0.710917, -1.965000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.968000));
    __patches[1].push_back(source(5.233750, 0.710868, 2.016000));
    __patches[1].push_back(source(5.233750, 0.710795, -2.024000));
    __patches[1].push_back(source(5.233814, 0.710989, 1.979000));
    __patches[1].push_back(source(5.233143, 0.711014, 1.953000));
    __patches[1].push_back(source(5.233814, 0.710917, -1.926000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.991000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.992000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.960000));
    __patches[1].push_back(source(5.233718, 0.711183, 1.929000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.922000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.981000));
    __patches[1].push_back(source(5.233398, 0.710941, 1.928000));
    __patches[1].push_back(source(5.233143, 0.711111, 1.912000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.925000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.934000));
    __patches[1].push_back(source(5.233910, 0.711038, -1.980000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.920000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.924000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.960000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.914000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.876000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.890000));
    __patches[1].push_back(source(5.233942, 0.710965, 1.922000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.928000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.915000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.915000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.887000));
    __patches[1].push_back(source(5.233047, 0.710989, -1.890000));
    __patches[1].push_back(source(5.233494, 0.710795, 1.894000));
    __patches[1].push_back(source(5.233143, 0.711014, 1.891000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.891000));
    __patches[1].push_back(source(5.233814, 0.710989, 1.913000));
    __patches[1].push_back(source(5.233110, 0.711183, -1.858000));
    __patches[1].push_back(source(5.233143, 0.711111, 1.848000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.921000));
    __patches[1].push_back(source(5.233910, 0.711038, -1.841000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.840000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.875000));
    __patches[1].push_back(source(5.233270, 0.710965, 1.861000));
    __patches[1].push_back(source(5.233814, 0.710917, -1.837000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.895000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.828000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.828000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.848000));
    __patches[1].push_back(source(5.233047, 0.710989, -1.826000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.832000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.858000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.853000));
    __patches[1].push_back(source(5.233238, 0.711135, 1.832000));
    __patches[1].push_back(source(5.233143, 0.711014, 1.805000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.799000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.872000));
    __patches[1].push_back(source(5.233814, 0.710989, 1.830000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.796000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.801000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.843000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.809000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.805000));
    __patches[1].push_back(source(5.233910, 0.711038, -1.791000));
    __patches[1].push_back(source(5.233143, 0.711086, 1.786000));
    __patches[1].push_back(source(5.233143, 0.710917, -1.776000));
    __patches[1].push_back(source(5.233398, 0.710989, 1.766000));
    __patches[1].push_back(source(5.233238, 0.711062, -1.783000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.787000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.811000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.761000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.818000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.768000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.763000));
    __patches[1].push_back(source(5.233270, 0.710965, 1.760000));
    __patches[1].push_back(source(5.233494, 0.710795, 1.750000));
    __patches[1].push_back(source(5.233110, 0.711183, -1.744000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.746000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.802000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.754000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.759000));
    __patches[1].push_back(source(5.233206, 0.711135, 1.755000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.781000));
    __patches[1].push_back(source(5.233718, 0.711183, 1.736000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.726000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.731000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.720000));
    __patches[1].push_back(source(5.233942, 0.710965, 1.725000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.737000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.738000));
    __patches[1].push_back(source(5.233047, 0.710989, -1.711000));
    __patches[1].push_back(source(5.233143, 0.711014, 1.743000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.713000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.717000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.709000));
    __patches[1].push_back(source(5.233814, 0.710989, 1.711000));
    __patches[1].push_back(source(5.233910, 0.711038, -1.715000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.689000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.774000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.689000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.685000));
    __patches[1].push_back(source(5.233143, 0.711111, 1.685000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.710000));
    __patches[1].push_back(source(5.233110, 0.711183, -1.684000));
    __patches[1].push_back(source(5.233622, 0.711086, 1.675000));
    __patches[1].push_back(source(5.233814, 0.710917, -1.668000));
    __patches[1].push_back(source(5.233686, 0.710965, 1.703000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.665000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.688000));
    __patches[1].push_back(source(5.233270, 0.710965, 1.657000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.643000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.688000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.649000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.654000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.695000));
    __patches[1].push_back(source(5.233143, 0.710917, -1.645000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.631000));
    __patches[1].push_back(source(5.233143, 0.711086, 1.687000));
    __patches[1].push_back(source(5.233910, 0.711038, -1.626000));
    __patches[1].push_back(source(5.233494, 0.710795, 1.623000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.620000));
    __patches[1].push_back(source(5.233942, 0.710868, 1.672000));
    __patches[1].push_back(source(5.233942, 0.710795, -1.650000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.653000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.631000));
    __patches[1].push_back(source(5.233814, 0.710989, 1.622000));
    __patches[1].push_back(source(5.233846, 0.710917, -1.640000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.626000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.631000));
    __patches[1].push_back(source(5.233846, 0.710844, 1.632000));
    __patches[1].push_back(source(5.233366, 0.711183, -1.611000));
    __patches[1].push_back(source(5.233270, 0.710965, 1.610000));
    __patches[1].push_back(source(5.233047, 0.710989, -1.613000));
    __patches[1].push_back(source(5.233143, 0.711014, 1.600000));
    __patches[1].push_back(source(5.233334, 0.711111, 1.592000));
    __patches[1].push_back(source(5.233270, 0.711062, -1.606000));
    __patches[1].push_back(source(5.233175, 0.711111, 1.583000));
    __patches[1].push_back(source(5.233110, 0.711183, -1.593000));
    __patches[1].push_back(source(5.233814, 0.710917, -1.577000));
    __patches[1].push_back(source(5.233910, 0.710965, 1.614000));
    __patches[1].push_back(source(5.233750, 0.710868, 1.609000));
    __patches[1].push_back(source(5.233750, 0.710795, -1.608000));
    __patches[1].push_back(source(5.234038, 0.710965, -1.597000));
    __patches[1].push_back(source(5.233047, 0.711014, -1.559000));
}


} //# namespace DPPP
} //# namespace LOFAR
