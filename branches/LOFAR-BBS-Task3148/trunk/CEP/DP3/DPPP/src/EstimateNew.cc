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
#include <DPPP/PhaseShift.h>
#include <BBSKernel/EstimateUtil.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <casa/BasicSL/Constants.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_fstream.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>

#include <DPPP/Cursor.h>
#include <DPPP/CursorUtilCasa.h>

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
vector<vec2> __centers;

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

template <typename T>
void log_array(const string &prefix, const T *it, size_t n)
{
    ostringstream oss;
    oss << prefix;
    oss << setprecision(17);
    oss << "[";
    for(size_t i = 0; i < n; ++i)
    {
        oss << " " << (*it++);
    }
    oss << " ]";
    LOG_DEBUG(oss.str());
}

} //# unnamed namespace


void estimateImpl(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state, size_t ts);
//    const PatchList &patches);

//void estimateImpl2(DPBuffer &target,
//    const vector<DPBuffer> &buffers,
//    const casa::Array<casa::DComplex> &coeff,
//    const casa::Array<casa::DComplex> &coeffSub,
//    EstimateState &state, size_t ts);

void splitUVW(double *buffer, const double *uvw, const BBS::BaselineSeq &baselines, size_t nSt);
void simulate(const Position &reference,
    const Patch &patch,
    const BBS::BaselineSeq &baselines,
    const BBS::Axis::ShPtr &freqAxis,
    size_t nSt,
    const double *uvw,
    dcomplex *buffer);
void estimate2(const vector<const fcomplex*> &obs,
    const vector<const dcomplex*> &sim,
    const bool *flag,
    const float *weight,
    const dcomplex *mix,
    double *unknowns,
    const BBS::BaselineSeq &baselines,
    const BBS::Axis::ShPtr &freqAxis,
    size_t nSt);
void rotateUVW(const Position &from, const Position &to, double *uvw, size_t nSt);
void apply(dcomplex *buffer, const double *unknowns,
    const BBS::BaselineSeq &baselines,
    size_t nCh);
void subtract(fcomplex *buffer, const dcomplex *model,
    const dcomplex *mix, size_t nCh, size_t nDr,
    const BBS::BaselineSeq &baselines);

void estimate_csr(size_t nDirection,
    size_t nStation,
    size_t nBaseline,
    size_t nChannel,
    vector<const_cursor<fcomplex> > data,
    vector<const_cursor<dcomplex> > model,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<bool> flag,
    const_cursor<float> weight,
    const_cursor<dcomplex> mix,
    cursor<double> coeff);

void splitUVW_csr(size_t nStation, size_t nBaseline,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<double> uvw,
    cursor<double> split);

void rotateUVW_csr(const Position &from, const Position &to, size_t nStation,
    cursor<double> uvw);

void simulate_csr(const Position &reference,
    const Patch &patch,
    size_t nStation,
    size_t nBaseline,
    size_t nChannel,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<double> freq,
    const_cursor<double> uvw,
    cursor<dcomplex> vis);

void apply_csr(size_t nBaseline, size_t nChannel,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<double> coeff,
    cursor<dcomplex> vis);

void subtract_csr(size_t nBaseline, size_t nChannel,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<dcomplex> mix,
    const_cursor<dcomplex> model,
    cursor<fcomplex> data);

