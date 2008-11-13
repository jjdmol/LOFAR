//# AxisMapping.cc: Map the cells of one axis to another
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/AxisMapping.h>
#include <Common/LofarLogger.h> 

namespace LOFAR {
namespace BBS {

  // Map one axis onto another.
  AxisMapping::AxisMapping (const Axis& from, const Axis& to)
  {
    size_t nr = from.size();
    size_t maxto = to.size() - 1;
    itsMapping.reserve (nr);
    itsCenters.reserve (nr);
    itsBorders.reserve (nr);
    size_t inx=0;
    for (size_t i=0; i<nr; ++i) {
      size_t inxn = to.find (from.center(i), true, inx).first;
      if (inxn > maxto) inxn = maxto;
      if (inxn != inx  &&  i > 0) {
        itsBorders.push_back (i);
      }
      inx = inxn;
      itsMapping.push_back (inx);
      itsCenters.push_back ((from.center(i) - to.lower(inx)) / to.width(inx));
    }
    itsBorders.push_back (nr);
  }


  const AxisMapping& AxisMappingCache::makeMapping (const Axis& from,
                                                    const Axis& to)
  {
    pair<map<AxisKey,AxisMapping>::iterator, bool> result = 
      itsCache.insert (make_pair(AxisKey(from.getId(), to.getId()),
                                 AxisMapping(from, to)));
    // Make sure it got inserted.
    ASSERT (result.second);
    // Return the new mapping.
    return result.first->second;
  }


  Location GridMapping::findLocation (AxisMappingCache& cache,
                                      const Location& location,
                                      const Grid& src,
                                      const Grid& dest)
  {
    const AxisMapping& mapx = cache.get (*src.getAxis(0), *dest.getAxis(0));
    const AxisMapping& mapy = cache.get (*src.getAxis(1), *dest.getAxis(1));
    return Location (mapx[location.first], mapy[location.second]);
  }

  Location GridMapping::findLocation (AxisMappingCache& cache,
                                      uint cellId,
                                      const Grid& src,
                                      const Grid& dest)
  {
    return findLocation (cache, src.getCellLocation(cellId), src, dest);
  }

  uint GridMapping::findCellId (AxisMappingCache& cache,
                                const Location& location,
                                const Grid& src,
                                const Grid& dest)
  {
    return dest.getCellId (findLocation (cache, location, src, dest));
  }

  uint GridMapping::findcellId (AxisMappingCache& cache,
                                uint cellId,
                                const Grid& src,
                                const Grid& dest)
  {
    return dest.getCellId (findLocation (cache, cellId, src, dest));
  }


} // namespace BBS
} // namespace LOFAR
