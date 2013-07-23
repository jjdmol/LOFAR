//# DH_EchoPing.h: Example DataHolder storing no individual fields
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

#ifndef DH_ECHOPING_H
#define DH_ECHOPING_H

#include <lofar_config.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace Test
  {

// This class is an example DataHolder which is only used in the
// Example programs.

class DH_EchoPing: public DataHolder
{
  public:
    explicit DH_EchoPing (const string& name="dh_echoping");
  
    DH_EchoPing(const DH_EchoPing&);
  
    virtual ~DH_EchoPing();
  
    DataHolder* clone() const;
  
    /// Set the SeqNr attribute.
    void setSeqNr (uint seqnr);
    /// Get the SeqNr attribute.
    uint getSeqNr() const;
  
    /// Set the EchoTime attribute.
    void setEchoTime (timeval t);
    /// Get the EchoTime attribute.
    timeval getEchoTime() const;

    /// Set the PingTime attribute.
    void setPingTime (timeval t);
    /// Get the PingTime attribute.
    timeval getPingTime() const;

    /// Allocate the buffers.
    virtual void init();
  
   
   private:
    /// Forbid assignment.
    DH_EchoPing& operator= (const DH_EchoPing&);
  
    // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
    virtual void fillDataPointers();
    
    uint*   itsSeqNr;
    uint64*  itsPingTime;
};


inline void DH_EchoPing::setSeqNr (uint seqnr)
  { *itsSeqNr = seqnr; }

inline uint DH_EchoPing::getSeqNr() const
  { return *itsSeqNr; }


inline void DH_EchoPing::setPingTime (timeval t)
  { *itsPingTime = (uint64) t.tv_usec + ((uint64)t.tv_sec * 1000000l); }

inline timeval DH_EchoPing::getPingTime() const
  { timeval t;
    t.tv_sec = *itsPingTime / 1000000l;
    t.tv_usec = *itsPingTime - ((uint64) t.tv_sec * 1000000l);
    return t;
  }
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR

#endif 
