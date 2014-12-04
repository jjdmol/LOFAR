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
#include <LofarFT/OperationImage.h>

namespace LOFAR {
namespace LofarFT {

namespace
{
  bool dummy = OperationFactory::instance().
    registerClass<OperationImage>("image");
}

  
OperationImage::OperationImage(ParameterSet& parset): Operation(parset)
{
    needsData = true;
    needsImage = true;
    needsWeight = true;
    needsFTMachine = true;
}

void OperationImage::init()
{
  Operation::init();
  itsImageName = itsParset.getString("output.imagename");
}

void OperationImage::run()
{
  itsImager->makeimage ("corrected", itsImageName + ".flatnoise");
  normalize(itsImageName + ".flatnoise", itsImageName + ".avgpb", itsImageName + ".flatgain");
}

void OperationImage::showHelp (ostream& os, const std::string& name)
{

  os<< COLOR_OPERATION << 
  "Operation \"image\": create a dirty image" << COLOR_RESET << endl <<
  "  There are no parameters specific for operation \"image\""<<endl << endl;

  Operation::showHelp(os,name);
};


} //# namespace LofarFT
} //# namespace LOFAR
