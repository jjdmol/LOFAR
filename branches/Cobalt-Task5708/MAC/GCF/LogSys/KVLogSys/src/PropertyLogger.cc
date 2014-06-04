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

#include <lofar_config.h>

#include "PropertyLogger.h"
#include "KeyValueLoggerDaemon.h"
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <ManagerIdentifier.hxx>

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;   
using namespace PAL;   
using namespace TM;   
  namespace LogSys
  {
GCFDummyPort PropertyLogger::_dummyPort(0, "KVLD", F_PML_PROTOCOL);

PropertyLogger::PropertyLogger(KeyValueLoggerDaemon& daemon) :
  _daemon(daemon)
{
  vector<string> foundMACTypes;
  GCFPVSSInfo::getAllTypes("T*", foundMACTypes);
  for (vector<string>::iterator iter = foundMACTypes.begin();
       iter != foundMACTypes.end(); ++iter)
  {
    dpQuerySubscribeSingle("'*'", formatString("_DPT=\"%s\"", iter->c_str()));
  }
}

void PropertyLogger::propValueChanged(const string& propName, const GCFPValue& value)
{
  // manNumToSkip contains only PVSS API managers of the local system
  for (TManNumToSkip::iterator iter = _manNumToSkip.begin();
       iter != _manNumToSkip.end(); ++iter)
  {
    if ((iter->second == GCFPVSSInfo::getLastEventManNum()) && 
        (GCFPVSSInfo::getLastEventManType() == API_MAN) &&
        (GCFPVSSInfo::getLastEventSysId() == GCFPVSSInfo::getLocalSystemId()))
    {      
      // this property change may not be send to KeyValueLoggerMaster
      LOG_DEBUG(formatString(
          "API manager '%d' (of the local system) has requested to dump this update.",
          iter->second));
      return;
    }    
  }
  GCFPropValueEvent e(F_VCHANGEMSG);
  e.pValue = &value;
  e.pPropName = propName.c_str();
  e.internal = false;
  _daemon.dispatch(e, _dummyPort);
}

void PropertyLogger::skipUpdatesFrom(uint8 manNum, const TM::GCFPortInterface& p)
{
  _manNumToSkip[&p] = manNum;
}

void PropertyLogger::clientGone(const TM::GCFPortInterface& p)
{
  _manNumToSkip.erase(&p);
}
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
