//# OperationClean.cc:
//#
//# Copyright (C) 2014
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
//# $Id: $

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <LofarFT/OperationEmpty.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
// Register OperationClean with the OperationFactory. Use an anonymous
// namespace. This ensures that the variable `dummy' gets its own private
// storage area and is only visible in this compilation unit.
namespace
{
  bool dummy = OperationFactory::instance().
    registerClass<OperationEmpty>("empty");
}

OperationEmpty::OperationEmpty(ParameterSet& parset): OperationImage(parset)
{
  needsFTMachine=false;
}

void OperationEmpty::run()
{
  casa::CoordinateSystem coords;
  AlwaysAssert (itsImager->imagecoordinates(coords), AipsError);
  int fieldid = 0; // TODO get from parameters
  itsImager->makeEmptyImage(coords, itsImageName, fieldid);
  itsImager->unlock();
}


void OperationEmpty::showHelp (ostream& os, const string& name)
{
  os<< COLOR_OPERATION << 
  "Operation \"empty\": create an empty image" << COLOR_RESET << endl <<
  "  There are no parameters specific for operation\"empty\"" << endl << endl;

  Operation::showHelp(os,name);
};


} //# namespace LofarFT

} //# namespace LOFAR
