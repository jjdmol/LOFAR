//#  GCF_PropertyBase.cc: 
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

#include <GCF/GCF_PropertyBase.h>
#include <GCF/GCF_PropertySetBase.h>
#include <GCF/GCF_Answer.h>

GCFPropertyBase::~GCFPropertyBase()
{
  assert (_pPropertySet == 0);  
}

const string GCFPropertyBase::getFullName () const
{
  if (_pPropertySet == 0)
  {
    return _name;
  }
  else
  {
    string scope = _pPropertySet->getScope();
    if (scope.length() == 0)
    {
      return _name;
    }
    else
    {
      string fullName = scope + GCF_PROP_NAME_SEP + _name;
      return fullName;
    }
  }
}

void GCFPropertyBase::dispatchAnswer(GCFEvent& answer)
{
  if (_pAnswerObj != 0)
  {
    _pAnswerObj->handleAnswer(answer);
  }  
}

void GCFPropertyBase::subscribed ()
{
  GCFPropAnswerEvent e(F_SUBSCRIBED_SIG);
  e.pPropName = _name.c_str();
  dispatchAnswer(e);
}

void GCFPropertyBase::valueChanged (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VCHANGEMSG_SIG);
  e.pValue = &value;
  e.pPropName = _name.c_str();
  e.internal = false;
  dispatchAnswer(e);
}

void GCFPropertyBase::valueGet (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP_SIG);
  e.pValue = &value;
  e.pPropName = _name.c_str();
  dispatchAnswer(e);
}
