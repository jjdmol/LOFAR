//# MeqGaussianCoherency.h: Spatial coherence function of an elliptical
//#     gaussian source.
//#
//# Copyright (C) 2005
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

#ifndef MNS_MEQGAUSSIANCOHERENCY_H
#define MNS_MEQGAUSSIANCOHENRECY_H

// \file
// Spatial coherence function of an elliptical gaussian source.

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqGaussianSource.h>
#include <BBSKernel/MNS/MeqStatUVW.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward Declarations


class MeqGaussianCoherency: public MeqJonesExprRep
{
public:
    enum
    {
        IN_I,
        IN_Q,
        IN_U,
        IN_V,
        IN_MAJOR,
        IN_MINOR,
        IN_PHI,
        N_InputPort
    } InputPort;

    MeqGaussianCoherency(const MeqGaussianSource *source, MeqStatUVW *station1, MeqStatUVW *station2);
    ~MeqGaussianCoherency();

    // Calculate the results for the given domain.
    virtual MeqJonesResult getJResult(const MeqRequest &request);

private:
#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    MeqMatrix computeCoherence(const MeqRequest &request,
        const MeqMatrix &uBaseline, const MeqMatrix &vBaseline,
        const MeqMatrix &major, const MeqMatrix &minor, const MeqMatrix &phi);

    const MeqGaussianSource    *itsSource;
    MeqStatUVW                 *itsStation1, *itsStation2;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
