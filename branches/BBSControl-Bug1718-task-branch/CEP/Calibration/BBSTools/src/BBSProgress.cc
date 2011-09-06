//# BBSProgress.cc: Uses BBSControl blackboard.progress information to give
//# an indication to the user how far the current calibration has progressed
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: BBSProgress.cc 17855 2011-08-16 10:36:27Z duscha $

#include <lofar_config.h>

#include <iostream>
#include <stdlib.h>                              // getenv to get user name
#include <vector>
#include <BBSControl/CalSessionTransactors.h>    // functions to BBSControl DB

using namespace std;
//using namespace pqxx;

//pqxx:connection connectToBBSControl(void);       // don't do that in a function?
string determineBBSSession();
void determineProgress(const string &key, vector<double> &progress, string node="overall");
void usage(const char *progname);


//*********************************************************
//
// Main
//
//*********************************************************

int main(int argc, char **argv)
{
  bool verbose=false;        // argument flags

  string dbserver="ldb001", user="", dbuser="postgres";   // default values for CEP cluster 
  string key="default", node="";                          // default values for BBS database

  // Parse command line arguments
  // -d             database server to connect to
  // -k             key to look up BBS session
  // -n             display only this node's progress
  // -p             postgres user name to use to log on to dbserver
  // -u             user database to show progress for
  // -v             verbose messages
  //
  int opterr = 0, c=0; 
  //int c=0;
  //extern char *optarg;
  //extern int optind, optopt, opterr;
  
  while ((c = getopt (argc, argv, "d:k:n:p:u:v")) != -1)
    switch (c)
    {
      case 'd':                 // db
        dbserver = optarg;
        break;
      case 'v':                 // verbose
        verbose = true;
        break;
      case 'k':                 // BBS db key
        key = optarg;
        break;
      case 'n':                 // node
        node = optarg;
        break;
      case 'p':                 // postgres user role
        dbuser = optarg;
        break;
      case 'u':                 // BBS user db
        user = optarg;
        break;
      case 'h':
      case '?':                 // usage help
        if (optopt == 'k')
          cerr << "Option -" << optopt << " requires an argument.\n" << endl;
        else if (optopt == 'n')
          cerr << "Option -" << optopt << " requires an argument.\n" << endl;
        else if (optopt == 'p')
          cerr << "Option -" << optopt << " requires an argument.\n" << endl;
        else if (optopt == 'u')
          cerr << "Option -" << optopt << " requires an argument.\n" << endl;
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                  "Unknown option character `\\x%x'.\n",
                  optopt);
        return 1;
     default:
       abort ();
  }

  if(verbose)       // display current settings
  {
    cout << "Settings:" << endl;
    cout << "dbserver = " << dbserver << endl;
    cout << "key = " << key << endl;
    cout << "database = " << user << endl;
    cout << "node = " << node << endl;
    cout << "dbuser = " << dbuser << endl;
    cout << "verbose = " << verbose << endl;
  }

  // Connect to BBS db using libpqxx
  
  // Wait for processing to start (How do we do that?)
  
  // Continuously query blackboard.progress
  // by default overall progress
  // optionally progress of all nodes?

    // Display progress
  
  exit(0);    // exit successfully
}


//*********************************************************
//
// Helper functions
//
//*********************************************************

// Show usage of program
//
void usage(char *progname)
{
  //d:k:n:p:u:v
  cout << "Usage: " << progname << "<options>" << endl;
  cout << "-d <host>      database server to connect to" << endl;
  cout << "-k <key>       key to look up BBS session" << endl;
  cout << "-n <node>      display only this node's progress" << endl;
  cout << "-p <user>      postgres user role to use to log on to dbserver" << endl;
  cout << "-u <db>        user database to show progress for" << endl;
  cout << "-v             verbose messages" << endl;
  cout << "-h             display this help information" << endl;
}

/*
pqxx:connection connectToBBSControl(const string &dbname, const string &dbserver, int port=0, const string &dbuser)
{
  string connectString;   
  
  // Make this more error proof?
  connectString+="host=" + dbserver;
  if(port != 0)
    connectString+=" port=" + port;
  connectString+=" user=" + dbuser;
  connectString+=" db=" + dbname;
  
  connection Conn(connectString);
  
  return Conn;
}
*/


//*********************************************************
//
// Progress functions
//
//*********************************************************


string determineBBSSession()
{
  string key="";    // session key belonging to this user
  string username;

  username=getenv("USER");  // get user name

  // Look for a running BBS session under this user name

  return key;
}


// This calculates the over all percentual progress of the BBS session
//
double determineProgress(const string &key)
{
  double percentalProgress=0;


  return percentalProgress;
}


void determineIndividualProgress(const string &key, vector<double> &progress, string node="all")
{
  //
  
  // Loop over all workers and determineProgress()
}

//*********************************************************
//
// Output functions
//
//*********************************************************