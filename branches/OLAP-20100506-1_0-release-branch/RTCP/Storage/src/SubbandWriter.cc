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
#include <Storage/MeasurementSetFormat.h>

#include <boost/lexical_cast.hpp>


#include <time.h>
#include <sys/stat.h>

namespace LOFAR {
namespace RTCP {


SubbandWriter::SubbandWriter(const Parset &parset, unsigned subband, unsigned outputType)
{
  CN_Configuration configuration(parset);
  CN_ProcessingPlan<> plan(configuration);
  plan.removeNonOutputs();

#if defined HAVE_AIPSPP
  if (outputType == 0) {
    // create root directory of the observation tree
    if (mkdir(parset.getMSBaseDir().c_str(), 0777) != 0 && errno != EEXIST) {
      unsigned savedErrno = errno; // first argument below clears errno
      throw SystemCallException(("mkdir " + parset.getMSBaseDir()).c_str(), savedErrno, THROW_ARGS);
    }
  }

  if (outputType == 0 && parset.outputCorrelatedData()) {
    MeasurementSetFormat myFormat(&parset, 512);
          
    /// Make MeasurementSet filestructures and required tables
    myFormat.addSubband(subband);

    LOG_INFO_STR("MeasurementSet created");
  }
#endif // defined HAVE_AIPSPP

  ProcessingPlan::planlet &outputConfig = plan.plan[outputType];
  StreamableData	  *dataTemplate = outputConfig.source;

  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    StreamableData *data = dataTemplate->clone();

    data->allocate();
    itsFreeQueue.append(data);
  }

  itsInputThread  = new InputThread(parset, subband, outputType, itsFreeQueue, itsReceiveQueue);
  itsOutputThread = new OutputThread(parset, subband, outputType, outputConfig, itsFreeQueue, itsReceiveQueue);
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
