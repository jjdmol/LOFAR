//# Progress.cc: Uses BBSControl blackboard.progress information to give
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
//# $Id: BBSProgress.cc 17855 2011-08-31 17:36:27Z duscha $

//# Always #include <lofar_config.h> first!


#include <lofar_config.h>
// gethostname() and getpid()
#include <unistd.h>
#include <pqxx/except>
#include <BBSControl/Exceptions.h>
#include <BBSTools/Progress.h>


// Now here's an ugly kludge: libpqxx defines four different top-level
// exception classes. In order to avoid a lot of code duplication we clumped
// together four catch blocks in order to catch all pqxx related exceptions.
// A DatabaseException will be thrown, containing the original pqxx
// exception class type and the description.
#if defined(CATCH_PQXX_AND_RETHROW)
# error CATCH_PQXX_AND_RETHROW is already defined and should not be redefined
#else
# define CATCH_PQXX_AND_RETHROW					\
  catch (pqxx::broken_connection& e) {				\
    THROW (DatabaseException, "pqxx::broken_connection:\n"	\
	   << e.what());					\
  }								\
  catch (pqxx::sql_error& e) {					\
    THROW (DatabaseException, "pqxx::sql_error:\n"		\
	   << "Query: " << e.query() << endl << e.what());	\
  }								\
  catch (pqxx::in_doubt_error& e) {				\
    THROW (DatabaseException, "pqxx::in_doubt_error:\n"		\
	   << e.what());					\
  }								\
  catch (pqxx::internal_error& e) {				\
    THROW (DatabaseException, "pqxx::internal_error:\n"		\
	   << e.what());					\
  }
#endif



using namespace std;
using namespace LOFAR;
using namespace LOFAR::BBS;

// Default constructor
Progress::Progress()
{
}

Progress::Progress(const string &db="", const string &key="default", const string &host="localhost", const string &user="postgres", const string &port="")
: itsDb(db), 
  itsKey(key),
  itsHost(host),
  itsUser(user),
  itsPort(port)
{
  string connectString;   
  
  // Make this more error proof?
  /*
  connectString+="host=" + host;
  if(port != 0)                           // if a different port was specified use this
    connectString+=" port=" + port;
  connectString+=" user=" + user;
  connectString+=" db=" + db;
  */

  // determine user db if not given otherwise
  if(db.empty())
  {
    itsDb=getDb();
  }

  // Get the SessionId of this process

  // Build connection string.
  string opts("dbname='" + db + "' user='" + user + "' host='" + host + "'");
  if(!port.empty()) 
  {
    opts += " port=" + port;
  }
  try
  {
    cout << "Connecting to database: " << opts;
    itsConnection.reset(new pqxx::connection(opts));
  
    // Ensure the database representation of this shared state is
    // initialized and get the corresponding ID.
    //itsConnection->perform(pqxx::PQInitSession(key, itsSessionId));
  }
  CATCH_PQXX_AND_RETHROW
}


Progress::~Progress()
{
}


/*
void Progress::determineProcessId(void)
{
  // Determine the ProcessId of this worker.
  char hostname[512];
  int status = gethostname(hostname, 512);
  itsProcessId = ProcessId(string(hostname), getpid());
}
*/

int32 Progress::determineSessionId(void)
{
  int32 status = -1;
  int32 sessionId = -1;
  
  try
  {
    // get_session_info() to get corresponding SessionId for key
    //itsConnection->perform(status);
  }
  CATCH_PQXX_AND_RETHROW;
  
  return sessionId;
}


string Progress::getDb(void)
{
  string user;

  user=getenv("USER");       // get user name from environment variables
  itsDb=user;

  return user;
}


double Progress::getProgress(void)
{
  // create transactor
}

void Progress::getProgress(double &progress, string &step)
{

}

void Progress::getProgress(vector<double> &progress)
{
  // create transactor
  
  // get session info
  
  // loop over all nodes belonging to this session
 
}

void Progress::getProgress(std::map<std::string, double> &progress)
{
  // Loop over worker Pids
  
  // call getProgress(double &progress)
}

//*********************************************
//
// Helper functions
//
//*********************************************

bool Progress::checkAlive(pqxx::connection Conn)
{

}

void getWorkers(vector<int32> workers, WorkerType &type=KERNEL)
{

}
