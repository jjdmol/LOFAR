This file describes the tests in this directory (./LOFAR/LCS/databases/TransportPL/test/):


*** Example ***
 Demonstrates the simplest case: 
 Data is transported from one DataHolder to another via TH_Mem.


*** ExampleMem ***
 Tests in-memory transport of fixed and variable size data.
 Data with and without an extra (variable size) blob is transported from one DataHolder 
 to another via TH_Mem.


*** ExamplePL ***
 Tests transport of fixed and variable size data via a database (TH_PL).


*** ExamplePL2 ***
 Tests multiple data transports via a database (TH_PL).
 Three different data buffers are transported after another.


*** ExamplePL3 ***
 Tests the possibility to execute a query when reading from a database.
 The queryDB() and updateDB() methods of DH_PL are tested.


*** ExamplePL4 ***
 Tests the queryDB(), updateDB() and insertDB() methods of DH_PL in combination with 
 variable size data.


*** TestBidirectional ***
 Tests bidirectional data transport:
 Data is transported from one DataHolder to another and back to the first DataHolder.
 This is tested for in-memory (TH_Mem) and database (TH_PL) transport.
