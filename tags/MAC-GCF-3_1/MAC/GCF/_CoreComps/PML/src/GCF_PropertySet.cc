//#  GCF_PropertySet.cc: 
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

#include <GCF/GCF_PropertySet.h>
#include <GCF/GCF_PropertyBase.h>
#include <GCF/GCF_Property.h>
#include <GPA_APCFileReader.h>

GCFPropertySet::GCFPropertySet(string apcName, 
                               string scope,
                               GCFAnswer* pAnswerObj) :
  GCFPropertySetBase(scope, pAnswerObj)
{
  GPAAPCFileReader apcFileReader;
  apcFileReader.readFile(apcName, "");
  const list<TAPCProperty>* pPropsFromAPC;
  pPropsFromAPC = &apcFileReader.getProperties();
  GCFProperty* pProperty;
  
  for (list<TAPCProperty>::const_iterator pAPCProperty = pPropsFromAPC->begin(); 
       pAPCProperty != pPropsFromAPC->end(); ++pAPCProperty)
  {
    pProperty = new GCFProperty(pAPCProperty->name, *this);
    addProperty(pAPCProperty->name, *pProperty);
  }
  if (pAnswerObj)
  {
    setAnswer(pAnswerObj);
  }    
}

TGCFResult GCFPropertySet::requestValue(const string propName) const
{
  GCFPropertyBase* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->requestValue();    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFPropertySet::subscribe(const string propName) const
{
  GCFProperty* pProperty = static_cast<GCFProperty*>(getProperty(propName));
  if (pProperty)
  {
    return pProperty->subscribe();    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFPropertySet::unsubscribe(const string propName) const
{
  GCFProperty* pProperty = static_cast<GCFProperty*>(getProperty(propName));
  if (pProperty)
  {
    return pProperty->unsubscribe();    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}
