//#  PropertyProxy.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include "PropertyProxy.h"
#include "MISSession.h"
#include <GCF/PAL/GCF_Answer.h>

namespace LOFAR
{
using namespace GCF::Common;
using namespace GCF::TM;
using namespace GCF::PAL;
 namespace AMI
 {

GCFDummyPort PropertyProxy::_dummyPort(0, "AMI", F_PML_PROTOCOL);

void PropertyProxy::propValueGet(const string& propName, const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _session.dispatch(e, _dummyPort);
}

 } // namespace AMI
} // namespace LOFAR
