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
    GPITH_Port (TM::GCFPortInterface& port);
    
    virtual ~GPITH_Port();
    
    virtual bool init();
    // Make an instance of the transportholder
    virtual GPITH_Port* make() const;
  
    /// Get the type of transport.
    virtual string getType() const;
  
    /// Read the data.
            bool  recvBlocking       (void*, int, int, int = 0, LOFAR::DataHolder* = 0);
    virtual int32 recvNonBlocking 	 (void* buf, int32 nbytes, int32 tag, int offset = 0, LOFAR::DataHolder* = 0);
  
    /// Write the data.
            bool sendBlocking    (void*, int, int, LOFAR::DataHolder* = 0);    
    virtual bool sendNonBlocking (void* buf, int32 nbytes, int32 tag, DataHolder* dh = 0);

            void waitForSent(void* buf, int nbytes, int tag);
            void waitForReceived(void* buf, int nbytes, int tag);

            bool isClonable() const;
            void reset();
    
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
    
    typedef enum {
      CmdNone = 0,
      CmdRecvNonBlock,
    } CmdTypes;
     
    int32   itsReadOffset;      // For partial reads.
  
   // Administration for non-blocking receiving. In the recv-call
    // these fields are filled so that waitForRecv knows what to do.
    int16   itsLastCmd;
};

inline GPITH_Port::GPITH_Port(TM::GCFPortInterface& port) : 
  _port          (port),
  itsReadOffset  (0),
  itsLastCmd     (CmdNone) 
{}


inline void GPITH_Port::waitForSent(void*, int, int) { }

inline void GPITH_Port::waitForReceived(void*, int, int) {}

inline bool GPITH_Port::isClonable() const
  { return false; }  

inline void GPITH_Port::reset()
  { }

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
