//# NoiseGenerator.cc: Add parameterized Gaussian noise to the visibilities.
//#
//# Copyright (C) 2008
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
#include <BBSKernel/NoiseGenerator.h>

#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

namespace LOFAR
{
namespace BBS 
{

NoiseGenerator::NoiseGenerator()
    :   itsMean(0.0),
        itsSigma(1.0)
{
}

NoiseGenerator::~NoiseGenerator()
{
}

void NoiseGenerator::attachChunk(VisData::Ptr chunk)
{
    // Check preconditions.
    ASSERT(chunk);

    // Set chunk.
    itsChunk = chunk;
}

void NoiseGenerator::detachChunk()
{
    // Release chunk.
    itsChunk.reset();
}

void NoiseGenerator::setSeed(uint32 seed)
{
    itsGenerator.seed(seed);
}

void NoiseGenerator::setDistributionParms(double mean, double sigma)
{
    itsMean = mean;
    itsSigma = sigma;
}

void NoiseGenerator::process()
{
    // Check preconditions.
    ASSERT(itsChunk);

    typedef boost::normal_distribution<float> DistributionT;
    
    boost::variate_generator<GeneratorT, DistributionT> variate(itsGenerator,
        DistributionT(itsMean, itsSigma));

    size_t nBl = itsChunk->vis_data.shape()[0];
    size_t nT = itsChunk->vis_data.shape()[1];
    size_t nF = itsChunk->vis_data.shape()[2];
    size_t nP = itsChunk->vis_data.shape()[3];

    for(size_t bl = 0; bl < nBl; ++bl)
    {
    for(size_t t = 0; t < nT; ++t)
    {
    for(size_t f = 0; f < nF; ++f)
    {
    for(size_t p = 0; p < nP; ++p)
    {
        itsChunk->vis_data[bl][t][f][p] += sample_t(variate(), variate());
    }
    }
    }
    }
    
/*
    typedef boost::multi_array<sample_t, 4>::iterator IteratorT;
    IteratorT it = itsChunk->vis_data.begin();
    IteratorT end = itsChunk->vis_data.end();
    while(it != end)
    {
        sample_t x = sample_t(variate(), variate());
        *it = sample_t(10.0f,10.0f);
        ++it;
    }
*/    
}

} //# namespace BBS
} //# namespace LOFAR
