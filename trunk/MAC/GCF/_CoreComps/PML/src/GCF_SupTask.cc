//#  GCF_SupTask.cc: 
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

#include "GCF_SupTask.h"
#include "GPM_Controller.h"

GCFSupervisedTask::GCFSupervisedTask(State inital, string& name) :
  GCFTask(initial, name)
{
  _pController = new GPMController(*this);
}

GCFSupervisedTask::~GCFSupervisedTask()
{
  delete _pController;
  _pController = 0;
}

TGCFResult GCFSupervisedTask::loadAPC(const string apcName, 
                                      const string scope)
{
  return (_pController->loadAPC(apcName, scope) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::unloadAPC(const string apcName, 
                                        const string scope)
{
  return (_pController->unloadAPC(apcName, scope) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::reloadAPC(const string apcName, 
                                        const string scope)
{
  return (_pController->reloadAPC(apcName, scope) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::loadMyProperties(TPropertySet& newSet, 
                                               const string scope)
{
  return (_pController->loadMyProperties(newSet, scope) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::unloadMyProperties(const string scope)
{
  return (_pController->unloadMyProperties(scope) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::set(const string& propName, 
                                  const GCFPValue& value)
{
  return (_pController->set(propName, value) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::get(const string& propName)
{
  return (_pController->get(propName) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}

TGCFResult GCFSupervisedTask::getMyOldValue(const string& propName, GCFPValue& value)
{
  return (_pController->getMyOldValue(propName, value) == PM_NO_ERROR ? 
          GCF_NO_ERROR : 
          GCF_PML_ERROR);
}
 
