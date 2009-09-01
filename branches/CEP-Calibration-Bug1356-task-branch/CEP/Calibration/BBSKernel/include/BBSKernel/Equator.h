//# Equator.h: Generate normal equations that tie a model to an observation.
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

#ifndef LOFAR_BBSKERNEL_EQUATOR_H
#define LOFAR_BBSKERNEL_EQUATOR_H

// \file
// Generate normal equations that tie a model to an observation.

#include <BBSKernel/Model.h>
#include <BBSKernel/SolverInterfaceTypes.h>
#include <BBSKernel/VisData.h>

#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Equator
{
public:
    Equator(const VisData::Ptr &chunk, const Model::Ptr &model,
        const Grid &grid, const CoeffIndex &coeffIndex);

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

    // Set the (inclusive) range of cells of the solution grid to process.
    void setCellSelection(const Location &start, const Location &end);

    // Generate equations.
    // TODO: Use output iterators instead of a non-const reference.
    void process(vector<CellEquation> &out);

private:
    // Nested class that represents an interval of cells along an axis. Used to
    // create the mapping from cells in the solution grid to intervals of cells
    // in the observation grid.
    class Interval
    {
    public:
        Interval();

        unsigned int    start;
        unsigned int    end;
    };

    // Nested class that holds the temporary buffers needed while processing
    // a baseline and some counters and timers used to gather processing
    // statistics. The reason this is collected in a class is that it is easy to
    // create a thread-private instance for each thread should this code be
    // parallelized. Also, it is a way of grouping a number of members that
    // belong together semantically.
    class BlContext
    {
    public:
        BlContext();

        void resize(unsigned int nCoeff);
        void reset();

        // Mapping from available partial derivatives (parmId, coeffId) to a
        // coefficient index in the normal matrix.
        vector<unsigned int>    index;
        // References to the (appoximated) partial derivatives.
        vector<Matrix>          partial;
        // Value of the (approximated) partial derivatives.
        vector<double>          partialRe, partialIm;

        // Statistics and timers.
        size_t                  count;

        enum BlContextTimer
        {
            MODEL_EVAL,
            EQUATE,
            BUILD_INDEX,
            TRANSPOSE,
            MAKE_NORM,
            N_BlContextTimer
        };

        static string           timerNames[N_BlContextTimer];
        NSTimer                 timers[N_BlContextTimer];
    };

    // Return the number of solvable coefficients.
    size_t getCoeffCount() const;

    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the observation (chunk) grid.
    void makeGridMapping();

    // Create a mapping from cells of axis "from" to cell intervals on axis
    // "to". Additionally, the interval of cells of axis "from" that intersect
    // axis "to" (the domain) is returned.
    pair<Interval, vector<Interval> > makeAxisMapping(const Axis::ShPtr &from,
        const Axis::ShPtr &to) const;

    // Create a mapping from (parmId, coeffId) to a coefficient index in the
    // normal equations.
    void makeCoeffMapping(const CoeffIndex &index);

    // Generate normal equations for a single baseline.
    template <typename T_ITER>
    void blProcess(T_ITER outputEqIt, const baseline_t &baseline,
        const Location &selStart, const Location &selEnd, BlContext &context);

    // Observed visibilities.
    VisData::Ptr                        itsChunk;
    // Model of the sky and the instrument.
    Model::Ptr                          itsModel;
    // Solution grid.
    Grid                                itsGrid;

    // Selection of the visibility data to process.
    vector<baseline_t>                  itsBaselines;
    int                                 itsProductMask[4];

    // Is the intersection between the solution grid and the available
    // visibility data empty?
    bool                                itsIntersectionEmpty;
    // Location in the solution grid of the current chunk's start and end cell.
    Location                            itsChunkStart, itsChunkEnd;
    // Location in the solution grid of the current selection (clipped against
    // the available visibility data).
    Location                            itsSelectionStart, itsSelectionEnd;
    // The number of cells in the current selection (clipped against the
    // available visibility data).
    unsigned int                        itsSelectedCellCount;

    // Mapping of cells in the solution grid to intervals of cells along the
    // observation grid's axes.
    vector<Interval>                    itsFreqIntervals, itsTimeIntervals;

    // Mapping from (parmId, coeffId) to a coefficient index in the normal
    // equations.
    map<PValueKey, unsigned int>        itsCoeffMap;

    // Baseline processing buffers.
    BlContext                           itsBlContext;
};

// @}

// -------------------------------------------------------------------------- //
// - Equator implementation                                                 - //
// -------------------------------------------------------------------------- //

inline size_t Equator::getCoeffCount() const
{
    return itsCoeffMap.size();
}

