//# Resampler.cc: Resample input to a different sample grid.
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Expr/Resampler.h>
#include <casa/Arrays.h>

namespace LOFAR
{
namespace BBS
{

Resampler::Resampler(const Expr<JonesMatrix>::ConstPtr &arg,
    unsigned int resolution, double flagDensityThreshold)
    :   UnaryExpr<JonesMatrix, JonesMatrix>(arg),
        itsGridId(resolution),
        itsFlagDensityThreshold(flagDensityThreshold)
{
}

const JonesMatrix Resampler::evaluateExpr(const Request &request, Cache &cache,
    unsigned int grid) const
{
    // Evaluate arguments.
    const JonesMatrix arg = argument0()->evaluate(request, cache, itsGridId);

    // Create axis mappings.
    vector<Span> axisMap[2];
    makeAxisMap(request[itsGridId][FREQ], request[grid][FREQ],
        std::back_inserter(axisMap[FREQ]));
    makeAxisMap(request[itsGridId][TIME], request[grid][TIME],
        std::back_inserter(axisMap[TIME]));

//    for(unsigned int i = 0; i < axisMap[FREQ].size(); ++i)
//    {
//        Span &span = axisMap[FREQ][i];
//        LOG_DEBUG_STR("" << i << ": " << span.src << " -> " << span.dst << " "
//            << span.weight);
//    }

//    for(unsigned int i = 0; i < axisMap[TIME].size(); ++i)
//    {
//        Span &span = axisMap[TIME][i];
//        LOG_DEBUG_STR("" << i << ": " << span.src << " -> " << span.dst << " "
//            << span.weight);
//    }

    // Allocate result.
    JonesMatrix result;

    // Resample flags.
    if(arg.hasFlags())
    {
        const FlagArray flags = arg.flags();
        result.setFlags(resampleFlags(flags, axisMap));

        for(unsigned int i = 0; i < arg.size(); ++i)
        {
            const Element in(arg.getElement(i));

            Element out;
            out.assign(resampleWithFlags(in.value(), flags, axisMap));
            for(Element::const_iterator it = in.begin(), end = in.end();
                it != end; ++it)
            {
                out.assign(it->first, resampleWithFlags(it->second, flags,
                    axisMap));
            }

            result.setElement(i, out);
        }
    }
    else
    {
        for(unsigned int i = 0; i < arg.size(); ++i)
        {
            const Element in(arg.getElement(i));

            Element out;
            out.assign(resample(in.value(), axisMap));
            for(Element::const_iterator it = in.begin(), end = in.end();
                it != end; ++it)
            {
                out.assign(it->first, resample(it->second, axisMap));
            }

            result.setElement(i, out);
        }
    }

    return result;
}

FlagArray Resampler::resampleFlags(const FlagArray &in,
    const vector<Span> (&map)[2]) const
{
    if(in.rank() == 0)
    {
        return in;
    }

    // No. of samples on each axis of the output.
    const unsigned int nFreq = map[FREQ].back().dst + 1;
    const unsigned int nTime = map[TIME].back().dst + 1;

    // Allocate the output.
    FlagArray out(nFreq, nTime, FlagType(0));

    // Allocate temporary 2-D arrays to store the number of samples per output
    // cell and the number of flagged samples per output cell.
    casa::Matrix<unsigned int> count(nFreq, nTime, 0);
    casa::Matrix<unsigned int> flagged(nFreq, nTime, 0);

    for(vector<Span>::const_iterator itTime = map[TIME].begin(),
        itTimeEnd = map[TIME].end(); itTime != itTimeEnd; ++itTime)
    {
        for(vector<Span>::const_iterator itFreq = map[FREQ].begin(),
            itFreqEnd = map[FREQ].end(); itFreq != itFreqEnd; ++itFreq)
        {
            ++count(itFreq->dst, itTime->dst);

            const FlagType flag = in(itFreq->src, itTime->src);
            ASSERT(flag == 0 || flag == 1);
            if(flag)
            {
                ASSERT(flag == 1);
                ++flagged(itFreq->dst, itTime->dst);
                out(itFreq->dst, itTime->dst) |= flag;
                ASSERT(out(itFreq->dst, itTime->dst) == 1);
            }
        }
    }

    // Remove flags for all output cells where the accumulated weight is larger
    // than the minimal weight.
    for(unsigned int t = 0; t < nTime; ++t)
    {
        for(unsigned int f = 0; f < nFreq; ++f)
        {
            DBGASSERT(count(f, t) > 0);

            // If the fractional number of flagged samples is below the user
            // specified threshold, make sure the output sample is not flagged.
            if((static_cast<double>(flagged(f, t)) / count(f, t))
                < itsFlagDensityThreshold)
            {
                out(f, t) = FlagType(0);
            }
        }
    }

    return out;
}

Matrix Resampler::resampleWithFlags(const Matrix &in, const FlagArray &flags,
    const vector<Span> (&map)[2]) const
{
    // Only complex input supported for now.
    ASSERT(in.isComplex());

    if(!in.isArray())
    {
        return in;
    }

    if(flags.rank() == 0 && *flags.begin())
    {
        return Matrix(makedcomplex(0.0, 0.0));
    }

    // No. of samples on the frequency axis of the input.
    const unsigned int nFreqIn = in.nx();

    // Get pointers to the input data.
    const double *inRe, *inIm;
    in.dcomplexStorage(inRe, inIm);

    // No. of samples on each axis of the output.
    const unsigned int nFreq = map[FREQ].back().dst + 1;
    const unsigned int nTime = map[TIME].back().dst + 1;

    // Allocate the output.
    Matrix out(makedcomplex(0.0, 0.0), nFreq, nTime);
    double *outRe, *outIm;
    out.dcomplexStorage(outRe, outIm);

    // Allocate a temporary 2-D array to store the sum of the weights.
    casa::Matrix<double> weight(nFreq, nTime, 0.0);

    for(vector<Span>::const_iterator itTime = map[TIME].begin(),
        itTimeEnd = map[TIME].end(); itTime != itTimeEnd; ++itTime)
    {
        for(vector<Span>::const_iterator itFreq = map[FREQ].begin(),
            itFreqEnd = map[FREQ].end(); itFreq != itFreqEnd; ++itFreq)
        {
            if(!flags(itFreq->src, itTime->src))
            {
                const size_t inIdx = itTime->src * nFreqIn + itFreq->src;
                const size_t outIdx = itTime->dst * nFreq + itFreq->dst;
                const double sampleWeight = itFreq->weight * itTime->weight;

                outRe[outIdx] += sampleWeight * inRe[inIdx];
                outIm[outIdx] += sampleWeight * inIm[inIdx];
                weight(itFreq->dst, itTime->dst) += sampleWeight;
            }
        }
    }

    // Reset pointers.
    out.dcomplexStorage(outRe, outIm);

    // Normalize the output samples by dividing by the sum of the weights.
    for(casa::Matrix<double>::const_iterator weightIt = weight.begin(),
        weightItEnd = weight.end(); weightIt != weightItEnd;
        ++weightIt, ++outRe, ++outIm)
    {
        if(*weightIt > 0.0)
        {
            *outRe /= *weightIt;
            *outIm /= *weightIt;
        }
    }

    return out;
}

Matrix Resampler::resample(const Matrix &in, const vector<Span> (&map)[2]) const
{
    // Only complex input supported for now.
    ASSERT(in.isComplex());

    if(!in.isArray())
    {
        return in;
    }

    // No. of samples on the frequency axis of the input.
    const unsigned int nFreqIn = in.nx();

    // Get pointers to the input data.
    const double *inRe, *inIm;
    in.dcomplexStorage(inRe, inIm);

    // No. of samples on each axis of the output.
    const unsigned int nFreq = map[FREQ].back().dst + 1;
    const unsigned int nTime = map[TIME].back().dst + 1;

    // Allocate the output.
    Matrix out(makedcomplex(0.0, 0.0), nFreq, nTime);
    double *outRe, *outIm;
    out.dcomplexStorage(outRe, outIm);

    // Allocate a temporary 2-D array to store the sum of the weights.
    casa::Matrix<double> weight(nFreq, nTime, 0.0);

    for(vector<Span>::const_iterator itTime = map[TIME].begin(),
        itTimeEnd = map[TIME].end(); itTime != itTimeEnd; ++itTime)
    {
        for(vector<Span>::const_iterator itFreq = map[FREQ].begin(),
            itFreqEnd = map[FREQ].end(); itFreq != itFreqEnd; ++itFreq)
        {
            const size_t inIdx = itTime->src * nFreqIn + itFreq->src;
            const size_t outIdx = itTime->dst * nFreq + itFreq->dst;
            const double sampleWeight = itFreq->weight * itTime->weight;

            outRe[outIdx] += sampleWeight * inRe[inIdx];
            outIm[outIdx] += sampleWeight * inIm[inIdx];
            weight(itFreq->dst, itTime->dst) += sampleWeight;
        }
    }

    // Reset pointers.
    out.dcomplexStorage(outRe, outIm);

    // Normalize the output samples by dividing by the sum of the weights.
    for(casa::Matrix<double>::const_iterator weightIt = weight.begin(),
        weightItEnd = weight.end(); weightIt != weightItEnd;
        ++weightIt, ++outRe, ++outIm)
    {
        DBGASSERT(*weightIt > 0.0);
        *outRe /= *weightIt;
        *outIm /= *weightIt;
    }

    return out;
}

} //# namespace BBS
} //# namespace LOFAR
