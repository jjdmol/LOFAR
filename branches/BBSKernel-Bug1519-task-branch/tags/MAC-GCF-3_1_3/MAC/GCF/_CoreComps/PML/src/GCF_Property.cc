//#  GCF_Property.cc: 
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

#include <GCF/GCF_Property.h>
#include <GCF/GCF_PropertySet.h>
#include <Utils.h>
#include <GPM_Defines.h>

GCFProperty::GCFProperty (string propName) :
   GCFPropertyBase(propName, 0),
   _isSubscribed(false),
   _isBusy(false)
{
  if (!Utils::isValidPropName(propName.c_str()))
  {
    LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
        "Property %s meets not the name convention! Set to \"\"",
        propName.c_str()));
  }
}

GCFProperty::GCFProperty (string propName, 
                          GCFPropertySet& propertySet) :
   GCFPropertyBase(propName, &propertySet),
   _isSubscribed(false),
   _isBusy(false)
{
  if (!Utils::isValidPropName(propName.c_str()))
  {
    LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
        "Property %s meets not the name convention! Set to \"\"",
        propName.c_str()));
  }
}

TGCFResult GCFProperty::subscribe ()
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
    result = GCFPropertyBase::subscribe();
    if (result == GCF_NO_ERROR)
    {
      _isBusy = true;
    }
  }
  return result;
}

TGCFResult GCFProperty::unsubscribe ()
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
    result = GCFPropertyBase::unsubscribe();
    _isSubscribed = false;
  }
  
  return result;
}

void GCFProperty::subscribed ()
{
  assert(!_isSubscribed);
  assert(_isBusy);
  
  _isSubscribed = true;
  _isBusy = false;
  GCFPropertyBase::subscribed();
}

bool GCFProperty::exists ()
{
  bool existsInDB(true);
  
  existsInDB = GCFPropertyBase::exists();

  if (!existsInDB) _isSubscribed = _isBusy = false;
    
  return existsInDB;
}
