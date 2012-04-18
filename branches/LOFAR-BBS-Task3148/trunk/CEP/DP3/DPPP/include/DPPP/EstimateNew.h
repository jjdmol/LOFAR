//# EstimateNew.h: Experimental parameter estimation using a flattened
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

#ifndef DPPP_ESTIMATENEW_H
#define DPPP_ESTIMATENEW_H

// \file
// Experimental parameter estimation using a flattened expression tree.

#include <DPPP/DPBuffer.h>
#include <BBSKernel/VisDimensions.h>
#include <ParmDB/Grid.h>
#include <Common/lofar_vector.h>
#include <boost/multi_array.hpp>

#include <BBSKernel/Solver.h>
#include <Common/OpenMP.h>

#define ESTIMATE_TIMER 1

#ifdef ESTIMATE_TIMER
#include <Common/Timer.h>
#endif

namespace LOFAR
{
namespace DPPP
{

// \addtogroup NDPPP
// @{

void __init_source_list();
void __init_lmn(unsigned int dir, double pra, double pdec);


struct EstimateState
{
    EstimateState()
    {
    }

    void init(size_t nStat, size_t nTime,
        const BBS::BaselineSeq &baselines,
        double freq, const BBS::SolverOptions &options)
    {
        this->nStat = nStat;
        this->nTime = nTime;
        this->freq = freq;
        this->baselines = baselines;

        this->lsqOptions = options;

        size_t nBl = baselines.size();
        size_t nCr = 4;
        size_t nDr = 2;

        size_t nThread = OpenMP::maxThreads();
        sim.resize(boost::extents[nThread][nDr][nBl][nCr]);

        J.resize(boost::extents[nTime][nStat][nDr][4 * 2]);
        typedef boost::multi_array<double, 4>::element* iterator;
        for(iterator it = J.data(), end = J.data() + J.num_elements();
            it != end;)
        {
            *it++ = 1.0;
            *it++ = 0.0;
            *it++ = 0.0;
            *it++ = 0.0;
            *it++ = 0.0;
            *it++ = 0.0;
            *it++ = 1.0;
            *it++ = 0.0;
        }

        dIndex.resize(boost::extents[baselines.size()][nCr][nDr * 8]);
        for(size_t bl = 0; bl < nBl; ++bl)
        {
            const size_t p = baselines[bl].first;
            const size_t q = baselines[bl].second;

            if(p == q)
            {
                continue;
            }

            for(size_t cr = 0; cr < nCr; ++cr)
            {
                const size_t elp = (cr / 2) * 4;
                const size_t elq = (cr & 1) * 4;

                size_t offset;
                for(size_t dr = 0; dr < nDr; ++dr)
                {
                    offset = p * nDr * 8 + dr * 8;
                    dIndex[bl][cr][dr * 8] = offset + elp;
                    dIndex[bl][cr][dr * 8 + 1] = offset + elp + 1;
                    dIndex[bl][cr][dr * 8 + 2] = offset + elp + 2;
                    dIndex[bl][cr][dr * 8 + 3] = offset + elp + 3;

                    offset = q * nDr * 8 + dr * 8;
                    dIndex[bl][cr][dr * 8 + 4] = offset + elq;
                    dIndex[bl][cr][dr * 8 + 5] = offset + elq + 1;
                    dIndex[bl][cr][dr * 8 + 6] = offset + elq + 2;
                    dIndex[bl][cr][dr * 8 + 7] = offset + elq + 3;
                }
            }
        }
    }

    size_t                              nStat;
    size_t                              nTime;
    double                              freq;
    BBS::BaselineSeq                    baselines;
    boost::multi_array<dcomplex, 4>     sim;
    boost::multi_array<double, 4>       J;
    boost::multi_array<unsigned int, 3> dIndex;
    BBS::SolverOptions                  lsqOptions;

#ifdef ESTIMATE_TIMER
    NSTimer                         tTot, tSim, tEq, tLM, tSub;
#endif
};

void estimate(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &buffers,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffSub,
    EstimateState &state,
    size_t ts);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
