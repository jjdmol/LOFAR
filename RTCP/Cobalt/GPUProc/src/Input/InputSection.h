/* InputSection.h: Catch RSP ethernet frames and synchronize RSP inputs
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_GPUPROC_INPUT_SECTION_H
#define LOFAR_GPUPROC_INPUT_SECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <string>
#include <vector>

#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>

#include "BeamletBuffer.h"
#include "InputThread.h"
#include "LogThread.h"

namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class InputSection
    {
    public:
      InputSection(const Parset &, const std::vector<Parset::StationRSPpair> &inputs);
      ~InputSection();

      std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > itsBeamletBuffers;

    private:
      void                                                createInputStreams(const Parset &, const std::vector<Parset::StationRSPpair> &inputs);
      void                                                createInputThreads(const Parset &, const std::vector<Parset::StationRSPpair> &inputs);

      std::string itsLogPrefix;

      std::vector<SmartPtr<Stream > >                     itsInputStreams;

      unsigned itsNrRSPboards;

      SmartPtr<LogThread>                                 itsLogThread;
      std::vector<SmartPtr<InputThread<SAMPLE_TYPE> > >   itsInputThreads;
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

