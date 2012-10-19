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
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/IonosphereExpr.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/PiercePoint.h>
#include <BBSKernel/Expr/PolynomialLayer.h>
#include <BBSKernel/Expr/IonPhaseShift.h>

#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

IonosphereExpr::~IonosphereExpr()
{
}

IonosphereExpr::Ptr IonosphereExpr::create(const IonosphereConfig &config,
    Scope &scope)
{
    LOG_INFO_STR("Using ionosphere model: "
        << IonosphereConfig::asString(config.getModelType()));
    ASSERT(IonosphereConfig::isDefined(config.getModelType()));

    switch(config.getModelType())
    {
        case IonosphereConfig::MIM:
        {
            return IonosphereExpr::Ptr(new MIMExpr(config, scope));
        }

        case IonosphereConfig::EXPION:
        {
            return IonosphereExpr::Ptr(new ExpIonExpr(config, scope));
        }

        default:
            THROW(BBSKernelException, "Unsupported ionosphere model"
                " encountered.");
    }
}

MIMExpr::MIMExpr(const IonosphereConfig &config, Scope &scope)
{
    itsHeight = scope(INSTRUMENT, "MIM:Height");

    // Make sure degree is at least 1 (a gradient over the field of view).
    if(config.degree() == 0)
    {
        THROW(BBSKernelException, "MIM polynomial degree should be at least 1"
            " (a phase gradient).");
    }

    // Get ionosphere model parameters.
    unsigned int nCoeff = config.degree() + 1;

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
            oss << "MIM:Coeff:" << i << ":" << j;
            itsCoeff.push_back(scope(INSTRUMENT, oss.str()));
        }
    }

    LOG_DEBUG_STR("Number of coefficients in ionospheric phase screen model: "
        << itsCoeff.size());
}

Expr<JonesMatrix>::Ptr MIMExpr::construct(const casa::MPosition &refPosition,
    const casa::MPosition &station, const Expr<Vector<2> >::ConstPtr &azel)
    const
{
    PiercePoint::Ptr piercePoint(new PiercePoint(station, azel, itsHeight));

    PolynomialLayer::Ptr shift(new PolynomialLayer(refPosition, piercePoint,
        itsCoeff.begin(), itsCoeff.end()));

    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
}

ExpIonExpr::ExpIonExpr(const IonosphereConfig&, Scope &scope)
{
    // Create parameter nodes for ionosphere model parameters.
    itsR0 = scope(INSTRUMENT, "r_0");
    itsBeta = scope(INSTRUMENT, "beta");
    itsHeight = scope(INSTRUMENT, "height");

    // Find out which calibrator pierce points are available in the instrument
    // parameter database.
    //
    // TODO: Come up with a more robust approach for this.
    vector<string> parmNames = ParmManager::instance().find(INSTRUMENT,
        "Piercepoint:X:*:*");
    size_t nPiercePoints = parmNames.size();

    if(nPiercePoints == 0)
    {
        THROW(BBSKernelException, "No information about calibrator pierce"
            " points found in the instrument parameter database.");
    }

    // Create parameter nodes for calibrator pierce point related information.
    itsCalPiercePoint.reserve(nPiercePoints);
    itsTECWhite.reserve(nPiercePoints);
    for(size_t i = 0; i < nPiercePoints; ++i)
    {
        // Strip "Piercepoint:X" from the name.
        string suffix = parmNames[i].substr(13);
        ASSERT(!suffix.empty());

        AsExpr<Vector<3> >::Ptr piercePoint(new AsExpr<Vector<3> >());
        piercePoint->connect(0, scope(INSTRUMENT, "Piercepoint:X" + suffix));
        piercePoint->connect(1, scope(INSTRUMENT, "Piercepoint:Y" + suffix));
        piercePoint->connect(2, scope(INSTRUMENT, "Piercepoint:Z" + suffix));
        itsCalPiercePoint.push_back(piercePoint);

        AsExpr<Vector<2> >::Ptr tecWhite(new AsExpr<Vector<2> >());
        tecWhite->connect(0, scope(INSTRUMENT, "TECfit_white:0" + suffix));
        tecWhite->connect(1, scope(INSTRUMENT, "TECfit_white:1" + suffix));
        itsTECWhite.push_back(tecWhite);
    }
}

Expr<JonesMatrix>::Ptr ExpIonExpr::construct(const casa::MPosition&,
    const casa::MPosition &station, const Expr<Vector<2> >::ConstPtr &azel)
    const
{
    PiercePoint::Ptr piercePoint(new PiercePoint(station, azel, itsHeight));

    IonPhaseShift::Ptr shift(new IonPhaseShift(piercePoint, itsR0, itsBeta));
    shift->setCalibratorPiercePoints(itsCalPiercePoint.begin(),
        itsCalPiercePoint.end());
    shift->setTECWhite(itsTECWhite.begin(), itsTECWhite.end());

    return shift;
}

} //# namespace BBS
} //# namespace LOFAR