// TODO: Merge with Demixer::demix()...
void demix2(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &streams,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffRes,
    EstimateState &state,
    size_t ts,
    size_t nTime,
    size_t timeFactor,
    const PatchList &patches)
{
    LOG_DEBUG_STR("sizeof(source): " << sizeof(source));
    LOG_DEBUG_STR("#directions: " << patches.size());
    LOG_DEBUG_STR("target #time: " << target.size());
    LOG_DEBUG_STR("streams #dir: " << streams.size() << " #time: "
        << streams[0].size());
    LOG_DEBUG_STR("coeff #time: " << coeff.size() << " shape: "
        << coeff[0].shape());
    LOG_DEBUG_STR("coeffRes #time: " << coeffRes.size() << " shape: "
        << coeffRes[0].shape());

    const size_t nThread = OpenMP::maxThreads();
    const size_t nDr = patches.size();
    const size_t nSt = state.nStat;
    const size_t nBl = state.baselines.size();
    const size_t nCh = state.axisDemix->size();
    const size_t nCr = 4;

    ASSERT(streams.size() == nDr);
    for(size_t dr = 0; dr < nDr; ++dr)
    {
        ASSERT(streams[dr].size() == nTime);
    }
    ASSERT(target.size() >= nTime);// && target.size() % nTime == 0);
    ASSERT(coeff.size() == nTime);
//    ASSERT(coeffSub.size() == nTime);

    // Copy solutions from global solution array to thread private solution
    // array (solution propagation between chunks).
    boost::multi_array<double, 2> unknowns(boost::extents[nThread][nDr * nSt * 8]);
    const size_t nSlot = min(nTime, nThread);
    const size_t tSource = (ts == 0 ? 0 : ts - 1);
    for(size_t i = 0; i < nSlot; ++i)
    {
        copy(&(state.J[tSource][0][0][0]), &(state.J[tSource][0][0][0]) + nDr * nSt * 8, &(unknowns[i][0]));
    }

    boost::multi_array<double, 3> uvw(boost::extents[nThread][nSt][3]);
    boost::multi_array<dcomplex, 5> buffer(boost::extents[nThread][nDr][nBl][nCh]
        [nCr]);

    const size_t nChRes = state.axisResidual->size();
    boost::multi_array<dcomplex, 4> residual(boost::extents[nThread][nBl][nChRes][nCr]);

    vector<BBS::baseline_t> tmp_bl(nBl);
    for(size_t i = 0; i < nBl; ++i)
    {
        tmp_bl[i] = state.baselines[i];
    }

    vector<double> tmp_freq(nCh);
    for(size_t i = 0; i < nCh; ++i)
    {
        tmp_freq[i] = state.axisDemix->center(i);
    }

    vector<double> tmp_freq_res(nChRes);
    for(size_t i = 0; i < nChRes; ++i)
    {
        tmp_freq_res[i] = state.axisResidual->center(i);
    }

#pragma omp parallel for
    for(size_t i = 0; i < nTime; ++i)
    {
        const size_t thread = OpenMP::threadNum();

        // compute elevation and determine which sources are visible.
            // NB. carefully consider makeNorm() indices!!

        // zero solutions for non-visible sources.

        // Simulate.
        size_t strides[3] = {1, nCr, nCh * nCr};
        size_t strides_split[2] = {1, 3};

        const_cursor<BBS::baseline_t> cr_baseline(&(tmp_bl[0]));
        const_cursor<double> cr_freq(&(tmp_freq[0]));
        const_cursor<double> cr_freqRes(&(tmp_freq_res[0]));

        cursor<double> cr_split(&(uvw[thread][0][0]), 2, strides_split);
        for(size_t dr = 0; dr < nDr; ++dr)
        {
            fill(&(buffer[thread][dr][0][0][0]),
                &(buffer[thread][dr][0][0][0]) + nBl * nCh * nCr, dcomplex());

            const_cursor<double> cr_uvw = casa_const_cursor(streams[dr][i].getUVW());
            splitUVW_csr(nSt, nBl, cr_baseline, cr_uvw, cr_split);

            cursor<dcomplex> cr_model(&(buffer[thread][dr][0][0][0]), 3, strides);
            simulate_csr(patches[dr].position(), patches[dr], nSt, nBl, nCh,
                cr_baseline, cr_freq, cr_split, cr_model);
        }

//        for(size_t dr = 0; dr < nDr; ++dr)
//        {
//            splitUVW(&(uvw[thread][0][0]), streams[dr][i].getUVW().data(),
//                state.baselines, nSt);

//            fill(&(buffer[thread][dr][0][0][0]), &(buffer[thread][dr][0][0][0]) + nBl * nCh * nCr, dcomplex());
//            simulate(patches[dr].position(), patches[dr], state.baselines,
//                state.axisDemix, nSt, &(uvw[thread][0][0]), &(buffer[thread][dr][0][0][0]));
//        }

        // Estimate.
        vector<const_cursor<fcomplex> > cr_data(nDr);
        vector<const_cursor<dcomplex> > cr_model(nDr);
        for(size_t dr = 0; dr < nDr; ++dr)
        {
            cr_data[dr] = casa_const_cursor(streams[dr][i].getData());
            cr_model[dr] = const_cursor<dcomplex>(&(buffer[thread][dr][0][0][0]),
                3, strides);
        }

        const_cursor<bool> cr_flag = casa_const_cursor(streams[0][i].getFlags());
        const_cursor<float> cr_weight =
            casa_const_cursor(streams[0][i].getWeights());
        const_cursor<dcomplex> cr_mix = casa_const_cursor(coeff[i]);

        size_t strides_unknowns[3] = {1, 8, nSt * 8};
        cursor<double> cr_unknowns(&(unknowns[thread][0]), 3, strides_unknowns);
        estimate_csr(nDr, nSt, nBl, nCh, cr_data, cr_model, cr_baseline,
            cr_flag, cr_weight, cr_mix, cr_unknowns);

//        vector<const fcomplex*> obs(nDr);
//        vector<const dcomplex*> sim(nDr);
//        for(size_t dr = 0; dr < nDr; ++dr)
//        {
//            obs[dr] = streams[dr][i].getData().data();
//            sim[dr] = &(buffer[thread][dr][0][0][0]);
//        }
//        estimateImpl(target[i], streamsT[i], coeff[i], coeffSub[i], state,
//            ts + i);
//        estimate(streams[i], coeff[i], <simulations>, solutions);
//        estimate2(obs, sim, streams[0][i].getFlags().data(),
//            streams[0][i].getWeights().data(), coeff[i].data(),
//            &(unknowns[thread][0]), state.baselines, state.axisDemix, nSt);

        // Tweak solutions.

        const size_t nTimeResidual = min(timeFactor, target.size() - i * timeFactor);
        LOG_DEBUG_STR("timeFactor: " << timeFactor << "  nTime: " << nTime << " nTimeResidual: " << nTimeResidual);

        size_t strides_model[3] = {1, nCr, nChRes * nCr};
        cursor<dcomplex> cr_model_res(&(residual[thread][0][0][0]), 3, strides_model);

        for(size_t j = 0; j < nTimeResidual; ++j)
        {
            for(size_t dr = 0; dr < nDr; ++dr)
            {
                // Re-simulate for residual if required.
                if(timeFactor != 1 || nCh != nChRes)
                {
                    fill(&(residual[thread][0][0][0]),
                        &(residual[thread][0][0][0]) + nBl * nChRes * nCr,
                        dcomplex());

                    const_cursor<double> cr_uvw = casa_const_cursor(target[i * timeFactor + j].getUVW());
                    splitUVW_csr(nSt, nBl, cr_baseline, cr_uvw, cr_split);

                    rotateUVW_csr(Position(state.ra, state.dec), patches[dr].position(), nSt, cr_split);

                    simulate_csr(patches[dr].position(), patches[dr], nSt, nBl, nChRes,
                        cr_baseline, cr_freqRes, cr_split, cr_model_res);

//                    splitUVW(&(uvw[thread][0][0]),
//                        target[i * timeFactor + j].getUVW().data(),
//                        state.baselines, nSt);

//                    rotateUVW(Position(state.ra, state.dec), patches[dr].position(), &(uvw[thread][0][0]), nSt);

//                    fill(&(residual[thread][0][0][0]), &(residual[thread][0][0][0]) + nBl * nChRes * nCr, dcomplex());
//                    simulate(patches[dr].position(), patches[dr], state.baselines,
//                        state.axisResidual, nSt, &(uvw[thread][0][0]),
//                        &(residual[thread][0][0][0]));
                }
                else
                {
                    copy(&(buffer[thread][dr][0][0][0]), &(buffer[thread][dr][0][0][0]) + nBl * nChRes * nCr, &(residual[thread][0][0][0]));
                }

                // Apply solutions.
                size_t strides_coeff[2] = {1, 8};
                const_cursor<double> cr_coeff(&(unknowns[thread][dr * nSt * 8]), 2, strides_coeff);
                apply_csr(nBl, nChRes, cr_baseline, cr_coeff, cr_model_res);

                // Subtract.
                cursor<fcomplex> cr_residual = casa_cursor(target[i * timeFactor + j].getData());

                const casa::IPosition tmp_strides_mix_res = coeffRes[i * timeFactor + j].steps();
                LOG_DEBUG_STR("nDr: " << nDr << " " << tmp_strides_mix_res[2] << " " << tmp_strides_mix_res[3] << " " << tmp_strides_mix_res[4]);
                ASSERT(static_cast<size_t>(tmp_strides_mix_res[2]) == (nDr + 1) * (nDr + 1)
                    && static_cast<size_t>(tmp_strides_mix_res[3]) == nCr * (nDr + 1) * (nDr + 1)
                    && static_cast<size_t>(tmp_strides_mix_res[4]) == nChRes * nCr * (nDr + 1) * (nDr + 1));
                const_cursor<dcomplex> cr_mix(&(coeffRes[i * timeFactor + j](casa::IPosition(5, nDr, dr, 0, 0, 0))), 3, tmp_strides_mix_res.storage() + 2);
                subtract_csr(nBl, nChRes, cr_baseline, cr_mix, cr_model_res, cr_residual);

//                unsigned int bl = 1, ch = 0, cr = 0;
//                LOG_DEBUG_STR("dr: " << dr << " bl: " << bl << " ch: " << ch << " cr: " << setprecision(17) << " old: " << buffer[thread][dr][bl][ch][cr] << " new: " << residual[thread][bl][ch][cr]);

//                // Apply solutions.
//                apply(&(residual[thread][0][0][0]),
//                    &(unknowns[thread][dr * nSt * 8]),
//                    state.baselines,
//                    nChRes);

//                // Subtract.
//                subtract(target[i * timeFactor + j].getData().data(),
//                    &(residual[thread][0][0][0]),
//                    coeffRes[i * timeFactor + j].data() + nDr + dr * (nDr + 1),
//                    nChRes, nDr + 1, state.baselines);
            }
        }

        // Copy solutions to global solution array.
        copy(&(unknowns[thread][0]), &(unknowns[thread][0]) + nDr * nSt * 8,
            &(state.J[ts + i][0][0][0]));
    }
}


//double (&response)[2][2])
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

