//# PointSource.h: Point source with optional spectral index and (intrinsic) rotation measure.
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

#ifndef DPPP_POINTSOURCE_H
#define DPPP_POINTSOURCE_H

// \file
// Point source with optional spectral index and (intrinsic) rotation measure.

#include <DPPP/Position.h>
#include <DPPP/Stokes.h>
#include <Common/lofar_math.h>
#include <Common/lofar_vector.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace DPPP
{

// \addtogroup NDPPP
// @{

class PointSource
{
public:
    PointSource()
        :   itsRefFreq(0.0),
            itsPolarizedFraction(0.0),
            itsPolarizationAngle(0.0),
            itsRotationMeasure(0.0)
    {
    }

    PointSource(const Position &position)
        :   itsPosition(position),
            itsRefFreq(0.0),
            itsPolarizedFraction(0.0),
            itsPolarizationAngle(0.0),
            itsRotationMeasure(0.0)
    {
    }

    PointSource(const Position &position, const Stokes &stokes)
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

// @}

} //# namespace DPPP
} //# namespace LOFAR

#endif
