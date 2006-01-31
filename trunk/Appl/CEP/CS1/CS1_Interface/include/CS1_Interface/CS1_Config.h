//#  CS1_Config.h: CS1 configuration file with compile-time constants
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_CS1_CONFIG_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_CS1_CONFIG_H

/* This is included by C++ and assembly files.  Do not put anything but
   constants here! */

#define NR_STATIONS		   37
#define NR_POLARIZATIONS	   2
#define NR_SUBBAND_CHANNELS	   256
#define NR_TAPS			   16
#define SAMPLE_RATE		   200704 /* 155648 */

#if SAMPLE_RATE % (NR_TAPS * NR_SUBBAND_CHANNELS) != 0
#error  "Bad value for SAMPLE_RATE"
#endif

#define NR_INPUT_SAMPLES	   ((NR_TAPS - 1) * NR_SUBBAND_CHANNELS + SAMPLE_RATE)
#define NR_SAMPLES_PER_INTEGRATION (SAMPLE_RATE / NR_SUBBAND_CHANNELS)
#define CHANNEL_BANDWIDTH	   NR_SAMPLES_PER_INTEGRATION
#define NR_BASELINES		   (NR_STATIONS * (NR_STATIONS + 1) / 2)

#endif
