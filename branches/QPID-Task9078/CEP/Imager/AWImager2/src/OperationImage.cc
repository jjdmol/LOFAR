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
#include <AWImager2/OperationImage.h>

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

  if (!itsMS.tableDesc().isColumn("CORRECTED_DATA"))
  {
    throw(AipsError("CORRECTED_DATA column not found."));
  }

  itsImageName = itsParset.getString("output.imagename");
}

void OperationImage::run()
{
  String type = itsParset.getString("image.type","corrected");
  itsImager->makeimage (type, itsImageName + ".flatnoise");
  LOG_DEBUG_STR("Normalizing...");
  normalize(itsImageName + ".flatnoise", itsImageName + ".avgpb", itsImageName + ".flatgain");
  LOG_DEBUG_STR("done.");
}

void OperationImage::showHelp (ostream& os, const std::string& name)
{

  os<< COLOR_OPERATION << 
  "Operation \"image\": create a dirty image" << endl
  << COLOR_RESET << endl <<
  "Parameters:                                                         "<<endl<<
  "  " << COLOR_PARAMETER << "image.type" << COLOR_RESET << "       : corrected, model, residual, psf"<<endl<<
  "                      string,  " << COLOR_DEFAULT << "default corrected" << COLOR_RESET <<endl;

  Operation::showHelp(os,name);
};


} //# namespace LofarFT
} //# namespace LOFAR