// data[dr], model[dr], flag, weight: nBl x nCh x nCr
// mix: nBl x nCh x nCr x nDr x nDr
// unknowns: nDr x nSt x 8 (or 4 x 2)
// baselines: nBl
void estimate_csr(size_t nDr,
    size_t nSt,
    size_t nBl,
    size_t nCh,
    vector<const_cursor<fcomplex> > data,
    vector<const_cursor<dcomplex> > model,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<bool> flag,
    const_cursor<float> weight,
    const_cursor<dcomplex> mix,
    cursor<double> coeff)
{
    ASSERT(data.size() == nDr && model.size() == nDr);
    const size_t nCr = 4;

    LOG_DEBUG_STR("#dr: " << nDr << " #st: " << nSt << " #bl: " << nBl
        << " #ch: " << nCh << " #cr: " << nCr);

    casa::LSQFit solver(nDr * nSt * 4 * 2);
//    solver.setMaxIter(50);
    LOG_DEBUG_STR("#coeff: " << nDr * nSt * 4 * 2);

    // Each visibility provides information about two (complex) unknowns per
    // station per direction. A visibility is measured by a specific
    // interferometer, which is the combination of two stations. Thus, in total
    // each visibility provides information about (no. of directions) x 2 x 2
    // x 2 (scalar) unknowns = (no. of directions) x 8. For each of these
    // unknowns the value of the partial derivative of the model with respect
    // to the unknow has to be computed.
    const unsigned int nPartial = nDr * 8;

    // Construct index template for each correlation.
    boost::multi_array<unsigned int, 2> dIndexTemplate(boost::extents[nCr]
        [nPartial]);
    for(size_t cr = 0; cr < nCr; ++cr)
    {
        size_t idx0 = (cr / 2) * 4 * coeff.stride(0);
        size_t idx1 = (cr & 1) * 4 * coeff.stride(0);

        for(size_t dr = 0; dr < nDr; ++dr)
        {
            dIndexTemplate[cr][dr * 8 + 0] = idx0 + 0 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 1] = idx0 + 1 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 2] = idx0 + 2 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 3] = idx0 + 3 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 4] = idx1 + 0 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 5] = idx1 + 1 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 6] = idx1 + 2 * coeff.stride(0);
            dIndexTemplate[cr][dr * 8 + 7] = idx1 + 3 * coeff.stride(0);
            idx0 += coeff.stride(2);
            idx1 += coeff.stride(2);
        }
    }

    // Allocate space for intermediate results.
    boost::multi_array<dcomplex, 2> M(boost::extents[nDr][4]);
    boost::multi_array<dcomplex, 3> dM(boost::extents[nDr][4][4]);
    boost::multi_array<double, 1> dR(boost::extents[nPartial]);
    boost::multi_array<double, 1> dI(boost::extents[nPartial]);
    boost::multi_array<unsigned int, 2> dIndex(boost::extents[nCr][nPartial]);

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
    const_cursor<BBS::baseline_t> c_baseline = baseline;
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

    // Set default iteration axis of mixing coefficients to 1.
//    mix.axis(1);
    ASSERT(false);

    // Iterate until convergence.
    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].start();
#endif

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = baseline->first;
            const size_t q = baseline->second;

            if(p != q)
            {
                // Update partial derivative index for current baseline.
                const size_t offsetP = p * coeff.stride(1);
                const size_t offsetQ = q * coeff.stride(1);
                for(size_t cr = 0; cr < nCr; ++cr)
                {
                    for(size_t dr = 0; dr < nDr; ++dr)
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

                for(size_t ch = 0; ch < nCh; ++ch)
                {
                    for(size_t dr = 0; dr < nDr; ++dr)
                    {
                        coeff.forward(1, p);
                        Jp_00 = dcomplex(coeff[0], coeff[1]);
                        Jp_01 = dcomplex(coeff[2], coeff[3]);
                        Jp_10 = dcomplex(coeff[4], coeff[5]);
                        Jp_11 = dcomplex(coeff[6], coeff[7]);
                        coeff.backward(1, p);

                        coeff.forward(1, q);
                        Jq_00 = dcomplex(coeff[0], -coeff[1]);
                        Jq_01 = dcomplex(coeff[2], -coeff[3]);
                        Jq_10 = dcomplex(coeff[4], -coeff[5]);
                        Jq_11 = dcomplex(coeff[6], -coeff[7]);
                        coeff.backward(1, q);

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
                        coeff.forward(2);
                    }
                    coeff.backward(2, nDr);

                    for(size_t cr = 0; cr < nCr; ++cr)
                    {
                        if(!flag[cr])
                        {
                            for(size_t tg = 0; tg < nDr; ++tg)
                            {
                                dcomplex model = 0.0, partial;
                                for(size_t dr = 0; dr < nDr; ++dr)
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
                                mix -= nDr;
                                mix.forward(0);
                            } // Target directions.
                            mix.backward(0, nDr);
                        }

                        // Move to the next correlation.
                        mix.forward(2);
                    } // Correlations.

                    // Move to the next channel.
                    mix.backward(2, nCr);
                    mix.forward(3);

                    for(size_t dr = 0; dr < nDr; ++dr)
                    {
                        model[dr].forward(1);
                        data[dr].forward(1);
                    }
                    flag.forward(1);
                    weight.forward(1);
                } // Channels.

                // Reset cursors to the start of the baseline.
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    model[dr].backward(1, nCh);
                    data[dr].backward(1, nCh);
                }
                flag.backward(1, nCh);
                weight.backward(1, nCh);
                mix.backward(3, nCh);
            }

            // Move cursors to the next baseline.
            for(size_t dr = 0; dr < nDr; ++dr)
            {
                model[dr].forward(2);
                data[dr].forward(2);
            }
            flag.forward(2);
            weight.forward(2);
            mix.forward(4);
            ++baseline;
        } // Baselines.

        // Reset all cursors for the next iteration.
        for(size_t dr = 0; dr < nDr; ++dr)
        {
            model[dr].backward(2, nBl);
            data[dr].backward(2, nBl);
        }
        flag.backward(2, nBl);
        weight.backward(2, nBl);
        mix.backward(4, nBl);
        baseline -= nBl;

#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].start();
#endif
        // Do solve iteration.
        casa::uInt rank;
        bool status = solver.solveLoop(rank, &(*coeff), true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].stop();
#endif

        // Update iteration count.
        ++nIterations;

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
        for(size_t dr = 0; dr < nDr; ++dr)
        {
            ASSERT(model[dr].address() == c_model[dr].address());
            ASSERT(data[dr].address() == c_data[dr].address());
        }
        ASSERT(flag.address() == c_flag.address());
        ASSERT(weight.address() == c_weight.address());
        ASSERT(mix.address() == c_mix.address());
        ASSERT(baseline.address() == c_baseline.address());
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() //<< " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);
}

void splitUVW_csr(size_t nSt, size_t nBl,
    const_cursor<BBS::baseline_t> baseline, const_cursor<double> uvw,
    cursor<double> split)
{
    cursor<double> known(split);
    vector<bool> flag(nSt, false);

    split[0] = 0.0;
    split[1] = 0.0;
    split[2] = 0.0;
    flag[0] = true;

    for(size_t i = 0; i < nBl; ++i)
    {
        const size_t p = baseline->first;
        const size_t q = baseline->second;
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
        ++baseline;
    }
    ASSERTSTR(static_cast<size_t>(std::count(flag.begin(), flag.end(), true))
        == flag.size(), "Unable to split baseline UVW into station UVW.");
}

