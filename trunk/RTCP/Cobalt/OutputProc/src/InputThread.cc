//# InputThread.cc
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

#include "InputThread.h"

#include <Common/Timer.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <CoInterface/Stream.h>


namespace LOFAR
{
  namespace Cobalt
  {
    InputThread::InputThread(const Parset &parset,
                             unsigned streamNr, Pool<StreamableData> &outputPool,
                             const std::string &logPrefix)
      :
      itsLogPrefix(logPrefix + "[InputThread] "),
      itsInputDescriptor(getStreamDescriptorBetweenIONandStorage(parset, CORRELATED_DATA, streamNr)),
      itsOutputPool(outputPool),
      itsDeadline(parset.realTime() ? parset.stopTime() : 0)
    {
    }


    void InputThread::process()
    {
      try {
        LOG_INFO_STR(itsLogPrefix << "Creating connection from " << itsInputDescriptor << "..." );
        SmartPtr<Stream> streamFromION(createStream(itsInputDescriptor, true, itsDeadline));
        LOG_INFO_STR(itsLogPrefix << "Creating connection from " << itsInputDescriptor << ": done" );

        for(SmartPtr<StreamableData> data; (data = itsOutputPool.free.remove()) != NULL; itsOutputPool.filled.append(data)) {
          data->read(streamFromION, true, 1); // Cobalt writes with an alignment of 1

          LOG_DEBUG_STR(itsLogPrefix << "Read block with seqno = " << data->sequenceNumber());
        }
      } catch (SocketStream::TimeOutException &) {
        LOG_WARN_STR(itsLogPrefix << "Connection from " << itsInputDescriptor << " timed out");
      } catch (Stream::EndOfStreamException &) {
        LOG_INFO_STR(itsLogPrefix << "Connection from " << itsInputDescriptor << " closed");
      } catch (SystemCallException &ex) {
        LOG_WARN_STR(itsLogPrefix << "Connection from " << itsInputDescriptor << " failed: " << ex.text());
      }

      // Append end-of-stream marker
      itsOutputPool.filled.append(NULL);
    }
  } // namespace Cobalt
} // namespace LOFAR

