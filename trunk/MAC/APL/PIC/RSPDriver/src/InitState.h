//#  -*- mode: c++ -*-
//#
//#  InitState.h: class to control initialization of all RSP boards
//#
//#  Copyright (C) 2002-2004
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

#ifndef INITSTATE_H_
#define INITSTATE_H_

#include <blitz/array.h>

namespace LOFAR {
  namespace RSP {

    /**
     * Singleton class to control station initialization
     */
    class InitState
    {
    public:

      typedef enum {
	INIT = 0,
	WRITE_TDS,
	WRITE_RSU,
	WRITE_BS,
	WRITE_RCUENABLE,
      } CurrentState;

      /*@{*/
      /**
       * Constructor/destructor
       */
      static InitState& instance();
      virtual ~InitState();
      /*@}*/

      void init(CurrentState state = INIT); // initialise the class

      /*
       * Get state.
       */
      CurrentState getState() const { return m_state; }

      void setTDSDone(int boardid, bool value = true);
      void setRSUDone(int boardid, bool value = true);
      void setBSDone(int blpid, bool value = true);
      void setRCUEnableDone(int blpid, bool value = true);

      bool isTDSDone() const;
      bool isRSUDone() const;
      bool isBSDone() const;
      bool isRCUEnableDone() const;

    private:

      /**
       * Direct construction not allowed.
       */
      InitState();

      /*
       * The current state we're in
       */
      CurrentState m_state;


      /*
       * Keep tab on each RSPBoard whether
       * initialization has completed.
       */
      blitz::Array<bool, 1> m_tds_done;
      blitz::Array<bool, 1> m_rsu_done;
      blitz::Array<bool, 1> m_bs_done;
      blitz::Array<bool, 1> m_rcuenable_done;

      /**
       * Singleton class.
       */
      static InitState* m_instance;
    };
  };
};
     
#endif /* INITSTATE_H_ */
