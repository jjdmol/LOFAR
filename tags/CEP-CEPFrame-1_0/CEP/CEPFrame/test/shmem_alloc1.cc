//  shmem_alloc1.cc:
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$


#include <Common/shmem/shmem_alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARRAY_SIZE 1000

void usage_exit(char* progname)
{
  fprintf(stderr, "%s: [-log | -linear]\n", progname);
  exit(1);
}

int main(int argc, char** argv)
{
  int i, j, k;
  int* iarray[MAX_ARRAY_SIZE];
  int totalused = 0;
  int log = 0;
  int result = 0;
  int array_size = 0;

  shmem_init();

  if (argc != 2)
  {
    usage_exit(argv[0]);
  }

  if (!strncmp(argv[1], "-log", 4))
  {
    j = 1;
    log = 1;
    array_size = 20;
  }
  else if (!strncmp(argv[1], "-linear", 7))
  {
    j = 333;
    log = 0;
    array_size = MAX_ARRAY_SIZE;
  }
  else usage_exit(argv[0]);

  shmem_malloc_stats();

  for (i = 0; i < array_size; i++)
  {
    /* allocate ever larger blocks */
    if (log) j *= 2;

    iarray[i] = (int*)shmem_malloc(j*sizeof(int));

    fprintf(stderr, "shmem_id = %d\nshmem_offset = %ld\n",
	    shmem_id(iarray[i]),
	    (long) shmem_offset(iarray[i]));

    totalused += j*sizeof(int);

    fprintf(stderr, "total used = %.2f MB\n", ((double)totalused)/(1024.0*1024.0));

    if (iarray[i] == NULL)
    {
      shmem_malloc_stats();
      perror("shmalloc");
      exit(1);
    }

    /* and initialize them! otherwise they are not mapped */
    for (k = 0; k < j; k++)
    {
      iarray[i][k] = k;
    }
    int jj = (log  ?  1 : j);
    for (int ii = 0; ii < i; ii++)
    {
        if (log) jj *= 2;
	for (k = 0; k < jj; k++)
	{
	    if (iarray[ii][k] != k)
	    {
		fprintf(stderr, "error at %d %d\n", ii, k);
		result = 1;
		break;
	    }
	}
    }
  }

  shmem_malloc_stats();

  for (i = 0; i < array_size; i++)
  {
    shmem_free((void*)iarray[i]);
  }

  shmem_malloc_stats();

  return result; 
}
