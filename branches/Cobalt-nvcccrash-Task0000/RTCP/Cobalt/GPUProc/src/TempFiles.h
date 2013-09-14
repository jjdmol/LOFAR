//# TempFiles.h: classes to wrap temporary files and directories in C++
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
//# $Id: gpu_incl.h 24905 2013-05-15 00:05:11Z amesfoort $

#ifndef LOFAR_GPUPROC_CUDA_GPU_TEMPFILES_H
#define LOFAR_GPUPROC_CUDA_GPU_TEMPFILES_H

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <Common/SystemCallException.h>

namespace LOFAR {
  namespace Cobalt {
    class TempDir {
    public:
      TempDir( const std::string &parentDir ) {
        // construct the template
        const std::string tmpTemplate = parentDir + "/XXXXXX";

        // copy the template to a modifiable char buffer
        std::vector<char> buffer(tmpTemplate.size() + 1, 0);
        std::copy(tmpTemplate.begin(), tmpTemplate.end(), buffer.begin());

        // create the temp dir using the template
        if (mkdtemp(&buffer[0]) == NULL)
          THROW_SYSCALL("mkdtemp");

        // collect actual directory name
        dirName = &buffer[0];
      }

      ~TempDir() {
        // remove our temp dir
        //
        // TODO: Error checking
        rmdir(dirName.c_str());
      }

      std::string name() const {
        return dirName;
      }
    private:
      std::string dirName;
    };
  }
}

#endif

