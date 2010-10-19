//# MeqDipoleBeamExternal.h: Dipole voltage beam based on external functions.
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

#ifndef MNS_MEQDIPOLEBEAMEXTERNAL_H
#define MNS_MEQDIPOLEBEAMEXTERNAL_H

#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqJonesResult.h>
#include <BBSKernel/MNS/ExternalFunction.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

class MeqDipoleBeamExternal: public MeqJonesExprRep
{
public:
    enum
    {
        IN_AZEL = 0,
        IN_ORIENTATION,
        N_InputPort
    } InputPort;

    MeqDipoleBeamExternal(const string &moduleTheta, const string &modulePhi,
        MeqExpr azel, MeqExpr orientation, double scaleFactor);

    virtual MeqJonesResult getJResult(const MeqRequest &request);

private:
    void evaluate(const MeqRequest &request, const MeqMatrix &in_az,
        const MeqMatrix &in_el, const MeqMatrix &in_orientation,
        MeqMatrix &out_E11, MeqMatrix &out_E12,
        MeqMatrix &out_E21, MeqMatrix &out_E22);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    ExternalFunction    itsThetaFunction, itsPhiFunction;
    double              itsScaleFactor;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
