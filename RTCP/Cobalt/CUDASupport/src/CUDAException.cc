//# CUDAException.cc: Exception class for wrapping CUDA errors
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

#include <lofar_config.h>

#include <CUDASupport/CUDAException.h>
#include <cuda_runtime_api.h>

namespace LOFAR
{
  namespace Cobalt
  {
    CUDAException::CUDAException(const std::string &text, cudaError_t error,
                                 const std::string &file, int line,
                                 const std::string &func, Backtrace *bt) :
      Exception(text + ": " + cudaGetErrorString(error),
                file, line, func, bt)
    {
    }

    CUDAException::~CUDAException() throw()
    {
    }

    const std::string& CUDAException::type() const
    {
      static const std::string theType("CUDAException");
      return theType;
    }

  } // namespace Cobalt

} // namespace LOFAR
