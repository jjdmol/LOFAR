//  AppController.cc:
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

#include "AppController.h"
#include "BaseSim.h"
#include "SimulatorParseClass.h"
#include TRANSPORTERINCLUDE

namespace LOFAR
{

AppController::AppController()
: itsArgc (0),
  itsArgv (0),
  itsPreDone (true),
  itsPostDone(true)
{}

AppController::~AppController()
{}

void AppController::setarg (int argc, const char** argv)
{
  itsArgc = argc;
  itsArgv = argv;
}

void AppController::getarg (int* argc, const char** argv[])
{
  *argc = itsArgc;
  *argv = itsArgv;
}

void AppController::baseDefine (const KeyValueMap& params)
{
  // Initialize MPI environment.
  TRANSPORTER::init (itsArgc, itsArgv);
  // Set current application number if defined in parameters.
  KeyValueMap::const_iterator iter = params.find ("appl");
  if (iter != params.end()) {
    if (iter->second.dataType() == KeyValue::DTInt) {
      //      Step::setCurAppl (iter->second.getInt());
    }
  }
  // Let derived class define the simulation.
  define (params);
  itsPreDone = false;
  /////  itsSimul.shortcutConnections();
  /////  itsSimul.simplifyConnections();
}

void AppController::baseCheck()
{
//   itsSimul.checkConnections();
  check();
}

void AppController::basePrerun()
{
  prerun();
  itsPreDone = true;
  itsPostDone = false;
}

void AppController::baseRun (int nsteps)
{
  if (!itsPreDone) {
    basePrerun();
  }
  run (nsteps);
}

void AppController::basePostrun()
{
  if (!itsPreDone) {
    basePrerun();
  }
  postrun();
  itsPostDone = true;
  itsPreDone = false;
}

void AppController::baseDump()
{
  if (!itsPreDone) {
    basePrerun();
  }
  dump();
}

void AppController::baseDHFile (const string& dh, const string& name)
{
  if (!itsPreDone) {
    basePrerun();
  }
 //  itsSimul.setDHFile (dh, name);
}

void AppController::baseQuit()
{
  if (!itsPostDone) {
    basePostrun();
  }
  quit();
  // Close environment.
  TRANSPORTER::finalize();
}

void AppController::define(const KeyValueMap& map){
}

void AppController::check()
{}

void AppController::prerun()
{
//   itsSimul.preprocess();
}

void AppController::run(int iter){
}

void AppController::dump() const
{}

void AppController::postrun()
{
//   itsSimul.postprocess();
}

void AppController::quit()
{}
}
