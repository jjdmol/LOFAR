//#  GSA_SysConnGuard.cc:
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
//#  MERCHANTABILITY or FITNESS FOR A PMRTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include "GSA_SysConnGuard.h"
#include <GCF/PAL/GCF_SysConnGuard.h>
#include <GSA_Defines.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PVString.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;
  namespace PAL
  {

GSASysConnGuard::~GSASysConnGuard()
{
  dpeUnsubscribe("__gcf_wd.sys");
}

void GSASysConnGuard::start ()
{
  ASSERT(!_isSubscribed);
  if (dpeSubscribe("__gcf_wd.sys") != SA_NO_ERROR) {
    LOG_ERROR("Subscription is necessary for the System Connection Guard service."
              " Please check the existens of the DP __gcf_wd");
  }
}

void GSASysConnGuard::stop ()
{
  dpeUnsubscribe("__gcf_wd.sys");
}

void GSASysConnGuard::dpeSubscribed(const string& dpName)
{
  if (dpName.find("__gcf_wd.sys") < dpName.length()) {
    _isSubscribed = true;
  }
}

void GSASysConnGuard::dpeSubscriptionLost(const string& /*dpName*/)
{  
  _isSubscribed = false;
  _sysConnGuard.serviceEvent("ALL", true);
}

void GSASysConnGuard::dpeValueChanged(const string& /*dpName*/, const GCFPValue& value)
{
  GCFPVString* pValue = (GCFPVString*) &value;
  string sysIDStr;
  sysIDStr.assign(pValue->getValue(), 1, pValue->getValue().length() - 2);
  unsigned int sysID = atoi(sysIDStr.c_str());
  switch (pValue->getValue()[0]) {
    case 'd':
      _sysConnGuard.serviceEvent(GCFPVSSInfo::getSystemName(sysID), true);
      break;
    case 'c':
      _sysConnGuard.serviceEvent(GCFPVSSInfo::getSystemName(sysID), false);
      break;
  }
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
