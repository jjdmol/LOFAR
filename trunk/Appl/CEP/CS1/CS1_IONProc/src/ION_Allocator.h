//# ION_Allocator.h: Class that allocates memory in the large-TLB area of the
//# I/O Node
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_CS1_ION_PROC_ION_ALLOCATOR_H
#define LOFAR_CS1_ION_PROC_ION_ALLOCATOR_H

#include <Common/Allocator.h>
#include <CS1_Interface/SparseSet.h>
#include <map>
#include <pthread.h>

namespace LOFAR {

class ION_Allocator: public Allocator
{
  public:
    // Clone the allocator.
    virtual ION_Allocator *clone() const;
    
    // Allocate memory.
    virtual void *allocate(size_t nbytes, size_t alignment = 1);
    
    // Deallocate memory.
    virtual void deallocate(void *);

  private:
    static pthread_mutex_t	    mutex;
    static SparseSet<char *>	    freeList;
    static std::map<char *, size_t> sizes;
};

// @}

} // end namespace LOFAR

#endif
