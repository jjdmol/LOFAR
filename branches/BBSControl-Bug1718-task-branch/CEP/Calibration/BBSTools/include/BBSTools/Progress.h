//# Progress.h: Uses BBSControl blackboard.progress information to give
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
//# $Id: Progress.h 17855 2011-08-16 10:36:27Z duscha $


// This class provides access methods to the blackboard.progress table
// in order to report and retrieve progress on (BBS) data reduction.
// Database transactions are executed using transactors in the BBSControl
// CalSessionTransactors-class, but low-level connections with the db
// are implemented using plain libpqxx.


#ifndef _PROGRESS_H_
#define _PROGRESS_H_
#endif _PROGRESS_H_

#if defined(HAVE_PQXX)
# include <pqxx/connection>
# include <pqxx/trigger>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <vector>
#include <map>
#include <Common/lofar_smartptr.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{

namespace BBS
{

class Progress
{
public:
  Progress();         //! default constructor
  //! Extended constructor
  Progress(const std::string &db, const std::string &key, const std::string &host, const std::string &user, const std::string &port);
  ~Progress();        //! Destructor

  //! Set the progress for the current Kernel with pid (should this be in this library?)
  void setProgress(const std::string &pid, int32 chunkCount, const std::string &step);


  //! Get the current progress as percentage (without step information) for itsPid/itsSession
  double getProgress(void);
  //! Get the current progress as percentage with step information for itsPid/itsSession
  void getProgress(double &progress, std::string &step);
  
  double getProgress(const std::string &node);
  //! Get the current progress as percentage including current step
  void getProgress(const std::string &node, double percentage, std::string &step);
  
  //! Get the current progress as percentage
  void getProgress(std::vector<double> &progress);
  //! Get the current progress as percentage including current step for each node
  void getProgress(std::map<std::string, double> &progress);

  //! Get the sessionID of this BBS run
  int32 getSessionID();

private:
  scoped_ptr<pqxx::connection> itsConnection;   //! connection to the db

//  int32 itsProcessId;               //! process id of this client
  int32 itsSessionId;               //! SessionId of this BBS run
  std::string itsDb;                //! database working on
  std::string itsKey;               //! BBS key of this session
  std::string itsHost;              //! host server
  std::string itsUser;              //! user name to log on to host
  std::string itsPort;              //! Port to connect to

  int32 getSessionId();                //! get the session_id of the BBS session of this key
  std::string getDb();                 //! determine username from environment variables
  std::string getNodename(int32 pid);                 //! retrieve hostname of node from pid  

  int32 getNumChunks();                //! get the number of total chunks of this session
  bool checkAlive(pqxx::connection Conn);             //! check if the BBS run is still alive
};

} // end namespace BBS

} // end namespace LOFAR
