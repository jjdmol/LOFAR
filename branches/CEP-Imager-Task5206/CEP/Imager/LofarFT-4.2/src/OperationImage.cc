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
    needsData=true;
    needsImage=true;
    needsFTMachine=true;
}

void OperationImage::run()
{
  itsImager->makeimage ("corrected", "testimage");
}

void OperationImage::showHelp (ostream& os, const std::string& name)
{
  Operation::showHelp(os,name);

  os<<
  "Operation \"image\": create a dirty image                         "<<endl<<
  "  No more parameters for operation\"image\"                       "<<endl;
};


} //# namespace LofarFT
} //# namespace LOFAR
