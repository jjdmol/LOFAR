//# IonosphereExpr.cc: Wrapper class that constructs ionosphere expressions
//# based on the IonosphereConfig instance provided to the constructor. The
//# class also stores a reference to any shared auxilliary data.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/IonosphereExpr.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/PolynomialLayer.h>

#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

IonosphereExpr::~IonosphereExpr()
{
}

IonosphereExpr::Ptr IonosphereExpr::create(const IonosphereConfig &config,
    Scope &scope)
{
    return IonosphereExpr::Ptr(new PolynomialLayerExpr(config, scope));
}

PolynomialLayerExpr::PolynomialLayerExpr(const IonosphereConfig &config,
    Scope &scope)
{
    // Get ionosphere model parameters.
    unsigned int nCoeff = config.getDegree() + 1;

    // Make sure degree is at least 1 (a gradient over the field of view).
    if(nCoeff <= 1)
    {
        LOG_WARN("Ionosphere model degree should be at least 1 (gradient); will"
            " change from specified degree to gradient.");

        nCoeff = 2;
    }

    itsCoeff.reserve(nCoeff * nCoeff - 1);
    for(unsigned int i = 0; i < nCoeff; ++i)
    {
        for(unsigned int j = 0; j < nCoeff; ++j)
        {
            // For the moment we do not include MIM:0:0 (absolute TEC).
            if(i == 0 && j == 0)
            {
                continue;
            }

            ostringstream oss;
            oss << "MIM:" << i << ":" << j;
            itsCoeff.push_back(scope(INSTRUMENT, oss.str()));
        }
    }

    LOG_DEBUG_STR("Number of coefficients in ionospheric phase screen model: "
        << itsCoeff.size());
}

Expr<JonesMatrix>::Ptr
PolynomialLayerExpr::construct(const casa::MPosition &position,
    const Expr<Vector<4> >::ConstPtr &piercePoint) const
{
    PolynomialLayer::Ptr angle(new PolynomialLayer(position,
        piercePoint, itsCoeff.begin(), itsCoeff.end()));
    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(angle, angle));
}

} //# namespace BBS
} //# namespace LOFAR
