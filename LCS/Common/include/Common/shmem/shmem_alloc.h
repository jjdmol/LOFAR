/*  shmem_alloc.h
 *
 *  Copyright (C) 2002
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 * $Log$
 * Revision 1.3  2002/05/22 13:32:55  wierenga
 * %[BugId: 4]%
 * Fix errors and warnings found by KAI compiler.
 *
 * Revision 1.2  2002/05/22 11:44:11  wierenga
 *
 * %[BugId: 4]%
 * removed shmem_debug
 *
 * Revision 1.1  2002/05/22 11:24:38  wierenga
 * %[BugId: 4]%
 * Moved shmem_alloc code from BaseSim/src/ShMem to Common/shmem.
 *
 * Revision 1.4  2002/05/15 14:47:57  wierenga
 * New version of TH_ShMem TranportHolder.
 * Semaphore implementation working properly now.
 * Great performance on dual-processor machine ~1000M per sec.
 *
 * Revision 1.3  2002/05/08 14:22:02  wierenga
 * New synchronisation method (not using atomic.h). Cleanup of implementation.
 *
 * Revision 1.2  2002/04/04 14:20:15  wierenga
 * using atomic_inc/dec for sync after memcpy from shared memory
 *
 * Revision 1.1  2002/04/03 07:01:23  wierenga
 * Initial version of shared memory TransportHolder.
 *
 */

#ifndef _SHMEM_ALLOC_H_
#define _SHMEM_ALLOC_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Common/shmem/dlmalloc.h"
#include <stddef.h>

#ifdef HAVE_ATOMIC_H

// use atomic operations that are also used in linux kernel
// make sure it also works on SMP machines
#define __SMP__
#include <asm/atomic.h>

#else

#include <sys/types.h>
#include <unistd.h>

#endif

#define shmem_calloc               dlcalloc
#define shmem_free                 dlfree
#define shmem_cfree                dlcfree
#define shmem_malloc               dlmalloc
#define shmem_memalign             dlmemalign
#define shmem_realloc              dlrealloc
#define shmem_valloc               dlvalloc
#define shmem_pvalloc              dlpvalloc
#define shmem_mallinfo             dlmallinfo
#define shmem_mallopt              dlmallopt
#define shmem_malloc_trim          dlmalloc_trim
#define shmem_malloc_stats         dlmalloc_stats
#define shmem_usable_size          dlmalloc_usable_size
#define shmem_independent_calloc   dlindependent_calloc
#define shmem_independent_comalloc dlindependent_comalloc

#ifdef __cplusplus
extern "C"
{
#endif

/* shared memory operations */
void   shmem_init(void);
int    shmem_id(void* address);
size_t shmem_offset(void* address);
void*  shmem_connect(int shmid, size_t offset);
void   shmem_disconnect(void* segment, size_t offset);

/* semaphore typedef */
#ifdef HAVE_ATOMIC_H
typedef atomic_t shmem_cond_t;
#else
typedef struct
{
  pid_t owner;
  bool  thetry[2];
  int   turn;
  int   count;
} shmem_cond_t;
#endif

/* semaphore operations */
void shmem_cond_init(volatile shmem_cond_t* condition);
void shmem_cond_signal(volatile shmem_cond_t* condition);
void shmem_cond_wait(volatile shmem_cond_t* condition);

#ifdef __cplusplus
}
#endif
  
#endif
