//  shmem_alloc2.cc:
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
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10000

int  client(void);
void server(void);

void usage_exit(char* progname)
{
    fprintf(stderr, "usage: %s [-client | -server]\n", progname);
    exit(1);
}

int main(int argc, char** argv)
{
    int result = 0;

    if (argc != 2) usage_exit(argv[0]);

    if (!strncmp(argv[1], "-client", 7))
    {
	result = client();
    }
    else if (!strncmp(argv[1], "-server", 7))
    {
	server();
    }
    else usage_exit(argv[0]);
    
    return result;
}

int client(void)
{
    int*   mem = NULL;
    int    i;
    int    id;
    long   offset;
    pid_t  server_pid;
    int    result = 0;
    
    shmem_init();

    // read shmid and offset from stdin
    scanf("%d %ld %d", &id, &offset, &server_pid);

    mem = (int*)shmem_connect(id, (size_t) offset);

    if (NULL == mem)
    {
	fprintf(stderr, "shmem_connect failed\n");
	exit(1);
    }

    for (i = 0; i < ARRAY_SIZE; i++)
    {
        printf("mem[%d] = %d\n", i, mem[i]);
	if (mem[i] != i)
	{
	    printf("mismatch\n");
	    result = 1;
	}
    }

    // signal server that I've finished
    kill(server_pid, SIGCONT);
    
    return result;
}

void server(void)
{
    int* mem = NULL;
    int  i;

    shmem_init();

    mem = (int*)shmem_malloc(ARRAY_SIZE*sizeof(int));
#if 1
    mem = (int*)shmem_malloc(ARRAY_SIZE*sizeof(int)*10);
    mem = (int*)shmem_malloc(ARRAY_SIZE*sizeof(int));
#endif
    if (NULL == mem)
    {
        fprintf(stderr, "shmalloc failed\n");
	exit(1);
    }

    for (i = 0; i < ARRAY_SIZE; i++)
    {
	mem[i] = i;
    }
    
    printf("%d %ld %d\n", shmem_id(mem), (long) shmem_offset(mem), getpid());
    fflush(stdout);
    
    // suspend myself
    kill(getpid(), SIGSTOP);

    // client will send SIGCONT signal
}
