//#  GCF_RawPort.h: connection to a remote process
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

#ifndef GCF_RAWPORT_H
#define GCF_RAWPORT_H

#include <GCF_Defines.h>
#include "GCF_PortInterface.h"
#include "GCF_PeerAddr.h"
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

// forward declaration
class GCFTask;
class GCFPort;
class GTMTimer;

class GCFRawPort : public GCFPortInterface
{
 public:
  GCFRawPort(GCFTask& task, string& name, TPortType type, int protocol = 0);

  GCFRawPort();

  virtual ~GCFRawPort();

  void init(GCFTask& task, string& name, TPortType type, int protocol = 0 ); 

  ////////////////////// GCFPortInterface methods

   /**
   * Timer functions.
   * Upon expiration of a timer a F_TIMER_SIG will be
   * received on the port.
   */
  virtual long setTimer(long  delay_sec,
			long  delay_usec    = 0,
			long  interval_sec  = 0,
			long  interval_usec = 0,
			const void* arg     = 0);

  virtual long setTimer(double delay_seconds, 
			double interval_seconds = 0.0,
			const void*  arg        = 0);

  virtual int  cancelTimer(long   timerid,
			   const void** arg = 0);

  /*
   * @note This function is suspect of hanging machines. It
   * relies directly on the ACE implementation and there is
   * a possible bug in the ACE implementation.
   * DO NOT USE THIS FUNCTION.
   */
  virtual int  cancelAllTimers();

  virtual int  resetTimerInterval(long timerid,
				  long sec,
				  long usec = 0);


  protected:
    void schedule_disconnected();
    void schedule_close();
    void schedule_connected();

    inline bool isSlave() const {return _pMaster != 0;}
    virtual void    setMaster(GCFPort* pMaster);
    virtual int     dispatch(GCFEvent& event);
    bool findAddr(GCFPeerAddr& addr);
    friend class GCFPort; // to access the setMaster method
    virtual void handleTimeout(const GTMTimer& timer);
    friend class GTMTimer;

 private:

    /**
    * Don't allow copying of the GCFRawPort object.
    */
    typedef struct
    {
        GTMTimer* pTimer;
    } TTimer;
       
    
    GCFRawPort(const GCFRawPort&);
    GCFRawPort& operator=(const GCFRawPort&);

    GCFPort* _pMaster;
    map<unsigned long, GTMTimer*> _timers;
    typedef map<unsigned long, GTMTimer*>::iterator TTimerIter;
};

#endif
