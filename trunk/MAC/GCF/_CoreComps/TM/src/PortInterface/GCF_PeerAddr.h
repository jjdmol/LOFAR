//#  GCF_PeerAddr.h: describes components of an application and how they
//#                   are connected together.
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

#ifndef GCF_PEERADDR_H
#define GCF_PEERADDR_H

#include "GTM_Config.h"
#include <GCF_Defines.h>

class GCFPeerAddr
{
 public:

  GCFPeerAddr();
  GCFPeerAddr(string& taskname,
	    const char* host,
	    string& portname,
	    const char* porttype,
	    int portnumber = 0);
  virtual ~GCFPeerAddr();

  string& getTaskname();
  const char* getHost() const;
  int         getPortnumber() const;
  string& getPortname();
  const char* getPorttype() const;

  void setTaskname(string& taskname);
  void setHost(const char* host);
  void setPortname(string& portname);
  void setPorttype(const char* porttype);
  void setPortnumber(int portnumber);

 protected:

 private:

  string _taskname;
  char _host[MAXHOSTNAMELEN   + 1];
  string _portname;
  char _porttype[MAX_PORTTYPE_LEN + 1];
  int  _portnumber;
};

#endif
