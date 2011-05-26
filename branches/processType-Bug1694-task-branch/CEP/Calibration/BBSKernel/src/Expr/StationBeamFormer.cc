//# StationBeamFormer.cc: Beam forming the signals from individual dipoles
//# or tiles into a single station beam.
//#
//# Copyright (C) 2011
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
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Exceptions.h>
#include <Common/lofar_algorithm.h>

#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::min;

StationBeamFormer::StationBeamFormer
    (const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &reference,
    const Expr<JonesMatrix>::ConstPtr &beam0,
    const Station::ConstPtr &station,
    double referenceFreq, bool conjugate)
    :   itsDirection(direction),
        itsReference(reference),
        itsStation(station),
        itsReferenceFreq(referenceFreq),
        itsConjugateFlag(conjugate)
{
    ASSERT(itsStation->nField() == 1);

    connect(itsDirection);
    connect(itsReference);

    itsElementBeam.push_back(beam0);
    connect(beam0);
}

StationBeamFormer::StationBeamFormer
    (const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &reference,
    const Expr<JonesMatrix>::ConstPtr &beam0,
    const Expr<JonesMatrix>::ConstPtr &beam1,
    const Station::ConstPtr &station,
    double referenceFreq, bool conjugate)
    :   itsDirection(direction),
        itsReference(reference),
        itsStation(station),
        itsReferenceFreq(referenceFreq),
        itsConjugateFlag(conjugate)
{
    ASSERT(itsStation->nField() == 2);

    connect(itsDirection);
    connect(itsReference);

    itsElementBeam.push_back(beam0);
    connect(beam0);

    itsElementBeam.push_back(beam1);
    connect(beam1);
}

StationBeamFormer::~StationBeamFormer()
{
    typedef vector<Expr<JonesMatrix>::ConstPtr>::const_reverse_iterator Iter;
    for(Iter it = itsElementBeam.rbegin(); it != itsElementBeam.rend(); ++it)
    {
        disconnect(*it);
    }
    disconnect(itsReference);
    disconnect(itsDirection);
}

unsigned int StationBeamFormer::nArguments() const
{
    return 2 + itsElementBeam.size();
}

ExprBase::ConstPtr StationBeamFormer::argument(unsigned int i) const
{
    DBGASSERT(i < nArguments());
    switch(i)
    {
        case 0:
            return itsDirection;
        case 1:
            return itsReference;
        default:
            return itsElementBeam[i - 2];
    }
}

const JonesMatrix StationBeamFormer::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Check preconditions.
    const size_t nElementBeam = itsElementBeam.size();

    // Evaluate arguments.
    vector<FlagArray> flags;

    const Vector<3> direction = itsDirection->evaluate(request, cache, grid);
    flags.push_back(direction.flags());

    const Vector<3> reference = itsReference->evaluate(request, cache, grid);
    flags.push_back(reference.flags());

    vector<JonesMatrix> beam;
    for(size_t i = 0; i < nElementBeam; ++i)
    {
        beam.push_back(itsElementBeam[i]->evaluate(request, cache, grid));
        flags.push_back(beam.back().flags());
    }

    EXPR_TIMER_START();

    // Evaluate flags.
    JonesMatrix result;
    result.setFlags(mergeFlags(flags.begin(), flags.end()));

    // Compute main value.
    vector<JonesMatrix::View> views;
    for(size_t i = 0; i < nElementBeam; ++i)
    {
        views.push_back(beam[i].view());
    }

    result.assign(evaluateImpl(request[grid], direction.view(),
        reference.view(), views));

    // Compute perturbed values.
    Vector<3>::Iterator dirIt(direction);
    Vector<3>::Iterator refIt(reference);
    bool atEnd = dirIt.atEnd() && refIt.atEnd();

    vector<JonesMatrix::Iterator> beamIt;
    for(size_t i = 0; i < nElementBeam; ++i)
    {
        beamIt.push_back(JonesMatrix::Iterator(beam[i]));
        atEnd = atEnd && beamIt.back().atEnd();
    }

    PValueKey key;
    while(!atEnd)
    {
        key = min(dirIt.key(), refIt.key());
        for(size_t i = 0; i < nElementBeam; ++i)
        {
            key = min(key, beamIt[i].key());
        }

        for(size_t i = 0; i < nElementBeam; ++i)
        {
            views[i] = beamIt[i].value(key);
        }

        result.assign(key, evaluateImpl(request[grid], dirIt.value(key),
            refIt.value(key), views));

        dirIt.advance(key);
        refIt.advance(key);
        atEnd = dirIt.atEnd() && refIt.atEnd();
        for(size_t i = 0; i < nElementBeam; ++i)
        {
            beamIt[i].advance(key);
            atEnd = atEnd && beamIt[i].atEnd();
        }
    }

    EXPR_TIMER_STOP();
    return result;
}

