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

#include <BBSTools/Progress.h>

using namespace std;

// Default constructor
Progress::Progress()
{
}

Progress::Progress(const string &key, const string &host="localhost", const string &user, const string &db, int port=0)
: itsKey(key),
  itsHost(host),
  itsUser(user),
  itsDB(db),
  itsPort(port)
{
  string connectString;   
  
  // Make this more error proof?
  connectString+="host=" + dbserver;
  if(port != 0)
    connectString+=" port=" + port;
  connectString+=" user=" + dbuser;
  connectString+=" db=" + dbname;
  
  connection Conn(connectString);         // connect to database
  itsConnection=Conn;                     // update information in
}


Progress::~Progress()
{
}

bool Progress::checkAlive(pqxx::connection Conn)
{

}

double Progress::getProgress(const std::string &pid)
{
  // create transactor
}

void Progress::getProgress(const vector<double> &progress)
{
  // create transactor
  
  // get session info
  
  // loop over all nodes belonging to this session
  
}

void Progress::getProgress()
{

}

