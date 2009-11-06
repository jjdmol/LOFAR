//#  Copyright (C) 2009
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
//#  $Id: Mutex.h 13919 2009-09-02 21:28:43Z romein $

#ifndef LOFAR_INTERFACE_PROCESSINGPLAN_H
#define LOFAR_INTERFACE_PROCESSINGPLAN_H

#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#include <Interface/StreamableData.h>
#include <Interface/Allocator.h>

namespace LOFAR {

namespace RTCP {

/*
 * ProcessingPlan: derives what data sets to calculate and where to store them,
 *                 given a processing scheme.
 *
 * This class is designed to be used by all parts of the pipeline. First, a generic
 * plan is defined which will be followed by CNProc. For example:
 *
 *  transform(0,inputData);
 *  transform(inputData,outputData);
 *  send(outputData);
 *
 * tells this class that inputData will be created out of nothing, it will be
 * transformed into outputData, which will be an output of the pipeline. This
 * class will automatically decide which transformations are really needed to
 * obtain the desired outputs.
 *
 * Subsequently, CNProc can ask the data sets to be assigned to arenas (assignArenas).
 * The IONProc and Storage can ask to filter only the outputs (removeNonOutputs) and
 * allocate them (allocateOutputs). 
 *
 * Functions like calculate(set) can be used by CNProc to decide whether a certain
 * transformation is necessary, and output(set) can be used to decide whether a certain
 * set is an output.
 */
class ProcessingPlan
{
  public:
    struct planlet {
      StreamableData     *set;        // 0: barrier, require this source until here
      StreamableData     *source;     // 0: depend on nothing
      bool     output;
      bool     calculate;
      int      arena;                 // -1: not allocated, >= 0: allocated

      const char         *name;           // name of planlet or data set, for logging purposes
      const char         *filenameSuffix; // for outputs: extension to use for this output

      bool isOutput() const { return output; } // for filtering
    };

    // ----- Processing steps: use these functions to describe the processing chain 

    // create a set from a source (source can be 0)
    void transform( StreamableData *source, StreamableData *set, const char *name = "" );

    // require source for something else
    void require( StreamableData *source );

    // send set (i.e. as output) to be stored in a file or directory with a certain extension
    void send( StreamableData *set, const char *extension = "" );

    // ----- Construct the plan: assign an arena to all
    //       products that have to be calculated.
    void assignArenas( bool assignAll = false );

    // gives the arena in which a data set resides.
    // -1: set not allocated
    // -2: set not found
    int arena( const StreamableData *set ) const;

    // gives the final owner of the arena, or 0 if the
    // arena is not used.
    planlet const *finalOwner( int arena ) const;

    // find a data set in the plan, or return 0 if not found
    planlet const *find( const StreamableData *set ) const;

    // number of outputs
    unsigned nrOutputs() const;

    // wipe the plan except for the outputs
    void removeNonOutputs();

    // return a copy of a certain output
    StreamableData *cloneOutput( unsigned outputNr ) const;

    // allocate only the outputs, using the provided allocator
    void allocateOutputs( Allocator &allocator );

    // return whether `set' is an output
    bool output( const StreamableData *set ) const;

    // return whether `set' should be calculated
    bool calculate( const StreamableData *set ) const;

    // the plan
    std::vector<planlet> plan;
};

inline void ProcessingPlan::transform( StreamableData *source, StreamableData *set, const char *name ) {
  // assert: source in plan
  assert( !source || arena( source ) > -2 );

  planlet p;

  p.set = set;
  p.source = source;
  p.calculate = false;
  p.output = false;
  p.arena = -1;
  p.name = name;

  plan.push_back( p );
}

inline void ProcessingPlan::require( StreamableData *source ) {
  transform( source, 0 );
  plan.back().calculate = true;
}

inline void ProcessingPlan::send( StreamableData *set, const char *extension ) {
  require( set ); // fake planlet to indicate we need this set

  // the entry we just created is an output -- configure it as such
  plan.back().output = true;
  plan.back().filenameSuffix = extension;

  // recursively set calculate=true for this item and all ancestors.
  for(unsigned i = plan.size() - 1; plan[i].source; ) {

    for( unsigned j = 0; j < i; j++ ) {
      if( plan[j].set == plan[i].source ) {
        plan[j].calculate = true;

        // continue with j
        i = j;
        break;
      }

      // assert that the source of j is in the plan before j
      // transform() guarantees this, but we REALLY don't want
      // this loop to stall, so we recheck.
      assert( j < i-1 );
    }
  }
}

inline void ProcessingPlan::assignArenas( bool assignAll ) {
  /* complexity is O(n^3), but n is small for the time being */

  int maxArena = -1;

  for( unsigned i = 0; i < plan.size(); i++ ) {
    /* determine an arena for planlet i */
    int a;

    if( !plan[i].set ) {
      continue;
    }

    if( !plan[i].calculate && !assignAll ) {
      continue;
    }

    /* assign first arena that is valid */
    /* NOTE: potential optimisation: assign an arena that does not
       have to be expanded in size */
    for( a = 0; a <= maxArena; a++ ) {
      /* does arena a work out? */
      unsigned j;
      const planlet *owner = finalOwner( a );

      for( j = i; j < plan.size(); j++ ) {
        if( !plan[j].calculate ) {
          continue;
        }

        if( plan[j].source == owner->set ) {
          /* this arena is needed by i or further down the road */
          break;
        }
      }

      if( j == plan.size() ) {
        /* no conflicts */
        plan[i].arena = a;
        break;
      }
    }

    if( a > maxArena ) {
      /* no arena found */
      plan[i].arena = ++maxArena;
    }
  }
}

inline int ProcessingPlan::arena( const StreamableData *set ) const {
  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].set == set ) {
      return plan[i].arena;
    }
  }

  return -2;
}

inline ProcessingPlan::planlet const *ProcessingPlan::finalOwner( int arena ) const {
  const planlet *owner = 0;

  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].arena == arena ) {
      owner = &plan[i];
    }
  }

  return owner;
}

inline ProcessingPlan::planlet const *ProcessingPlan::find( const StreamableData *set ) const {
  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].set == set ) {
      return &plan[i];
    }
  }

  return 0;
}

inline unsigned ProcessingPlan::nrOutputs() const {
  unsigned n = 0;

  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].output ) {
      n++;
    }
  }

  return n;
}

inline void ProcessingPlan::removeNonOutputs() {
  plan.erase(
    std::remove_if( plan.begin(), plan.end(), 
      std::not1( std::mem_fun_ref( &planlet::isOutput ) ) ),
    plan.end()
  );
}

inline StreamableData *ProcessingPlan::cloneOutput( unsigned outputNr ) const {
  unsigned n = 0;

  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].output ) {
      if (++n == outputNr) {
        return plan[i].source->clone();
      }
    }
  }

  return 0;
}

inline void ProcessingPlan::allocateOutputs( Allocator &allocator ) {
  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].output ) {
      plan[i].source->allocate( allocator );
    }
  }
}

inline bool ProcessingPlan::output( const StreamableData *set ) const {
  // need to iterate to find the send command, the transform
  // that makes the set itself does not contain the output flag

  if( !set ) {
    return false;
  }

  for( unsigned i = 0; i < plan.size(); i++ ) {
    if( plan[i].source == set && plan[i].output ) {
      return true;
    }
  }

  return false;
}

inline bool ProcessingPlan::calculate( const StreamableData *set ) const {
  const planlet *p = find( set );

  return p && p->calculate;
}

}

}

#endif
