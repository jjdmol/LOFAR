//#  GCF_ExtProperty.cc: 
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

#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/Utils.h>
#include <GPM_Defines.h>

namespace LOFAR {
 namespace GCF {
  using namespace Common;
  namespace PAL {
   
//
// GCFExtProperty(propInfo)
//
GCFExtProperty::GCFExtProperty (const TPropertyInfo& propInfo) :
   GCFProperty(propInfo, 0),
   _isSubscribed(false)
{
}

//
// GCFExtProperty(propInfo, propSet)
//
GCFExtProperty::GCFExtProperty (const TPropertyInfo& propInfo, 
                          GCFExtPropertySet& propertySet) :
   GCFProperty(propInfo, &propertySet),
   _isSubscribed(false)
{
}

//
// subscribe()
//
TGCFResult GCFExtProperty::subscribe ()
{  
	if (_isBusy) {
		return (GCF_BUSY);
	}

	if (_isSubscribed) {
		return (GCF_ALREADY_SUBSCRIBED);
	}

	TGCFResult result(GCFProperty::subscribe());
	if (result == GCF_NO_ERROR) {
		_isBusy = true;
	}

	return result;
}

//
// unsubscribe()
//
TGCFResult GCFExtProperty::unsubscribe ()
{
	if (_isBusy) {
		return (GCF_BUSY);
	}

	if (!_isSubscribed) {
		return (GCF_NOT_SUBSCRIBED);
	}

	TGCFResult result(GCFProperty::unsubscribe());
	_isSubscribed = false;

	return result;
}

//
// subscribed()
//
void GCFExtProperty::subscribed()
{
	ASSERT(!_isSubscribed);
	ASSERT(_isBusy);

	_isSubscribed = true;
	_isBusy = false;
	GCFProperty::subscribed();
}

//
// subscriptionLost()
//
void GCFExtProperty::subscriptionLost() 
{
	ASSERT(_isSubscribed);
	_isSubscribed = false;
}

//
// exists()
//
bool GCFExtProperty::exists()
{
	if (GCFProperty::exists()) {
		return true;
	}

	_isSubscribed = false;
	_isBusy 	  = false;

	return false;
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

