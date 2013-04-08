//# CUDAException.h: Exception class for wrapping CUDA errors
//#
//# Copyright (C) 2013
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

#ifndef LOFAR_CUDASUPPORT_CUDAEXCEPTION_H
#define LOFAR_CUDASUPPORT_CUDAEXCEPTION_H

// \file
// Exception class for wrapping CUDA errors

//# Includes
#include <Common/Exception.h>
#include <driver_types.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // Exception class to translate a CUDA error into a %LOFAR exception.
    EXCEPTION_CLASS(CUDAException, Exception);

  } // namespace Cobalt

} // namespace LOFAR

// Macro to call a CUDA runtime function and throw a CUDAException if it fails.
#define CUDA_CALL(func)                                                 \
  do {                                                                  \
    cudaError_t err = func;                                             \
    if (err != cudaSuccess) {                                           \
      THROW (LOFAR::Cobalt::CUDAException,                              \
             #func << ": " << cudaGetErrorString(err));                 \
    }                                                                   \
  } while(0)

#endif

