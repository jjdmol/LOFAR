//# ElementBeamExpr.cc: Wrapper class that constructs the correct beam expr based on the BeamConfig instance provided in the constructor. It also caches any shared auxilliary data.
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
#include <BBSKernel/ElementBeamExpr.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Expr/YatawattaDipole.h>

namespace LOFAR
{
namespace BBS
{

ElementBeamExpr::~ElementBeamExpr()
{
}

ElementBeamExpr::Ptr ElementBeamExpr::create(const BeamConfig &config,
    Scope &scope)
{
    LOG_INFO_STR("Using element beam type: "
        << config.getElementTypeAsString());
    ASSERT(config.getElementType() != BeamConfig::UNKNOWN);

    switch(config.getElementType())
    {
        case BeamConfig::HAMAKER_LBA:
        case BeamConfig::HAMAKER_HBA:
        {
            return ElementBeamExpr::Ptr(new HamakerBeamExpr(config, scope));
        }

        case BeamConfig::YATAWATTA_LBA:
        case BeamConfig::YATAWATTA_HBA:
        {
            return ElementBeamExpr::Ptr(new YatawattaBeamExpr(config, scope));
        }

        default:
            THROW(BBSKernelException, "Unsupported element type encountered.");
    }
}

HamakerBeamExpr::HamakerBeamExpr(const BeamConfig &config, Scope&)
{
    ASSERT(config.getElementType() == BeamConfig::HAMAKER_LBA
        || config.getElementType() == BeamConfig::HAMAKER_HBA);

    casa::Path path = config.getElementPath();
    path.append("element_beam_" + config.getElementTypeAsString() + ".coeff");
    LOG_INFO_STR("Element beam config file: " << path.expandedName());

    // Read beam coefficients from file.
    itsCoeff.init(path);
}

Expr<JonesMatrix>::Ptr
HamakerBeamExpr::construct(const Expr<Vector<2> >::ConstPtr &direction,
    const Expr<Scalar>::ConstPtr &orientation) const
{
    return Expr<JonesMatrix>::Ptr(new HamakerDipole(itsCoeff, direction,
        orientation));
}

YatawattaBeamExpr::YatawattaBeamExpr(const BeamConfig &config, Scope&)
{
    ASSERT(config.getElementType() == BeamConfig::YATAWATTA_LBA
        || config.getElementType() == BeamConfig::YATAWATTA_HBA);

    // TODO: Transparantly handle platforms that use a different extension for
    // loadable modules.
    itsModulePath[0] = config.getElementPath();
    itsModulePath[0].append("element_beam_" + config.getElementTypeAsString()
        + "_theta.so");

    itsModulePath[1] = config.getElementPath();
    itsModulePath[1].append("element_beam_" + config.getElementTypeAsString()
        + "_phi.so");

    LOG_INFO_STR("Element beam loadable modules: ["
        << itsModulePath[0].expandedName() << ","
        << itsModulePath[1].expandedName() << "]");
}

Expr<JonesMatrix>::Ptr
YatawattaBeamExpr::construct(const Expr<Vector<2> >::ConstPtr &direction,
    const Expr<Scalar>::ConstPtr &orientation) const
{
    return Expr<JonesMatrix>::Ptr(new YatawattaDipole(itsModulePath[0],
        itsModulePath[1], direction, orientation));
}

} //# namespace BBS
} //# namespace LOFAR
