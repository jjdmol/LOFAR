//
//  FPortInterface.h: container class for all port implementations
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef GCF_PORTINTERFACE_HXX
#define GCF_PORTINTERFACE_HXX

#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>

#include "GCF_Defines.hxx"
#include "GCF_Event.hxx"

class GCFPortInterface
{
 public:

  /**
   * port types
   */
  typedef enum {
    SAP = 1, // Service Access Point       (port connector)
    SPP,     // Service Provision Point    (port acceptor)
  } FPortType;

  GCFPortInterface(FPortType type, const char *name = 0) :
    _name(0), _isConnected(false), _type(type)
    {
      if (name != 0)
      {
        _name = strdup(name);
      }      
    }
  virtual ~GCFPortInterface() {};

  virtual void close() = 0;

  /**
   * send/recv functions
   */
  virtual ssize_t send(const GCFEvent& event,
		       void* buf = 0, size_t count = 0) = 0;
  virtual ssize_t sendv(const GCFEvent& event,
			const iovec buffers[], int n) = 0;

  virtual ssize_t recv(void* buf, size_t count)     = 0;
  virtual ssize_t recvv(iovec buffers[], int n) = 0;

  /**
   * Timer functions.
   * Upon expiration of a timer a F_TIMER_SIG will be
   * received on the port.
   */
/*  virtual long setTimer(long  delay_sec,
			long  delay_usec    = 0,
			long  interval_sec  = 0,
			long  interval_usec = 0,
			const void* arg     = 0) = 0;

  virtual long setTimer(double delay_seconds, 
			double interval_seconds = 0.0,
			const void*  arg        = 0) = 0;

  virtual int  cancelTimer(long   timerid,
			   const void** arg = 0) = 0;

  virtual int  cancelAllTimers() = 0;

  virtual int  resetTimerInterval(long timerid,
				  long sec,
				  long usec = 0) = 0;
*/
  /**
   * Attribute access functions
   */
    const char* getName()     const {return _name;}
    FPortType   getType()     const {return _type;}
    bool        isConnected() const {return _isConnected;}

  protected:
    char *_name;
    bool _isConnected;
    FPortType _type;
};

#endif /* GCF_PORTINTERFACE_HXX */
