//# CompileDefinitions.h
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

#ifndef LOFAR_GPUPROC_KERNEL_COMPILE_DEFINITIONS_H
#define LOFAR_GPUPROC_KERNEL_COMPILE_DEFINITIONS_H

#include <iosfwd>
#include <map>
#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    class CompileDefinitions
    {
    public:
      // Return a reference to the value of the element \a key.
      std::string& operator[](const std::string& key);
    private:
      // Print compile definitions in a usuable form to output stream \a os.
      friend std::ostream&
      operator<<(std::ostream& os, const CompileDefinitions& defs);

      // Store compile definitions as key/value pairs.
      std::map<std::string, std::string> defs;
    };
  }
}

#endif
