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
#include <GCF/TM/GCF_PeerAddr.h>
#include "GTM_Defines.h"
#include "GTM_Config.h"

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

int GTMNameService::init(const char* nsConfigFile)
{
  unsigned int length = strlen(nsConfigFile) + strlen(".ns") + 1;
  char fname[length];
  strcpy(fname, nsConfigFile);
  strcat(fname, ".ns");

  if (_pConfig) delete _pConfig;
  try {
    _pConfig = new GTMConfig(fname);
  }
  catch (...)
  {
    LOG_ERROR(LOFAR::formatString (
	     "Failed to open NameService config file '%s'",
	     fname));
    return -1;
  }
  
  return 0;
}

int GTMNameService::query(const string& taskname,
			GCFPeerAddr& peeraddr)
{
  if (!_pConfig) return -1;

  string label("host");
  const char* host = (*_pConfig)(taskname, label);

  if (host)
  {
    string porttype("");
    string hostName(host);
    peeraddr.setTaskname(taskname);
    peeraddr.setHost(hostName);
    peeraddr.setPortnumber(0);
    peeraddr.setPorttype(porttype);

    return 0; // success
  }
  
  return -1; // failure
}

int GTMNameService::queryPort(const string& taskname,
			    const string& portname,
			    GCFPeerAddr& peeraddr)
{
  if (!_pConfig) return -1;

  string label(portname + string(".type"));

  const char* type = (*_pConfig)(taskname, label);

  label = portname + string(".port");

  const char* port = (*_pConfig)(taskname, label);

  label = portname + string(".host");

  const char* host = (*_pConfig)(taskname, label);

  peeraddr.setPortname(portname);
  if (type)     
  {
    string porttype(type);
    peeraddr.setPorttype(porttype);
  }
  if (port)     peeraddr.setPortnumber(atoi(port));
  if (host)
  {
    string hostName(host);        
    peeraddr.setHost(hostName);
  }

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
