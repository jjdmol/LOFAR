//#  SingleInstanceLogicalDeviceFactory.h: template factory class for single instance logical devices
//#
//#  Copyright (C) 2002-2005
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

#ifndef SingleInstanceLogicalDeviceFactory_H
#define SingleInstanceLogicalDeviceFactory_H

//# Includes
#include <APL/APLCommon/LogicalDeviceFactoryBase.h>

//# local includes

//# Common Includes

// forward declaration

namespace LOFAR
{
  
namespace APLCommon
{
  template <class T>
  class SingleInstanceLogicalDeviceFactory : public LogicalDeviceFactoryBase
  {
    public:

      SingleInstanceLogicalDeviceFactory() : LogicalDeviceFactoryBase(), m_instance() {}; 
      virtual ~SingleInstanceLogicalDeviceFactory() {};
      
      virtual boost::shared_ptr<LogicalDevice> createLogicalDevice(const string& taskName, 
                                                                              const string& parameterFile,
                                                                              GCF::TM::GCFTask* pStartDaemon)
      {
        if(m_instance == 0)
        {
          m_instance.reset(new T(taskName, parameterFile, pStartDaemon));
        }
        else
        {
          // The LD exists already. Two options:
          // 1. The one and only LD with this name is rescheduled
          // 2. The LD can be shared with several parents (SO, SRG). The paramset
          //    contains the details about the new parent.
          m_instance->updateParameterFile(parameterFile);
        }
        return m_instance;
      };

      virtual bool sharingAllowed()
      {
        return true;
      }

    protected:
      // protected copy constructor
      SingleInstanceLogicalDeviceFactory(const SingleInstanceLogicalDeviceFactory&);
      // protected assignment operator
      SingleInstanceLogicalDeviceFactory& operator=(const SingleInstanceLogicalDeviceFactory&);

    private:
      boost::shared_ptr<APLCommon::LogicalDevice> m_instance;
  };
};//APLCommon
};//LOFAR
#endif