void rotateUVW_csr(const Position &from, const Position &to, size_t nSt,
    cursor<double> uvw)
{
    casa::Matrix<double> oldUVW(3,3);
    casa::Matrix<double> newUVW(3,3);
    PhaseShift::fillTransMatrix(oldUVW, from[0], from[1]);
    PhaseShift::fillTransMatrix(newUVW, to[0], to[1]);

    casa::Matrix<double> tmp(casa::product(casa::transpose(newUVW), oldUVW));
    const double *R = tmp.data();

    for(size_t i = 0; i < nSt; ++i)
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

void simulate_csr(const Position &reference,
    const Patch &patch,
    size_t nSt,
    size_t nBl,
    size_t nCh,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<double> freq,
    const_cursor<double> uvw,
    cursor<dcomplex> vis)
{
    vector<dcomplex> buf_shift(nSt * nCh);
    vector<dcomplex> buf_spectrum(nCh * 4);

    for(Patch::const_iterator it = patch.begin(), end = patch.end(); it != end;
        ++it)
    {
        // Compute LMN coordinates.
        double lmn[3];
        radec2lmn(reference, it->position(), lmn);

        // Compute station phase shifts.
        dcomplex *shift = &(buf_shift[0]);
        for(size_t st = 0; st < nSt; ++st)
        {
            const double phase = casa::C::_2pi * (uvw[0] * lmn[0]
                + uvw[1] * lmn[1] + uvw[2] * (lmn[2] - 1.0));
            uvw.forward(1);

            for(size_t ch = 0; ch < nCh; ++ch)
            {
                const double chPhase = phase * (*freq) / casa::C::c;
                *shift = dcomplex(cos(chPhase), sin(chPhase));
                ++freq;
                ++shift;
            } // Channels
            freq -= nCh;
        } // Stations
        uvw.backward(1, nSt);

        // Compute source spectrum.
        dcomplex *spectrum = &(buf_spectrum[0]);
        for(size_t ch = 0; ch < nCh; ++ch)
        {
            Stokes stokes = it->stokes(*freq);
            ++freq;

            *spectrum++ = dcomplex(stokes.I + stokes.Q, 0.0);
            *spectrum++ = dcomplex(stokes.U, stokes.V);
            *spectrum++ = dcomplex(stokes.U, -stokes.V);
            *spectrum++ = dcomplex(stokes.I - stokes.Q, 0.0);
        } // channels
        freq -= nCh;

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = baseline->first;
            const size_t q = baseline->second;

            if(p != q)
            {
                const dcomplex *shiftP = &(buf_shift[p * nCh]);
                const dcomplex *shiftQ = &(buf_shift[q * nCh]);
                const dcomplex *spectrum = &(buf_spectrum[0]);
                for(size_t ch = 0; ch < nCh; ++ch)
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
                vis.backward(1, nCh);
            }

            // Move to next baseline.
            vis.forward(2);
            ++baseline;
        } // Baselines.
        vis.backward(2, nBl);
        baseline -= nBl;
    } // Components.
}

void apply_csr(size_t nBl, size_t nCh,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<double> coeff,
    cursor<dcomplex> vis)
{
    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2,
        Jq_01_s3, Jq_11_s3;

    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = baseline->first;
        const size_t q = baseline->second;

        if(p != q)
        {
            coeff.forward(1, p);
            Jp_00 = dcomplex(coeff[0], coeff[1]);
            Jp_01 = dcomplex(coeff[2], coeff[3]);
            Jp_10 = dcomplex(coeff[4], coeff[5]);
            Jp_11 = dcomplex(coeff[6], coeff[7]);
            coeff.backward(1, p);

            coeff.forward(1, q);
            Jq_00 = dcomplex(coeff[0], -coeff[1]);
            Jq_01 = dcomplex(coeff[2], -coeff[3]);
            Jq_10 = dcomplex(coeff[4], -coeff[5]);
            Jq_11 = dcomplex(coeff[6], -coeff[7]);
            coeff.backward(1, q);

            for(size_t ch = 0; ch < nCh; ++ch)
            {
                Jq_00_s0 = Jq_00 * (*vis);
                Jq_10_s0 = Jq_10 * (*vis);
                ++vis;

                Jq_01_s1 = Jq_01 * (*vis);
                Jq_11_s1 = Jq_11 * (*vis);
                ++vis;

                Jq_00_s2 = Jq_00 * (*vis);
                Jq_10_s2 = Jq_10 * (*vis);
                ++vis;

                Jq_01_s3 = Jq_01 * (*vis);
                Jq_11_s3 = Jq_11 * (*vis);
                ++vis;
                vis -= 4;

                *vis = Jp_00 * (Jq_00_s0 + Jq_01_s1)
                    + Jp_01 * (Jq_00_s2 + Jq_01_s3);
                ++vis;

                *vis = Jp_00 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_01 * (Jq_10_s2 + Jq_11_s3);
                ++vis;

                *vis = Jp_10 * (Jq_00_s0 + Jq_01_s1)
                    + Jp_11 * (Jq_00_s2 + Jq_01_s3);
                ++vis;

                *vis = Jp_10 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_11 * (Jq_10_s2 + Jq_11_s3);
                ++vis;
                vis -= 4;

                // Move to next channel.
                vis.forward(1);
            } // Channels.
            vis.backward(1, nCh);
        }

        vis.forward(2);
        ++baseline;
    } // Baselines
}

void subtract_csr(size_t nBl, size_t nCh,
    const_cursor<BBS::baseline_t> baseline,
    const_cursor<dcomplex> mix,
    const_cursor<dcomplex> model,
    cursor<fcomplex> data)
{
    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = baseline->first;
        const size_t q = baseline->second;

        if(p != q)
        {
            for(size_t ch = 0; ch < nCh; ++ch)
            {
                // Update visibilities.
                *data -= static_cast<fcomplex>(*mix * *model);
                ++mix;
                ++model;
                ++data;
                *data -= static_cast<fcomplex>(*mix * *model);
                ++mix;
                ++model;
                ++data;
                *data -= static_cast<fcomplex>(*mix * *model);
                ++mix;
                ++model;
                ++data;
                *data -= static_cast<fcomplex>(*mix * *model);
                ++mix;
                ++model;
                ++data;

                // Move to next channel.
                mix -= 4;
                mix.forward(1);
                model -= 4;
                model.forward(1);
                data -= 4;
                data.forward(1);
            } // Channels.
            mix.backward(1, nCh);
            model.backward(1, nCh);
            data.backward(1, nCh);
        }

        mix.forward(2);
        model.forward(2);
        data.forward(2);
        ++baseline;
    } // Baselines.
}

// =============================================================================

void estimate(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &buffers,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffSub,
    EstimateState &state,
    size_t ts)
//    const PatchList &patches)
{
    LOG_DEBUG_STR("sizeof(source): " << sizeof(source));
    LOG_DEBUG_STR("#model directions: " << __NDIR);
    LOG_DEBUG_STR("target #time: " << target.size());
    LOG_DEBUG_STR("buffers #dir: " << buffers.size() << " #time: "
        << buffers[0].size());
    LOG_DEBUG_STR("coeff #time: " << coeff.size() << " shape: "
        << coeff[0].shape());
    LOG_DEBUG_STR("coeffSub #time: " << coeffSub.size() << " shape: "
        << coeffSub[0].shape());

    ASSERT(buffers.size() == __NDIR);
    const size_t nTime = buffers[0].size();
    ASSERT(target.size() == nTime);
    ASSERT(buffers[1].size() == nTime);
    ASSERT(coeff.size() == nTime);
    ASSERT(coeffSub.size() == nTime);

    // Transpose axes of input buffer array.
    vector<vector<DPBuffer> > buffersT(nTime);
    for(size_t t = 0; t < nTime; ++t)
    {
        for(size_t dr = 0; dr < buffers.size(); ++dr)
        {
            buffersT[t].push_back(buffers[dr][t]);
        }
    }
/*
    // Propagate solutions from previous block.
    if(ts > 0)
    {
        for(size_t i = 0; i < nTime; ++i)
        {
            propagate(state, ts - 1, ts + i);
        }
    }
*/

//#pragma omp parallel for
    for(size_t i = 0; i < nTime; ++i)
    {
        estimateImpl(target[i], buffersT[i], coeff[i], coeffSub[i], state,
            ts + i);
    }
}


