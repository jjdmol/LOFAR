//#  ObjectId.cc: Implementation of the Object-Id key generator
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

#include <PL/ObjectId.h>

/****************************************************************************
 * The methods get_random_fd() and get_random_bytes() were taken from       *
 * gen_uuid.c which is part of the linux package e2fsprogs. These sources   *
 * may be distributed under the terms of the GNU Library General Public     *
 * License.                                                                 *
 ****************************************************************************/

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

/* 
 * It would probably be better to add a test in configure, but most systems
 * support srandom() and random() nowadays. (GML: 18-09-2003)
 */
#define HAVE_SRANDOM 1

#ifdef HAVE_SRANDOM
#define srand(x) 	srandom(x)
#define rand() 		random()
#endif

static int get_random_fd(void)
{
  struct timeval	tv;
  static int	fd = -2;
  int		i;

  if (fd == -2) {
    gettimeofday(&tv, 0);
    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
      fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
    srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
  }
  /* Crank the random number generator a few times */
  gettimeofday(&tv, 0);
  for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
    rand();
  return fd;
}


/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void get_random_bytes(void *buf, int nbytes)
{
  int i, n = nbytes, fd = get_random_fd();
  int lose_counter = 0;
  unsigned char *cp = (unsigned char *) buf;

  if (fd >= 0) {
    while (n > 0) {
      i = read(fd, cp, n);
      if (i <= 0) {
	if (lose_counter++ > 16)
	  break;
	continue;
      }
      n -= i;
      cp += i;
      lose_counter = 0;
    }
  }
	
  /*
   * We do this all the time, but this is the only source of
   * randomness if /dev/random/urandom is out to lunch.
   */
  for (cp = (unsigned char *) buf, i = 0; i < nbytes; i++)
    *cp++ ^= (rand() >> 7) & 0xFF;
  return;
}

    
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      Implementation of class methods                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
    
namespace LCS
{
  namespace PL
  {
    
    ObjectId::ObjectId() : itsIsInitialized(false)
    {
    }


    const ObjectId::oid_t& ObjectId::get() const
    {
      if (!itsIsInitialized) {
	init();
	itsIsInitialized = true;
      }
      return itsOid;
    }


    void ObjectId::set(const ObjectId::oid_t& aOid)
    {
      itsOid = aOid;
      itsIsInitialized = true;
    }


    void ObjectId::init() const
    {
      get_random_bytes(&itsOid, sizeof(itsOid));
    }

  } // namespace PL

} // namespace LCS
