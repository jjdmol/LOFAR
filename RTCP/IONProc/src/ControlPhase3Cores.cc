//#  BeamletBufferToComputeNode.cc: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <ControlPhase3Cores.h>
#include <Interface/CN_Command.h>

#include <boost/format.hpp>
using boost::format;


namespace LOFAR {
namespace RTCP {


ControlPhase3Cores::ControlPhase3Cores(const Parset *ps, const std::vector<Stream *> &phaseThreeStreams )
:
  itsLogPrefix(str(format("[obs %u] ") % ps->observationID())),
  itsPhaseThreeStreams(phaseThreeStreams),
  itsNrBeamsPerPset(ps->nrBeamsPerPset()),
  itsThread(0)
{
  if (!itsPhaseThreeStreams.empty() && ps->phaseThreeDisjunct()) {
    // psets dedicated to phase 3 have a different schedule -- they iterate over
    // beams instead of subbands, and never need station data as input

    // Psets with both phase 2 and phase 3 either use different cores for both phases, or
    // have phase 2 automatically transition into phase 3 for all cores.

    // If different cores are used, both sets need to be sent a PROCESS command. Also,
    // this command must be sent AFTER phase 2 cores are activated, because communication
    // with the compute cores is synchronous, but cores for phase 3 might not be ready yet
    // even though phase 2 cores can already be started.
    
    // If the same cores are used for phases 2 and 3, only the cores in phase 2 need to be
    // sent a PROCESS command, which is done in BeamletBufferToComputeNode.cc.
    itsThread = new Thread(this, &ControlPhase3Cores::mainLoop, "[ControlPhase3Cores] ", 65536);
  }  
}


ControlPhase3Cores::~ControlPhase3Cores()
{
  LOG_DEBUG_STR(itsLogPrefix << "ControlPhase3Cores::~ControlPhase3Cores");

  itsNrIterationsToDo.noMore();

  delete itsThread;
}


void ControlPhase3Cores::addIterations( unsigned count )
{
  itsNrIterationsToDo.up( count );
}


void ControlPhase3Cores::mainLoop()
{
  unsigned block = 0;
  unsigned currentPhaseThreeComputeCore = 0;
  const unsigned nrPhaseThreeComputeCores = itsPhaseThreeStreams.size();

  while (itsNrIterationsToDo.down()) {
    CN_Command command(CN_Command::PROCESS, block ++);

    for (unsigned beam = 0; beam < itsNrBeamsPerPset; beam ++) {
      Stream *stream = itsPhaseThreeStreams[currentPhaseThreeComputeCore];

      // tell CN to process data
      command.write(stream);

      if (++ currentPhaseThreeComputeCore == nrPhaseThreeComputeCores)
        currentPhaseThreeComputeCore = 0;
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
