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
    // This exception class can be used to translate a CUDA error into a
    // %LOFAR exception.
    class CUDAException : public Exception
    {
    public:
      CUDAException(const std::string &text, cudaError_t error,
                    const std::string &file = "", int line = 0,
                    const std::string &func = "", Backtrace *bt = 0);
			
      virtual ~CUDAException() throw();
      virtual const std::string &type() const;
    };

  } // namespace Cobalt

} // namespace LOFAR

// A handy macro that calls an arbitrary CUDA runtime function and throws a
// CUDAException if the call fails.
#define CUDA_CALL(...)                                                  \
  do {                                                                  \
    __VA_ARGS__;                                                        \
    cudaError_t err = cudaGetLastError();                               \
    if (err != cudaSuccess) {                                           \
      throw LOFAR::Cobalt::CUDAException(#__VA_ARGS__, err, THROW_ARGS); \
    }                                                                   \
  } while(0)


#endif

