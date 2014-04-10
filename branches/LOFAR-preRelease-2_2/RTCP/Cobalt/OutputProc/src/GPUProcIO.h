//# GPUProcIO.h
//# Copyright (C) 2009-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_RTCP_STORAGE_GPUPROCIO_H
#define LOFAR_RTCP_STORAGE_GPUPROCIO_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <string>
#include <vector>

#include <Stream/Stream.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/FinalMetaData.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // Receive and process a full observation, being rank 'myRank'. Will:
    //   * Receive a Parset over the controlStream
    //   * Fulfill roles for parset.settings.outputProcHosts[myRank]:
    //       - Start SubbandWriters/TABWriters
    //       - Receive input and write output for all of them
    //   * Call readFinalMetaData to obtain the final metadata from GPUProc,
    //     and send it to all writers.
    //   * Call writeFeedbackLTA to obtain the LTA feedback from all writers,
    //     and write it to GPUProc.
    // \return \c true upon success, \c false upon failure.
    bool process(Stream &controlStream, size_t myRank);

  } // namespace Cobalt
} // namespace LOFAR

#endif

