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

#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/Utils.h>
#include <GPM_Defines.h>

GCFExtProperty::GCFExtProperty (const TPropertyInfo& propInfo) :
   GCFProperty(propInfo, 0),
   _isSubscribed(false)
{
}

GCFExtProperty::GCFExtProperty (const TPropertyInfo& propInfo, 
                          GCFExtPropertySet& propertySet) :
   GCFProperty(propInfo, &propertySet),
   _isSubscribed(false)
{
}

TGCFResult GCFExtProperty::subscribe ()
{  
  TGCFResult result(GCF_NO_ERROR);
  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (_isSubscribed)
  {
    result = GCF_ALREADY_SUBSCRIBED;
  }
  else
  {
    result = GCFProperty::subscribe();
    if (result == GCF_NO_ERROR)
    {
      _isBusy = true;
    }
  }
  return result;
}

TGCFResult GCFExtProperty::unsubscribe ()
{
  TGCFResult result(GCF_NO_ERROR);
  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (!_isSubscribed)
  {
    result = GCF_NOT_SUBSCRIBED;
  }
  else
  {
    result = GCFProperty::unsubscribe();
    _isSubscribed = false;
  }
  
  return result;
}

void GCFExtProperty::subscribed ()
{
  assert(!_isSubscribed);
  assert(_isBusy);
  
  _isSubscribed = true;
  _isBusy = false;
  GCFProperty::subscribed();
}

void GCFExtProperty::subscriptionLost () 
{
  assert(_isSubscribed);
  _isSubscribed = false;
}

bool GCFExtProperty::exists ()
{
  bool existsInDB(true);
  
  existsInDB = GCFProperty::exists();

  if (!existsInDB) _isSubscribed = _isBusy = false;
    
  return existsInDB;
}