void rotateUVW(const Position &from, const Position &to, double *uvw, size_t nSt)
{
    casa::Matrix<double> oldUVW(3,3);
    casa::Matrix<double> newUVW(3,3);
    PhaseShift::fillTransMatrix (oldUVW, from[0], from[1]);
    PhaseShift::fillTransMatrix (newUVW, to[0], to[1]);

    casa::Matrix<double> itsMat1(casa::product(casa::transpose(newUVW), oldUVW));
    const double *mat1 = itsMat1.data();

    for(size_t i = 0; i < nSt; ++i)
    {
        double u = uvw[0] * mat1[0] + uvw[1] * mat1[3] + uvw[2] * mat1[6];
        double v = uvw[0] * mat1[1] + uvw[1] * mat1[4] + uvw[2] * mat1[7];
        double w = uvw[0] * mat1[2] + uvw[1] * mat1[5] + uvw[2] * mat1[8];

        uvw[0] = u;
        uvw[1] = v;
        uvw[2] = w;

        uvw += 3;
    }
}

void splitUVW(double *buffer, const double *uvw, const BBS::BaselineSeq &baselines, size_t nSt)
{
    buffer[0] = 0.0;
    buffer[1] = 0.0;
    buffer[2] = 0.0;

    vector<bool> flag(nSt, false);
    flag[0] = true;

    size_t nBl = baselines.size();
    size_t count = 1;
    for(size_t j = 0; j < nBl; ++j)
    {
        const size_t p = baselines[j].first;
        const size_t q = baselines[j].second;

        if(p != q && flag[p] != flag[q])
        {
            if(flag[p])
            {
                buffer[3 * q + 0] = uvw[0] + buffer[p * 3 + 0];
                buffer[3 * q + 1] = uvw[1] + buffer[p * 3 + 1];
                buffer[3 * q + 2] = uvw[2] + buffer[p * 3 + 2];
                flag[q] = true;
            }
            else
            {
                buffer[3 * p + 0] = -uvw[0] + buffer[q * 3 + 0];
                buffer[3 * p + 1] = -uvw[1] + buffer[q * 3 + 1];
                buffer[3 * p + 2] = -uvw[2] + buffer[q * 3 + 2];
                flag[p] = true;
            }

            ++count;
        }

        uvw += 3;
    }
    ASSERTSTR(count == nSt, "Could not split UVW, missing: " << nSt - count);
}

void simulate(const Position &reference,
    const Patch &patch,
    const BBS::BaselineSeq &baselines,
    const BBS::Axis::ShPtr &freqAxis,
    size_t nSt,
    const double *uvw,
    dcomplex *buffer)
{
    size_t nCh = freqAxis->size();
    size_t nBl = baselines.size();

    vector<dcomplex> shift(nSt * nCh);
    vector<dcomplex> spectrum(nCh * 4);

    for(Patch::const_iterator it = patch.begin(), end = patch.end(); it != end;
        ++it)
    {
        double lmn[3];
        radec2lmn(reference, it->position(), lmn);

        dcomplex *c_shift = &(shift[0]);
        const double *c_uvw = uvw;

        // Compute station phase shifts.
        for(size_t j = 0; j < nSt; ++j)
        {
            const double phase = casa::C::_2pi * (c_uvw[0] * lmn[0]
                + c_uvw[1] * lmn[1] + c_uvw[2] * (lmn[2] - 1.0));
            c_uvw += 3;

            for(size_t ch = 0; ch < nCh; ++ch)
            {
                const double chPhase = phase * freqAxis->center(ch) / casa::C::c;
                *c_shift++ = dcomplex(cos(chPhase), sin(chPhase));
            }
        }

        // Compute source spectrum.
        dcomplex *c_spectrum = &(spectrum[0]);
        for(size_t ch = 0; ch < nCh; ++ch)
        {
            Stokes stokes = it->stokes(freqAxis->center(ch));
            c_spectrum[0] = dcomplex(stokes.I + stokes.Q, 0.0);
            c_spectrum[1] = dcomplex(stokes.U, stokes.V);
            c_spectrum[2] = dcomplex(stokes.U, -stokes.V);
            c_spectrum[3] = dcomplex(stokes.I - stokes.Q, 0.0);
            c_spectrum += 4;
        }

        dcomplex *c_buffer = buffer;
        for(size_t j = 0; j < nBl; ++j)
        {
            const size_t p = baselines[j].first;
            const size_t q = baselines[j].second;

            if(p == q)
            {
                c_buffer += nCh * 4;
                continue;
            }

            const dcomplex *shiftP = &(shift[p * nCh]);
            const dcomplex *shiftQ = &(shift[q * nCh]);
            const dcomplex *c_spectrum = &(spectrum[0]);
            for(size_t ch = 0; ch < nCh; ++ch)
            {
                const dcomplex blShift = (*shiftQ++) * conj(*shiftP++);
                c_buffer[0] += blShift * c_spectrum[0];
                c_buffer[1] += blShift * c_spectrum[1];
                c_buffer[2] += blShift * c_spectrum[2];
                c_buffer[3] += blShift * c_spectrum[3];
                c_spectrum += 4;
                c_buffer += 4;
            }
        }
    }
}

void apply(dcomplex *buffer, const double *unknowns,
    const BBS::BaselineSeq &baselines,
    size_t nCh)
{
    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2,
        Jq_01_s3, Jq_11_s3;

    const size_t nBl = baselines.size();
    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = baselines[bl].first;
        const size_t q = baselines[bl].second;

        if(p == q)
        {
            buffer += nCh * 4;
            continue;
        }

        const double *Jp = unknowns + p * 8;
        Jp_00 = dcomplex(Jp[0], Jp[1]);
        Jp_01 = dcomplex(Jp[2], Jp[3]);
        Jp_10 = dcomplex(Jp[4], Jp[5]);
        Jp_11 = dcomplex(Jp[6], Jp[7]);

        const double *Jq = unknowns + q * 8;
        Jq_00 = dcomplex(Jq[0], -Jq[1]);
        Jq_01 = dcomplex(Jq[2], -Jq[3]);
        Jq_10 = dcomplex(Jq[4], -Jq[5]);
        Jq_11 = dcomplex(Jq[6], -Jq[7]);

        for(size_t ch = 0; ch < nCh; ++ch)
        {
            const dcomplex XX = buffer[0];
            const dcomplex XY = buffer[1];
            const dcomplex YX = buffer[2];
            const dcomplex YY = buffer[3];

            Jq_00_s0 = Jq_00 * XX;
            Jq_10_s0 = Jq_10 * XX;
            Jq_01_s1 = Jq_01 * XY;
            Jq_11_s1 = Jq_11 * XY;
            Jq_00_s2 = Jq_00 * YX;
            Jq_10_s2 = Jq_10 * YX;
            Jq_01_s3 = Jq_01 * YY;
            Jq_11_s3 = Jq_11 * YY;

            buffer[0] = Jp_00 * (Jq_00_s0 + Jq_01_s1)
                + Jp_01 * (Jq_00_s2 + Jq_01_s3);

            buffer[1] = Jp_00 * (Jq_10_s0 + Jq_11_s1)
                + Jp_01 * (Jq_10_s2 + Jq_11_s3);

            buffer[2] = Jp_10 * (Jq_00_s0 + Jq_01_s1)
                + Jp_11 * (Jq_00_s2 + Jq_01_s3);

            buffer[3] = Jp_10 * (Jq_10_s0 + Jq_11_s1)
                + Jp_11 * (Jq_10_s2 + Jq_11_s3);

            buffer += 4;
        }
    }
}

