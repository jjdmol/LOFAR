//#  GTM_TopologyService.cc: describes components of an application and how they
//#                       are connected together.
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

#include "GTM_TopologyService.h"
#include "GTM_Config.h"
#include "GTM_NameService.h"
#include "GCF_PeerAddr.h"

GTMTopologyService* GTMTopologyService::_pInstance = 0;

GTMTopologyService::GTMTopologyService() : _pConfig(0)
{
}

GTMTopologyService::~GTMTopologyService()
{
  if (_pConfig) delete _pConfig;
}

int GTMTopologyService::init(const char* top_config_file)
{
  char fname[strlen(top_config_file)+strlen(".top")+1];
  strcpy(fname, top_config_file);
  strcat(fname, ".top");

  if (_pConfig) delete _pConfig;
  
  try { _pConfig = new GTMConfig(fname); }
  catch (...)
  {
    LOG_ERROR((
		      "Failed to open TopologyService config file '%s'\n",
		      top_config_file));
  }

  return 0;
}

GTMTopologyService* GTMTopologyService::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GTMTopologyService();
  }

  return _pInstance;
}

int GTMTopologyService::getPeerAddr(string& localtaskname,
				  string& localportname,
				  GCFPeerAddr& peeraddr)
{
  if (!_pConfig) return -1;

  string sRemoteTaskName;
  char remote_task_name[MAX_TASKNAME_LEN + 1];

  // use app topology config to find peer task and port name
  // use name server to find peer address
  const char* port_peer = (*_pConfig)(localtaskname, localportname.c_str());

  if (port_peer)
  {
    //remote_task_name = port_peer;
    strncpy(remote_task_name, port_peer, MAX_TASKNAME_LEN);
    char* colon = strchr(remote_task_name, ':');
    if (colon) *colon = '\0';
    else
    {
      LOG_ERROR((
		 "Missing remote port name in port connection "
		 "for port '%s'\n", localportname));
    }
    sRemoteTaskName = remote_task_name;
    if (GTMNameService::instance()->query(sRemoteTaskName,
					peeraddr) < 0)
    {
      LOG_ERROR((
			"Could not find task '%s' in "
			"name service configuration file.\n",
			remote_task_name));
    }
    string sPortName = colon + 1;
    peeraddr.setPortname(sPortName); // remote portname position

    // now find the port type info
    if (GTMNameService::instance()->queryPort(sRemoteTaskName,
					    peeraddr.getPortname(),
					    peeraddr))
    {
      LOG_ERROR((
			"Could not find port '%s' of task '%s' in "
			"name service configuration file.\n",
			peeraddr.getPortname(), remote_task_name));
    }
  }
  else
  {
    LOG_ERROR((
	       "Task '%s' or local port '%s' not found in "
	       "application topology configuration file.\n",
	       localtaskname, localportname));
  }

  return 0;
}
