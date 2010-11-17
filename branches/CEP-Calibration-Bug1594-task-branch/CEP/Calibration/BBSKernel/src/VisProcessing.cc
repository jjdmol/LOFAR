//# VisProcessing.cc: Various operations that can be performed on a buffer of
//# visibility data.
//#
//# Copyright (C) 2010
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
//#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/VisProcessing.h>
#include <BBSKernel/StationResponse.h>

namespace LOFAR
{
namespace BBS
{

static void applyBaseline(boost::multi_array<dcomplex, 4>::reference vis,
    boost::multi_array<double, 5>::reference cov,
    const JonesMatrix::View &lhs, const JonesMatrix::View &rhs)
{
    const size_t nTime = vis.shape()[0];
    const size_t nFreq = vis.shape()[1];
    const size_t nCorrelations = vis.shape()[2];
    LOG_DEBUG_STR("nTime: " << nTime << " nFreq: " << nFreq
        << " nCorrelations: " << nCorrelations);
    ASSERT(nCorrelations == 4);

    // Get pointers and strides to iterate over the elements of the Jones
    // matrices.
    const double *lhsRe[2][2], *lhsIm[2][2], *rhsRe[2][2], *rhsIm[2][2];
    size_t lhsStride[2][2], rhsStride[2][2];

    for(size_t i = 0; i < 2; ++i)
    {
    for(size_t j = 0; j < 2; ++j)
    {
        LOG_DEBUG_STR("i: " << i << " j: " << j);
        ASSERT(lhs(i, j).isComplex());
        ASSERT(rhs(i, j).isComplex());

        ASSERT(!lhs(i, j).isArray()
            || (static_cast<size_t>(lhs(i, j).nx()) == nFreq
            && static_cast<size_t>(lhs(i, j).ny()) == nTime));
        ASSERT(!rhs(i, j).isArray()
            || (static_cast<size_t>(rhs(i, j).nx()) == nFreq
            && static_cast<size_t>(rhs(i, j).ny()) == nTime));

        lhs(i, j).dcomplexStorage(lhsRe[i][j], lhsIm[i][j]);
        rhs(i, j).dcomplexStorage(rhsRe[i][j], rhsIm[i][j]);
        lhsStride[i][j] = lhs(i, j).isArray() ? 1 : 0;
        rhsStride[i][j] = rhs(i, j).isArray() ? 1 : 0;
    }
    }

    // Apply corrections to visibilities and noise covariance.
    dcomplex lhs_complex[2][2];
    dcomplex rhs_complex_conj[2][2];
    dcomplex mueller[4][4];
    dcomplex vis_tmp[4];
    dcomplex cov_complex[4][4];
    dcomplex cov_tmp[4][4];

    for(size_t i = 0; i < nTime; ++i)
    {
    for(size_t j = 0; j < nFreq; ++j)
    {
        // Construct lhs and rhs complex Jones matrices to make life easier.
        lhs_complex[0][0] = makedcomplex(*lhsRe[0][0], *lhsIm[0][0]);
        lhs_complex[0][1] = makedcomplex(*lhsRe[0][1], *lhsIm[0][1]);
        lhs_complex[1][0] = makedcomplex(*lhsRe[1][0], *lhsIm[1][0]);
        lhs_complex[1][1] = makedcomplex(*lhsRe[1][1], *lhsIm[1][1]);

        rhs_complex_conj[0][0] = makedcomplex(*rhsRe[0][0], -(*rhsIm[0][0]));
        rhs_complex_conj[0][1] = makedcomplex(*rhsRe[0][1], -(*rhsIm[0][1]));
        rhs_complex_conj[1][0] = makedcomplex(*rhsRe[1][0], -(*rhsIm[1][0]));
        rhs_complex_conj[1][1] = makedcomplex(*rhsRe[1][1], -(*rhsIm[1][1]));

        if((i == 0 && j == 0) || (i == nTime - 1 && j == 0))
        {
            ostringstream oss;
            oss << "Left Jones matrix:" << endl;
            oss << "[ " << lhs_complex[0][0] << " " << lhs_complex[0][1] << " ]" << endl;
            oss << "[ " << lhs_complex[1][0] << " " << lhs_complex[1][1] << " ]" << endl << endl;
            oss << "Right Jones matrix (conjugated):" << endl;
            oss << "[ " << rhs_complex_conj[0][0] << " " << rhs_complex_conj[0][1] << " ]" << endl;
            oss << "[ " << rhs_complex_conj[1][0] << " " << rhs_complex_conj[1][1] << " ]" << endl;

            LOG_DEBUG_STR("" << oss.str());
        }

        // Compute the Mueller matrix.
        mueller[0][0] = lhs_complex[0][0] * rhs_complex_conj[0][0];
        mueller[0][1] = lhs_complex[0][0] * rhs_complex_conj[0][1];
        mueller[1][0] = lhs_complex[0][0] * rhs_complex_conj[1][0];
        mueller[1][1] = lhs_complex[0][0] * rhs_complex_conj[1][1];

        mueller[0][2] = lhs_complex[0][1] * rhs_complex_conj[0][0];
        mueller[0][3] = lhs_complex[0][1] * rhs_complex_conj[0][1];
        mueller[1][2] = lhs_complex[0][1] * rhs_complex_conj[1][0];
        mueller[1][3] = lhs_complex[0][1] * rhs_complex_conj[1][1];

        mueller[2][0] = lhs_complex[1][0] * rhs_complex_conj[0][0];
        mueller[2][1] = lhs_complex[1][0] * rhs_complex_conj[0][1];
        mueller[3][0] = lhs_complex[1][0] * rhs_complex_conj[1][0];
        mueller[3][1] = lhs_complex[1][0] * rhs_complex_conj[1][1];

        mueller[2][2] = lhs_complex[1][1] * rhs_complex_conj[0][0];
        mueller[2][3] = lhs_complex[1][1] * rhs_complex_conj[0][1];
        mueller[3][2] = lhs_complex[1][1] * rhs_complex_conj[1][0];
        mueller[3][3] = lhs_complex[1][1] * rhs_complex_conj[1][1];

        if((i == 0 && j == 0) || (i == nTime - 1 && j == 0))
        {
            ostringstream oss;
            oss << "Mueller matrix:" << endl;
            oss << "[ " << mueller[0][0] << " " << mueller[0][1] << " " << mueller[0][2] << " " << mueller[0][3] << " ]" << endl;
            oss << "[ " << mueller[1][0] << " " << mueller[1][1] << " " << mueller[1][2] << " " << mueller[1][3] << " ]" << endl;
            oss << "[ " << mueller[2][0] << " " << mueller[2][1] << " " << mueller[2][2] << " " << mueller[2][3] << " ]" << endl;
            oss << "[ " << mueller[3][0] << " " << mueller[3][1] << " " << mueller[3][2] << " " << mueller[3][3] << " ]" << endl;
            LOG_DEBUG_STR("" << oss.str());
        }

        // Apply Mueller matrix to visibilities.
        vis_tmp[0] = mueller[0][0] * vis[i][j][0]
            + mueller[0][1] * vis[i][j][1]
            + mueller[0][2] * vis[i][j][2]
            + mueller[0][3] * vis[i][j][3];

        vis_tmp[1] = mueller[1][0] * vis[i][j][0]
            + mueller[1][1] * vis[i][j][1]
            + mueller[1][2] * vis[i][j][2]
            + mueller[1][3] * vis[i][j][3];

        vis_tmp[2] = mueller[2][0] * vis[i][j][0]
            + mueller[2][1] * vis[i][j][1]
            + mueller[2][2] * vis[i][j][2]
            + mueller[2][3] * vis[i][j][3];

        vis_tmp[3] = mueller[3][0] * vis[i][j][0]
            + mueller[3][1] * vis[i][j][1]
            + mueller[3][2] * vis[i][j][2]
            + mueller[3][3] * vis[i][j][3];

        vis[i][j][0] = vis_tmp[0];
        vis[i][j][1] = vis_tmp[1];
        vis[i][j][2] = vis_tmp[2];
        vis[i][j][3] = vis_tmp[3];

        // Construct the complex noise covariance matrix.
        cov_complex[0][0] = makedcomplex(cov[i][j][0][0], 0.0);
        cov_complex[0][1] = makedcomplex(cov[i][j][1][0], cov[i][j][0][1]);
        cov_complex[0][2] = makedcomplex(cov[i][j][2][0], cov[i][j][0][2]);
        cov_complex[0][3] = makedcomplex(cov[i][j][3][0], cov[i][j][0][3]);

        cov_complex[1][0] = conj(cov_complex[0][1]);
        cov_complex[1][1] = makedcomplex(cov[i][j][1][1], 0.0);
        cov_complex[1][2] = makedcomplex(cov[i][j][2][1], cov[i][j][1][2]);
        cov_complex[1][3] = makedcomplex(cov[i][j][3][1], cov[i][j][1][3]);

        cov_complex[2][0] = conj(cov_complex[0][2]);
        cov_complex[2][1] = conj(cov_complex[1][2]);
        cov_complex[2][2] = makedcomplex(cov[i][j][2][2], 0.0);
        cov_complex[2][3] = makedcomplex(cov[i][j][3][2], cov[i][j][2][3]);

        cov_complex[3][0] = conj(cov_complex[0][3]);
        cov_complex[3][1] = conj(cov_complex[1][3]);
        cov_complex[3][2] = conj(cov_complex[2][3]);
        cov_complex[3][3] = makedcomplex(cov[i][j][3][3], 0.0);


        if((i == 0 && j == 0) || (i == nTime - 1 && j == 0))
        {
            ostringstream oss;
            oss << "Covariance matrix in:" << endl;
            oss << "[ " << cov_complex[0][0] << " " << cov_complex[0][1] << " " << cov_complex[0][2] << " " << cov_complex[0][3] << " ]" << endl;
            oss << "[ " << cov_complex[1][0] << " " << cov_complex[1][1] << " " << cov_complex[1][2] << " " << cov_complex[1][3] << " ]" << endl;
            oss << "[ " << cov_complex[2][0] << " " << cov_complex[2][1] << " " << cov_complex[2][2] << " " << cov_complex[2][3] << " ]" << endl;
            oss << "[ " << cov_complex[3][0] << " " << cov_complex[3][1] << " " << cov_complex[3][2] << " " << cov_complex[3][3] << " ]" << endl;
            LOG_DEBUG_STR("" << oss.str());
        }

        // Apply Mueller matrix to the noise covariance matrix.
        for(size_t k = 0; k < 4; ++k)
        {
        for(size_t l = 0; l < 4; ++l)
        {
            cov_tmp[k][l] = cov_complex[k][0] * conj(mueller[l][0])
                + cov_complex[k][1] * conj(mueller[l][1])
                + cov_complex[k][2] * conj(mueller[l][2])
                + cov_complex[k][3] * conj(mueller[l][3]);
        }
        }


        if((i == 0 && j == 0) || (i == nTime - 1 && j == 0))
        {
            ostringstream oss;
            oss << "Covariance matrix temporary:" << endl;
            oss << "[ " << cov_tmp[0][0] << " " << cov_tmp[0][1] << " " << cov_tmp[0][2] << " " << cov_tmp[0][3] << " ]" << endl;
            oss << "[ " << cov_tmp[1][0] << " " << cov_tmp[1][1] << " " << cov_tmp[1][2] << " " << cov_tmp[1][3] << " ]" << endl;
            oss << "[ " << cov_tmp[2][0] << " " << cov_tmp[2][1] << " " << cov_tmp[2][2] << " " << cov_tmp[2][3] << " ]" << endl;
            oss << "[ " << cov_tmp[3][0] << " " << cov_tmp[3][1] << " " << cov_tmp[3][2] << " " << cov_tmp[3][3] << " ]" << endl;
            LOG_DEBUG_STR("" << oss.str());
        }

        for(size_t k = 0; k < 4; ++k)
        {
        for(size_t l = 0; l < 4; ++l)
        {
            cov_complex[k][l] = mueller[k][0] * cov_tmp[0][l]
                + mueller[k][1] * cov_tmp[1][l]
                + mueller[k][2] * cov_tmp[2][l]
                + mueller[k][3] * cov_tmp[3][l];
        }
        }

        if((i == 0 && j == 0) || (i == nTime - 1 && j == 0))
        {
            ostringstream oss;
            oss << "Covariance matrix out:" << endl;
            oss << "[ " << cov_complex[0][0] << " " << cov_complex[0][1] << " " << cov_complex[0][2] << " " << cov_complex[0][3] << " ]" << endl;
            oss << "[ " << cov_complex[1][0] << " " << cov_complex[1][1] << " " << cov_complex[1][2] << " " << cov_complex[1][3] << " ]" << endl;
            oss << "[ " << cov_complex[2][0] << " " << cov_complex[2][1] << " " << cov_complex[2][2] << " " << cov_complex[2][3] << " ]" << endl;
            oss << "[ " << cov_complex[3][0] << " " << cov_complex[3][1] << " " << cov_complex[3][2] << " " << cov_complex[3][3] << " ]" << endl;
            LOG_DEBUG_STR("" << oss.str());
        }

        // The result should be Hermetian so stores as a real matrix with the
        // imaginary components in the upper triangular matrix and the real
        // components in the lower triangular matrix.
        cov[i][j][0][0] = real(cov_complex[0][0]);
        cov[i][j][0][1] = imag(cov_complex[0][1]);
        cov[i][j][1][0] = real(cov_complex[0][1]);
        cov[i][j][0][2] = imag(cov_complex[0][2]);
        cov[i][j][2][0] = real(cov_complex[0][2]);
        cov[i][j][0][3] = imag(cov_complex[0][3]);
        cov[i][j][3][0] = real(cov_complex[0][3]);

        cov[i][j][1][1] = real(cov_complex[1][1]);
        cov[i][j][1][2] = imag(cov_complex[1][2]);
        cov[i][j][2][1] = real(cov_complex[1][2]);
        cov[i][j][1][3] = imag(cov_complex[1][3]);
        cov[i][j][3][1] = real(cov_complex[1][3]);

        cov[i][j][2][2] = real(cov_complex[2][2]);
        cov[i][j][2][3] = imag(cov_complex[2][3]);
        cov[i][j][3][2] = real(cov_complex[2][3]);

        cov[i][j][3][3] = real(cov_complex[3][3]);

        // Move pointers.
        for(size_t k = 0; k < 2; ++k)
        {
        for(size_t l = 0; l < 2; ++l)
        {
            lhsRe[k][l] += lhsStride[k][l];
            lhsIm[k][l] += lhsStride[k][l];
            rhsRe[k][l] += rhsStride[k][l];
            rhsIm[k][l] += rhsStride[k][l];
        }
        }
    }
    }
}


void apply(const ModelConfig &config, const VisBuffer::Ptr &buffer,
    const BaselineMask &mask)
{
    // For now, assume default correlation order.
    ASSERT(buffer->nCorrelations() == 4);
    ASSERT(buffer->correlations()[0] == Correlation::XX);
    ASSERT(buffer->correlations()[1] == Correlation::XY);
    ASSERT(buffer->correlations()[2] == Correlation::YX);
    ASSERT(buffer->correlations()[3] == Correlation::YY);

    const BeamConfig beamConfig = config.getBeamConfig();
    StationResponse response(buffer->instrument(), beamConfig.getConfigName(),
        beamConfig.getConfigPath(), buffer->getReferenceFreq(), true);

    response.setPointing(buffer->getPhaseReference());
    response.setDirection(buffer->getPhaseReference());
    response.setEvalGrid(buffer->grid());

    vector<JonesMatrix::View> station;
    station.reserve(buffer->nStations());

    for(size_t i = 0; i < buffer->nStations(); ++i)
    {
        const JonesMatrix::View rep = response.evaluate(i);
        LOG_DEBUG_STR("isComplex: " << rep(0, 0).isComplex() << " "
            << rep(0, 1).isComplex() << " " << rep(1, 0).isComplex() << " "
            << rep(1, 1).isComplex());
        LOG_DEBUG_STR("isArray: " << rep(0, 0).isArray() << " "
            << rep(0, 1).isArray() << " " << rep(1, 0).isArray() << " "
            << rep(1, 1).isArray());
        station.push_back(rep);
    }

    for(size_t i = 0; i < buffer->nBaselines(); ++i)
    {
        baseline_t baseline = buffer->baselines()[i];

        if(!mask(baseline))
        {
            continue;
        }

        applyBaseline(buffer->samples[i], buffer->covariance[i],
            station[baseline.first], station[baseline.second]);
    }
}







} //# namespace BBS
} //# namespace LOFAR
