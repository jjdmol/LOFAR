//# KernelFactory.cc
//#
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include "KernelFactory.h"

#include <string>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Config.h>

using namespace std;
using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    KernelFactoryBase::~KernelFactoryBase()
    {
    }

    CompileDefinitions
    KernelFactoryBase::compileDefinitions(const Kernel::Parameters& /*param*/) const
    {
      CompileDefinitions defs;
      defs["COMPLEX"] = "2"; // TODO: get rid of this: replace with proper complex type names
      defs["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);

      /*
      defs["NR_CHANNELS"] = lexical_cast<string>(param.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(param.nrSamplesPerChannel);
      defs["NR_SAMPLES_PER_SUBBAND"] = 
        lexical_cast<string>(param.nrSamplesPerSubband);
      defs["NR_STATIONS"] = lexical_cast<string>(param.nrStations);
      */
      return defs;
    }

    CompileFlags
    KernelFactoryBase::compileFlags(const Kernel::Parameters& /*param*/) const
    {
      CompileFlags flags;
      return flags;
    }

  }
}
