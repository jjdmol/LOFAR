//#  GPA_PropertySet.cc:
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

#include "GPA_PropertySet.h"
#include "GPA_PropSetSession.h"
#include <GCF/GCF_PVString.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace TM;
using namespace Common;
  namespace PAL 
  {

const char* PS_CAT_NAMES[] =
{
  "temporary",
  "permanent",
  "perm-autoloaded",
  "temp-autoloaded",
};

//
// enable (psname, pstype, category)
//
TPAResult GPAPropertySet::enable(string& name, string& type, Common::TPSCategory category)
{
	_name = name;
	_type = type;
	_category = category;

	if (!GCFPVSSInfo::typeExists(_type)) {
		LOG_INFO(formatString ( "Type %s is not knwon in the PVSS DB!", _type.c_str()));
		return(PA_DPTYPE_UNKNOWN);
	}

	if (GCFPVSSInfo::propExists(_name) && PS_IS_TEMPORARY(_category)) {
		LOG_INFO("DP for temporary prop. set may not already exists!");
		return (PA_PROP_SET_ALREADY_EXISTS);
	}

	if (!GCFPVSSInfo::propExists(_name) && !PS_IS_TEMPORARY(_category)) {
		LOG_INFO("DP for permanent prop. set must already exists");
		return(PA_PROP_SET_NOT_EXISTS);
	}

	if (GCFPVSSInfo::propExists(_name + PS_ENABLED_EXT)) {
		LOG_ERROR("Framework DP, for enabled PS, may not already exists before enabling it!");
		return(PA_PROP_SET_ALREADY_EXISTS);
	}

	TSAResult saResult(dpCreate(_name+string(PS_ENABLED_EXT), string("GCFPaPsEnabled")));
	if (saResult == SA_NO_ERROR) {
		return (PA_NO_ERROR);
	}

	if (saResult == SA_DPTYPE_UNKNOWN) {
		LOG_FATAL("Please check the existens of dpType 'GCFPaPsEnabled' in PVSS DB!!!");
	}

	return (PA_INTERNAL_ERROR);
}

TPAResult GPAPropertySet::disable(bool& mustWait)
{
  TPAResult paResult(PA_NO_ERROR);

  _counter = 0;
  GCFPVString indication("d|" + _name);
  dpeSet("__pa_PSIndication", indication, 0.0, false);
  if (GCFPVSSInfo::propExists(_name + PS_ENABLED_EXT)) {
    LOG_DEBUG(formatString("DP %s%s must be removed!", _name.c_str(), PS_ENABLED_EXT.c_str()));
    if (dpDelete(_name + PS_ENABLED_EXT) != SA_NO_ERROR) {
      paResult = PA_INTERNAL_ERROR;
    }
    else {
      _counter += 1;
    }
  }
  if (PS_IS_TEMPORARY(_category)) {
    if (GCFPVSSInfo::propExists(_name)) {
      LOG_DEBUG(formatString("DP %s still exists! Will be removed too!", _name.c_str()));
      if (dpDelete(_name) != SA_NO_ERROR) {
        paResult = PA_INTERNAL_ERROR;
      }          
      else {
        _counter += 1;
      }
    }
  }
  
  mustWait = (_counter > 0);
  
  return paResult;
}

TPAResult GPAPropertySet::load(bool& mustWait)
{
  TPAResult paResult(PA_NO_ERROR);
  mustWait = false;
  if (PS_IS_TEMPORARY(_category))
  {
    LOG_DEBUG("Prop. set must be created");
    if (dpCreate(_name, _type) != SA_NO_ERROR)
    {
      paResult = PA_INTERNAL_ERROR;        
    } 
    else
    {
      mustWait = true;      
    }                
  }
  
  return paResult;
}

TPAResult GPAPropertySet::unload(bool& mustWait)
{
  TPAResult paResult(PA_NO_ERROR);
  mustWait = false;
  _counter = 0;
  if (PS_IS_TEMPORARY(_category))
  {
    LOG_DEBUG("Must delete related DP.");
    if (dpDelete(_name) != SA_NO_ERROR)
    {
      paResult = PA_INTERNAL_ERROR;        
    } 
    else
    {
      _counter += 1;
      mustWait = true;      
    }                
  }
  
  return paResult;
}

void GPAPropertySet::dpCreated(const string& dpName)
{
  if (dpName.find(_name + PS_ENABLED_EXT) < dpName.length())
  {
    string enabledDPContent = PS_CAT_NAMES[_category];
    enabledDPContent += '|';
    enabledDPContent += _type;
    
    GCFPVString pvEnabledDPContent(enabledDPContent);
    
    dpeSet(_name + PS_ENABLED_EXT, pvEnabledDPContent, 0.0, false);
    
    GCFPVString indication("e|" + _name);
    dpeSet("__pa_PSIndication", indication, 0.0, false);
  }
  GCFEvent e(PA_PROP_SET_DP_CREATED);
  _session.dispatch(e);
}

void GPAPropertySet::dpDeleted(const string& /*dpName*/)
{
  _counter--;
  if (_counter == 0)
  {
    GCFEvent e(PA_PROP_SET_DP_DELETED);
    _session.dispatch(e);
  }
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
