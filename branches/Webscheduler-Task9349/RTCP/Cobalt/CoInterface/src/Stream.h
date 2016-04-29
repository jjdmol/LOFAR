//# Stream.h: functions to construct streams between ION/CN/Storage
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

#ifndef LOFAR_INTERFACE_STREAM_H
#define LOFAR_INTERFACE_STREAM_H

#include <string>

#include <CoInterface/OutputTypes.h>
#include <CoInterface/Parset.h>
#include <Stream/Stream.h>

namespace LOFAR
{
  namespace Cobalt
  {

    uint16 storageBrokerPort(int observationID);
    std::string getStorageControlDescription(int observationID);

    // The returned descriptor can be supplied to LCS/Stream StreamFactory.h
    std::string getStreamDescriptorBetweenIONandStorage(const Parset &parset, OutputType outputType, unsigned streamNr, const std::string &bind_local_iface = "");

  } // namespace Cobalt
} // namespace LOFAR

#endif

