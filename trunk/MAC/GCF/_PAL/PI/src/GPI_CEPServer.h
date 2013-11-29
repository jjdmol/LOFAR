//#  GPI_CEPServer.h: representation of a Supervisory Server in a ERTC env.
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

#ifndef GPI_CEPSERVER_H
#define GPI_CEPSERVER_H

#include <GPI_PMLlightServer.h>
#include <GCF/Protocols/DH_PIProtocol.h>

namespace LOFAR 
{

class CSConnection;

 namespace GCF 
 {
  namespace PAL
  {

// This special PMLlight server class handles all messages from and to a CEP-PIA 
// and all message from the PA determined for a CEP-PIA. For this purpose it 
// uses the special DataHolder implementation and the special TransportHolder 
// implementation combined with GCFTCPPort (thePortToClient) of the baseclass. 
// For communication with PA it uses the GCFTCPPort (thePortToPA) of the 
// baseclass too. Because the DataHolder’s are constructed with Blob’s and the 
// GCF port concept is based on TM::GCFEvents, this class must also convert Blob’s 
// to TM::GCFEvents and visa versa. 
// Note: Messages received from PA needed not be converted to Blob’s. 
// They will be first processed by the GPIPropertySet, which generates PI 
// messages for the PIA. These messages than has to be converted to Blob’s. 
 
class GPICEPServer: public GPIPMLlightServer
{
  public:
		GPICEPServer (GPIController& controller);
		virtual ~GPICEPServer ();
   
    // specialized implementation of the method defined in the baseclass
    // it converts the message (based on the TM::GCFEvent concept and determined for
    // the CEP-PMLlight) to Blob's (from LCS/Common)
    virtual void sendMsgToClient(TM::GCFEvent& msg);        
      
  protected: // event handle methods
    void linkPropSet(const PALinkPropSetEvent& requestIn);
    void unlinkPropSet(const PAUnlinkPropSetEvent& requestIn);

  private: // helper methods
        
  private: // state methods
    // overrides the state method of the baseclass to convert incomming messages
    // from CEP-PMLlight in the right way
    TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);
    
  private: // (copy) constructors
    GPICEPServer();
    // Don't allow copying of this object.
    // <group>
    GPICEPServer (const GPICEPServer&);
    GPICEPServer& operator= (const GPICEPServer&);
    // </group>

  private: // data members
    DH_PIProtocol   _dhServer;
    CSConnection*   _pReadConn;
    CSConnection*   _pWriteConn;
        
  private: // admin. data members
    char*         _valueBuf;
    unsigned int  _upperboundValueBuf;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
