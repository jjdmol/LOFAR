//#  GTM_SBTCPPort.h: TCP connection to a remote process
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

#ifndef GTM_SBTCPPORT_H
#define GTM_SBTCPPORT_H

#include <GCF/TM/GCF_TCPPort.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace SB 
  {

// forward declaration

/**
 * This is the class, which implements the special port with the TCP message 
 * transport protocol. It uses socket pattern to do this. Is can act as MSPP 
 * (port provider), SPP (server) and SAP (client).
 */
class GTMSBTCPPort : public TM::GCFTCPPort
{
 public:

    /// Construction methods
    /** @param protocol NOT USED */    
    explicit GTMSBTCPPort (TM::GCFTask& 				   task,
						   const string& 				   name,
						   TM::GCFPortInterface::TPortType type,
						   int 							   protocol, 
						   bool 						   transportRawData = false);
    explicit GTMSBTCPPort ();
  
    virtual ~GTMSBTCPPort ();
  
  public:

    /**
     * open/close functions
     */
    virtual bool open ();


  private:
    /**
     * Don't allow copying this object.
     */
    GTMSBTCPPort (const GTMSBTCPPort&);
    GTMSBTCPPort& operator= (const GTMSBTCPPort&);
};
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR

#endif
