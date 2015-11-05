//#  -*- mode: c++ -*-
//#
//#  WGSettings.h: Waveform Generator control information
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

#ifndef WGSETTINGS_H_
#define WGSETTINGS_H_

//#include <APL/RTCCommon/RegisterState.h>

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class WGSettings
    {
    public:
      /**
       * Settings of the Waveform Generator
       */
      struct WGRegisterType
      {
	// MODE_OFF, MODE_CALC, MODE_SINGLE, MODE_REPEAT
	uint8  mode;         // byte  0
	uint8  phase;        // byte  1
	uint16 nof_samples;  // bytes 2,3
	uint32 freq;         // bytes 4,5,6,7
	uint32 ampl;         // bytes 8,9,10,11

	// preset field is not sent to hardware because DIAG_WG[XY]_SIZE = 8 bytes
	uint8  preset;
      };

      static const int MODE_OFF    = 0x0;
      static const int MODE_CALC   = 0x1;
      static const int MODE_SINGLE = 0x3;
      static const int MODE_REPEAT = 0x5;

      static const int PRESET_SINE        = 0;
      static const int PRESET_SQUARE      = 1;
      static const int PRESET_TRIANGLE    = 2;
      static const int PRESET_RAMP        = 3;
      static const int N_WAVEFORM_PRESETS = PRESET_RAMP + 1;

      /**
       * Constructors for a WGSettings object.
       */
      WGSettings() { }
	  
      /* Destructor for WGSettings. */
      virtual ~WGSettings() {}

      /* get reference to wg settings array */
      blitz::Array<WGRegisterType, 1>& operator()();
      
      /* get reference to the wave array (int32 in FTS2) */
      blitz::Array<int32, 2>& waveforms();

      /* preset waveform methods (int32 in FTS2) */
      static void initWaveformPresets();
      static blitz::Array<int32, 1> preset(int index);

      //RTC::RegisterState& getState() { return m_state; }

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      blitz::Array<WGRegisterType, 1> m_registers;
      blitz::Array<int32, 2>          m_waveforms;
      //RTC::RegisterState              m_state;

      static blitz::Array<int32, 2>   m_presets;
    };

    inline blitz::Array<WGSettings::WGRegisterType, 1>& WGSettings::operator()() { return m_registers; }
    inline blitz::Array<int32, 2>& WGSettings::waveforms() { return m_waveforms; }
  };
}; // namespace LOFAR

#endif /* WGSETTINGS_H_ */
