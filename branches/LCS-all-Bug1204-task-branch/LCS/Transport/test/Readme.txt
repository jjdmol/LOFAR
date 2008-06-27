This file describes the tests in this directory (./LOFAR/LCS/Transport/test/):

*** Example ***
 Demonstrates the simplest case: 
 Data is transported from one DataHolder to another via TH_Mem.


*** TestBidirectional ***
 Tests bidirectional data transport:
 Data is transported from one DataHolder to another and back to the first DataHolder.
 This is tested for in-memory (TH_Mem) transport.


*** ExampleMem ***
 Tests in-memory transport of fixed and variable size data.
 Data with and without an extra (variable size) blob is transported from one DataHolder 
 to another via TH_Mem.


*** ExampleBlMem ***
 Tests blocking in-memory transport of fixed and variable size data.
 This is the same test as ExampleMem except blocking transport is used. This is tested 
 by having separate threads for reading and writing.


*** ExampleNonBlocking ***
 This example illustrates the principle (and danger!) of non-blocking
 communication. Your data can be modified before it has completely been sent.


*** ExampleShMem ***
 Tests transport of fixed and variable size data via shared memory (in an MPI environment).


*** ExampleMPI ***
 Tests transport of fixed and variable size data via MPI. Also tests bidirectional transport
 with MPI.


*** ExampleVarBuf ***  
 Illustrates transport of a user-defined, variable size datafield in the blob.


*** ExampleSocket ***
 Tests transport of fixed and variable size data via sockets. Also tests bidirectional
 transport with sockets.

*** tTH_Socket ***
 Demonstrates how the TH_Socket class should be used. It tests all possible modes of the
 socket: blocking/nonblocking, listener at client/server side, unidirectional/bidirectional.
