//#  WH_FilterInput.cc: Input workholder for filter tester
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

#include <WH_FilterInput.h>

using namespace LOFAR;

WH_FilterInput::WH_FilterInput(const string& name) {
}

WH_FilterInput::~WH_FilterInput() {
}

WorkHolder* WH_FilterInput::construct(const string& name) {
  return new WH_FilterInput(name);
}

WH_FilterInput* WH_FilterInput::make(const string& name) {
  return new WH_FilterInput(name);
}

void WH_FilterInput::preprocess() {
}

void WH_FilterInput::process() {
}

void WH_FilterInput::dump() {
}


