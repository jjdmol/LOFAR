//  Simulator.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
/////////////////////////////////////////////////////////////////////////////

#include "CEPFrame/Simulator.h"
#include "CEPFrame/SimulatorParseClass.h"
#include TRANSPORTERINCLUDE

namespace LOFAR
{

Simulator::Simulator()
: itsSimul(),
  itsArgc (0),
  itsArgv (0),
  itsPreDone (true),
  itsPostDone(true)
{}

Simulator::~Simulator()
{}

void Simulator::setarg (int argc, const char** argv)
{
  itsArgc = argc;
  itsArgv = argv;
}

void Simulator::getarg (int* argc, const char** argv[])
{
  *argc = itsArgc;
  *argv = itsArgv;
}

void Simulator::baseDefine (const KeyValueMap& params)
{
  // Initialize MPI environment.
  TRANSPORTER::init (itsArgc, itsArgv);
  // Set current application number if defined in parameters.
  KeyValueMap::const_iterator iter = params.find ("appl");
  if (iter != params.end()) {
    if (iter->second.dataType() == KeyValue::DTInt) {
      Step::setCurAppl (iter->second.getInt());
    }
  }
  // Let derived class define the simulation.
  define (params);
  itsPreDone = false;
  /////  itsSimul.shortcutConnections();
  /////  itsSimul.simplifyConnections();
}

void Simulator::baseCheck()
{
  itsSimul.checkConnections();
  check();
}

void Simulator::basePrerun()
{
  prerun();
  itsPreDone = true;
  itsPostDone = false;
}

void Simulator::baseRun (int nsteps)
{
  if (!itsPreDone) {
    basePrerun();
  }
  run (nsteps);
}

void Simulator::basePostrun()
{
  if (!itsPreDone) {
    basePrerun();
  }
  postrun();
  itsPostDone = true;
  itsPreDone = false;
}

void Simulator::baseDump()
{
  if (!itsPreDone) {
    basePrerun();
  }
  dump();
}

void Simulator::baseDHFile (const string& dh, const string& name)
{
  if (!itsPreDone) {
    basePrerun();
  }
  itsSimul.setDHFile (dh, name);
}

void Simulator::baseQuit()
{
  if (!itsPostDone) {
    basePostrun();
  }
  quit();
  // Close environment.
  TRANSPORTER::finalize();
}

void Simulator::check()
{}

void Simulator::prerun()
{
  itsSimul.preprocess();
}

void Simulator::dump() const
{}

void Simulator::postrun()
{
  itsSimul.postprocess();
}

void Simulator::quit()
{}

}
