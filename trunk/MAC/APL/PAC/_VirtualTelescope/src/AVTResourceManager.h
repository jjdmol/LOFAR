//#  AVTResourceManager.h: schedules logical devices
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

#ifndef AVTResourceManager_H
#define AVTResourceManager_H

//# Includes
//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/utility.hpp>
#include <map>
#include <vector>
#include <string>

//# local includes

// forward declaration
class GCFEvent;
class GCFPortInterface;

namespace AVT
{
  class AVTResourceManager; // forward declaration because of typedef
  typedef boost::shared_ptr<AVTResourceManager> AVTResourceManagerPtr;

  class AVTResourceManager : boost::noncopyable // prohibits access to copy construction and assignment
  {
    public:
      
      /*
      * returns the one and only static instance if it exists; creates and returns it otherwise
      */
      static AVTResourceManagerPtr instance();
      
      /*
      * requests access to a resource. The first requester gets master access
      */
      void requestResource(const string& taskName, const string& resourceName);
      /*
      * releases access to a resource. If the task was master, then the second requester gets master access
      */
      void releaseResource(const string& taskName, const string& resourceName);
      /*
      * returns true if the task is master
      */
      bool isMaster(const string& taskName, const string& resourceName);

    protected:

    private:
      struct checked_deleter
      {
        /*
        * deletes the static instance if there are no references to it
        */
        void operator() (AVTResourceManager* _p);
      };
      friend struct checked_deleter;
      typedef boost::weak_ptr<AVTResourceManager> AVTResourceManagerPtrInternal;
      
      /*
      * constructor and destructor are private
      */
      AVTResourceManager(); 
      virtual ~AVTResourceManager();

      static AVTResourceManagerPtrInternal s_avtResourceManagerInstance;
      
      // end of singleton overhead
      
      typedef std::vector<std::string>                ResourceClientsT;
      typedef ResourceClientsT::iterator              ResourceClientsIterT;
      typedef std::map<std::string,ResourceClientsT>  ResourceRequestsT;
      typedef ResourceRequestsT::iterator             ResourceRequestsIterT;
      
      ResourceRequestsT                               m_resourceRequests;
  };

};
#endif
