//# cpu_utils.h: Helper functions for cpu specific functionality
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
//# $Id: cpu_utils.h 25651 2013-07-12 12:19:11Z mol $

// \file
// Include for processor optimalizetion functionality

#ifndef LOFAR_GPUPROC_CPU_INCL_H
#define LOFAR_GPUPROC_CPU_INCL_H
#include <CoInterface/Parset.h>
namespace LOFAR
{
  namespace Cobalt
  {
    // Set the correct processer affinity for a COBALT system
    // This will result in less then optimal performance on NON COBALT SYSTEMS!
    void setProcessorAffinity(unsigned procId);
  }
}
#endif
