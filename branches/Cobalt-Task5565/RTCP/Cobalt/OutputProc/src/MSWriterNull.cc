//# MSWriterNull.cc: a null MSWriter
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "MSWriterNull.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {


    MSWriterNull::MSWriterNull ()
    {
    }


    MSWriterNull::~MSWriterNull()
    {
    }


    void MSWriterNull::write(StreamableData *)
    {
      // We do not know why the creation of the propper writer failed.
      // Assume nothing and only report that we did not write anything
      itsConfiguration.replace("percentageWritten", str(format("%u") % 0));
    }

    void MSWriterNull::augment(const FinalMetaData &finalMetaData)
    {
      (void)finalMetaData;  // mirror implementation in MSWriter.cc
    }
  } // namespace Cobalt
} // namespace LOFAR

