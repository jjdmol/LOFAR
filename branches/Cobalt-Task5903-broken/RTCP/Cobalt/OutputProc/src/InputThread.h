//# InputThread.h
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

#ifndef LOFAR_RTCP_STORAGE_INPUT_THREAD_H
#define LOFAR_RTCP_STORAGE_INPUT_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <string>

#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>
#include <CoInterface/StreamableData.h>


namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * InputThread receives data blocks from a Stream,
     * and inserts them into a Pool<>, to be sent to
     * an OutputThread.
     *
     * The Stream is created from the parset through using
     * getStreamDescriptorBetweenIONandStorage.
     *
     * This class is designed to handle visibilities only.
     */
    class InputThread
    {
    public:
      InputThread(const Parset &parset,
                  unsigned streamNr,
                  Pool<StreamableData> &outputPool,
                  const std::string &logPrefix);

      virtual void process();

    private:
      const std::string itsLogPrefix, itsInputDescriptor;
      Pool<StreamableData> &itsOutputPool;
      const double itsDeadline;
    };
  } // namespace Cobalt
} // namespace LOFAR

#endif

