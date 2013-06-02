//# FindKernels.h: configure tests to be able to find GPU kernels
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

// NOTE: Include this file in the main .cc file to make sure it is included
// only once!

#ifndef COBALT_GPUPROC_FINDKERNELS
#define COBALT_GPUPROC_FINDKERNELS

#include <cstdlib>

// Need indirect quote macro to quote the VALUE of a define instead of its name
#define QUOTE_L(x) # x
#define QUOTE(x) QUOTE_L(x)

// Set LOFARROOT, which gpu::createPTX uses to find GPU kernels
#ifdef LOFARROOT
int __lofarroot_dummy = setenv("LOFARROOT", QUOTE(LOFARROOT), 1);
#endif

#endif

