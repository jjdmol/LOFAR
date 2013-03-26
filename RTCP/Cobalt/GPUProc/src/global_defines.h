//# global_defines.h
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

#ifndef LOFAR_GPUPROC_GLOBAL_DEFINES_H
#define LOFAR_GPUPROC_GLOBAL_DEFINES_H

#define NR_STATION_FILTER_TAPS  16
#define USE_NEW_CORRELATOR
#define NR_POLARIZATIONS         2 // TODO: get the nr of pol symbol from an LCS/Common header
#define NR_TAPS                 16
#define USE_2X2
#undef USE_CUSTOM_FFT
#undef USE_TEST_DATA
#undef USE_B7015

namespace LOFAR
{
  namespace Cobalt
  {
    extern bool profiling;
    extern unsigned nrGPUs;
  }
}

#endif

