//#  GTM_TopologyService.h: describes components of an application and how they
//#                      are connected together.
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

#ifndef GTM_TOPOLOGYSERVICE_H
#define GTM_TOPOLOGYSERVICE_H

#include <Common/lofar_string.h>

// forward declaration
class GTMConfig; 
class GCFPeerAddr;
/**
 * This singleton class provides the possibility to load the topology of all 
 * used ports on a host.
 */
class GTMTopologyService
{
  public:
    static GTMTopologyService* instance ();

    virtual ~GTMTopologyService ();
    
    int init (const char* top_config_file);
    
    int getPeerAddr (const string& localtaskname,
                	   string& localportname,
                	   GCFPeerAddr& peeraddr);

  private:
    GTMTopologyService ();
    
    /**
     * Don't allow copying of the GTMTopologyService object.
     */
    GTMTopologyService (const GTMTopologyService&);
    GTMTopologyService& operator= (const GTMTopologyService&);

  private:
    static GTMTopologyService* _pInstance;
    GTMConfig* _pConfig;
};

#endif
