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
//# $Id: BBSProgress.cc 17855 2011-08-16 10:36:27Z duscha $


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

namespace LOFAR
{

namespace BBS
{

class Progress
{
public:
  Progress();         //! default constructor
  //! Extended constructor
  Progress(const std::string &key, const std::string &host, const std::string &user, const std::string &db, int port);
  ~Progress();        //! Destructor

  //! Set the progress for the current Kernel with pid (should this be in this library?)
  void setProgress(const std::string &pid, int32 chunkCount, const std::string &step);
  //! Get the current progress as percentage (without step information)
  double getProgress(int32 pid);
  
  double getProgress(const std::string &node);
  //! Get the current progress as percentage including current step
  void getProgress(const std::string &node, double percentage, std::string &step);
  
  //! Get the current progress as percentage
  void getProgress(std::vector<double> &progress);
  //! Get the current progress as percentage including current step
  void getProgress(std::map<std::string &node, double &progress>);

  //! Get the sessionID of this BBS run
  int32 getSessionID();

private:
  int32 itsSessionID;               //! SessionID of this BBS run

  pqxx::connection itsConnection;   //! connection to the db
  std::string itsHost;              //! host server
  int itsPort;                      //! Port to connect to
  std::string itsUser;              //! user name to log on to host
  std::string itsDB;                //! database working on
  
  bool Progress::checkAlive(pqxx::connection Conn);   //! check if the connection is still alive
  std::string getNodename(int32 pid);                 //! retrieve hostname of node from pid  
}

} // end namespace BBS

} // end namespace LOFAR
