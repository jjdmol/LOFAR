//#  GCF_PeerAddr.h: This class holds information about port host and the port 
//#                  number. This will be used to hold the remote and local address.
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

#include <Common/lofar_string.h>

/**
 * This class holds information about port host and the port number. This will 
 * be used to hold the remote and local address.
 */
class GCFPeerAddr
{
  public:

    GCFPeerAddr ();
    GCFPeerAddr (string& taskname,
          	     string& host,
          	     string& portname,
          	     string& porttype,
          	     int portnumber = 0);
    virtual ~GCFPeerAddr ();
    
    const string& getTaskname () const;
    const string& getHost () const;
    int   getPortnumber () const;
    const string& getPortname () const;
    const string& getPorttype () const;
    
    void setTaskname (const string& taskname);
    void setHost (const string& host);
    void setPortname (const string& portname);
    void setPorttype (const string& porttype);
    void setPortnumber (int portnumber);

  private:
    string _taskname;
    string _host;
    string _portname;
    string _porttype;
    int  _portnumber;
};

#endif
