//  ApplicationHolder.cc:
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

#include <lofar_config.h>

#include <CEPFrame/ApplicationHolder.h>
#include <tinyCEP/SimulatorParseClass.h>
#include TRANSPORTERINCLUDE

namespace LOFAR
{

ApplicationHolder::ApplicationHolder()
  : TinyApplicationHolder(),
    itsComposite(),
    itsPreDone (true),
    itsPostDone(true)
{}

ApplicationHolder::~ApplicationHolder()
{}

void ApplicationHolder::baseDefine (const KeyValueMap& params)
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
}

void ApplicationHolder::baseCheck()
{
  check();
}

void ApplicationHolder::basePrerun()
{
  prerun();
  itsPreDone = true;
  itsPostDone = false;
}

void ApplicationHolder::baseRun (int nsteps)
{
  if (!itsPreDone) {
    basePrerun();
  }
  run (nsteps);
}

void ApplicationHolder::basePostrun()
{
  if (!itsPreDone) {
    basePrerun();
  }
  postrun();
  itsPostDone = true;
  itsPreDone = false;
}

void ApplicationHolder::baseDump()
{
  if (!itsPreDone) {
    basePrerun();
  }
  dump();
}

void ApplicationHolder::baseQuit()
{
  if (!itsPostDone) {
    basePostrun();
  }
  quit();
  // Close environment.
  TRANSPORTER::finalize();
}

void ApplicationHolder::check()
{}

void ApplicationHolder::prerun()
{
  itsComposite.preprocess();
}

void ApplicationHolder::dump() const
{}

void ApplicationHolder::postrun()
{
  itsComposite.postprocess();
}

void ApplicationHolder::quit()
{}

}
