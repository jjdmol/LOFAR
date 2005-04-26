//#  VBFuncStateProperty.cc: Implementation of the Virtual VBFuncStateProperty task
//#
//#  Copyright (C) 2002-2004
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

#include "VBFuncStateProperty.h"
#include "VBQualityGuard.h"

namespace LOFAR
{

using namespace GCF::Common;
using namespace GCF::PAL;
  
  namespace AVB
  {
    
INIT_TRACER_CONTEXT(VBFuncStateProperty, LOFARLOGGER_PACKAGE);

     
VBFuncStateProperty::VBFuncStateProperty (const string& propName, VBQAnswer& answer) :
  GCFExtProperty(TPropertyInfo(propName.c_str(), LPT_CHAR)),
  _value(100)
{
  setAnswer(&answer);
}

void VBFuncStateProperty::startMonitoring()
{
  if (requestValue() != GCF_NO_ERROR)
  {
    GCFPVChar newValue(0);
    GCFProperty::valueGet(newValue);
    _value.setValue(0);
  }
}

void VBFuncStateProperty::valueChanged (const GCFPValue& newValue)
{
  GCFProperty::valueChanged(newValue);
  _value.copy(newValue);
}

void VBFuncStateProperty::valueGet (const GCFPValue& newValue)
{
  _value.copy(newValue);
  if (subscribe() != GCF_NO_ERROR)
  {
    GCFPVChar newValue(0);
    GCFProperty::valueGet(newValue);
    _value.setValue(0);
  }
}

void VBFuncStateProperty::subscriptionLost ()
{
  GCFPropAnswerEvent e(F_UNSUBSCRIBED);
  string fullName(getFullName());
  e.pPropName = fullName.c_str();
  dispatchAnswer(e);
}
  } // namespace AVB
} // namespace LOFAR
