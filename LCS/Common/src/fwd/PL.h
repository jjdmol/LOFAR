//#  PL.h: Forward declarations of classes in the PL package.
//#
//#  Copyright (C) 2002-2003
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

#ifndef COMMON_FWD_PL_H
#define COMMON_FWD_PL_H

#include <config.h>

namespace LOFAR
{
  namespace PL
  {

    //# Forward Declarations
    template<typename T> class TPersistentObject;
    template<typename T> class DBRep;

  } // namespace PL

} // namespace LOFAR

using LOFAR::PL::TPersistentObject;
using LOFAR::PL::DBRep;

#endif
