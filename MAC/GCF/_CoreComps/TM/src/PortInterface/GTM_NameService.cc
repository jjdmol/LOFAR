//#  GTM_NameSerivce.cc: name service for framework tasks
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

#include "GTM_NameService.h"
#include "GCF_PeerAddr.h"

GTMNameService* GTMNameService::_pInstance = 0;

GTMNameService::GTMNameService() : _pConfig(0)
{
}

GTMNameService::~GTMNameService()
{
  if (_pConfig) delete _pConfig;
}

GTMNameService* GTMNameService::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GTMNameService();
  }

  return _pInstance;
}

int GTMNameService::init(const char *ns_config_file)
{
  char fname[strlen(ns_config_file)+strlen(".ns")+1];
  strcpy(fname, ns_config_file);
  strcat(fname, ".ns");

  if (_pConfig) delete _pConfig;
  try {
    _pConfig = new GTMConfig(fname);
  }
  catch (...)
  {
    LOG_ERROR((
	       "Failed to open NameService config file '%s'\n",
	       fname));
  }
  
  return 0;
}

int GTMNameService::query(string& taskname,
			GCFPeerAddr& peeraddr)
{
  if (!_pConfig) return -1;

  const char* host = (*_pConfig)(taskname, "host");

  if (host)
  {
    peeraddr.setTaskname(taskname);
    peeraddr.setHost(host);
    peeraddr.setPortnumber(0);
    peeraddr.setPorttype("");

    return 0; // success
  }
  
  return -1; // failure
}

int GTMNameService::queryPort(string& taskname,
			    string& portname,
			    GCFPeerAddr& peeraddr)
{
  if (!_pConfig) return -1;

  char label[MAX_PORTNAME_LEN + 1];

  strncpy(label, portname.c_str(), MAX_PORTNAME_LEN);
  strncat(label, ".type", MAX_PORTNAME_LEN - sizeof(label));

  const char* type = (*_pConfig)(taskname, label);

  strncpy(label, portname.c_str(), MAX_PORTNAME_LEN);
  strncat(label, ".port", MAX_PORTNAME_LEN - sizeof(label));

  const char* port = (*_pConfig)(taskname, label);

  strncpy(label, portname.c_str(), MAX_PORTNAME_LEN);
  strncat(label, ".host", MAX_PORTNAME_LEN - sizeof(label));

  const char* host = (*_pConfig)(taskname, label);

  peeraddr.setPortname(portname);
  if (type)     peeraddr.setPorttype(type);
  if (port)     peeraddr.setPortnumber(atoi(port));
  if (host)     peeraddr.setHost(host);

  // type and port are optional
  // but at least one must be specified
  if (type || port) return 0; // success
  
  return -1; // failure
}

const char* GTMNameService::getTask(int index)
{
  if (!_pConfig) return 0;

  return (*_pConfig).block(index);
}
