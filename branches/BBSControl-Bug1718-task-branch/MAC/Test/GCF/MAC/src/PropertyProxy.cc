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
#include "SupervisedTask.h"

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;
  namespace Test
  {
void PropertyProxy::propSubscribed(const string& propName)
{
  _task.propSubscribed(propName);
}

void PropertyProxy::propUnsubscribed(const string& propName)
{
  _task.propUnsubscribed(propName);
}

void PropertyProxy::propValueGet(const string& propName, const GCFPValue& value)
{
  _task.valueGet(propName, value);
}

void PropertyProxy::propValueChanged(const string& propName, const GCFPValue& value)
{
  _task.propValueChanged(propName, value);
}

void PropertyProxy::propValueSet(const string& propName)
{
  _task.propValueSet(propName);
}
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
