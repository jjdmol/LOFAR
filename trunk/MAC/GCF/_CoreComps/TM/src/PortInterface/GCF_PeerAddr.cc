//#  GCF_PeerAddr.cc: describes components of an application and how they
//#                are connected together.
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

#include "GCF_PeerAddr.h"

GCFPeerAddr::GCFPeerAddr()
{
}

GCFPeerAddr::GCFPeerAddr(string& taskname,
		     const char* host,
		     string& portname,
		     const char* porttype,
		     int portnumber)
{
  _taskname = taskname;
  strncpy(_host, host, MAXHOSTNAMELEN);
  _portname = portname;
  strncpy(_porttype, porttype, MAX_PORTTYPE_LEN);
  _portnumber = portnumber;
}

GCFPeerAddr::~GCFPeerAddr()
{
}

string& GCFPeerAddr::getTaskname()
{
  return _taskname;
}

const char* GCFPeerAddr::getHost() const
{
  return _host;
}

string& GCFPeerAddr::getPortname() 
{
  return _portname;
}

const char* GCFPeerAddr::getPorttype() const
{
  return _porttype;
}

int GCFPeerAddr::getPortnumber() const
{
  return _portnumber;
}

void GCFPeerAddr::setTaskname(string& taskname)
{
  _taskname = taskname;
}

void GCFPeerAddr::setHost(const char* host)
{
  strncpy(_host, host, MAXHOSTNAMELEN);
}

void GCFPeerAddr::setPortname(string& portname)
{
  _portname = portname;
}

void GCFPeerAddr::setPorttype(const char* porttype)
{
  strncpy(_porttype, porttype, MAX_PORTTYPE_LEN);
}

void GCFPeerAddr::setPortnumber(int portnumber)
{
  _portnumber = portnumber;
}
