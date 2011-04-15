//#  SubbandWriter.cc: Writes visibilities in an AIPS++ measurement set
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/ParameterSet.h>
#include <Common/SystemCallException.h>
#include <Interface/Exceptions.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Storage/SubbandWriter.h>

#include <time.h>

#include <boost/format.hpp>
using boost::format;

namespace LOFAR {
namespace RTCP {


SubbandWriter::SubbandWriter(const Parset &parset, const ProcessingPlan::planlet &outputConfig, unsigned index, const string &host, const string &dir, const string &filename, bool isBigEndian)
{
  const std::string logPrefix = str(format("[obs %u output %u index %u] ") % parset.observationID() % outputConfig.outputNr % index);
  
  StreamableData	  *dataTemplate = outputConfig.source;

  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    StreamableData *data = dataTemplate->clone();

    data->allocate();
    itsFreeQueue.append(data);
  }

  itsInputThread  = new InputThread(parset, outputConfig, index, host, filename, itsFreeQueue, itsReceiveQueue);
  itsOutputThread = new OutputThread(parset, outputConfig, index, dir, filename, itsFreeQueue, itsReceiveQueue, isBigEndian);
}


SubbandWriter::~SubbandWriter()
{
  delete itsInputThread;
  delete itsOutputThread;

  while (!itsReceiveQueue.empty())
    delete itsReceiveQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}

} // namespace RTCP
} // namespace LOFAR
