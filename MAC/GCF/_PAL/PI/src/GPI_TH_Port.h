//# GPI_TH_Port.h: GCFPort based TransportHolder
//#              Based on the GCFPortInterface wrapper class from GCF/TM
//#
//# Copyright (C) 2000, 2001
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

#ifndef GPI_TH_PORT_H
#define GPI_TH_PORT_H

#include <lofar_config.h>
#include <Transport/TransportHolder.h>
#include <GCF/TM/GCF_Event.h>
#include <Common/LofarTypes.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFPortInterface;
  }  
  namespace PAL
  {

class GPITH_Port: public TransportHolder
{
  public:
    GPITH_Port (TM::GCFPortInterface& port) : _port(port) {}
    
    virtual ~GPITH_Port();
    
    // Make an instance of the transportholder
    virtual GPITH_Port* make() const;
  
    /// Get the type of transport.
    virtual string getType() const;
  
    /// Read the data.
    virtual bool recvNonBlocking 	(void* buf, int32 nbytes, int32 tag);
    virtual bool recvVarNonBlocking (int32 tag);
  
    /// Write the data.
    virtual bool sendNonBlocking	(void* buf, int32 nbytes, int32 tag);
  
    virtual bool connectionPossible (int32 srcRank, int32 dstRank) const;
    
    virtual bool isBidirectional() const
    	{ return (true); }
  
    virtual bool isBlocking() const
      { return (false); }
    
  private:
    TM::GCFPortInterface& _port;
    
    class GPIBlobEvent : public TM::GCFEvent
    {
      public:
        GPIBlobEvent() : TM::GCFEvent(0) {};
        virtual ~GPIBlobEvent() {};
  
        void* blobData;
        int32 blobSize;
  
        void* pack(unsigned int& packsize);
  
      private:
        GPIBlobEvent(GPIBlobEvent&);
        GPIBlobEvent& operator= (const GPIBlobEvent&);
    }; 
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
