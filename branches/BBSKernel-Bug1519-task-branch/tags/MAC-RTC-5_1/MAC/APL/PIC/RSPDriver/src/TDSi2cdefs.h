//#  -*- mode: c++ -*-
//#
//#  TDSi2cdefs.h: i2c defintions for the TDS board.
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

#ifndef TDSI2CDEFS_H_
#define TDSI2CDEFS_H_

#include <Common/LofarTypes.h>

#define TDS_PROGRAMPLLS_RESULT_SIZE 904
#define TDS_160MHZ_RESULT_SIZE      34 // (34 + TDS_PROGRAMPLLS_RESULT_SIZE)
#define TDS_200MHZ_RESULT_SIZE      34 // (34 + TDS_PROGRAMPLLS_RESULT_SIZE)
#define TDS_OFF_RESULT_SIZE         31

namespace LOFAR {
  namespace RSP {

    extern uint8 tds_160MHz_result[TDS_160MHZ_RESULT_SIZE];
    extern uint8 tds_200MHz_result[TDS_200MHZ_RESULT_SIZE];
    extern uint8 tds_off_result[TDS_OFF_RESULT_SIZE];
    extern uint8 tds_programPLLs_result[TDS_PROGRAMPLLS_RESULT_SIZE];

  };
};
     
#endif /* TDSI2CDEFS_H_ */
