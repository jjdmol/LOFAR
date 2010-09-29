//# TestGridder.cc: Test visibility gridder.
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
//#
//# @author Ger van Diepen <diepen at astron dot nl>


#include <LofarGridder/LofarGridder.h>
#include <gridding/VisGridderFactory.h>

using namespace askap::synthesis;

namespace LOFAR
{

  LofarGridder::LofarGridder()
  {}

  LofarGridder::~LofarGridder()
  {}

  // Clone a copy of this Gridder
  IVisGridder::ShPtr LofarGridder::clone() 
  {
    return IVisGridder::ShPtr(new LofarGridder(*this));
  }

  IVisGridder::ShPtr LofarGridder::makeGridder (const ParameterSet&)
  {
    std::cout << "LofarGridder::makeGridder" << std::endl;
    return IVisGridder::ShPtr(new LofarGridder());
  }

  const std::string& LofarGridder::gridderName()
  {
    static std::string name("LofarGridder");
    return name;
  }

  void LofarGridder::registerGridder()
  {
    VisGridderFactory::registerGridder (gridderName(), &makeGridder);
  }

  void LofarGridder::initIndices(const IConstDataAccessor&) 
  {
  }

  void LofarGridder::initConvolutionFunction(const IConstDataAccessor&)
  {
    itsSupport=0;
    itsOverSample=1;
    itsCSize=2*(itsSupport+1)*itsOverSample+1; // 3
    itsCCenter=(itsCSize-1)/2; // 1
    itsConvFunc.resize(1);
    itsConvFunc[0].resize(itsCSize, itsCSize); // 3, 3, 1
    itsConvFunc[0].set(0.0);
    itsConvFunc[0](itsCCenter,itsCCenter)=1.0; // 1,1,0 = 1
  }
    
  void LofarGridder::correctConvolution(casa::Array<double>& image)
  {
  }

} //# end namespace
