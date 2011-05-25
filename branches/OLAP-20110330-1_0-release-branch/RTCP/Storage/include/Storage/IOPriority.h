//#  IOPriority.h: define some Linux specific IO priority macro's
//#
//#  Copyright (C) 2002-2005
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
//# $Id:$

#ifndef LOFAR_STORAGE_IOPRIORITY_H
#define LOFAR_STORAGE_IOPRIORITY_H

#define IOPRIO_BITS		(16)
#define IOPRIO_CLASS_SHIFT	(13)
#define IOPRIO_PRIO_MASK	((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask)	((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)	((mask) & IOPRIO_PRIO_MASK)
#define IOPRIO_PRIO_VALUE(class, data)	(((class) << IOPRIO_CLASS_SHIFT) | data)

#include <pwd.h>
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/resource.h>

#if defined __linux__ 
#include <linux/version.h>
#endif

enum {
        IOPRIO_WHO_PROCESS = 1,
        IOPRIO_WHO_PGRP,
        IOPRIO_WHO_USER,
};


enum {
        IOPRIO_CLASS_NONE,
        IOPRIO_CLASS_RT,
        IOPRIO_CLASS_BE,
        IOPRIO_CLASS_IDLE,
};


inline int ioprio_set(int which, int who, int ioprio)
{
#if defined __linux__
  #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13))
    return syscall(SYS_ioprio_set, which, who, ioprio);
  #else
    return -1;
  #endif
#else
  return -1;
#endif
}

inline int ioprio_get(int which, int who)
{
#if defined __linux__
  #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13))
    return syscall(SYS_ioprio_get, which, who);
  #else
    return -1;
  #endif
#else
  return -1;
#endif
}

#endif
