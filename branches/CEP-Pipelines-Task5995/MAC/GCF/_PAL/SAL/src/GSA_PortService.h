//#  GSA_PortService.h: manages the properties with its use count
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

#ifndef GSA_PORTSERVICE_H
#define GSA_PORTSERVICE_H

#include <GSA_Service.h>
#include <GCF/GCF_PVString.h>

namespace LOFAR {
 namespace GCF {
  namespace PAL {
/**
   This class manages the properties with its use count, which are created 
   (resp. deleted) by means of the base class GSAService.
*/

class GCFPVSSPort;
class GCFPVSSUIMConverter;

class GSAPortService : public GSAService
{
public:
  	GSAPortService(GCFPVSSPort& port);
  	virtual ~GSAPortService();

    virtual void start ();
    virtual void stop ();
    /**
     * send/recv functions
     */
    virtual ssize_t send (void* buf, size_t count, const string& destDpName);
    virtual ssize_t recv (void* buf, size_t count);
    bool registerPort(GCFPVSSPort& p);
    void unregisterPort(const string& portID);
    void setConverter(GCFPVSSUIMConverter& converter) 
      { _pConverter = &converter;}
    GCFPVSSPort& getPort() const { return _port;}
    const string& getCurPortId() const { return _curPeerID.getValue();}
    
protected:
    void dpCreated(const string& propName);
    void dpDeleted(const string& /*propName*/) {};
    void dpeValueGet(const string& /*propName*/, const Common::GCFPValue& /*value*/) {}; 
    void dpeValueChanged(const string& propName, const Common::GCFPValue& value);
    void dpeValueSet(const string& /*propName*/) {};
    void dpeSubscribed(const string& propName);
    void dpeSubscriptionLost (const string& propName);
    void dpeUnsubscribed(const string& /*propName*/) {};

private: 
	// helper methods
    GCFPVSSPort* findClient(const string& c);
    void disconnectedEvent(const string& curPeerID);
    
	// data members
    GCFPVSSPort& _port;
    
	// admin. data members
    bool _isSubscribed;
    unsigned char* _msgBuffer;
    unsigned int _bytesLeft;
    typedef map<string /*peer ID*/, GCFPVSSPort*> TClients;
    TClients _clients;
    
    Common::GCFPVString   _curPeerID;
    Common::GCFPVString   _curPeerAddr;
    GCFPVSSUIMConverter*  _pConverter;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
