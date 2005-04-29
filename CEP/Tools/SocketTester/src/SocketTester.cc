/* Socket tester program
 * intended to test complex communication topologies between
 * two or three sets of "nodes".
 * Especially written to test Linux - BGL - Linux topologies
 */

#include <Transport/Socket.h>
#include <mpi.h>

#define SENDER        0      /* boolean */
#define ACKTAG        734
#define LISTENER      1

namespace LOFAR
{
  void read_config(int  myrank,
		   int nodes, 
		   int  *ackMode, 
		   int  *sleeptime,
		   int  *runs,
		   int  *mymode,
		   char **myip,
		   int *myport) {
  
  FILE *cfg = fopen("SocketTester.cfg","r");
  fscanf(cfg,"ackMode %i", ackMode);
  printf("ackMode = %i\n", *ackMode);
  fscanf(cfg,"sleeptime %i", sleeptime);
  printf("sleeptime = %i\n", *sleeptime);
  fscanf(cfg,"runs %i", runs);
  printf("runs = %i\n", *runs);

  char anip[32];
  int anode, amode;
  for(int n = 1; n < nodes; n++) {
    fscanf(cfg,"node%3i %1i %s  %i", &anode, &amode, anip, myport);
    if (anode == myrank) {
      *mymode = amode;
      strcpy(*myip, anip);
      printf("node: %i     Mode: %1i    IP: %s    Port: %i", myrank, *mymode, *myip, *myport);
    }
  }
}

int main(int argc, char*argv[]) {

  MPI_Init(&argc,&argv);
  int myrank, nodes;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank); 
  MPI_Comm_size(MPI_COMM_WORLD, &nodes); 
  int mymode, myport;
  int ackMode;
  int sleeptime;
  int runs;
  char myip[16];

  read_config(myrank,
	      nodes,
	      &ackMode, 
	      &sleeptime,
	      &runs,
	      &mymode,
	      &myip,
	      &myport);

  if (myrank != 0) { /* rank 0 is the master, see below */

    Socket mysock();

    /* Start listeners  */
    if (LISTENER) mysock.openListener(myport);
    
    /* Make connections */
    if (LISTENER) {
      Socket newsock = mysock.accept();
    } else {
      mysock.connect(string(myip),
		     myport)
      do_connect(myip);
    }
    
    /* Synchronise all nodes */
    MPI_Barrier(MPI_COMM_WORLD);
    
    /* start the data transfer */

    int r,n;
    for (r=0; r<runs; r++) {
      MPI_Bcast(n,1,MPI_INTEGER, 0, MPI_COMM_WORLD)  /* receive the broadcast of the next node to transport  */
	if (n == myrank || n == -1) {
	    /* perform the actual data transport */
	  if (SENDER) {
	    mysock.send(*message, len);
	  } else {
	    /* receiver */
	    mysock.recv(*buf,len);
	  }
	  
	}
      if (ackMode) {
	if (n == -1) {
	  /* ackMode && alltogether; need global ack to master */
	  int collval,myval=1;
	  MPI_Reduce(&myval,&collval,1,MPI_INTEGER,MPI_COMM_WORLD);
	} else {
	  /*  ackMode needs single acknowledge to master */
	  int ack = 1;
	  MPI_Send(&ack,1,MPI_INTEGER,0,ACKTAG,MPI_COMM_WORLD);
	}
      }
      usleep sleeptime; 
    }

  } else { /* my rank == 0 */
    /* I am the master 
     * All I have to do is make the others work
     */

    /* Wait for all connections to be completed */
    MPI_Barrier(MPI_COMM_WORLD);

    int nextnode; /* node number of the next node that receives a trigger
		   * by an MPI_Bcast;
		   * nextnode == -1 indicates all nodes 
		   */
    int n;
    int recvbuf;
    MPI_Status status;
    for (r=0; r<runs; r++) {
      for (n=1; n<nodes; n++) {
	switch (mode) {
	case 0:                            /* One-by-one mode */
	  nextnode = n;
	  MPI_Bcast(nextnode);             /* trigger */
	  if (ackMode) MPI_Recv(&recvbuf,1,MPI_INTEGER,nextnode,ACKTAG,MPI_COMM_WORLD,&status); /* acknowledge */
	  break;
	case 1:                            /* Random order nodes 1...nodes */
	  nextnode = random(nodes); 
	  MPI_Bcast(nextnode);             /* trigger */
	  if (ackMode) MPI_Recv(&recvbuf,1,MPI_INTEGER,nextnode,ACKTAG,MPI_COMM_WORLD,&status); /* acknowledge */
	  break;
	case2:                             /* All together mode */
	  int collval,myval=0;
	  nextnode = -1;                   /* will trigger all nodes */
	  n += nodes;                      /* ends the nodes loop  */
	  MPI_Bcast(nextnode);             /* trigger */
	  if (ackMode) 	  
	  MPI_Reduce(&myval,&collval,1,MPI_INTEGER,MPI_COMM_WORLD);   /* global acknowledge */
	  break;
	}
      }
    }
  }
  MPI_Finilize();
}
} //namespace
