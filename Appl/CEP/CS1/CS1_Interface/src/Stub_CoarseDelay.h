//# Stub_Delay.h: Stub for connection of delay control with RSP inputs
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_STUB_COARSE_DELAY_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_STUB_COARSE_DELAY_H

#include <APS/ParameterSet.h>
#include <tinyCEP/TinyDataManager.h>

namespace LOFAR {

class TH_Socket;
class Connection;

class Stub_CoarseDelay
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_CoarseDelay(bool isInput, const ACC::APS::ParameterSet &pSet);

  ~Stub_CoarseDelay();

  // Connect the given objects to the stubs.
  void connect(int RSP_nr, TinyDataManager &dm, int dhNr);

private:
  bool			       itsIsInput;  // Is this stub an input for a step
  const ACC::APS::ParameterSet &itsPS;
  int			       itsNRSP;     // total number of RSPinputs
  TH_Socket		       **itsTHs;
  Connection		       **itsConnections;
};

} //namespace

#endif // include guard
