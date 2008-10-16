//# DipoleBeamExternal.h: Dipole voltage beam based on external functions.
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
#include <BBSKernel/MNS/MeqExternalFunction.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup MNS
// @{

class DipoleBeamExternal: public JonesExprRep
{
public:
    enum
    {
        IN_AZEL = 0,
        IN_ORIENTATION,
        N_InputPort
    } InputPort;

    DipoleBeamExternal(const string &moduleTheta, const string &modulePhi,
        Expr azel, Expr orientation, double scaleFactor);

    virtual JonesResult getJResult(const Request &request);

private:
    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, const Matrix &in_orientation,
        Matrix &out_E11, Matrix &out_E12,
        Matrix &out_E21, Matrix &out_E22);

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
