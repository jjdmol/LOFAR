//# TH_File.h: To/from file transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef TRANSPORT_TH_FILE_H
#define TRANSPORT_TH_FILE_H

#include <Transport/TransportHolder.h>
#include <stdio.h>

namespace LOFAR
{

/**
   This class defines the transport mechanism To/From a single dataholder.
   A connection between two dataholders has to be made; Only one of them is 
   active and produces or consumes data from the transport mechanism. The 
   other dataholder is supposed to do nothing at all (probably not added to 
   the simul) but is needed to use the Step::connect... methods.
*/

class TH_File: public TransportHolder
{
public:
  enum direction{Write,Read,UnDefined};

  TH_File();
  /**
     The constructor.
     Arguments:
        filename:    The file name, including path, to/from which the data is 
	             written/read
	aDirection:  Defines whether data is written(only send() method active)
	             or read (only recv method active)
  **/
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
  virtual bool recvNonBlocking(void* buf, int nbytes, int tag,
			       int nBytesRead=0, DataHolder* dh=0);  

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

  /// Get the type of transport, i.e. "TH_File"
  virtual string getType() const;

  /// Is TH_File clonable?
  virtual bool isClonable() const;

  /// Return a copy of this transportholder
  virtual TH_File* clone() const;

  static void finalize();
  static int getCurrentRank();
  static int getNumberOfNodes();

 private:
  string    itsFileName;
  direction itsDirection;

  FILE      *itsInFile;
  FILE      *itsOutFile;

  char      itsSeparator[9];
  int       itsSepLen;
};

inline bool TH_File::init()
  { return true; }

inline bool TH_File::isClonable() const
  { return true; }

}

#endif
