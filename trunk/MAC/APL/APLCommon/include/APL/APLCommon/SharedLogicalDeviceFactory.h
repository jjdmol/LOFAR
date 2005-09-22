//#  SharedLogicalDeviceFactory.h: template factory class for shared logical devices
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

#ifndef SharedLogicalDeviceFactory_H
#define SharedLogicalDeviceFactory_H

//# Includes
#include <APLCommon/LogicalDeviceFactoryBase.h>

//# local includes

//# Common Includes

// forward declaration

namespace LOFAR
{
  
namespace APLCommon
{
  template <class T>
  class SharedLogicalDeviceFactory : public LogicalDeviceFactoryBase
  {
    public:

      SharedLogicalDeviceFactory() : LogicalDeviceFactoryBase(), m_instances() {}; 
      virtual ~SharedLogicalDeviceFactory() {};
      
      virtual boost::shared_ptr<LogicalDevice> createLogicalDevice(const string& taskName, 
                                                                              const string& parameterFile,
                                                                              GCF::TM::GCFTask* pStartDaemon)
      {
        boost::shared_ptr<LogicalDevice> theInstance;
        map<string, boost::weak_ptr<LogicalDevice> >::iterator it = m_instances.find(taskName);
        if(it != m_instances.end())
	{
          // create a shared_ptr from the weak_ptr, thereby increasing the usecount
          if(theInstance = it->second.lock())
	  {
            // The LD exists already. Two options:
            // 1. The one and only LD with this name is rescheduled
            // 2. The LD can be shared with several parents (SO, SRG). The paramset
            //    contains the details about the new parent.
            theInstance->updateParameterFile(parameterFile);
	  }
        }
        if(theInstance == 0)
        {
          theInstance.reset(new T(taskName, parameterFile, pStartDaemon));
	  boost::weak_ptr<LogicalDevice> weakInstance(theInstance);
          m_instances[taskName] = weakInstance;
        }
        return theInstance;
      };

      virtual bool sharingAllowed()
      {
        return true;
      }

    protected:
      // protected copy constructor
      SharedLogicalDeviceFactory(const SharedLogicalDeviceFactory&);
      // protected assignment operator
      SharedLogicalDeviceFactory& operator=(const SharedLogicalDeviceFactory&);

    private:
      // Using weak pointers here because the LD itself is responsible for it's lifetime
      // A weak pointer does not increase the use count of the object 
      map<string, boost::weak_ptr<LogicalDevice> > m_instances;
  };
};//APLCommon
};//LOFAR
#endif
