//# Evaluator.h: Evaluate a model and assign the result to or subtract it from
//# the visibility data in the chunk.
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

#ifndef LOFAR_BB_BBS_EVALUATOR_H
#define LOFAR_BB_BBS_EVALUATOR_H

// \file
// Evaluate a model and assign the result to or subtract it from the visibility
// data in the chunk.

#include <BBSKernel/Model.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Expr/MeqRequest.h>
#include <Common/Timer.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// @{

class Evaluator
{
public:
    enum Mode
    {
        ASSIGN = 0,
        SUBTRACT,
        N_Mode
    };
    
    Evaluator(const VisData::Pointer &chunk, const Model::Pointer &model);
    ~Evaluator();

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

    // Evaluate the model and assign/subtract visibilities to/from the chunk.
    void process(Mode mode = ASSIGN);

private:
    //# Define the signature for a function that processes the data of a single
    //# baseline.
    typedef void (Evaluator::*BlProcessor) (uint threadId,
        const baseline_t &baseline, const Request &request);

    void blAssign(uint threadId, const baseline_t &baseline, 
        const Request &request);
    void blSubtract(uint threadId, const baseline_t &baseline,
        const Request &request);

    VisData::Pointer    itsChunk;
    Model::Pointer      itsModel;

    vector<baseline_t>  itsBaselines;
    int                 itsProductMask[4];
    
    enum Timer
    {
        PRECOMPUTE,
        COMPUTE,
        N_Timer
    };

    NSTimer             itsTimers[N_Timer];
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
