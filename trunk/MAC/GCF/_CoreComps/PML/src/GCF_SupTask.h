//#  GCF_SupTask.h: 
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

#ifndef GCF_SUBTASK_H
#define GCF_SUBTASK_H

#include <TM/GCF_Task.h>
#include <SAL/GCF_PValue.h>
#include <GCFCommon/GCF_Defines.h>

class GPMController;

class GCFSupervisedTask : public GCFTask
{
  public:
    virtual ~GCFSupervisedTask();
  
  protected:
    GCFSupervisedTask(State initial, string& name);
    
    TGCFResult loadAPC(const string apcName, const string scope);
    TGCFResult unloadAPC(const string apcName, const string scope);
    TGCFResult reloadAPC(const string apcName, const string scope);
    TGCFResult loadMyProperties(TPropertySet& newSet);
    TGCFResult unloadMyProperties(const string scope);
    TGCFResult set(const string propName, const GCFPValue& value);
    TGCFResult get(const string propName);
    TGCFResult getMyOldValue(const string propName, GCFPValue** pValue);
    friend class GPMController;
    virtual void valueChanged(const string& propName, const GCFPValue& value) = 0;
    virtual void valueGet(const string& propName, const GCFPValue& value) = 0;
    virtual void apcLoaded(const string& apcName, const string& scope) = 0;
    virtual void apcUnloaded(const string& apcName, const string& scope) = 0;
    virtual void apcReloaded(const string& apcName, const string& scope) = 0;
    virtual void myPropertiesLoaded(const string& scope) = 0;
    virtual void myPropertiesUnloaded(const string& scope) = 0;
  
  private:
    GPMController* _pController;
};
#endif
