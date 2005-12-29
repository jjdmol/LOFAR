//#  -*- mode: c++ -*-
//#
//#  RCUSettings.h: RCU control information
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

#ifndef RCUSETTINGS_H_
#define RCUSETTINGS_H_

#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class RCUSettings
    {
    public:

      RCUSettings() {}
      virtual ~RCUSettings() {}

      class Control
      {
      public:

	Control() : m_value(0x00000000) {}

	// no virtual to prevent creation of virtual pointer table
	// which adds to the size of the struct
	~Control() {}

	typedef enum {
	  MODE_OFF           = 0, // 0x00000000 
	  MODE_LBL_HPF10MHZ  = 1, // 0x00017900
	  MODE_LBL_HPF30MHZ  = 2, // 0x00057900
	  MODE_LBH_HPF10MHZ  = 3, // 0x00037A00
	  MODE_LBH_HPF30MHZ  = 4, // 0x00077A00
	  MODE_HB_110_190MHZ = 5, // 0x0007A400
	  MODE_HB_170_230MHZ = 6, // 0x00079400
	  MODE_HB_210_290MHZ = 7, // 0x00078400
	} RCUMode;

	static const int N_MODES = 8;

	/**
	 * Set the mode of the receiver.
	 * @note There is no get mode, since the mapping from raw value to mode is not 1-on-1
	 */
	void setMode(RCUMode mode) {
	  m_value &= ~MODE_MASK;                 // clear mode bits
	  m_value |= (m_mode[mode % N_MODES] & MODE_MASK); // set new mode bits
	}

	/**
	 * Set the raw control bytes of a RCU
	 * Each RCU has 4 bytes:
	 *    mask      meaning    explanation
	 * 0x0000007F INPUT_DELAY Sample delay for the data from the RCU.
	 * 0x00000080 SPEC_INV    Enable spectral inversion (1) if needed.
	 *
	 * 0x00000100 LBL-EN      supply LBL antenna on (1) or off (0)
	 * 0x00000200 LBH-EN      sypply LBH antenna on (1) or off (0)
	 * 0x00000400 HB-EN       supply HB on (1) or off (0)
	 * 0x00000800 BANDSEL     low band (1) or high band (0)
	 * 0x00001000 HB-SEL-0    HBA filter selection
	 * 0x00002000 HB-SEL-1    HBA filter selection
	 *             Options : HBA-SEL-0 HBA-SEL-1 Function
	 *                           0          0      210-270 MHz
	 *                           0          1      170-230 MHz
	 *                           1          0      110-190 MHz
	 *                           1          1      all off
	 * 0x00004000 VL-EN       low band supply on (1) or off (0)
	 * 0x00008000 VH-EN       high band supply on (1) or off (0)
	 *
	 * 0x00010000 VDIG-EN     ADC supply on (1) or off (0)
	 * 0x00020000 LB-SEL-0    LBA input selection
	 * 0x00040000 LB-SEL-1    HP filter selection
	 *             Options : LB-SEL-0 LB-SEL-1 Function
	 *                           0        0    10-90 MHz + 10 MHz HPF
	 *                           0        1    30-80 MHz + 10 MHz HPF
	 *                           1        0    10-90 MHz + 30 MHz HPF
	 *                           1        1    30-80 MHz + 30 MHz HPF
	 * 0x00080000 ATT-CNT-4   on (1) is  1dB attenuation
	 * 0x00100000 ATT-CNT-3   on (1) is  2dB attenuation
	 * 0x00200000 ATT-CNT-2   on (1) is  4dB attenuation
	 * 0x00300000 ATT-CNT-1   on (1) is  8dB attenuation
	 * 0x00800000 ATT-CNT-0   on (1) is 16dB attenuation
	 *
	 * 0x01000000 PRSG        pseudo random sequence generator on (1), off (0)
	 * 0x02000000 RESET       on (1) hold board in reset
	 * 0xFC000000 TBD         reserved
	 */
	void   setRaw(uint32 raw) { m_value = raw; }
	uint32 getRaw() const { return m_value; }

	/**
	 * Enable (true) or disable (false) pseudo random sequence generator.
	 */
	void setPRSG(bool value) {
	  if (value) m_value |= PRSG_MASK;  // set PRSG bit
	  else       m_value &= ~PRSG_MASK; // clear PRSG bit
	}
	bool getPRSG() const { return (m_value & PRSG_MASK) >> (16 + 8); }

	/**
	 * Enable (true) or disable (false) reset on RCU.
	 */
	void setReset(bool value) {
	  if (value) m_value |= RESET_MASK;  // set RESET bit
	  else       m_value &= ~RESET_MASK; // clear RESET bit
	}
	bool getReset() const { return (m_value & RESET_MASK) >> (17 + 8); }

	/**
	 * Set attenuation. Valid values are 0..31 (5 bits).
	 */
	void setAttenuation(uint8 value) {

	  // useful bits should be is in lower 5 bits
	  value &= 0x1F;

	  m_value &= ~ATT_MASK;                 // clear mode bits
	  // cast value to uint32 to allow << 11, set new mode bits
	  m_value |= (((uint32)value << (11 + 8)) & ATT_MASK);
	}
	uint8 getAttenuation() const { return (m_value & ATT_MASK) >> (11 + 8); }

	/**
	 * Enable (true) or disable (false) spectral inversion.
	 */
	void setSpecinv(bool value) {
	  if (value) m_value |= SPECINV_MASK;  // set RESET bit
	  else       m_value &= ~SPECINV_MASK; // clear RESET bit
	}
	bool getSpecinv() const { return m_value & SPECINV_MASK; }

	/**
	 * Set sample delay (true time delay). Valid values are 0..127 (7 bits)
	 */
	void setDelay(uint8 value) {
	  m_value &= ~DELAY_MASK;
	  m_value |= (value  & DELAY_MASK);
	}
	uint8 getDelay() const { return m_value & DELAY_MASK; }

      private:

	// constants used to set the appropriate mode
	static const uint32 m_mode[];

	// masks used to set/get bits
	static const uint32 MODE_MASK    = 0x0007FF00;
	static const uint32 ATT_MASK     = 0x00F80000;
	static const uint32 PRSG_MASK    = 0x01000000;
	static const uint32 RESET_MASK   = 0x02000000;
	static const uint32 SPECINV_MASK = 0x00000080;
	static const uint32 DELAY_MASK   = 0x0000007F;

	uint32 m_value;
      };

      /* get reference settings array */
      blitz::Array<Control, 1>& operator()();
      blitz::Array<bool, 1>& getModifiedFlags();

    public:

      /*
       * Keep flag to track wether an RCU settings has been modified.
       */
      void setModified(int rcu, bool modified = true);
      bool getModified(int rcu) const;
      void clearModified(); // clear all flags

      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      blitz::Array<Control, 1> m_registers;
      blitz::Array<bool, 1>    m_modified;
    };
  
    inline blitz::Array<RCUSettings::Control, 1>& RCUSettings::operator()()       { return m_registers; }
    inline blitz::Array<bool, 1>&                 RCUSettings::getModifiedFlags() { return m_modified; }

  };
}; // namespace LOFAR

#endif /* RCUSETTINGS_H_ */
