//# IncoherentStokes.cu
//#
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

// \file

// This file contains a CUDA implementation of the GPU kernel for the Incoherent
// Stokes part of the beam-former pipeline. It adds the Stokes parameters of the
// station beams without correcting for delays; hence the term \e
// incoherent. For Stokes \e I this means calculating the sum of squares of the
// complex voltages. Compare this with Coherent Stokes, where you calculate the
// square of sums of the complex voltages.

#if !(INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR >= 1)
#error Precondition violated: INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR >= 1
#endif

#if !(NR_CHANNELS >= 1)
#error Precondition violated: NR_CHANNELS >= 1
#endif

#if !(NR_INCOHERENT_STOKES == 1 || NR_INCOHERENT_STOKES == 4)
#error Precondition violated: NR_INCOHERENT_STOKES == 1 || NR_INCOHERENT_STOKES == 4
#endif

#if !(NR_SAMPLES_PER_CHANNEL && NR_SAMPLES_PER_CHANNEL % INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR == 0)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL && NR_SAMPLES_PER_CHANNEL % INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR == 0
#endif

#if !(NR_STATIONS >= 1)
#error Precondition violated: NR_STATIONS >= 1
#endif

// 3-D output array of incoherent stokes values. Its dimensions are
// <tt>[stokes][time][channels]</tt>, where <tt>[stokes]</tt> can be either 1
// (Stokes \e I), or 4 (Stokes \e I, \e Q, \e U, and \e V).
typedef float (*IncoherentStokesType)[NR_INCOHERENT_STOKES][NR_SAMPLES_PER_CHANNEL / INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR][NR_CHANNELS];

// 4-D array of input samples. Note that, actually, the data is 3-D
// (<tt>[stations][channels][time]</tt>), but the time dimension has been split
// in two parts to make time integration easier.
typedef float4 (*InputType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL / INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR][INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR];

// Compute the \e incoherent Stokes parameters. The incoherent Stokes
// parameters are calculated by adding the Stokes parameters of the station
// beams without correcting for delays due to tied-array beam forming.
//
// Pre-processor input symbols (some are tied to the execution configuration)
// Symbol                  | Valid Values            | Description
// ----------------------- | ----------------------- | -----------
// INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR | >= 1  | number of samples to sum into one output sample
// NR_CHANNELS             | >= 1                    | number of frequency channels per subband
// NR_INCOHERENT_STOKES    | 1, 4                    | number of Stokes parameters; either 1 (\e I) or 4 (\e I,\e Q,\e U,\e V)
// NR_SAMPLES_PER_CHANNEL  | multiple of INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR | number of input samples per channel
// NR_STATIONS             | >= 1                    | number of antenna fields
//
// \param stokes [out] 3-D array of incoherent Stokes parameters
// \param input [in] 4-D array of input samples
extern "C" __global__ void incoherentStokes(IncoherentStokesType stokes,
                                            const InputType input)
{
  /* uint time = get_global_id(0); */
  /* uint channel = get_global_id(1); */
  uint time = blockIdx.x * blockDim.x + threadIdx.x;
  uint channel = blockIdx.y * blockDim.y + threadIdx.y;

  if (time >= NR_SAMPLES_PER_CHANNEL / INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR)
    return;

  float stokesI = 0;
#if NR_INCOHERENT_STOKES == 4
  float stokesQ = 0, halfStokesU = 0, halfStokesV = 0;
#endif

  for (uint station = 0; station < NR_STATIONS; station++) {
    for (uint t = 0; t < INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR; t++) {
      /* float4 sample = (*input)[station][channel][time][t]; */
      /* float2 X = sample.xy; */
      /* float2 Y = sample.zw; */
      float4 sample = (*input)[station][channel][time][t];
      float2 X = make_float2(sample.x, sample.y);
      float2 Y = make_float2(sample.z, sample.w);
      float powerX = X.x * X.x + X.y * X.y;
      float powerY = Y.x * Y.x + Y.y * Y.y;

      stokesI += powerX + powerY;
#if NR_INCOHERENT_STOKES == 4
      stokesQ += powerX - powerY;
      halfStokesU += X.x * Y.x + X.y * Y.y;
      halfStokesV += X.y * Y.x - X.x * Y.y;
#endif
    }
  }

  (*stokes)[0][time][channel] = stokesI;
#if NR_INCOHERENT_STOKES == 4
  (*stokes)[1][time][channel] = stokesQ;
  (*stokes)[2][time][channel] = 2 * halfStokesU;
  (*stokes)[3][time][channel] = 2 * halfStokesV;
#endif
}

