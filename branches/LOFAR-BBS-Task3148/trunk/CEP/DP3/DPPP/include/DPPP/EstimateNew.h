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

#include <ParmDB/SourceDB.h>

#define ESTIMATE_TIMER 1

#ifdef ESTIMATE_TIMER
#include <DPPP/HWTimer.h>
#endif

namespace LOFAR
{
namespace DPPP
{
using BBS::SourceDB;
using BBS::ParmDB;
using BBS::ParmValue;
using BBS::ParmValueSet;
using BBS::SourceInfo;

// \addtogroup NDPPP
// @{

//void __init_source_list(const string &fname);
//void __init_lmn(unsigned int dir, double pra, double pdec);


class Position
{
public:
    Position()
    {
        fill(itsPosition, itsPosition + 2, 0.0);
    }

    Position(double alpha, double delta)
    {
        itsPosition[0] = alpha;
        itsPosition[1] = delta;
    }

    const double &operator[](size_t i) const
    {
        return itsPosition[i];
    }

    double &operator[](size_t i)
    {
        return itsPosition[i];
    }

private:
    double  itsPosition[2];
};

class Stokes
{
public:
    Stokes()
    :   I(0.0),
        Q(0.0),
        U(0.0),
        V(0.0)
    {
    }

    double  I, Q, U, V;
};

class Source
{
public:
    Source()
        :   itsRefFreq(0.0),
            itsPolarizedFraction(0.0),
            itsPolarizationAngle(0.0),
            itsRotationMeasure(0.0)
    {
    }

    Source(const Position &position)
        :   itsPosition(position),
            itsRefFreq(0.0),
            itsPolarizedFraction(0.0),
            itsPolarizationAngle(0.0),
            itsRotationMeasure(0.0)
    {
    }

    Source(const Position &position, const Stokes &stokes)
        :   itsPosition(position),
            itsStokes(stokes),
            itsRefFreq(0.0),
            itsPolarizedFraction(0.0),
            itsPolarizationAngle(0.0),
            itsRotationMeasure(0.0)
    {
    }

    void setPosition(const Position &position)
    {
        itsPosition = position;
    }

    void setStokes(const Stokes &stokes)
    {
        itsStokes = stokes;
    }

    template <typename T>
    void setSpectralIndex(double refFreq, T first, T last)
    {
        itsRefFreq = refFreq;
        itsSpectralIndex.clear();
        itsSpectralIndex.insert(itsSpectralIndex.begin(), first, last);
    }

    void setPolarizedFraction(double fraction)
    {
        itsPolarizedFraction = fraction;
    }

    void setPolarizationAngle(double angle)
    {
        itsPolarizationAngle = angle;
    }

    void setRotationMeasure(double rm)
    {
        itsRotationMeasure = rm;
    }

    const Position &position() const
    {
        return itsPosition;
    }

    Stokes stokes(double freq) const
    {
        Stokes stokes(itsStokes);

        if(hasSpectralIndex())
        {
            // Compute spectral index as:
            // (v / v0) ^ (c0 + c1 * log10(v / v0) + c2 * log10(v / v0)^2 + ...)
            // Where v is the frequency and v0 is the reference frequency.

            // Compute log10(v / v0).
            double base = log10(freq) - log10(itsRefFreq);

            // Compute c0 + log10(v / v0) * c1 + log10(v / v0)^2 * c2 + ...
            // using Horner's rule.
            double exponent = 0.0;
            typedef vector<double>::const_reverse_iterator iterator_type;
            for(iterator_type it = itsSpectralIndex.rbegin(),
                end = itsSpectralIndex.rend(); it != end; ++it)
            {
                exponent = exponent * base + *it;
            }

            // Compute I * (v / v0) ^ exponent, where I is the value of Stokes
            // I at the reference frequency.
            stokes.I *= pow10(base * exponent);
        }

        if(hasRotationMeasure())
        {
            double lambda = casa::C::c / freq;
            double chi = 2.0 * (itsPolarizationAngle + itsRotationMeasure
                * lambda * lambda);
            double stokesQU = stokes.I * itsPolarizedFraction;
            stokes.Q = stokesQU * cos(chi);
            stokes.U = stokesQU * sin(chi);
        }

        return stokes;
    }

private:
    bool hasSpectralIndex() const
    {
        return itsSpectralIndex.size() > 0;
    }

