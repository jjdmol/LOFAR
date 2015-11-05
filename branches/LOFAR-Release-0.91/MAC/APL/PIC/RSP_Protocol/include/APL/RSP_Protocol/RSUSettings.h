//#  -*- mode: c++ -*-
//#
//#  RSUSettings.h: RSU control information
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

#ifndef RSUSETTINGS_H_
#define RSUSETTINGS_H_

#include <APL/RSP_Protocol/EPA_Protocol.ph>
//#include <APL/RTCCommon/RegisterState.h>

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class RSUSettings {
    public:
      RSUSettings() {}
      virtual ~RSUSettings() {}

      class ResetControl {
      public:
	ResetControl() : m_value(0) { }

	// no virtual to prevent creation of virtual pointer table
	// which adds to the size of the struct
	~ResetControl() {}

	typedef enum {
	  CTRL_OFF	= 0x00,
	  CTRL_SYNC	= 0x01,
	  CTRL_CLEAR	= 0x02,
	  CTRL_RESET	= 0x04
	} ResetMode;

	void  setRaw(uint8 raw) { m_value = raw; }
	uint8 getRaw() const { return m_value; }

	/**
	 * (re)set sync bit on RSU
	 */
	void setSync(bool value) {
	  if (value) m_value |= CTRL_SYNC;  // set SYNC bit
	  else 	   m_value &= ~CTRL_SYNC; // clear SYNC bit
	}
	bool getSync() const { return (m_value & CTRL_SYNC); }

	/**
	 * (re)set reset bit on RSU
	 */
	void setReset(bool value) {
	  if (value) m_value |= CTRL_RESET;  // set RESET bit
	  else 	   m_value &= ~CTRL_RESET; // clear RESET bit
	}
	bool getReset() const { return (m_value & CTRL_RESET); }

	/**
	 * (re)set clear bit on RSU
	 */
	void setClear(bool value) {
	  if (value) m_value |= CTRL_CLEAR;  // set CLEAR bit
	  else 	   m_value &= ~CTRL_CLEAR; // clear MASK bit
	}
	bool getClear() const { return (m_value & CTRL_CLEAR); }

      private:
	uint8	m_value;	// register is only 1 byte.
      }; // class ResetControl

    public:
      /* get reference settings array */
      blitz::Array<ResetControl, 1>& operator()();

      //RTC::RegisterState& getState();

      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      /*
       * dimensions: N_RSPBOARDS
       */
      blitz::Array<ResetControl, 1> m_registers;
      //RTC::RegisterState            m_state;
    };
  
    inline blitz::Array<RSUSettings::ResetControl, 1>& RSUSettings::operator()() { 
      return m_registers; 
    }

    //inline RTC::RegisterState& RSUSettings::getState() {
    //return m_state;
    //}

  }; // namespace RSP_Protocol
}; // namespace LOFAR

#endif /* RSUSETTINGS_H_ */
