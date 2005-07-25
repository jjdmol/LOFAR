//#  StationReceptorGroupFactory.h: factory class for StationReceptorGroups.
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

#ifndef StationReceptorGroupFactory_H
#define StationReceptorGroupFactory_H

//# Includes
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>
#include <APLCommon/LogicalDeviceFactory.h>

//# local includes
#include "StationReceptorGroup.h"
//# Common Includes

// forward declaration

namespace LOFAR
{
  
namespace ASR
{
  class StationReceptorGroupFactory : public APLCommon::LogicalDeviceFactory
  {
    public:

      StationReceptorGroupFactory() : APLCommon::LogicalDeviceFactory(), m_theSRGinstances() {}; 
      virtual ~StationReceptorGroupFactory() {};
      
      virtual boost::shared_ptr<APLCommon::LogicalDevice> createLogicalDevice(const string& taskName, 
                                                                              const string& parameterFile,
                                                                              GCF::TM::GCFTask* pStartDaemon)
      {
        map<string, boost::shared_ptr<APLCommon::LogicalDevice> >::iterator it = m_theSRGinstances.find(taskName);
        if(it == m_theSRGinstances.end())
        {
          boost::shared_ptr<APLCommon::LogicalDevice> theInstance(new StationReceptorGroup(taskName, parameterFile, pStartDaemon));
          m_theSRGinstances[taskName] = theInstance;
          return theInstance;
        }
        else
        {
        // The LD exists already. Two options:
        // 1. The one and only LD with this name is rescheduled
        // 2. The LD can be shared with several parents (SO, SRG). The paramset
        //    contains the details about the new parent.
          it->second->updateParameterFile(parameterFile);
          return it->second;
        }
      };

    protected:
      // protected copy constructor
      StationReceptorGroupFactory(const StationReceptorGroupFactory&);
      // protected assignment operator
      StationReceptorGroupFactory& operator=(const StationReceptorGroupFactory&);

    private:
      map<string, boost::shared_ptr<APLCommon::LogicalDevice> > m_theSRGinstances;
    
  };
};//AVT
};//LOFAR
#endif
