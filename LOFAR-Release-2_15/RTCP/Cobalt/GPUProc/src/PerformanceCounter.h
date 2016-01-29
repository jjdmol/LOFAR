//# PerformanceCounter.h
//#
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

// \file
// Support GPU kernel performance timing.

#ifndef LOFAR_PERFORMANCE_COUNTER_H
#define LOFAR_PERFORMANCE_COUNTER_H

#if defined (USE_CUDA) && defined (USE_OPENCL)
# error "Either CUDA or OpenCL must be enabled, not both"
#endif

#if defined (USE_CUDA)
# include "cuda/PerformanceCounter.h"
#elif defined (USE_OPENCL)
# include "opencl/PerformanceCounter.h"
#else
# error "Either CUDA or OpenCL must be enabled, not neither"
#endif

#endif

