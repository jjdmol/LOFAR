//# DH_PIProtocol.h: 
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef DH_PIPROTOCOL_H
#define DH_PIPROTOCOL_H

#include <lofar_config.h>
#include <set>
#include <Transport/DataHolder.h>

//

namespace LOFAR {
  namespace GCF {

class DH_PIProtocol: public DataHolder
{
  public:
    typedef enum 
    {
      NO_EVENTID_SET,
      REGISTER_SCOPE,
      SCOPE_REGISTERED,
      UNREGISTER_SCOPE,
      SCOPE_UNREGISTERED,
      LINK_PROPSET,
      PROPSET_LINKED,
      UNLINK_PROPSET,
      PROPSET_UNLINKED,
      VALUE_SET
    } TEventID;
    
    explicit DH_PIProtocol (const string& name = "PIProtocol");
  
    DH_PIProtocol(const DH_PIProtocol&);
  
    virtual ~DH_PIProtocol();
  
    DataHolder* clone() const;
  
    // Set the EventID data member.
    void setEventID (TEventID EventID);
    // Get the EventID data member.
    TEventID getEventID() const;
    
    // Set the seqNr data member.
    void setSeqNr (uint16 seqNr);
    // Get the seqNr data member.
    uint16 getSeqNr() const;
    
    // init protocol static fields.
    virtual void preprocess();
  
    // leave protocol static fields.
    virtual void postprocess();

  private:
    // Forbid assignment.
    DH_PIProtocol& operator= (const DH_PIProtocol&);
  
    // Fill the pointers (_eventID and _seqNr) to the data in the blob.
    virtual void fillDataPointers();

  private: // data member
    uint8*    _eventID;
    uint16*   _seqNr;
    
  private: // adminstrative members
    typedef std::set<uint16> TSeqNrSet;
    TSeqNrSet _seqNrSet;
    
};

inline void DH_PIProtocol::setEventID (DH_PIProtocol::TEventID EventID)
  { *_eventID = EventID; }

inline DH_PIProtocol::TEventID DH_PIProtocol::getEventID() const
  { return (TEventID) *_eventID; }

inline void DH_PIProtocol::setSeqNr (uint16 seqNr)
  { *_seqNr = seqNr; }

inline uint16 DH_PIProtocol::getSeqNr() const
  { return *_seqNr; }

  } // namespace GCF
} // namespace LOFAR

#endif 