const JonesMatrix::View StationBeamFormer::evaluateImpl(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &reference,
    const vector<JonesMatrix::View> &beam) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(!direction(0).isComplex() && direction(0).nx() == 1
        && static_cast<size_t>(direction(0).ny()) == nTime);
    ASSERT(!direction(1).isComplex() && direction(1).nx() == 1
        && static_cast<size_t>(direction(1).ny()) == nTime);
    ASSERT(!direction(2).isComplex() && direction(2).nx() == 1
        && static_cast<size_t>(direction(2).ny()) == nTime);

    ASSERT(!reference(0).isComplex() && reference(0).nx() == 1
        && static_cast<size_t>(reference(0).ny()) == nTime);
    ASSERT(!reference(1).isComplex() && reference(1).nx() == 1
        && static_cast<size_t>(reference(1).ny()) == nTime);
    ASSERT(!reference(2).isComplex() && reference(2).nx() == 1
        && static_cast<size_t>(reference(2).ny()) == nTime);
    ASSERT(beam.size() == itsStation->nField());

    // Compute angular reference frequency.
    const double omega0 = casa::C::_2pi * itsReferenceFreq;

    // Allocate result (initialized at 0+0i).
    Matrix E[2][2];
    Matrix AF(makedcomplex(0.0, 0.0), nFreq, nTime);

    size_t countX = 0, countY = 0;
    for(size_t i = 0; i < itsStation->nField(); ++i)
    {
        AntennaField::ConstPtr field = itsStation->field(i);

        // Account for the case where the delay center is not equal to the
        // field center (only applies to core HBA fields).
        const Vector3 &fieldCenter = field->position();
        casa::MVPosition delayCenter = itsStation->position().getValue();
        Vector3 offsetShift = {{fieldCenter[0] - delayCenter(0),
            fieldCenter[1] - delayCenter(1),
            fieldCenter[2] - delayCenter(2)}};

        // Compute array factors.
        Matrix AFX(makedcomplex(0.0, 0.0), nFreq, nTime);
        Matrix AFY(makedcomplex(0.0, 0.0), nFreq, nTime);
        for(size_t j = 0; j < field->nElement(); ++j)
        {
            const AntennaField::Element &element = field->element(j);
            if(element.flag[0] && element.flag[1])
            {
                continue;
            }

            // Compute the offset relative to the delay center.
            Vector3 offset = {{element.offset[0] + offsetShift[0],
                element.offset[1] + offsetShift[1],
                element.offset[2] + offsetShift[2]}};

            // Compute the delay for a plane wave approaching from the direction
            // of interest with respect to the phase center of the element.
            Matrix delay = (direction(0) * offset[0] + direction(1) * offset[1]
                + direction(2) * offset[2]) / casa::C::c;

            // Compute the delay for a plane wave approaching from the phase
            // reference direction with respect to the phase center of the
            // element.
            Matrix delay0 = (reference(0) * offset[0] + reference(1) * offset[1]
                + reference(2) * offset[2]) / casa::C::c;

            // Compute array factor contribution for this element.
            double *p_re, *p_im;
            AF.dcomplexStorage(p_re, p_im);
            const double *p_delay = delay.doubleStorage();
            const double *p_delay0 = delay0.doubleStorage();
            for(size_t t = 0; t < nTime; ++t)
            {
                const double delay_t = *p_delay++;
                const double shift0 = omega0 * (*p_delay0++);

                for(size_t f = 0; f < nFreq; ++f)
                {
                    const double shift = casa::C::_2pi * grid[FREQ]->center(f)
                        * delay_t - shift0;
                    *p_re = std::cos(shift);
                    *p_im = std::sin(shift);
                    ++p_re;
                    ++p_im;
                }
            }

            if(!element.flag[0])
            {
                AFX += AF;
                ++countX;
            }

            if(!element.flag[1])
            {
                AFY += AF;
                ++countY;
            }
        }

        // Conjugate array factors if required.
        if(itsConjugateFlag)
        {
            AFX = conj(AFX);
            AFY = conj(AFY);
        }

        if(i == 0)
        {
            E[0][0] = AFX * beam[i](0, 0);
            E[0][1] = AFX * beam[i](0, 1);
            E[1][0] = AFY * beam[i](1, 0);
            E[1][1] = AFY * beam[i](1, 1);
        }
        else
        {
            E[0][0] += AFX * beam[i](0, 0);
            E[0][1] += AFX * beam[i](0, 1);
            E[1][0] += AFY * beam[i](1, 0);
            E[1][1] += AFY * beam[i](1, 1);
        }
    }

    // Normalize.
    if(countX > 0)
    {
        E[0][0] /= countX;
        E[0][1] /= countX;
    }

    if(countY > 0)
    {
        E[1][0] /= countY;
        E[1][1] /= countY;
    }

    return JonesMatrix::View(E[0][0], E[0][1], E[1][0], E[1][1]);
}

} //# namespace BBS
} //# namespace LOFAR
