//#  WGSettings.h: implementation of the WGSettings class
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

#include "WGSettings.h"
#include "Marshalling.h"
#include "EPA_Protocol.ph"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

#include <math.h>

using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace std;
using namespace blitz;

// preset waveforms for the 4 types sine, square, triangle and ramp
Array<int16, 2> WGSettings::m_presets;

void WGSettings::initWaveformPresets()
{
  m_presets.resize(N_WAVEFORM_PRESETS, N_WAVE_SAMPLES);
  
  //
  // Initialize SINE waveform
  //
  for (int i = 0; i < N_WAVE_SAMPLES; i++)
  {
    WGSettings::m_presets(PRESET_SINE, i) = (int16)(sin((double)i * 2.0 * M_PI / N_WAVE_SAMPLES) * (1<<15));
  }
  cout << "sine=" << WGSettings::m_presets(PRESET_SINE, Range::all()) << endl;

  //
  // Initialize SQUARE waveform
  //
  for (int i = 0; i < N_WAVE_SAMPLES / 2; i++)
  {
    WGSettings::m_presets(PRESET_SQUARE, i) = ((1<<15)-1);
  }
  for (int i = 0; i < N_WAVE_SAMPLES / 2; i++)
  {
    WGSettings::m_presets(PRESET_SQUARE, i + N_WAVE_SAMPLES / 2) = -(1<<15);
  }
  cout << "square=" << WGSettings::m_presets(PRESET_SQUARE, Range::all()) << endl;

  //
  // Initialize TRIANGLE waveform
  //
  for (int i = 0; i < N_WAVE_SAMPLES / 4; i++)
  {
    WGSettings::m_presets(PRESET_TRIANGLE, i) = i * (1<<8);
  }
  for (int i = 0; i < N_WAVE_SAMPLES / 2; i++)
  {
    WGSettings::m_presets(PRESET_TRIANGLE, i + N_WAVE_SAMPLES / 4)
      = ((1<<15)-1) - i * (1<<8);
  }
  for (int i = 0; i < N_WAVE_SAMPLES / 4; i++)
  {
    WGSettings::m_presets(PRESET_TRIANGLE, i + 3 * N_WAVE_SAMPLES / 4)
      = -(1<<15) + i * (1<<8);
  }
  cout << "triangle=" << WGSettings::m_presets(PRESET_TRIANGLE, Range::all()) << endl;

  //
  // Initialize RAMP waveform
  //
  for (int i = 0; i < N_WAVE_SAMPLES / 2; i++)
  {
    WGSettings::m_presets(PRESET_RAMP, i) = i * (1<<7);
  }
  for (int i = 0; i < N_WAVE_SAMPLES / 2; i++)
  {
    WGSettings::m_presets(PRESET_RAMP, i + N_WAVE_SAMPLES / 2) = -(1<<15) + i * (1<<7);
  }
  cout << "ramp=" << WGSettings::m_presets(PRESET_RAMP, Range::all()) << endl;
}

unsigned int WGSettings::getSize()
{
  return MSH_ARRAY_SIZE(m_registers, WGRegisterType);
}

unsigned int WGSettings::pack  (void* buffer)
{
  unsigned int offset = 0;
  
  MSH_PACK_ARRAY(buffer, offset, m_registers, WGRegisterType);

  return offset;
}

unsigned int WGSettings::unpack(void *buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, m_registers, WGRegisterType, 1);

  return offset;
}

Array<int16, 1> WGSettings::preset(int index)
{
  // if index is invalid, return PRESET_SINE
  if (index < 0 || index >= WGSettings::N_WAVEFORM_PRESETS) index = 0;
  
  return WGSettings::m_presets(index, Range::all());
}

