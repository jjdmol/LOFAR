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

#include <GCF/GCF_PeerAddr.h>

GCFPeerAddr::GCFPeerAddr()
{
}

GCFPeerAddr::GCFPeerAddr(string& taskname,
		     string& host,
		     string& portname,
		     string& porttype,
		     int portnumber)
{
  _taskname = taskname;
  _host = host;
  _portname = portname;
  _porttype = porttype;
  _portnumber = portnumber;
}

GCFPeerAddr::~GCFPeerAddr()
{
}

const string& GCFPeerAddr::getTaskname() const 
{
  return _taskname;
}

const string& GCFPeerAddr::getHost() const 
{
  return _host;
}

const string& GCFPeerAddr::getPortname() const 
{
  return _portname;
}

const string& GCFPeerAddr::getPorttype() const 
{
  return _porttype;
}

int GCFPeerAddr::getPortnumber() const 
{
  return _portnumber;
}

void GCFPeerAddr::setTaskname(const string& taskname)
{
  _taskname = taskname;
}

void GCFPeerAddr::setHost(const string& host)
{
  _host = host;
}

void GCFPeerAddr::setPortname(const string& portname)
{
  _portname = portname;
}

void GCFPeerAddr::setPorttype(const string& porttype)
{
  _porttype = porttype;
}

void GCFPeerAddr::setPortnumber(int portnumber)
{
  _portnumber = portnumber;
}
