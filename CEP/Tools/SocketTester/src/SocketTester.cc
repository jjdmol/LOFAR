//#  SocketTester.cc: Testprogram for testing BG socket problems
//#
//#  Copyright (C) 2002-2004
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
//#  Socket tester program
//#  intended to test complex communication topologies between
//#  two or three sets of "nodes".
//#  Especially written to test Linux - BGL - Linux topologies
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <Common/lofar_sstream.h>
#include <Common/Net/Socket.h>
#include <unistd.h>			/* usleep */

#ifdef HAVE_MPI

#ifdef HAVE_SCAMPI
#define THREAD_SAFE
#endif

#include <mpi.h>

#define SENDER        0      /* boolean */
#define ACKTAG        734
#define LISTENER      1

#define IP_LEN	      16
#define BUFLEN	      128

namespace LOFAR {

//
// readconfig(myrank, nrnodes, ...)
//
// Reads in my parameters from the argv[0].cfg file.
//
void read_config(int32  myrank, 
		 int32  nodes, 
		 int32  *ackMode, 
		 int32  *sleeptime, 
		 int32  *runs, 
		 int32  *mymode, 
		 char   *myip, 
		 int32  *myport) {
  
  FILE *cfg = fopen("SocketTester.cfg", "r");
  fscanf(cfg, "ackMode %i", ackMode);
  printf("ackMode = %i\n", *ackMode);
  fscanf(cfg, "sleeptime %i", sleeptime);
  printf("sleeptime = %i\n", *sleeptime);
  fscanf(cfg, "runs %i", runs);
  printf("runs = %i\n", *runs);

  char anip[IP_LEN];
  int32 anode, amode;
  for(int32 n = 1; n < nodes; n++) {
    fscanf(cfg, "node%3i %1i %s  %i", &anode, &amode, anip, myport);
    if (anode == myrank) {
      *mymode = amode;
      strcpy(myip, anip);
      printf("node: %i     Mode: %1i    IP: %s    Port: %i", myrank, *mymode, myip, *myport);
    }
  }

  fclose (cfg);
}

//
// helper function for easy conversion of numbers to strings
//
template <class type>
inline std::string to_string(const type& value)
{
    std::ostringstream streamOut;
    streamOut << value;
    return streamOut.str();
}

} //namespace LOFAR

using namespace LOFAR;
//
// main
//
int main(int32 argc, char*argv[]) {

  MPI_Init(&argc, &argv);

  // Get info about MPI environment
  int32 myrank, nodes;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank); 
  MPI_Comm_size(MPI_COMM_WORLD, &nodes); 

  // Get mpi settings for me for this run.
  int32 mymode, myport;
  int32 ackMode;
  int32 sleeptime;
  int32 runs;
  char  myip[IP_LEN];
  read_config(myrank,nodes,&ackMode,&sleeptime,&runs,&mymode,&myip[0],&myport);

  if (myrank != 0) { /* rank 0 is the master, see below */
    Socket *mysock;

    /* Make connections */
    if (LISTENER) {
      Socket	listensock;
      listensock.initServer(to_string(myport));
      mysock = listensock.accept(-1);
    } else {
      mysock = new Socket("clientSocket", 
			  string(myip), 
		          to_string(myport));
    }
    
    /* Synchronise all nodes */
    MPI_Barrier(MPI_COMM_WORLD);
    
    /* start the data transfer */
    int32  r, n;
    char   message[BUFLEN], buf[BUFLEN];
    strcpy (message, "testmessage MPI");
    int32  len = strlen(message);
    for (r = 0; r < runs; r++) {
      /* receive the broadcast of the next node to transport  */
      MPI_Bcast(&n, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
      if (n == myrank || n == -1) {
	/* perform the actual data transport */
	if (SENDER) {
	  mysock->write(message, len);
	} else { /* receiver */
	  mysock->read(&buf[0], len);
	}
      }

      if (ackMode) {
	if (n == -1) {
	  /* ackMode && alltogether; need global ack to master */
	  int32 collval, myval = 1;
	  MPI_Reduce(&myval, &collval, 1, MPI_INTEGER, MPI_LOR, 0, MPI_COMM_WORLD);
	} else {
	  /*  ackMode needs single acknowledge to master */
	  int32 ack = 1;
	  MPI_Send(&ack, 1, MPI_INTEGER, 0, ACKTAG, MPI_COMM_WORLD);
	}
      }

      usleep (sleeptime); 
    }

  } else { /* my rank == 0 */
    /* I am the master 
     * All I have to do is make the others work
     */

    /* Wait for all connections to be completed */
    MPI_Barrier(MPI_COMM_WORLD);

    int32 nextnode; /* node number of the next node that receives a trigger
		   * by an MPI_Bcast;
		   * nextnode == -1 indicates all nodes 
		   */
    int32 r, n;
    int32 recvbuf;
    MPI_Status status;
    for (r = 0; r < runs; r++) {
      for (n = 1; n < nodes; n++) {
	switch (mymode) {
	case 0:                            /* One-by-one mode */
	  nextnode = n;
          MPI_Bcast(&nextnode, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Recv(&recvbuf, 1, MPI_INTEGER, nextnode, 
					ACKTAG, MPI_COMM_WORLD, &status); /* acknowledge */
	  }
	  break;
	case 1:                            /* Random order nodes 1...nodes */
	  nextnode = 1 + (random() % nodes); 
          MPI_Bcast(&nextnode, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Recv(&recvbuf, 1, MPI_INTEGER, nextnode, ACKTAG, MPI_COMM_WORLD, &status); /* acknowledge */
	  }
	  break;
	case 2:                             /* All together mode */
	  int32 collval, myval=0;
	  nextnode = -1;                   /* will trigger all nodes */
	  n += nodes;                      /* ends the nodes loop  */
          MPI_Bcast(&nextnode, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Reduce(&myval, &collval, 1, MPI_INTEGER, MPI_LOR, 0, MPI_COMM_WORLD);
	  }
	  break;
	} // switch
      } // for nodes
    } // for runs
  } // master or not

  MPI_Finalize();
  return (0);
} // main

#endif // MPI