void subtract(fcomplex *buffer, const dcomplex *model,
    const dcomplex *mix, size_t nCh, size_t nDr,
    const BBS::BaselineSeq &baselines)
{
    // mix: nDr x nDr x nCr x nCh x nBl
    const size_t nBl = baselines.size();
    const size_t nCr = 4;
    for(size_t bl = 0; bl < nBl; ++bl)
    {
        const size_t p = baselines[bl].first;
        const size_t q = baselines[bl].second;

        if(p == q)
        {
            buffer += nCh * nCr;
            model += nCh * nCr;
            mix += nCh * nCr * nDr * nDr;
            continue;
        }

        for(size_t ch = 0; ch < nCh; ++ch)
        {
            for(size_t cr = 0; cr < nCh; ++cr)
            {
                *buffer -= static_cast<fcomplex>(*mix * *model);
                ++buffer;
                ++model;
                mix += nDr * nDr;
            }
        }
    }
}

/*
#ifdef ESTIMATE_TIMER
    state.tSimNew[threadID].start();
#endif

    {
        // TEST OF NEW SIMULATE CODE
        vector<double> __split(nSt * 3);
        splitUVW(&(__split[0]), buffers[0].getUVW().data(), state.baselines, nSt);

        vector<dcomplex> __buffer(nBl * nCh * 4, dcomplex());
        Position(state.ra, state.dec);
        simulate(state.patchPos, state.patch, state.baselines, state.axisDemix,
            nSt, &(__split[0]), &(__buffer[0]));

        size_t __bl = 10;

        LOG_DEBUG_STR("thread: " << threadID << " new: " << setprecision(17) << __buffer[__bl * nCh * 4] << " " << __buffer[__bl * nCh * 4 + 1] << " " << __buffer[__bl * nCh * 4 + 2] << " " << __buffer[__bl * nCh * 4 + 3]);
        LOG_DEBUG_STR("thread: " << threadID << " old: " << setprecision(17) << sim[0][__bl][0][0] << " " << sim[0][__bl][0][1] << " " << sim[0][__bl][0][2] << " " << sim[0][__bl][0][3]);
        // TEST OF NEW SIMULATE CODE
    }

#ifdef ESTIMATE_TIMER
    state.tSimNew[threadID].stop();
    LOG_DEBUG_STR("thread: "  << threadID << " tSim: " << state.tSim[threadID] << " tSimNew: " << state.tSimNew[threadID]);
#endif

*/

