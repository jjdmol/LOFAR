//# InitializeCommand.cc:
//#
//# Copyright (C) 2007
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
//# $Id: InitializeCommand.cc 16068 2010-07-23 19:53:58Z zwieten $

#include <lofar_config.h>

#include <LofarFT/OperationPredict.h>

namespace LOFAR {
namespace LofarFT {

namespace
{
  bool dummy = OperationFactory::instance().
    registerClass<OperationPredict>("predict");
}


OperationPredict::OperationPredict(ParameterSet& parset): Operation(parset)
{
  needsData=true;
}

void OperationPredict::init()
{
  Operation::init();
  casa::String model = itsParset.getString("model");
  itsImager->initPredict(casa::Vector<casa::String>(1, model));
}

void OperationPredict::run()
{
  itsImager->predict();
}

void OperationPredict::showHelp (ostream& os, const string& name)
{
  os<< COLOR_OPERATION << 
  "Operation \"predict\": create a predicted image"<< COLOR_RESET <<endl<<
  "Parameters:                                                       "<<endl<<
  "  " << COLOR_PARAMETER << "predict.model" << COLOR_RESET << "       : model image to be use"<<endl<<
  "                     (string,  " << COLOR_DEFAULT << "no default" << COLOR_RESET ")"<<endl<<endl;
  Operation::showHelp(os,name);
};


} //# namespace LofarFT

} //# namespace LOFAR
