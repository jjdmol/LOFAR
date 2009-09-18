//# PhaseRef.cc: Phase reference position and derived values
//#
//# Copyright (C) 2002
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
#include <BBSKernel/Expr/PhaseRef.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Vector.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

PhaseRef::PhaseRef()
    :   itsRa(0.0),
        itsDec(0.0),
        itsSinDec(0.0),
        itsCosDec(1.0)
{
}

PhaseRef::PhaseRef(const casa::MDirection &phaseRef)
{
    itsPhaseRef = MDirection::Convert(phaseRef, MDirection::J2000)();
    Quantum<Vector<double> > angles = itsPhaseRef.getAngle();
    itsRa  = angles.getBaseValue()(0);
    itsDec = angles.getBaseValue()(1);
    itsSinDec = std::sin(itsDec);
    itsCosDec = std::cos(itsDec);
}    

PhaseRef::~PhaseRef()
{
}

} // namespace BBS
} // namespace LOFAR
