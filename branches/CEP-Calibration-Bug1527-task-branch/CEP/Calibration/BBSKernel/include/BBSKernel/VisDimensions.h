//# VisDimensions.h: Represents the dimensions of a block of visibility data
//# along for axes (frequency, time, baseline, correlation).
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_VISDIMENSIONS_H
#define LOFAR_BBSKERNEL_VISDIMENSIONS_H

#include <BBSKernel/Correlation.h>
#include <BBSKernel/IndexedSequence.h>
#include <BBSKernel/Types.h>
#include <ParmDB/Grid.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

typedef IndexedSequence<baseline_t>         BaselineSeq;
typedef IndexedSequence<Correlation::Type>  CorrelationSeq;

class VisDimensions
{
public:
    // Set the sample grid (frequency and time axis).
    void setGrid(const Grid &grid);

    // Set the baseline axis.
    template <typename T_ITER>
    void setBaselines(T_ITER first, T_ITER last);
    void setBaselines(const BaselineSeq &axis);

    // Set the correlation axis.
    template <typename T_ITER>
    void setCorrelations(T_ITER first, T_ITER last);
    void setCorrelations(const CorrelationSeq &axis);

    // No. of sample points along each axis.
    // @{
    size_t nFreq() const;
    size_t nTime() const;
    size_t nBaselines() const;
    size_t nCorrelations() const;
    // @}

    // Bounding box in frequency and time.
    Box domain() const;

    // Sample grid in frequency and time.
    const Grid &grid() const;

    // Access to the frequency and time axis the make up the grid. This is just
    // a convenience function. It delegates directly to the underlying Grid
    // object.
    Axis::ShPtr operator[](size_t i) const;

    // Access to the baseline axis.
    const BaselineSeq &baselines() const;

    // Access to the correlation axis.
    const CorrelationSeq &correlations() const;

private:
    Grid            itsGrid;
    BaselineSeq     itsBaselineAxis;
    CorrelationSeq  itsCorrelationAxis;
};

// Stream a VisDimensions instance in human readable form to an output stream.
ostream &operator<<(ostream &out, const VisDimensions &obj);

// @}

// -------------------------------------------------------------------------- //
// - Implementation: VisDimensions                                          - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
void VisDimensions::setBaselines(T_ITER first, T_ITER last)
{
    itsBaselineAxis = BaselineSeq(first, last);
}

template <typename T_ITER>
void VisDimensions::setCorrelations(T_ITER first, T_ITER last)
{
    itsCorrelationAxis = CorrelationSeq(first, last);
}

inline size_t VisDimensions::nFreq() const
{
    return itsGrid[FREQ]->size();
}

inline size_t VisDimensions::nTime() const
{
    return itsGrid[TIME]->size();
}

inline size_t VisDimensions::nBaselines() const
{
    return itsBaselineAxis.size();
}

inline size_t VisDimensions::nCorrelations() const
{
    return itsCorrelationAxis.size();
}

inline Box VisDimensions::domain() const
{
    return itsGrid.getBoundingBox();
}

inline const Grid &VisDimensions::grid() const
{
    return itsGrid;
}

inline Axis::ShPtr VisDimensions::operator[](size_t i) const
{
    return itsGrid[i];
}

inline const BaselineSeq &VisDimensions::baselines() const
{
    return itsBaselineAxis;
}

inline const CorrelationSeq &VisDimensions::correlations() const
{
    return itsCorrelationAxis;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
