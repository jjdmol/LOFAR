//# TH_File.cc: To/from file transport mechanism
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


#include <Transport/TH_File.h>
#include <Common/Debug.h>

namespace LOFAR
{

/**
 * Prototype variable declaration. Can be
 * used in functions requiring a prototype
 * argument (for the prototype design patterns).
 */

TH_File TH_File::proto;

TH_File::TH_File():
  itsFileName ("unknown"),
  itsDirection(UnDefined),
  itsInFile(NULL),
  itsOutFile(NULL)
{  
  sprintf(itsSeparator,"SEPARATOR");
  itsSepLen = strlen(itsSeparator);
}

TH_File::TH_File(string aFileName,
		 direction aDirection):
  itsFileName (aFileName),
  itsDirection(aDirection),
  itsInFile(NULL),
  itsOutFile(NULL)
{  
  sprintf(itsSeparator,"SEPARATOR");
  itsSepLen = strlen(itsSeparator);
}

TH_File::~TH_File()
{
  //  if (itsInFile) fclose(itsInFile);
  //if (itsOutFile) fclose(itsOutFile);
}

TH_File* TH_File::make() const
{
    return new TH_File(itsFileName, itsDirection);
}

string TH_File::getType() const
{
    return "TH_File";
}

bool TH_File::connectionPossible(int, int) const
{
  return true; //srcRank == dstRank;
}

bool TH_File::recvBlocking(void* buf, int nbytes, int)
{ 
  bool result = true;
  if (itsDirection == Read) {
    // test if the file is already opened
    if (itsInFile == NULL) {
      cdebug(4) << "Input file not open yet" << endl;
      itsInFile = fopen(itsFileName.c_str(), "r");
      if (!itsInFile) {
	cdebug(1) << "Error opening input file " << itsFileName << endl;
	result = false;
      } else {
	cdebug(4) << "Input file " << itsFileName << " opened" << endl;
      }
    }
    // read data from file
    if (itsInFile != NULL) {
      int n_read;
      n_read = fread(buf, 1, nbytes, itsInFile);
      FailWhen(result = (n_read != nbytes),
	       "Error during read");
      cdebug(8) << "Read main block " << n_read << endl; 
      char mySep[255];
      n_read = fread(mySep,
		     1,
		     itsSepLen,
		     itsInFile);
      cdebug(8) << "Read separator block " << n_read << " : " << mySep << endl; 
      FailWhen(result = ((n_read != itsSepLen)
			 && strncmp(itsSeparator,mySep, itsSepLen)),
	       "Error during read");
      cdebug(4) << "Read from File " << itsFileName << endl;
    } else {
      cdebug(1) << "Error reading from file" << endl;
      result = false;	
    }
  } else {
    cdebug(4) << "Skip Read from File"  << endl;
  }
  return result;
}

bool TH_File::recvNonBlocking(void* buf, int nbytes, int)
{ 
  cerr << "**Warning** TH_File::recvNonBlocking() is not implemented. " 
       << "recvBlocking() is used instead." << endl;    
  return recvBlocking(buf, nbytes, 0);
}

bool TH_File::sendBlocking(void* buf, int nbytes, int)
{
  bool result = true;
  if (itsDirection == Write) {
    // test if the file is already opened
    if (itsOutFile == NULL) {
      cdebug(4) << "Output file not open yet" << endl;
      itsOutFile = fopen(itsFileName.c_str(), "w");
      if (!itsOutFile) {
	cdebug(1) << "Error opening output file " << itsFileName << endl;
	result = false;
      } else {
	cdebug(4) << "Output file " << itsFileName << " opened" << endl;
      }
    }
    // write data to file
    if (itsOutFile) {
      int n_written;
      n_written = fwrite(buf, 1, nbytes, itsOutFile);
      FailWhen(result = (n_written != nbytes),
	       "Error during write");
      n_written = fwrite(itsSeparator,
			 1,
			 itsSepLen,
			 itsOutFile);
      FailWhen(result = (n_written != itsSepLen),
	       "Error during write");
      cdebug(4) << "Write to File " << itsFileName 
		<< " (" << nbytes 
		<< "," << itsSepLen 
		<< ") bytes" << endl;
    } else {
      cdebug(1) << "Error writing to file" << endl;
      result = false;	
    }
  } else {
    cdebug(4) << "Skip Write to File"  << endl;
  }
  return result;
}

bool TH_File::sendNonBlocking(void* buf, int nbytes, int)
{
  cerr << "**Warning** TH_File::sendNonBlocking() is not implemented. " 
       << "The sendBlocking() method is used instead." << endl;    
  return sendBlocking(buf, nbytes, 0);
}

bool TH_File::waitForSent(void*, int, int)
{
  return true;
}

bool TH_File::waitForReceived(void*, int, int)
{
  return true;
}

void TH_File::waitForBroadCast()
{}

void TH_File::waitForBroadCast(unsigned long&)
{}


void TH_File::sendBroadCast(unsigned long)
{}

int TH_File::getCurrentRank()
{
    return -1;
}

int TH_File::getNumberOfNodes()
{
    return 1;
}

void TH_File::finalize()
{
}

void TH_File::synchroniseAllProcesses()
{}

}