void estimate2(const vector<const fcomplex*> &obs,
    const vector<const dcomplex*> &sim,
    const bool *flag,
    const float *weight,
    const dcomplex *mix,
    double *unknowns,
    const BBS::BaselineSeq &baselines,
    const BBS::Axis::ShPtr &freqAxis,
    size_t nSt)
{
    // obs,sim,flag,weight: nBl * nCh * nCr
    // mix: nBl * nCh * nCr * nDr * nDr
    // unknowns: nDr * nSt * 4 * 2

//    const size_t threadID = OpenMP::threadNum();

#ifdef ESTIMATE_TIMER
//    state.tTot[threadID].start();
#endif

//    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
//        << " processing...");

    ASSERT(obs.size() == sim.size());
    const size_t nDr = obs.size();
    const size_t nBl = baselines.size();
    const size_t nCh = freqAxis->size();
    const size_t nCr = 4;

    LOG_DEBUG_STR("#dirs: " << nDr);

    casa::LSQFit solver(nDr * nSt * 4 * 2);
//    solver.setMaxIter(50);
    LOG_DEBUG_STR("#coeff: " << nDr * nSt * 4 * 2);

    // Each visibility provides information about two (complex) unknowns per
    // station per direction. A visibility is measured by a specific
    // interferometer, which is the combination of two stations. Thus, in total
    // each visibility provides information about (no. of directions) x 2 x 2
    // x 2 (scalar) unknowns = (no. of directions) x 8. For each of these
    // unknowns the value of the partial derivative of the model with respect
    // to the unknow has to be computed.
    const unsigned int nPartial = nDr * 8;

    // Construct index template for each correlation.
    boost::multi_array<unsigned int, 2> dIndexTemplate(boost::extents[nCr]
        [nPartial]);
    for(size_t cr = 0; cr < nCr; ++cr)
    {
        size_t idx0 = (cr / 2) * 4;
        size_t idx1 = (cr & 1) * 4;

        for(size_t dr = 0; dr < nDr; ++dr)
        {
            dIndexTemplate[cr][dr * 8 + 0] = idx0 + 0;
            dIndexTemplate[cr][dr * 8 + 1] = idx0 + 1;
            dIndexTemplate[cr][dr * 8 + 2] = idx0 + 2;
            dIndexTemplate[cr][dr * 8 + 3] = idx0 + 3;
            dIndexTemplate[cr][dr * 8 + 4] = idx1 + 0;
            dIndexTemplate[cr][dr * 8 + 5] = idx1 + 1;
            dIndexTemplate[cr][dr * 8 + 6] = idx1 + 2;
            dIndexTemplate[cr][dr * 8 + 7] = idx1 + 3;
            idx0 += nSt * 8;
            idx1 += nSt * 8;
        }
    }

    // Allocate space for intermediate results.
    boost::multi_array<dcomplex, 2> M(boost::extents[nDr][4]);
    boost::multi_array<dcomplex, 3> dM(boost::extents[nDr][4][4]);
    boost::multi_array<double, 1> dR(boost::extents[nPartial]);
    boost::multi_array<double, 1> dI(boost::extents[nPartial]);
    boost::multi_array<unsigned int, 2> dIndex(boost::extents[nCr][nPartial]);

    dcomplex Jp_00, Jp_01, Jp_10, Jp_11;
    dcomplex Jq_00, Jq_01, Jq_10, Jq_11;
    dcomplex Jp_00_s0, Jp_10_s0, Jp_00_s1, Jp_10_s1, Jp_01_s2, Jp_11_s2,
        Jp_01_s3, Jp_11_s3;
    dcomplex Jq_00_s0, Jq_10_s0, Jq_01_s1, Jq_11_s1, Jq_00_s2, Jq_10_s2,
        Jq_01_s3, Jq_11_s3;

    vector<const fcomplex*> c_obs(nDr);
    vector<const dcomplex*> c_sim(nDr);

    // Iterate until convergence.
    size_t nIterations = 0;
    while(!solver.isReady() && nIterations < 50)
    {
#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].start();
#endif
        c_obs = obs;
        c_sim = sim;
        const bool *c_flag = flag;
        const float *c_weight = weight;
        const dcomplex *c_mix = mix;

        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = baselines[bl].first;
            const size_t q = baselines[bl].second;

            if(p == q)
            {
                // Move cursors to the next baseline.
                c_flag += nCh * nCr;
                c_weight += nCh * nCr;
                c_mix += nCh * nCr * nDr * (nDr + 1);
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    c_obs[dr] += nCh * nCr;
                    c_sim[dr] += nCh * nCr;
                }

                continue;
            }

            // Update partial derivative index for current baseline.
            for(size_t cr = 0; cr < nCr; ++cr)
            {
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    dIndex[cr][dr * 8 + 0] = dIndexTemplate[cr][dr * 8 + 0] + p * 8;
                    dIndex[cr][dr * 8 + 1] = dIndexTemplate[cr][dr * 8 + 1] + p * 8;
                    dIndex[cr][dr * 8 + 2] = dIndexTemplate[cr][dr * 8 + 2] + p * 8;
                    dIndex[cr][dr * 8 + 3] = dIndexTemplate[cr][dr * 8 + 3] + p * 8;
                    dIndex[cr][dr * 8 + 4] = dIndexTemplate[cr][dr * 8 + 4] + q * 8;
                    dIndex[cr][dr * 8 + 5] = dIndexTemplate[cr][dr * 8 + 5] + q * 8;
                    dIndex[cr][dr * 8 + 6] = dIndexTemplate[cr][dr * 8 + 6] + q * 8;
                    dIndex[cr][dr * 8 + 7] = dIndexTemplate[cr][dr * 8 + 7] + q * 8;
                }
            }

            for(size_t ch = 0; ch < nCh; ++ch)
            {
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    const double *Jp = unknowns + (dr * nSt + p) * 8;
                    Jp_00 = dcomplex(Jp[0], Jp[1]);
                    Jp_01 = dcomplex(Jp[2], Jp[3]);
                    Jp_10 = dcomplex(Jp[4], Jp[5]);
                    Jp_11 = dcomplex(Jp[6], Jp[7]);

                    const double *Jq = unknowns + (dr * nSt + q) * 8;
                    Jq_00 = dcomplex(Jq[0], -Jq[1]);
                    Jq_01 = dcomplex(Jq[2], -Jq[3]);
                    Jq_10 = dcomplex(Jq[4], -Jq[5]);
                    Jq_11 = dcomplex(Jq[6], -Jq[7]);

//                    LOG_DEBUG_STR("bl: " << p << "-" << q << " ch: " << ch << " dr: " << dr);
//                    LOG_DEBUG_STR("Jp: " << Jp_00 << " " << Jp_01 << " " << Jp_10 << " " << Jp_11);
//                    LOG_DEBUG_STR("Jq: " << Jq_00 << " " << Jq_01 << " " << Jq_10 << " " << Jq_11);
//                    LOG_DEBUG_STR("sim: " << c_sim[dr][0] << " " << c_sim[dr][1] << " " << c_sim[dr][2] << " " << c_sim[dr][3]);

                    Jp_00_s0 = Jp_00 * c_sim[dr][0];
                    Jp_10_s0 = Jp_10 * c_sim[dr][0];
                    Jq_00_s0 = Jq_00 * c_sim[dr][0];
                    Jq_10_s0 = Jq_10 * c_sim[dr][0];

                    Jp_00_s1 = Jp_00 * c_sim[dr][1];
                    Jp_10_s1 = Jp_10 * c_sim[dr][1];
                    Jq_01_s1 = Jq_01 * c_sim[dr][1];
                    Jq_11_s1 = Jq_11 * c_sim[dr][1];

                    Jp_01_s2 = Jp_01 * c_sim[dr][2];
                    Jp_11_s2 = Jp_11 * c_sim[dr][2];
                    Jq_00_s2 = Jq_00 * c_sim[dr][2];
                    Jq_10_s2 = Jq_10 * c_sim[dr][2];

                    Jp_01_s3 = Jp_01 * c_sim[dr][3];
                    Jp_11_s3 = Jp_11 * c_sim[dr][3];
                    Jq_01_s3 = Jq_01 * c_sim[dr][3];
                    Jq_11_s3 = Jq_11 * c_sim[dr][3];

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
                    if(!(*c_flag))
                    {
                        for(size_t tg = 0; tg < nDr; ++tg)
                        {
                            dcomplex model = 0.0, partial;
                            for(size_t dr = 0; dr < nDr; ++dr)
                            {
                                // Update model visibility.
                                const dcomplex leakage = c_mix[dr * (nDr + 1) + tg];
                                model += leakage * M[dr][cr];

//                                LOG_DEBUG_STR("bl: " << p << "-" << q << " ch: " << ch << " tg: " << tg << " dr: " << dr << " leakage: " << c_mix[dr * (nDr + 1) + tg]);

                                // Compute partial derivatives.
                                partial = leakage * dM[dr][cr][0];
                                dR[dr * 8] = real(partial);
                                dI[dr * 8] = imag(partial);
                                dR[dr * 8 + 1] = -imag(partial);
                                dI[dr * 8 + 1] = real(partial);

                                partial = leakage * dM[dr][cr][1];
                                dR[dr * 8 + 2] = real(partial);
                                dI[dr * 8 + 2] = imag(partial);
                                dR[dr * 8 + 3] = -imag(partial);
                                dI[dr * 8 + 3] = real(partial);

                                partial = leakage * dM[dr][cr][2];
                                dR[dr * 8 + 4] = real(partial);
                                dI[dr * 8 + 4] = imag(partial);
                                dR[dr * 8 + 5] = imag(partial);
                                dI[dr * 8 + 5] = -real(partial);

                                partial = leakage * dM[dr][cr][3];
                                dR[dr * 8 + 6] = real(partial);
                                dI[dr * 8 + 6] = imag(partial);
                                dR[dr * 8 + 7] = imag(partial);
                                dI[dr * 8 + 7] = -real(partial);
                            } // source directions

                            // Compute the residual.
                            dcomplex residual = static_cast<dcomplex>(*(c_obs[tg])) - model;

//                            LOG_DEBUG_STR("bl: " << bl << " ch: " << ch << " cr: " << cr << " obs: " << (*(c_obs[tg])) << " weight: " << *c_weight << " model: " << model);
//                            log_array("dR: ", &(dR[0]), nPartial);
//                            log_array("dI: ", &(dI[0]), nPartial);
//                            log_array("dIndex: ", &(dIndex[cr][0]), nPartial);
//                            return;

                            // Update the normal equations.
                            solver.makeNorm(nPartial, &(dIndex[cr][0]),
                                &(dR[0]), static_cast<double>(*c_weight),
                                real(residual));
                            solver.makeNorm(nPartial, &(dIndex[cr][0]),
                                &(dI[0]), static_cast<double>(*c_weight),
                                imag(residual));
                        } // target directions
                    }

                    // Update cursors.
                    ++c_flag;
                    ++c_weight;
                    c_mix += nDr * (nDr + 1);
                    for(size_t dr = 0; dr < nDr; ++dr)
                    {
                        ++c_obs[dr];
                    }
                } // correlations

                // Update cursors.
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    c_sim[dr] += nCr;
                }
            } // channels
        } // baselines

#ifdef ESTIMATE_TIMER
//        state.tEq[threadID].stop();
#endif

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].start();
#endif
        // Do solve iteration.
        casa::uInt rank;
        bool status = solver.solveLoop(rank, unknowns, true);
        ASSERT(status);

#ifdef ESTIMATE_TIMER
//        state.tLM[threadID].stop();
#endif

        // Update iteration count.
        ++nIterations;
    }

    bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT
        || solver.isReady() == casa::LSQFit::DERIVLEVEL);
    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() //<< " timeslot: " << ts
        << " #iterations: " << nIterations << " converged: " << boolalpha
        << converged);
}


void estimateImpl(DPBuffer &target,
    const vector<DPBuffer> &buffers,
    const casa::Array<casa::DComplex> &coeff,
    const casa::Array<casa::DComplex> &coeffSub,
    EstimateState &state,
    size_t ts)
//    const PatchList &patches)
{
    const size_t threadID = OpenMP::threadNum();

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].start();
#endif

    LOG_DEBUG_STR("thread: " << OpenMP::threadNum() << " timeslot: " << ts
        << " processing...");

    const size_t nBl = state.baselines.size();
    const size_t nModels = __NDIR;
//    const size_t nTargets = buffers.size();
    const size_t idxTarget = coeffSub.shape()[0] - 1;
    const size_t nSt = state.nStat;
    const size_t nCr = 4;
    LOG_DEBUG_STR("total #dirs: " << coeffSub.shape()[0] << " target index: " << idxTarget);
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
        ASSERTSTR(found == nSt, "Could not split UVW, found: " << found << " model: " << i);

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

