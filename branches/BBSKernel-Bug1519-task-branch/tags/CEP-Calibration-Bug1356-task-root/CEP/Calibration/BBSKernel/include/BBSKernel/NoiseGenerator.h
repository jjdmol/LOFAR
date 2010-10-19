//# NoiseGenerator.h: Add parameterized Gaussian noise to the visibilities.
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

#ifndef LOFAR_BB_BBS_NOISEGENERATOR_H
#define LOFAR_BB_BBS_NOISEGENERATOR_H

// \file
// Add parameterized Gaussian noise to the visibilities.

#include <BBSKernel/VisData.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>

#include <boost/random/mersenne_twister.hpp>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class NoiseGenerator
{
public:
    NoiseGenerator();
    ~NoiseGenerator();

    // Attach/detach the chunk of data to process.
    // <group>
    void attachChunk(VisData::Pointer chunk);
    void detachChunk();
    // </group>

    void setSeed(uint32 seed);
    void setDistributionParms(double mean, double sigma);

    // Process the currently attached chunk.
    void process();
    
private:
    // NOTE: Both pseudo number generators and distributions support reading/
    // writing their state to/from an std::ostream.
    typedef boost::mt19937  GeneratorT;
    
    double              itsMean, itsSigma;
    GeneratorT          itsGenerator;
    VisData::Pointer    itsChunk;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
