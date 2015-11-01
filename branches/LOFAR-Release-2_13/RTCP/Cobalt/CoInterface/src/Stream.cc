//# Stream.cc
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <CoInterface/Stream.h>

#include <ctime>
#include <cstring>
#include <vector>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <CoInterface/Exceptions.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/PortBroker.h>
#include <Stream/NamedPipeStream.h>


using boost::format;
using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {

    uint16 storageBrokerPort(int observationID)
    {
      return 7000 + observationID % 1000;
    }


    string getStorageControlDescription(int observationID, int rank)
    {
      return str(format("[obs %d rank %d] control") % observationID % rank);
    }


    // The returned descriptor can be supplied to LCS/Stream StreamFactory.h
    string getStreamDescriptorBetweenIONandStorage(const Parset &parset, OutputType outputType, unsigned streamNr, const std::string &bind_local_iface)
    {
      string host = parset.getHostName(outputType, streamNr);
      uint16 port = storageBrokerPort(parset.settings.observationID);

      if (host == "")
        return str(format("file:%s") % parset.getFileName(outputType, streamNr));
      else
        return str(format("tcpbroker:%s:%u:ion-storage-obs-%u-type-%u-stream-%u:%s") % host % port % parset.settings.observationID % outputType % streamNr % bind_local_iface);
    }

  } // namespace Cobalt
} // namespace LOFAR

