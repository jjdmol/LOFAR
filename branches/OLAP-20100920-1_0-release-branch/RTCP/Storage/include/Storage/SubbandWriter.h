//#  SubbandWriter.h: Write subband(s) in an AIPS++ Measurement Set
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

#ifndef LOFAR_STORAGE_SUBBANDWRITER_H
#define LOFAR_STORAGE_SUBBANDWRITER_H

// \file
// Write subband(s) in an AIPS++ Measurement Set

#include <Blob/KeyValueMap.h>
#include <Interface/Config.h>
#include <Common/Timer.h>
#include <Common/lofar_vector.h>
#include <Interface/Parset.h>
#include <Interface/ProcessingPlan.h>
#include <Storage/InputThread.h>
#include <Storage/OutputThread.h>
#include <Storage/MSWriter.h>
#include <Stream/Stream.h>


namespace LOFAR {
namespace RTCP {


class SubbandWriter
{
  public:
    SubbandWriter(const Parset &parset, unsigned subband, unsigned outputType, ProcessingPlan::planlet &outputConfig, bool isBigEndian);
    ~SubbandWriter();

  private:
    static const unsigned   maxReceiveQueueSize = 5;

    Queue<StreamableData *> itsReceiveQueue, itsFreeQueue;

    InputThread		    *itsInputThread;
    OutputThread	    *itsOutputThread;
};


} // namespace RTCP
} // namespace LOFAR

#endif
