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

#ifndef CEPFRAME_TH_FILE_H
#define CEPFRAME_TH_FILE_H

#include <lofar_config.h>

#include "TransportHolder.h"
#include <stdio.h>

namespace LOFAR
{

/**
   This class defines the transport mechanism To/From a single dataholder.
   A connection between two dataholders has to be made; Only one of them is 
   active and produces or consumes data from the transport mechanism. The 
   other dataholder is supposed to do nothinbg at all (probably not added to 
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
	             writen/read
	aDirection:  Defines whether data is written(onle send() method active)
	             or read (only recv method active)
  **/
  TH_File(string    aFileName,
	  direction aDirection);
  virtual ~TH_File();

  /// method to make a TH_File instance; used for prototype pattern
  virtual TH_File* make() const;

  /**
     Receive the data. If the Direction is defined as Read, this method reads 
     the next DataHolder from file.
  */
  virtual bool recv(void* buf, int nbytes, int source, int tag);

  /**
     Send the data. If the Direction is defined as Write, this method writes 
     the next DataHolder to file.
  */
  virtual bool send(void* buf, int nbytes, int destination, int tag);

  /// Get the type of transport, i.e. "TH_File"
  virtual string getType() const;

  /**
     return wheterher the proposed connection between srcRank and dstRank 
     would be possible with this TransportHolder specialisation.
  **/
  virtual bool connectionPossible(int srcRank, int dstRank) const;

  bool isBlocking() const { return false; }

  /// Declare a TH_File prototype variable
  /// that can be used in functions
  /// requiring a TransportHolder prototype
  static TH_File proto;
  
  static void init (int argc, char *argv[]);
  static void finalize();
  static void waitForBroadCast();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int getCurrentRank();
  static int getNumberOfNodes();
  static void synchroniseAllProcesses();

 private:
  string    itsFileName;
  direction itsDirection;

  FILE      *itsInFile;
  FILE      *itsOutFile;

  char      itsSeparator[9];
  int       itsSepLen;
};

}

#endif