    bool hasRotationMeasure() const
    {
        return itsRotationMeasure > 0.0;
    }

    Position        itsPosition;
    Stokes          itsStokes;
    double          itsRefFreq;
    vector<double>  itsSpectralIndex;
    double          itsPolarizedFraction;
    double          itsPolarizationAngle;
    double          itsRotationMeasure;
};

struct Patch
{
    typedef vector<Source>::const_iterator const_iterator;

    string          name;
    Position        position;
    vector<Source>  sources;

    size_t size() const { return sources.size(); }
    const Source &operator[](size_t i) const
    {
        return sources[i];
    }

    const_iterator begin() const
    {
        return sources.begin();
    }

    const_iterator end() const
    {
        return sources.end();
    }

    void syncPos()
    {
        Position position = sources.front().position();
        double cosDec = cos(position[1]);
        double x = cos(position[0]) * cosDec;
        double y = sin(position[0]) * cosDec;
        double z = sin(position[1]);

        for(unsigned int i = 1; i < sources.size(); ++i)
        {
            position = sources[i].position();
            cosDec = cos(position[1]);
            x += cos(position[0]) * cosDec;
            y += sin(position[0]) * cosDec;
            z += sin(position[1]);
        }

        x /= size();
        y /= size();
        z /= size();

        this->position[0] = atan2(y, x);
        this->position[1] = asin(z);
    }
};

//typedef vector<Source>  Patch;
typedef vector<Patch>   PatchList;

struct EstimateState
{
    EstimateState()
    {
    }

    void init(size_t nDir, size_t nStat, size_t nTime,
        const BBS::BaselineSeq &baselines,
        const BBS::Axis::ShPtr &demix,
        const BBS::Axis::ShPtr &residual,
        double ra, double dec,
        const BBS::SolverOptions &options)
    {
        this->nStat = nStat;
        this->nTime = nTime;
        this->axisDemix = demix;
        this->axisResidual = residual;
        this->baselines = baselines;
        this->ra = ra;
        this->dec = dec;
        this->lsqOptions = options;

        size_t nBl = baselines.size();
        size_t nCr = 4;
        size_t nDr = nDir;

        size_t nThread = OpenMP::maxThreads();
//        sim.resize(boost::extents[nThread][nDr][nBl][nCr]);

#ifdef ESTIMATE_TIMER
        tTot.resize(nThread);
        tSim.resize(nThread);
        tEq.resize(nThread);
        tLM.resize(nThread);
        tSub.resize(nThread);
        tSimNew.resize(nThread);
#endif

        J.resize(boost::extents[nTime][nDr][nStat][4 * 2]);
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
    BBS::Axis::ShPtr                    axisDemix, axisResidual;
    BBS::BaselineSeq                    baselines;
//    boost::multi_array<dcomplex, 4>     sim;
    boost::multi_array<double, 4>       J;
    boost::multi_array<unsigned int, 3> dIndex;
    double                              ra, dec;
    BBS::SolverOptions                  lsqOptions;
//    Patch                               patch;
//    Position                            patchPos;
#ifdef ESTIMATE_TIMER
    vector<HWTimer>                     tTot, tSim, tEq, tLM, tSub, tSimNew;
#endif
};

Patch makePatch(SourceDB &sourceDB, const string &name);

void estimate(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &buffers,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffSub,
    EstimateState &state,
    size_t ts);
//    const PatchList &patches);

void demix2(vector<DPPP::DPBuffer> &target,
    const vector<vector<DPPP::DPBuffer> > &streams,
    const vector<casa::Array<casa::DComplex> > &coeff,
    const vector<casa::Array<casa::DComplex> > &coeffRes,
    EstimateState &state,
    size_t ts,
    size_t nTime,
    size_t timeFactor,
    const PatchList &patches);


// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
