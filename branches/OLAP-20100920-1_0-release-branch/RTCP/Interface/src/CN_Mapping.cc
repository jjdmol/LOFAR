//#  CN_Mapping.cc: map work to cores on BG/L psets
//#
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

#include <lofar_config.h>

#include <Interface/CN_Mapping.h>

namespace LOFAR {
namespace RTCP {

unsigned CN_Mapping::mapCoreOnPset(unsigned core, unsigned pset)
{
#if defined HAVE_BGP
  // TODO: there may be better mappings for partitions larger than one midplane
  static unsigned char mapX[] = { 0, 2, 6, 4 };
  static unsigned char mapY[] = { 0, 1, 5, 4 };
  static unsigned char mapZ[] = { 0, 1, 3, 2 };

  return core ^ mapX[(pset >> 0) & 3] ^ mapY[(pset >> 2) & 3] ^ mapZ[(pset >> 4) & 3];
#else
  return core;
#endif
}

unsigned CN_Mapping::reverseMapCoreOnPset(unsigned core, unsigned pset)
{
  // just the same function
  return mapCoreOnPset(core, pset);
}


} // namespace RTCP
} // namespace LOFAR
