//# Apply.cc: Apply (corrections for) direction (in)dependent effects to a
//# buffer of visibility data.
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
#include <BBSKernel/Apply.h>
#include <BBSKernel/Expr/Timer.h>

#include <Common/lofar_sstream.h>
#include <Common/Timer.h>

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

    // Get pointers and strides to iterate over the elements of the Jones
    // matrices.
    Matrix lhs_matrix[2][2], rhs_matrix[2][2];
    size_t lhs_stride[2][2], rhs_stride[2][2];
    const double *lhs_re[2][2], *lhs_im[2][2], *rhs_re[2][2], *rhs_im[2][2];

    for(size_t i = 0; i < 2; ++i)
    {
        for(size_t j = 0; j < 2; ++j)
        {
            // Find a better solution for this.
            lhs_matrix[i][j] = lhs(i, j);
            if(!lhs_matrix[i][j].isComplex())
            {
                lhs_matrix[i][j] = tocomplex(lhs_matrix[i][j], 0.0);
            }

            rhs_matrix[i][j] = rhs(i, j);
            if(!rhs_matrix[i][j].isComplex())
            {
                rhs_matrix[i][j] = tocomplex(rhs_matrix[i][j], 0.0);
            }

            ASSERT(lhs_matrix[i][j].isComplex());
            ASSERT(rhs_matrix[i][j].isComplex());

            ASSERT(!lhs_matrix[i][j].isArray()
                || (static_cast<size_t>(lhs_matrix[i][j].nx()) == nFreq
                && static_cast<size_t>(lhs_matrix[i][j].ny()) == nTime));
            ASSERT(!rhs_matrix[i][j].isArray()
                || (static_cast<size_t>(rhs_matrix[i][j].nx()) == nFreq
                && static_cast<size_t>(rhs_matrix[i][j].ny()) == nTime));

            lhs_matrix[i][j].dcomplexStorage(lhs_re[i][j], lhs_im[i][j]);
            rhs_matrix[i][j].dcomplexStorage(rhs_re[i][j], rhs_im[i][j]);
            lhs_stride[i][j] = lhs_matrix[i][j].isArray() ? 1 : 0;
            rhs_stride[i][j] = rhs_matrix[i][j].isArray() ? 1 : 0;
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
            lhs_complex[0][0] = makedcomplex(*lhs_re[0][0], *lhs_im[0][0]);
            lhs_complex[0][1] = makedcomplex(*lhs_re[0][1], *lhs_im[0][1]);
            lhs_complex[1][0] = makedcomplex(*lhs_re[1][0], *lhs_im[1][0]);
            lhs_complex[1][1] = makedcomplex(*lhs_re[1][1], *lhs_im[1][1]);

            rhs_complex_conj[0][0] =
                makedcomplex(*rhs_re[0][0], -(*rhs_im[0][0]));
            rhs_complex_conj[0][1] =
                makedcomplex(*rhs_re[0][1], -(*rhs_im[0][1]));
            rhs_complex_conj[1][0] =
                makedcomplex(*rhs_re[1][0], -(*rhs_im[1][0]));
            rhs_complex_conj[1][1] =
                makedcomplex(*rhs_re[1][1], -(*rhs_im[1][1]));

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

            // Only recompute the elements of the upper triangular matrix (the
            // result is Hermitian).
            for(size_t k = 0; k < 4; ++k)
            {
                for(size_t l = k; l < 4; ++l)
                {
                    cov_complex[k][l] = mueller[k][0] * cov_tmp[0][l]
                        + mueller[k][1] * cov_tmp[1][l]
                        + mueller[k][2] * cov_tmp[2][l]
                        + mueller[k][3] * cov_tmp[3][l];
                }
            }

            // The result is Hermetian, so store as a real matrix with the
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
                    lhs_re[k][l] += lhs_stride[k][l];
                    lhs_im[k][l] += lhs_stride[k][l];
                    rhs_re[k][l] += rhs_stride[k][l];
                    rhs_im[k][l] += rhs_stride[k][l];
                }
            }
        }
    }
}

void apply(const StationExprLOFAR::Ptr &expr, const VisBuffer::Ptr &buffer,
    const BaselineMask &mask)
{
    ASSERTSTR(buffer->hasCovariance(), "Covariance information required to"
        " compute corrected visibilities.");

    // For now, assume default correlation order.
    ASSERT(buffer->nCorrelations() == 4);
    ASSERT(buffer->correlations()[0] == Correlation::XX);
    ASSERT(buffer->correlations()[1] == Correlation::XY);
    ASSERT(buffer->correlations()[2] == Correlation::YX);
    ASSERT(buffer->correlations()[3] == Correlation::YY);

    NSTimer timer;
    timer.start();

    // Compute the station response (Jones matrix) of each station.
    Timer::instance().reset();

    expr->setEvalGrid(buffer->grid());

    vector<JonesMatrix::View> station;
    for(size_t i = 0; i < buffer->nStations(); ++i)
    {
        station.push_back(expr->evaluate(i).view());
    }

    // Process all selected baselines.
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

    timer.stop();

    ostringstream out;
    out << endl << "Apply statistics:" << endl;
    {
        const double elapsed = timer.getElapsed();
        const unsigned long long count = timer.getCount();
        double average = count > 0 ? elapsed / count : 0.0;

        out << "TIMER s APPLY ALL" << " total " << elapsed << " count " << count
            << " avg " << average << endl;
    }

    Timer::instance().dump(out);
    LOG_DEBUG(out.str());

    Timer::instance().reset();
}

} //# namespace BBS
} //# namespace LOFAR
