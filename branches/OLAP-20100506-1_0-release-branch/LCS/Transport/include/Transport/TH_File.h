//# TH_File.h: To/from file transport mechanism
//#
//# Copyright (C) 2000, 2001
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
//# $Id$


#ifndef TRANSPORT_TH_FILE_H
#define TRANSPORT_TH_FILE_H

// \file
// To/from file transport mechanism
//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Transport/TransportHolder.h>
#include <stdio.h>

namespace LOFAR
{

// \addtogroup Transport
// @{

// This class defines the transport mechanism To/From a single dataholder.
// A connection between two dataholders has to be made; Only one of them is 
// active and produces or consumes data from the transport mechanism. The 
// other dataholder is supposed to do nothing at all (probably not added to 
// the simul) but is needed to use the Step::connect... methods.

class TH_File: public TransportHolder
{
public:
  enum direction{Write,Read,UnDefined};

  TH_File();
  // The constructor.
  // Arguments:
  // \arg filename   The file name, including path, to/from which the data is 
  //	             written/read
  // \arg aDirection Defines whether data is written(only send() method active)
  //	             or read (only recv method active)

  TH_File(string    aFileName, direction aDirection);
  virtual ~TH_File();

  // Initialization
  virtual bool init();

  /**
     Receive the data. If the Direction is defined as Read, this method reads 
     the next DataHolder from file.
  */
  virtual bool recvBlocking(void* buf, int nbytes, int tag, 
			    int nBytesRead=0, DataHolder* dh=0);

  /**
     This method calls the blocking receive method.
  */
  virtual int32 recvNonBlocking(void* buf, int32 nbytes, int tag,
			       int32 nBytesRead=0, DataHolder* dh=0);  

  /**
     Send the data. If the Direction is defined as Write, this method writes 
     the next DataHolder to file.
  */
  virtual bool sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);

  /**
     This method calls the blocking receive method.
  */
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  // Wait until the data has been sent
  virtual void waitForSent(void* buf, int nbytes, int tag); 

  // Wait until the data has been received
  virtual void waitForReceived(void* bug, int nbytes, int tag);

  // Get the type of transport, i.e. "TH_File"
  virtual string getType() const;

  /// Is TH_File clonable?
  virtual bool isClonable() const;

  /// Return a copy of this transportholder
  virtual TH_File* clone() const;

  // Resets all members which are source or destination specific.
  virtual void reset();

  static void finalize();
  static int getCurrentRank();
  static int getNumberOfNodes();

 private:
  string    itsFileName;
  direction itsDirection;

  FILE      *itsInFile;
  FILE      *itsOutFile;
};

// @} // Doxygen endgroup Transport

inline bool TH_File::init()
  { return true; }

inline bool TH_File::isClonable() const
  { return true; }

inline void TH_File::reset()
  {}

}

#endif
