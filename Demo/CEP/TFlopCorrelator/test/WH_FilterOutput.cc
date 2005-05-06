//#  WH_FilterOutput.cc: Output workholder for filter tester
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <WH_FilterOutput.h>

using namespace LOFAR;

WH_FilterOutput::WH_FilterOutput(const string& name) {
}

WH_FilterOutput::~WH_FilterOutput() {
}

WorkHolder* WH_FilterOutput::construct(const string& name) {
  return new WH_FilterOutput(name);
}

WH_FilterOutput* WH_FilterOutput::make(const string& name) {
  return new WH_FilterOutput(name);
}

void WH_FilterOutput::preprocess() {
}

void WH_FilterOutput::process() {
}

void WH_FilterOutput::dump() {
}
