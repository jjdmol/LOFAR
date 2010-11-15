//# VisBuffer.cc: A buffer of visibility data and associated information (e.g.
//# flags, UVW coordinates).
//#
//# Copyright (C) 2007
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
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Exceptions.h>

#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>

#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCBaseline.h>
#include <casa/Quanta/MVuvw.h>

namespace LOFAR
{
namespace BBS
{

VisBuffer::VisBuffer(const VisDimensions &dims)
    :   flags(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        samples(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        covariance(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()][dims.nCorrelations()]),
        itsDims(dims)
{
    LOG_DEBUG_STR("VisBuffer size: "
        << (nBaselines() * nTime() * nFreq() * nCorrelations() * sizeof(flag_t)
        + nBaselines() * nTime() * nFreq() * nCorrelations() * sizeof(dcomplex)
        + nBaselines() * nTime() * nFreq() * nCorrelations() * nCorrelations()
            * sizeof(double))
        / (1024.0 * 1024.0)
        << " MB.");
}

VisBuffer::VisBuffer(const VisDimensions &dims, const Instrument &instrument,
    const casa::MDirection &phaseRef, double refFreq)
    :   flags(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        samples(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        covariance(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()][dims.nCorrelations()]),
        itsDims(dims)
{
    LOG_DEBUG_STR("VisBuffer size: "
        << (nBaselines() * nTime() * nFreq() * nCorrelations() * sizeof(flag_t)
        + nBaselines() * nTime() * nFreq() * nCorrelations() * sizeof(dcomplex)
        + nBaselines() * nTime() * nFreq() * nCorrelations() * nCorrelations()
            * sizeof(double))
        / (1024.0 * 1024.0)
        << " MB.");

    setInstrument(instrument);
    setPhaseReference(phaseRef);
    setReferenceFreq(refFreq);
}

void VisBuffer::setPhaseReference(const casa::MDirection &reference)
{
    itsPhaseReference = casa::MDirection::Convert(reference,
        casa::MDirection::J2000)();
}

void VisBuffer::computeUVW()
{
    // For LOFAR MeasurementSets the UVW coordinates computed by this function
    // differ from the coordinates computed by the correlator only in the choice
    // of the reference position on earth. The correlator picks the position of
    // station N/2 as the reference position whereas here we use the centroid
    // of the station positions as the reference.

    // Ensure the UVW buffer is large enough.
    uvw.resize(boost::extents[nStations()][nTime()][3]);

    // Initialize frame.
    casa::Quantum<casa::Double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

    casa::MeasFrame frame(itsInstrument.position());
    frame.set(itsPhaseReference);
    frame.set(mEpoch);

    // Compute UVW.
    for(size_t i = 0; i < nStations(); ++i)
    {
        // Use station positions relative to the array reference position (to
        // keep values small).
        casa::MVBaseline mvBaseline(itsInstrument[i].position().getValue(),
            itsInstrument.position().getValue());
        casa::MBaseline mBaseline(mvBaseline, casa::MBaseline::ITRF);
        mBaseline.getRefPtr()->set(frame);

        // Setup coordinate transformation engine.
        casa::MBaseline::Convert convertor(mBaseline, casa::MBaseline::J2000);

        // Compute UVW coordinates.
        for(size_t j = 0; j < nTime(); ++j)
        {
            qEpoch.setValue(grid()[TIME]->center(j));
            mEpoch.set(qEpoch);
            frame.set(mEpoch);

            // Create MVuvw from a baseline (MVBaseline) and a reference
            // direction (MVDirection). Baseline and reference direction are
            // _assumed_ to be in the same frame (see casacore documentation).
            casa::MVuvw mvUVW(convertor().getValue(),
                itsPhaseReference.getValue());

            const casa::Vector<casa::Double> &xyz = mvUVW.getValue();
            uvw[i][j][0] = xyz(0);
            uvw[i][j][1] = xyz(1);
            uvw[i][j][2] = xyz(2);
        }
    }
}

void VisBuffer::flagsAndWithMask(flag_t mask)
{
    typedef boost::multi_array<flag_t, 4>::element* iterator;
    for(iterator it = flags.data(), end = flags.data() + flags.num_elements();
        it != end; ++it)
    {
        *it &= mask;
    }
}

void VisBuffer::flagsOrWithMask(flag_t mask)
{
    typedef boost::multi_array<flag_t, 4>::element* iterator;
    for(iterator it = flags.data(), end = flags.data() + flags.num_elements();
        it != end; ++it)
    {
        *it |= mask;
    }
}

void VisBuffer::flagsSet(flag_t value)
{
    typedef boost::multi_array<flag_t, 4>::element* iterator;
    for(iterator it = flags.data(), end = flags.data() + flags.num_elements();
        it != end; ++it)
    {
        *it = value;
    }
}

void VisBuffer::flagsNot()
{
    typedef boost::multi_array<flag_t, 4>::element* iterator;
    for(iterator it = flags.data(), end = flags.data() + flags.num_elements();
        it != end; ++it)
    {
        *it = ~(*it);
    }
}

} //# namespace BBS
} //# namespace LOFAR
