//
//  WorkBlock.cc: 
//
//  Copyright (C) 2000-2004
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

#include <tinyCEP/WorkBlock.h>

namespace LOFAR
{

WorkBlock::WorkBlock (int inputs, int outputs, 
		      const string& name)
  : itsName     (name),
    itsNinputs  (inputs),
    itsNoutputs (outputs) {
}


WorkBlock::WorkBlock (const WorkBlock& that) 
  : itsName     (that.itsName),
    itsNinputs  (that.itsNinputs),
    itsNoutputs (that.itsNoutputs) {
}

WorkBlock::~WorkBlock() {
}

void WorkBlock::preprocess() {
}

void WorkBlock::process() {
}

void WorkBlock::postprocess() {
}

void WorkBlock::dump() {
}

} // namespace