template <typename T_ITER>
void Equator::blProcess(T_ITER outputEqIt, const baseline_t &baseline,
    const Location &selStart, const Location &selEnd, BlContext &context)
{
    // Evaluate the model.
    context.timers[BlContext::MODEL_EVAL].start();
    const JonesMatrix model = itsModel->evaluate(baseline);

    // If the model contains no flags, assume no sample is flagged.
    // TODO: This incurs a cost for models that do not contain flags because
    // a call to virtual FlagArray::operator() is made for each sample.
    const FlagArray flags =
        (model.hasFlags() ? model.flags() : FlagArray((FlagType())));
    context.timers[BlContext::MODEL_EVAL].stop();

    // Construct equations.
    context.timers[BlContext::EQUATE].start();

    // Find baseline index.
    // NB. VisDimensions::getBaselineIndex() could throw an exception.
    const size_t bl = itsChunk->getDimensions().getBaselineIndex(baseline);

    // Offset in the visibility grid of the start of the selected cells.
    const Location visStart(itsFreqIntervals[selStart.first].start,
        itsTimeIntervals[selStart.second].start);

    for(unsigned int prod = 0; prod < 4; ++prod)
    {
        const int extProd = itsProductMask[prod];
        if(extProd == -1)
        {
            continue;
        }

//        typedef boost::multi_array<sample_t, 4>::index_range Range;
//        typedef boost::multi_array<sample_t, 4>::array_view<2>::type View;
//        View vdata(itsChunk->vis_data[boost::indices[bl][Range()][Range()]
//            [extProd]]);

        // Determine which parameters have partial derivatives (e.g. when
        // solving for station-bound parameters, only a few parameters
        // per baseline are relevant).
        context.timers[BlContext::BUILD_INDEX].start();
        unsigned int nBlCoeff = 0;
        for(ValueSet::const_iterator it = model.getValueSet(prod).begin(),
            end = model.getValueSet(prod).end();
            it != end;
            ++it, ++nBlCoeff)
        {
            // Look-up coefficient index for this coefficient.
            context.index[nBlCoeff] = itsCoeffMap[it->first];

            // Get a reference to the partial derivative of the model with
            // respect to this coefficient.
            context.partial[nBlCoeff] = it->second;
        }
        context.timers[BlContext::BUILD_INDEX].stop();

        // If there are no coefficients to fit, continue to the next
        // polarization product.
        if(nBlCoeff == 0)
        {
            continue;
        }

        // Get the model visibilities.
        const Matrix modelValue = model.getValueSet(prod).value();

        // Loop over all selected cells and generate equations, adding them
        // to the associated normal matrix.
        T_ITER cellEqIt = outputEqIt;
        CellIterator cellIt(selStart, selEnd);
        while(!cellIt.atEnd())
        {
            // Get a reference to the normal matrix associated to this cell.
            casa::LSQFit &equation = cellEqIt->equation;

            // Samples to process (observed visibilities).
            const Interval &chInterval = itsFreqIntervals[cellIt->first];
            const Interval &tsInterval = itsTimeIntervals[cellIt->second];

//            size_t visOffset = (tsInterval.start - visStart.second)
//                * nChannels + (chInterval.start - visStart.first);

            for(unsigned int ts = tsInterval.start; ts <= tsInterval.end; ++ts)
            {
                // Skip timeslot if flagged.
                if(itsChunk->tslot_flag[bl][ts])
                {
//                    visOffset += nChannels;
                    continue;
                }

                // Construct two equations for each unflagged visibility.
                for(unsigned int ch = chInterval.start; ch <= chInterval.end;
                    ++ch)
                {
                    if(itsChunk->vis_flag[bl][ts][ch][extProd]
                        || flags((int)(ch - visStart.first),
                            (int)(ts - visStart.second)))
                    {
                        continue;
                    }

                    // Update statistics.
                    ++context.count;

                    // Compute right hand side of the equation pair.
                    const complex_t rhs =
                        static_cast<complex_t>
                        (itsChunk->vis_data[bl][ts][ch][extProd])
                            - modelValue.getDComplex((int)(ch - visStart.first),
                                (int)(ts - visStart.second));

                    // Tranpose the partial derivatives.
                    context.timers[BlContext::TRANSPOSE].start();
                    for(unsigned int i = 0; i < nBlCoeff; ++i)
                    {
                        const complex_t partial =
                            context.partial[i].getDComplex((int)(ch
                                - visStart.first), (int)(ts - visStart.second));

                        context.partialRe[i] = real(partial);
                        context.partialIm[i] = imag(partial);
                    }
                    context.timers[BlContext::TRANSPOSE].stop();

                    // Generate condition equations.
                    context.timers[BlContext::MAKE_NORM].start();
                    equation.makeNorm(nBlCoeff,
                        &(context.index[0]),
                        &(context.partialRe[0]),
                        1.0,
                        real(rhs));

                    equation.makeNorm(nBlCoeff,
                        &(context.index[0]),
                        &(context.partialIm[0]),
                        1.0,
                        imag(rhs));
                    context.timers[BlContext::MAKE_NORM].stop();
//                    // Move to next channel.
//                    ++visOffset;
                } // for(size_t ch = chStart; ch < chEnd; ++ch)

                // Move to next timeslot.
//                visOffset +=
//                    nChannels - (chInterval.end - chInterval.start + 1);
            } // for(size_t ts = tsStart; ts < tsEnd; ++ts)

            // Move to the next cell.
            ++cellEqIt;
            ++cellIt;
        }
    }

    context.timers[BlContext::EQUATE].stop();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
