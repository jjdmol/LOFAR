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
#include <Common/LofarLogger.h>
#include <Common/Net/Socket.h>
#include <unistd.h>			/* usleep */

#ifdef HAVE_MPI

#ifdef HAVE_SCAMPI
#define THREAD_SAFE
#endif

#include <mpi.h>

#define ACKTAG        734

#define IP_LEN	      16

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
		 int32  *listener,  // will be used as bool
		 int32  *sender,    // will be used as bool
		 int32  *mymode, 
		 int32  *pkgsize,
		 char   *myip, 
		 int32  *myport) {
  
  printf("Read cfg file on node %i\n", myrank);
  FILE *cfg = fopen("SocketTester.cfg", "r");
  if (cfg == NULL) {
    printf("Couldn't open file %i/n", errno);
    exit(0);
  }
  fscanf(cfg, "ackMode %i\n", ackMode);
  printf("ackMode = %i\n", *ackMode);
  fscanf(cfg, "sleeptime %i\n", sleeptime);
  printf("sleeptime = %i\n", *sleeptime);
  fscanf(cfg, "runs %i\n", runs);
  printf("runs = %i\n", *runs);
  fscanf(cfg, "listener %i\n", listener);
  printf("listener = %i\n", *listener);
  fscanf(cfg, "sender %i\n", sender);
  printf("sender = %i\n", *sender);
  fscanf(cfg, "mode %i\n", mymode);
  printf("mode = %i\n", *mymode);
  fscanf(cfg, "pkgsize %i\n", pkgsize);
  printf("pkgsize = %i\n", *pkgsize);

  char anip[IP_LEN];
  int32 anode, aport;
  for(int32 n = 1; n < nodes; n++) {
    fscanf(cfg, "node%3i %i %s\n", &anode, &aport, anip);
    printf("READ: node: %i     Port: %i IP: %s    \n", anode, aport, anip);
    if (anode == myrank) {
      *myport = aport;
      strcpy(myip, anip);
      printf("USE:  node: %i     Mode: %1i    IP: %s    Port: %i\n", 
	     myrank, *mymode, myip, *myport);
    }
  }

  fclose (cfg);
  printf("Finished reading cfg file\n");
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
  

  if (argc !=2) {
    printf("specify listenermode (0/1) as first command line argument");
    //exit(1);
  }
  bool listenermode = argv[1];
  printf("Listenermode : %i\n", listenermode);


  INIT_LOGGER("SocketTester");
  MPI_Init(&argc, &argv);

  // Get info about MPI environment
  int32 myrank=0; 
  int32 nodes=1;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank); 
  printf("MyRank %i\n",myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &nodes); 
  int  namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(processor_name,&namelen);
  printf("Process %d on %s\n", myrank, processor_name);

  // Get mpi settings for me for this run.
  int32 mymode, myport;
  int32 ackMode;
  int32 sleeptime;
  int32 runs, pkgsize;
  int32 sender,listener; // interpreted as bool
  char  myip[IP_LEN];
  sleep(5*myrank);
  read_config(myrank,nodes,&ackMode,&sleeptime,&runs,
	      &listener, & sender, &mymode, &pkgsize, &myip[0],&myport);

  LOG_TRACE_CALC_STR("runs = " << runs);
  LOG_TRACE_CALC_STR("sleeptime = " << sleeptime);
  LOG_TRACE_CALC_STR("trigger mode = " << mymode);

  if (myrank != 0) { /* rank 0 is the master, see below */
    Socket *mysock;

    /* Make connections */
    if (listener) {
      LOG_TRACE_FLOW_STR("Start listener");
      Socket	listensock("serverSocket");
      listensock.initServer(toString(myport));
      if (!listensock.ok()) {
	LOG_TRACE_FLOW_STR("Could not start listener " 
			   << listensock.errstr());
	exit(1);
      }
      listensock.setBlocking(true);
      mysock = listensock.accept(-1);
    } else {
      LOG_TRACE_FLOW_STR("Start Socket");
      mysock = new Socket("clientSocket", 
			  string(myip), 
		          to_string(myport));
      if (!mysock->ok()) {
	LOG_ERROR("Client socket creation failed");
	exit(0);
      }
      mysock->setBlocking(true);
      mysock->connect(10000);

    }
    LOG_TRACE_FLOW_STR("Finished socket connections");
    /* Synchronise all nodes */
    MPI_Barrier(MPI_COMM_WORLD);
    
    /* start the data transfer */
    int32  r, n;
    double totalsent = 0.;  // argegated sent data
    double totalmissed = 0.; // aggregated missed data
    double startTime=MPI_Wtime();
    int32  sent;
    char*  message = new char[pkgsize];
    char*  buf     = new char[pkgsize];
    int32 startmeasurement = runs/10;
 
    strcpy (message, "testmessage SocketTester");
    LOG_TRACE_FLOW_STR("start runs loop " << runs);
      
    for (r = 0; r < runs; r++) {
//       if (r == startmeasurement) {
// 	startTime = MPI_Wtime();
// 	totalsent = 0.;
// 	totalmissed = 0.;
//       }
      LOG_TRACE_FLOW_STR("Wait for BCast " << r);
      /* receive the broadcast of the next node to transport  */
      n=myrank;
      //!MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
      LOG_TRACE_FLOW_STR("Received BCast " << n);
      if (n == myrank || n == -1) {
	/* perform the actual data transport */
	if (sender) {
	  sent = mysock->write(message, pkgsize);
	  totalsent += sent;
	  totalmissed += (sent - pkgsize);
	} else { /* receiver */
	  sent =  mysock->read(&buf[0], pkgsize) ;
	  totalsent += sent;
	  totalmissed += (sent - pkgsize);
	}
      }

      if (ackMode) {
	if (n == -1) {
	  /* ackMode && alltogether; need global ack to master */
	  int32 collval, myval = 1;
	  MPI_Reduce(&myval, &collval, 1, MPI_INT, MPI_LOR, 0, MPI_COMM_WORLD);
	} else {
	  /*  ackMode needs single acknowledge to master */
	  int32 ack = 1;
	  MPI_Send(&ack, 1, MPI_INT, 0, ACKTAG, MPI_COMM_WORLD);
	}
      }

      usleep (sleeptime); 
    }
    cout << endl;
    double endTime = MPI_Wtime();
    cout << "   Starttime " << startTime
	 << "   EndTime " << endTime
         << "   runs "    << runs
	 << "   Buflen " << pkgsize 
	 << "   totalsent " << totalsent
         << "   (missed " << totalmissed << ")"
	 << endl;
    double totaltime = endTime - startTime;
    cout << "total sent = " << totalsent ;
    totalsent *= 8./(1024.*1024.*1024.); // in Gbits
    cout << "(" << totalsent << " Gbit)" 
	 << "   totaltime = " << totaltime 
	 << endl;
    cout << "Client average transfer rate = "
	 << totalsent / totaltime
	 << "Gbps per node " << endl;
    
  } else { /* my rank == 0 */
    /* I am the master 
     * All I have to do is make the others work
     */

    LOG_TRACE_FLOW_STR("Master; mode " << mymode);
    /* Wait for all connections to be completed */
    MPI_Barrier(MPI_COMM_WORLD);

    int32 nextnode; /* node number of the next node that receives a trigger
		   * by an MPI_Bcast;
		   * nextnode == -1 indicates all nodes 
		   */
    int32 r, n;
    int32 recvbuf;
    MPI_Status status;
    LOG_TRACE_FLOW_STR("Start Master loops; runs = " 
		       << runs << " nodes = " << nodes);
    double startTime = MPI_Wtime();
    for (r = 0; r < runs; r++) {
      for (n = 1; n < nodes; n++) {
	switch (mymode) {
	case 0:                            /* One-by-one mode */
	  nextnode = n;
	  LOG_TRACE_FLOW("Master sends BCast");
          //!MPI_Bcast(&nextnode, 1, MPI_INT, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Recv(&recvbuf, 1, MPI_INT, nextnode, 
		     ACKTAG, MPI_COMM_WORLD, &status); /* acknowledge */
	  }
	  break;
	case 1:                            /* Random order nodes 1...nodes */
	  nextnode = 1 + (random() % nodes); 
          MPI_Bcast(&nextnode, 1, MPI_INT, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Recv(&recvbuf, 1, MPI_INT, nextnode, ACKTAG, MPI_COMM_WORLD, &status); /* acknowledge */
	  }
	  break;
	case 2:                             /* All together mode */
	  int32 collval, myval=0;
	  nextnode = -1;                   /* will trigger all nodes */
	  n += nodes;                      /* ends the nodes loop  */
          MPI_Bcast(&nextnode, 1, MPI_INT, 0, MPI_COMM_WORLD);  // trigger
	  if (ackMode) {
	    MPI_Reduce(&myval, &collval, 1, MPI_INT, MPI_LOR, 0, MPI_COMM_WORLD);
	  }
	  break;
	} // switch
      } // for nodes
    } // for runs
//     double endTime = MPI_Wtime();
//     cout << "   Starttime " << startTime
// 	 << "   EndTime " << endTime
// 	 << "   runs " << runs
// 	 << "   Buflen " << pkgsize 
// 	 << endl;
//     cout << "average transfer rate = "
// 	 << runs*pkgsize/((endTime-startTime)*(1024*1024*1024))
// 	 << "Gbps per node " << endl;
  } // master or not
  
  MPI_Finalize();
  return (0);
} // main

#endif // MPI
