//#  ArenaMapping.h
//#
//#  Copyright (C) 2008
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
//#  $Id: Scheduling.h 12462 2009-01-16 13:47:41Z romein $

#ifndef LOFAR_CNPROC_ARENAMAPPING_H
#define LOFAR_CNPROC_ARENAMAPPING_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/Allocator.h>

namespace LOFAR {
namespace RTCP {

class ArenaMapping {
  public:
    virtual ~ArenaMapping();

    void addDataset( const unsigned dataset, const size_t size, const unsigned arena );
    void moveDataset( const unsigned dataset, const unsigned arena );

    unsigned nrArenas() const;
    unsigned nrDatasets() const;

    void createAllocators();
    Allocator *allocatorOf( const unsigned dataset ) const;

  private:
    struct mapping {
      unsigned dataset;
      size_t size;
      unsigned arena;

      Allocator *allocator;
    };

    std::vector<struct mapping> itsMapping;
    std::vector<Arena *> itsArenas;

    size_t arenaSize( const unsigned arena ) const;
};

inline ArenaMapping::~ArenaMapping()
{
  for( unsigned i = 0; i < itsMapping.size(); i++ ) {
    delete itsMapping[i].allocator;
  }
  itsMapping.clear();

  for( unsigned i = 0; i < itsArenas.size(); i++ ) {
    delete itsArenas[i];
  }
  itsArenas.clear();
}

inline void ArenaMapping::addDataset( const unsigned dataset, const size_t size, const unsigned arena )
{
  struct mapping m;

  m.dataset = dataset;
  m.size = size;
  m.arena = arena;
  m.allocator = 0;

  itsMapping.push_back( m );
}

inline void ArenaMapping::moveDataset( const unsigned dataset, const unsigned arena )
{
  for( unsigned i = 0; i < itsMapping.size(); i++ ) {
    if( dataset == itsMapping[i].dataset ) {
      itsMapping[i].arena = arena;
      break;
    }
  }
}

inline size_t ArenaMapping::nrArenas() const
{
  size_t maxArena = 0;

  if( itsMapping.size() == 0 ) {
    return 0;
  }

  for( unsigned i = 0; i < itsMapping.size(); i++ ) {
    const unsigned arena = itsMapping[i].arena;

    if( arena > maxArena ) {
      maxArena = arena;
    }
  }

  return maxArena + 1;
}

inline size_t ArenaMapping::nrDatasets() const
{
  return itsMapping.size();
}


inline size_t ArenaMapping::arenaSize( const unsigned arena ) const
{
  size_t neededSize = 0;

  for( unsigned i = 0; i < itsMapping.size(); i++ ) {
    const size_t size = itsMapping[i].size;

    if( itsMapping[i].arena != arena ) {
      continue;
    }

    if( size > neededSize ) {
      neededSize = size;
    }
  }

  return neededSize;
}

inline void ArenaMapping::createAllocators()
{
  const size_t arenas = nrArenas();
  const size_t datasets = nrDatasets();

  // create arenas
  itsArenas.clear();
  for( unsigned arena = 0; arena < arenas; arena++ ) {
    itsArenas.push_back( new MallocedArena( arenaSize( arena ), 32 ) );
  }

  // create allocators
  for( unsigned dataset = 0; dataset < datasets; dataset++ ) {
    itsMapping[dataset].allocator = new SparseSetAllocator( *itsArenas[itsMapping[dataset].arena] );
  }
}

inline Allocator *ArenaMapping::allocatorOf( const unsigned dataset ) const
{
  for( unsigned i = 0; i < itsMapping.size(); i++ ) {
    if( itsMapping[i].dataset == dataset ) {
      return itsMapping[i].allocator;
    }
  }

  return 0;
}


} // namespace RTCP
} // namespace LOFAR

#endif
