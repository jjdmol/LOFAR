//#  lofar_fstream.h:
//#
//#  Copyright (C) 2002
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

#ifndef LOFAR_COMMON_LOFAR_FSTREAM_H
#define LOFAR_COMMON_LOFAR_FSTREAM_H

// \file lofar_fstream
//

#include <fstream>

namespace LOFAR
{
  // \addtogroup Common
  // @{
  using std::ifstream;
  using std::ofstream;
  // @}
}

#ifdef MAKE_LOFAR_SYMBOLS_GLOBAL
#include <Common/lofar_global_symbol_warning.h>
using LOFAR::ifstream;
using LOFAR::ofstream;
#endif

#endif
