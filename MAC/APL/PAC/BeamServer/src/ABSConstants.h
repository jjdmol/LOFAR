//#
//#  ABSConstants.h: constants related to the BeamServer
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

#ifndef ABSCONSTANTS_H_
#define ABSCONSTANTS_H_

namespace ABS
{
  /**
   * Maximum number of beamlets produced by
   * the server. The number of beamlets of
   * all beams together can not be larger
   * than this value. This is always less
   * than or equal to N_SUBBANDS
   */
  static const int N_BEAMLETS = 256;
  
  /**
   * Maximum number of input subbands of the
   * system. Each spectral window should have
   * n_subbands <= N_SUBBANDS. This is always
   * greater than or equal to N_BEAMLETS.
   */
  static const int N_SUBBANDS = 256;
  
  /**
   * The number of antenna elements, an
   * element can have one or two polarizations.
   */
  static int const N_ELEMENTS      = 100;

  /**
   * Number of polarizations for each element.
   */
  static int const N_POLARIZATIONS = 2;
  
  /**
   * System sample clock frequency.
   */
  static double const SYSTEM_CLOCK_FREQ = 80e6; // 80 MHz
};
     
#endif /* ABSCONSTANTS_H_ */
