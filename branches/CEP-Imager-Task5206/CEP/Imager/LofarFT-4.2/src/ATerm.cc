//# ATerm.cc: Compute the LOFAR beam response on the sky.
//#
//# Copyright (C) 2011
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
#include <LofarFT/ATerm.h>

namespace LOFAR {
namespace LofarFT {
 
casa::CountedPtr<ATerm> ATerm::create(const casa::MeasurementSet &ms, ParameterSet& parset)
{
  return casa::CountedPtr<ATerm>(LOFAR::LofarFT::ATermFactory::instance().create(parset.getString("gridding.ATerm.name","ATermLofar"), ms, parset));
}

}
}

