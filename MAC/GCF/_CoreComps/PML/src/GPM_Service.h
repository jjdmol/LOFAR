//#  GPM_Service.h: 
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

#ifndef GPM_SERVICE_H
#define GPM_SERVICE_H

#include <SAL/GSA_Service.h>
#include "GPMController.h"

class GPMService : public GSAService
{
  public:
    GPMService(GPMController& controller) : _controller(controller) {};
    virtual ~GPMService();

    inline TSAResult subscribe(const string& propName)
    {
      return GSAService::subscribe(propName);
    }
    inline TSAResult unsubscribe(const string& propName)
    {
      return GSAService::unsubscribe(propName);
    }
    inline TSAResult get(const string& propName)
    {
      return GSAService::get(propName);
    }
    inline TSAResult set(const string& propName, const GCFPValue& value)
    {
      return GSAService::set(propName, value);
    }
    inline bool exists(const string& propName)
    {
      return GSAService::exists(propName);
    }

  protected:
    void propCreated(string& propName) {};
    void propDeleted(string& propName) {};
    inline void propSubscribed(string& propName)
    {
      _controller.propSubscribed(propName);
    }
    inline void propUnsubscribed(string& propName)
    {
      _controller.propUnsubscribed(propName);
    }
    inline void propValueGet(string& propName, GCFPValue& value)
    {
      _controller.valueGet(propName, value);
    }
    inline void propValueChanged(string& propName, GCFPValue& value)
    {
      _controller.valueChanged(propName, value);
    }
  
  private:
    GPMController& _controller;
};
#endif