//# UVWFlagger.cc: Flag visibilities based on various criteria.
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
#include <BBSKernel/UVWFlagger.h>

#include <Common/lofar_iomanip.h>
#include <BBSKernel/VisBuffer.h>
#include <casa/BasicSL/Constants.h>
#include <limits>

namespace LOFAR
{
namespace BBS
{

UVWFlagger::UVWFlagger(const VisBuffer::Ptr &buffer)
    :   itsBuffer(buffer),
        itsBaselineMask(true),
        itsMask(1),
        itsUVIntervalSqr(0.0, std::numeric_limits<double>::max()),
        itsFlagCount(0),
        itsSampleCount(0)
{
}

void UVWFlagger::setFlagMask(flag_t mask)
{
    itsMask = mask;
}

void UVWFlagger::setUVMin(double min)
{
    itsUVIntervalSqr.start = min * min;
}

void UVWFlagger::setUVMax(double max)
{
    itsUVIntervalSqr.end = max * max;
}

void UVWFlagger::setUVRange(double min, double max)
{
    setUVMin(min);
    setUVMax(max);
}

void UVWFlagger::setBaselineMask(const BaselineMask &mask)
{
    itsBaselineMask = mask;
}

bool UVWFlagger::isSelectionEmpty() const
{
    return itsBaselineMask.empty();
}

void UVWFlagger::process()
{
    if(itsBaselineMask.empty())
    {
        return;
    }

    if(!itsBuffer->hasUVW())
    {
        itsBuffer->computeUVW();
    }

    itsTimer.start();

    // Precompute 1.0 / lambda ^ 2 for all frequencies.
    vector<double> invLambdaSqr(itsBuffer->nFreq());
    for(size_t i = 0; i < itsBuffer->nFreq(); ++i)
    {
        const double freq = itsBuffer->grid()[FREQ]->center(i);
        invLambdaSqr[i] = (freq * freq) / (casa::C::c * casa::C::c);
    }

    for(size_t i = 0; i < itsBuffer->nBaselines(); ++i)
    {
        const baseline_t baseline = itsBuffer->baselines()[i];

        // Skip baseline if not selected for processing.
        if(!itsBaselineMask(baseline))
        {
            continue;
        }

        // Flag all samples for which the UV distance in wavelenghts falls
        // outside the interval.
        for(size_t j = 0; j < itsBuffer->nTime(); ++j)
        {
            const double u = itsBuffer->uvw[baseline.second][j][0]
                - itsBuffer->uvw[baseline.first][j][0];
            const double v = itsBuffer->uvw[baseline.second][j][1]
                - itsBuffer->uvw[baseline.first][j][1];
            const double distance = u * u + v * v;

            for(size_t k = 0; k < itsBuffer->nFreq(); ++k)
            {
                const double distanceLambdaSqr = invLambdaSqr[k] * distance;

                if(distanceLambdaSqr < itsUVIntervalSqr.start
                    || distanceLambdaSqr > itsUVIntervalSqr.end)
                {
                    // Update statistics.
                    itsFlagCount += itsBuffer->nCorrelations();

                    // Flag all correlations.
                    for(size_t l = 0; l < itsBuffer->nCorrelations(); ++l)
                    {
                        itsBuffer->flags[i][j][k][l] |= itsMask;
                    }
                }
            }
        }

        // Update statistics.
        itsSampleCount += itsBuffer->nTime() * itsBuffer->nFreq()
            * itsBuffer->nCorrelations();
    }

    itsTimer.stop();
}

void UVWFlagger::clearStats()
{
    itsSampleCount = 0;
    itsFlagCount = 0;
    itsTimer.reset();
}

void UVWFlagger::dumpStats(ostream &out) const
{
    out << "UVWFlagger statistics:" << endl;

    const double speed = itsTimer.getElapsed() > 0.0 ? itsSampleCount
        / itsTimer.getElapsed() : 0.0;
    const double percentage = itsSampleCount > 0
        ? static_cast<double>(itsFlagCount) / itsSampleCount * 100.0 : 0.0;
    const double average = itsTimer.getCount() > 0 ? itsTimer.getElapsed()
        / itsTimer.getCount() : 0.0;

    out << "Speed: " << fixed << speed << " samples/s" << endl;
    out << "No. of samples flagged: " << fixed << itsFlagCount << " ("
        << percentage << "%)" << endl;
    out << "TIMER s UVWFLAGGER ALL total " << itsTimer.getElapsed() << " count "
        << itsTimer.getCount() << " avg " << average << endl;
}

} //# namespace BBS
} //# namespace LOFAR
