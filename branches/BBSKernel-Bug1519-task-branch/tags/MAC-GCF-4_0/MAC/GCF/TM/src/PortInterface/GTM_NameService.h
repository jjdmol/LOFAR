//#  GTM_NameSerivce.h: name service for framework tasks
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

#ifndef GTM_NAMESERVICE_H
#define GTM_NAMESERVICE_H

#include <Common/lofar_string.h>

// forward declaration
class GTMConfig;
class GCFPeerAddr;

/**
 * This singleton class represents the name service, which provides the 
 * possibility to configure a port from a configuration file by its port 
 * name. The advantage is that the application does not need to be rebuild if a 
 * port number or host name is changed. 
 * Note: This class works together with the GTMTopologyService class. 
 */
class GTMNameService
{
 public:
  static GTMNameService* instance ();
  virtual ~GTMNameService ();

  int init (const char *ns_config_file);

  int query (const string& taskname,
	           GCFPeerAddr& peeraddr);

  int queryPort (const string& taskname,
                 const string& portname,
                 GCFPeerAddr& peeraddr);

  const char* getTask (int index);
    
 private:

  /**
   * No default constructor.
   */
  GTMNameService ();

  /**
   * Don't allow copying of the FNameService object.
   */
  GTMNameService (const GTMNameService&);
  GTMNameService& operator= (const GTMNameService&);

 private:

  static GTMNameService* _pInstance;
  GTMConfig* _pConfig;
};

#endif
