//#  AVTResourceManager.cc: Implementation of the Resource manager
//#
//#  Copyright (C) 2002-2004
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

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <APLCommon/APL_Defines.h>
#include "AVTResourceManager.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTDefines.h"
#include "AVTUtilities.h"

using namespace LOFAR;
using namespace AVT;
using namespace std;
using namespace boost;

AVTResourceManager::AVTResourceManagerPtrInternal AVTResourceManager::s_avtResourceManagerInstance;

void AVTResourceManager::checked_deleter::operator() (AVTResourceManager* _p)
{
  typedef char type_must_be_complete[ sizeof(_p) ];
  delete _p;
}

AVTResourceManager::AVTResourceManager() : boost::noncopyable(),
  m_resourceRequests()
{
  LOG_TRACE(formatString("%s",__func__));
}

AVTResourceManager::~AVTResourceManager()
{
  LOG_TRACE(formatString("%s",__func__));
}

AVTResourceManagerPtr AVTResourceManager::instance()
{
  LOG_TRACE(formatString("%s",__func__));
  
  AVTResourceManagerPtr tmp;
  if(s_avtResourceManagerInstance.expired())
  {
    tmp.reset(new AVTResourceManager, checked_deleter() );
    s_avtResourceManagerInstance = tmp;
  }
  else
  {
    tmp = boost::make_shared(s_avtResourceManagerInstance);
  }
  
  return tmp;
}

/*
* requests access to a resource. The first requester gets master access
*/
void AVTResourceManager::requestResource(const string& taskName, const string& resourceName)
{
  ResourceRequestsIterT requestsIt = m_resourceRequests.find(resourceName);
  if(requestsIt != m_resourceRequests.end())
  {
    // resource already in list, put the requester at the end of the requesters list
    requestsIt->second.push_back(taskName);
  }
  else
  {
    // resource was never requested before, create a new entry
    ResourceClientsT resClients;
    resClients.push_back(taskName);
    m_resourceRequests[resourceName] = resClients;
  }
}

/*
* releases access to a resource. If the task was master, then the second requester gets master access
*/
void AVTResourceManager::releaseResource(const string& taskName, const string& resourceName)
{
  ResourceRequestsIterT requestsIt = m_resourceRequests.find(resourceName);
  if(requestsIt != m_resourceRequests.end())
  {
    // resource in list, remove the requester from the requesters list
    ResourceClientsIterT clientsIt = requestsIt->second.begin();
    bool found;
    do
    {
      found = (*clientsIt == taskName);
      if(found)
      {
        requestsIt->second.erase(clientsIt);
      }
      ++clientsIt;
    } while(!found && clientsIt != requestsIt->second.end());
  }
  else
  {
    // resource was never requested
  }
}

/*
* returns true if the task is master
*/
bool AVTResourceManager::isMaster(const string& taskName, const string& resourceName)
{
  bool isMaster=false;
  ResourceRequestsIterT requestsIt = m_resourceRequests.find(resourceName);
  if(requestsIt != m_resourceRequests.end())
  {
    // resource in list, check if the task is the first in the clients list
    isMaster = ((requestsIt->second.size()>0) && (taskName == *(requestsIt->second.begin())));
  }
  else
  {
    // resource was never requested
  }
  return isMaster;
}


