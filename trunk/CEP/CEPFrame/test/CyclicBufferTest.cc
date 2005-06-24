//#  CyclicBufferTest.cc: test program for CyclicBuffer implementation
//#
//#  Copyright (C) 2000, 2001
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <CEPFrame/CyclicBuffer.h>
#include <CEPFrame/Lock.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

using namespace LOFAR;

#define CYCLIC_SIZE    100
#define NWRITERTHREADS 8
#define NREADERTHREADS 8
#define NTHREADS (NREADERTHREADS + NWRITERTHREADS)
#define NLOOPS         10

// prototype
void* threadmain(void* vtid);

typedef struct
{
  int                 tid;
  CyclicBuffer<int*>* buf;
  pthread_mutex_t*    m;
  pthread_cond_t*     c;
  struct timeval      starttime;
  int*                piGlobalInt;
  pthread_mutex_t*    pGlobalMutex;
} thread_args;

static int             iGlobalInt = 0;
static pthread_mutex_t mGlobalMutex = PTHREAD_MUTEX_INITIALIZER;
static CyclicBuffer<int*> buf;

int main(int argc, char** argv)
{
  pthread_t   thread[NTHREADS];
  thread_args args[NTHREADS];
  int i, id;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  condition = PTHREAD_COND_INITIALIZER;
  struct timeval starttime;

  int myint[CYCLIC_SIZE];

  // start multiple threads, all accessing the same CyclicBuffer.
  for (i=0; i < CYCLIC_SIZE; i++)
  {
    myint[i] = i;
    id = buf.AddBufferElement(&myint[i]);

    ASSERTSTR(id == i, "id != i");
  }

  // start NTHREADS threads
  gettimeofday(&starttime, NULL);
  for (i=0; i < NTHREADS; i++)
  {
    args[i].tid = i;
    args[i].buf = &buf;
    args[i].m = &mutex;
    args[i].c = &condition;
    args[i].starttime = starttime;
    args[i].piGlobalInt = &iGlobalInt;
    args[i].pGlobalMutex = &mGlobalMutex;
    if (pthread_create(&thread[i], NULL, threadmain, &args[i]) < 0)
    {
      perror("pthread_create");
      exit(1);
    }
  }

  for (i=0; i < NTHREADS; i++)
  {
    pthread_join(thread[i], NULL);
  }

  buf.DumpState();
}

double cbt_difftime(struct timeval* s, struct timeval* e)
{
  double secdiff;
  double usecdiff;

  secdiff  = e->tv_sec - s->tv_sec;
  usecdiff = e->tv_usec - s->tv_usec;

  return secdiff + (usecdiff * 1E-6);
}

void* threadmain(void* vtid)
{
  thread_args*        args  = (thread_args*)vtid;
  int                 tid   = args->tid;
  CyclicBuffer<int*>* buf   = args->buf;
  int*                myint = 0;
  int                 i, j;
  int id;
  int*           piInt = args->piGlobalInt;
  struct timeval starttime = args->starttime;
  struct timeval endtime;

  char fname[64];
  FILE* outfile = NULL;

  // let all the threads start and settle down
  sleep(1);

  if (tid < NWRITERTHREADS)
  {
      snprintf(fname, 64, "CB.%d.write", tid);
      outfile = fopen(fname, "w");
      if (NULL == outfile)
      {
	  perror("fopen");
	  exit(1);
      }

      // THIS IS A WRITER THREAD

      for (j = 0; j < NLOOPS * NREADERTHREADS; j++)
      {
	  for (i = 0; i < CYCLIC_SIZE; i++)
	  {
	      myint = buf->GetWriteLockedDataItem(&id);

	      // access on piInt is NOT exclusive at this point!!!
	      // need to lock
	      pthread_mutex_lock(args->pGlobalMutex);
	      *myint = (*piInt)++;
	      pthread_mutex_unlock(args->pGlobalMutex);

	      gettimeofday(&endtime, NULL);
	      fprintf(outfile, "0 %f %d %d %d\n", cbt_difftime(&starttime, &endtime),
		      tid, *myint, id); fflush(outfile);

	      buf->WriteUnlockElement(id);
	  }
      }
      fclose(outfile);
  }
  else
  {
      sprintf(fname, "CB.%d.read", tid-NWRITERTHREADS);
      outfile = fopen(fname, "w");
      if (NULL == outfile)
      {
	  perror("fopen");
	  exit(1);
      }

      // THIS IS A READER THREAD
      for (j = 0; j < NLOOPS * NWRITERTHREADS; j++)
      {
	  for (i = 0; i < CYCLIC_SIZE; i++)
	  {
	      myint = buf->GetReadDataItem(&id);

	      gettimeofday(&endtime, NULL);
	      fprintf(outfile, "1 %f %d %d %d\n", cbt_difftime(&starttime, &endtime),
		      tid, *myint, id); fflush(outfile);

	      buf->ReadUnlockElement(id);
	  }
      }
      
      fclose(outfile);
  }

  return NULL;
}