/*
#ifdef ESTIMATE_TIMER
    state.tSimNew[threadID].start();
#endif

    {
        // TEST OF NEW SIMULATE CODE
        vector<double> __split(nSt * 3);
        splitUVW(&(__split[0]), buffers[0].getUVW().data(), state.baselines, nSt);

        vector<dcomplex> __buffer(nBl * nCh * 4, dcomplex());
        simulate(patches[0].position(), patches[0], state.baselines, state.axisDemix,
            nSt, &(__split[0]), &(__buffer[0]));

        size_t __bl = 10;
        LOG_DEBUG_STR("CasA thread: " << threadID << " new: " << setprecision(17) << __buffer[__bl * nCh * 4] << " " << __buffer[__bl * nCh * 4 + 1] << " " << __buffer[__bl * nCh * 4 + 2] << " " << __buffer[__bl * nCh * 4 + 3]);
        LOG_DEBUG_STR("CasA thread: " << threadID << " old: " << setprecision(17) << sim[0][__bl][0][0] << " " << sim[0][__bl][0][1] << " " << sim[0][__bl][0][2] << " " << sim[0][__bl][0][3]);

        splitUVW(&(__split[0]), buffers[1].getUVW().data(), state.baselines, nSt);

        fill(__buffer.begin(), __buffer.end(), dcomplex());
        simulate(patches[1].position(), patches[1], state.baselines, state.axisDemix,
            nSt, &(__split[0]), &(__buffer[0]));

        LOG_DEBUG_STR("CygA thread: " << threadID << " new: " << setprecision(17) << __buffer[__bl * nCh * 4] << " " << __buffer[__bl * nCh * 4 + 1] << " " << __buffer[__bl * nCh * 4 + 2] << " " << __buffer[__bl * nCh * 4 + 3]);
        LOG_DEBUG_STR("CygA thread: " << threadID << " old: " << setprecision(17) << sim[1][__bl][0][0] << " " << sim[1][__bl][0][1] << " " << sim[1][__bl][0][2] << " " << sim[1][__bl][0][3]);
        // TEST OF NEW SIMULATE CODE
    }

#ifdef ESTIMATE_TIMER
    state.tSimNew[threadID].stop();
    LOG_DEBUG_STR("thread: "  << threadID << " tSim: " << state.tSim[threadID] << " tSimNew: " << state.tSimNew[threadID]);
#endif
*/


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

                    LOG_DEBUG_STR("bl: " << p << "-" << q << " ch: " << ch << " dr: " << dr);
                    LOG_DEBUG_STR("Jp: " << Jp_00 << " " << Jp_01 << " " << Jp_10 << " " << Jp_11);
                    LOG_DEBUG_STR("Jq: " << Jq_00 << " " << Jq_01 << " " << Jq_10 << " " << Jq_11);
                    LOG_DEBUG_STR("sim: " << sim[dr][bl][ch][0] << " " << sim[dr][bl][ch][1] << " " << sim[dr][bl][ch][2] << " " << sim[dr][bl][ch][3]);

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
                    for(size_t tg = 0; tg < nModels; ++tg)
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

                            LOG_DEBUG_STR("bl: " << p << "-" << q << " ch: " << ch << " tg: " << tg << " dr: " << dr << " leakage: " << weight);

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

                        LOG_DEBUG_STR("bl: " << bl << " ch: " << ch << " cr: " << cr << " obs: " << V_obs << " weight: " << obsw << " model: " << V_sim);
                        log_array("dR: ", &(dR[0]), nDerivative);
                        log_array("dI: ", &(dI[0]), nDerivative);
                        log_array("dIndex: ", &(state.dIndex[bl][cr][0]), nDerivative);
                        return;

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

/*
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

                Jq_00 = dcomplex(state.J[ts][q][dr][0], -state.J[ts][q][dr][1]);
                Jq_01 = dcomplex(state.J[ts][q][dr][2], -state.J[ts][q][dr][3]);
                Jq_10 = dcomplex(state.J[ts][q][dr][4], -state.J[ts][q][dr][5]);
                Jq_11 = dcomplex(state.J[ts][q][dr][6], -state.J[ts][q][dr][7]);

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
                    * coeffSub(casa::IPosition(5, idxTarget, dr, 0, ch, bl));

                sim_01 += (Jp_00 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_01 * (Jq_10_s2 + Jq_11_s3))
                    * coeffSub(casa::IPosition(5, idxTarget, dr, 1, ch, bl));

                sim_10 += (Jp_10 * (Jq_00_s0 + Jq_01_s1)
                    + Jp_11 * (Jq_00_s2 + Jq_01_s3))
                    * coeffSub(casa::IPosition(5, idxTarget, dr, 2, ch, bl));

                sim_11 += (Jp_10 * (Jq_10_s0 + Jq_11_s1)
                    + Jp_11 * (Jq_10_s2 + Jq_11_s3))
                    * coeffSub(casa::IPosition(5, idxTarget, dr, 3, ch, bl));
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
*/

#ifdef ESTIMATE_TIMER
    state.tTot[threadID].stop();
#endif
}

/*
void estimateImpl2(DPBuffer &target,
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
        ASSERTSTR(found == nSt, "Could not split UVW, found: " << found << " model: " << i);

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
            LOG_DEBUG_STR("================ MODEL: " << i << "================");
            LOG_DEBUG_STR("#baslines: " << nBl);

            // Split UVW.
            const casa::Matrix<double> &uvw = target.getUVW();

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

            // rotate
            casa::Matrix<double> oldUVW(3,3);
            casa::Matrix<double> newUVW(3,3);
            PhaseShift::fillTransMatrix (oldUVW, state.ra, state.dec);
            PhaseShift::fillTransMatrix (newUVW, __centers[i][0], __centers[i][1]);

            casa::Matrix<double> itsMat1(casa::product(casa::transpose(newUVW), oldUVW));
            const double* mat1 = itsMat1.data();

            for(size_t j = 0; j < nSt; ++j)
            {
                double u = uvw_split[j][0]*mat1[0] + uvw_split[j][1]*mat1[3] + uvw_split[j][2]*mat1[6];
                double v = uvw_split[j][0]*mat1[1] + uvw_split[j][1]*mat1[4] + uvw_split[j][2]*mat1[7];
                double w = uvw_split[j][0]*mat1[2] + uvw_split[j][1]*mat1[5] + uvw_split[j][2]*mat1[8];

                uvw_split[j][0] = u;
                uvw_split[j][1] = v;
                uvw_split[j][2] = w;
            }

            // verify
            const casa::Matrix<double> &uvwXXX = buffers[i].getUVW();
            for(size_t j = 0; j < nBl; ++j)
            {
                const size_t p = state.baselines[j].first;
                const size_t q = state.baselines[j].second;

                if(p == q)
                {
                    continue;
                }

                LOG_DEBUG_STR("U ok: " << setprecision(17) << uvwXXX(0, j) << " new: " << (uvw_split[q][0] - uvw_split[p][0]));
                LOG_DEBUG_STR("V ok: " << setprecision(17) << uvwXXX(1, j) << " new: " << (uvw_split[q][1] - uvw_split[p][1]));
                LOG_DEBUG_STR("W ok: " << setprecision(17) << uvwXXX(2, j) << " new: " << (uvw_split[q][2] - uvw_split[p][2]));
            }

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
*/

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
    __centers.resize(__NDIR);

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
    __centers[dir][0] = pra;
    __centers[dir][1] = pdec;

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
