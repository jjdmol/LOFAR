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


OperationPredict::OperationPredict(ParameterSet& parset): Operation(parset), OperationParamFTMachine(parset), OperationParamData(parset)
{}

void OperationPredict::init()
{}

void OperationPredict::run()
{
  Operation::run();
  OperationParamFTMachine::run();
  OperationParamData::run();
  
  casa::String model = itsInputParSet.getString("model");
  
  cout << "calling itsImager->predict" << endl;
  
  itsImager->predict(casa::Vector<casa::String>(1, model));
}

void OperationPredict::showHelp (ostream& os, const string& name)
{
  Operation::showHelp(os,name);
  os<<
  "Operation \"predict\": create a predicted image                   "<<endl<<
  "Parameters:                                                       "<<endl<<
  "  model           : model to be predicted                         "<<endl<<
  "                    (string,  ???                 \")             "<<endl<<
  "  MORE DOCUMENTATION TO BE ADDED                                  "<<endl;
};


} //# namespace LofarFT

} //# namespace LOFAR
