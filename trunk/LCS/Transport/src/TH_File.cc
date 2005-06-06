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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transport/TH_File.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

TH_File::TH_File():
  itsFileName ("unknown"),
  itsDirection(UnDefined),
  itsInFile(NULL),
  itsOutFile(NULL)
{
  LOG_TRACE_FLOW("TH_File default constructor");
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
  LOG_TRACE_FLOW("TH_File constructor"); 
  sprintf(itsSeparator,"SEPARATOR");
  itsSepLen = strlen(itsSeparator);
}

TH_File::~TH_File()
{
  LOG_TRACE_FLOW("TH_File destructor");
  //  if (itsInFile) fclose(itsInFile);
  //if (itsOutFile) fclose(itsOutFile);
}

string TH_File::getType() const
{
  return "TH_File";
}

TH_File* TH_File::clone() const
{
  return new TH_File(itsFileName, itsDirection);
}


bool TH_File::recvBlocking(void* buf, int nbytes, int, int, DataHolder*)
{ 
  LOG_TRACE_RTTI( "TH_File recvBlocking()" );
  bool result = true;
  if (itsDirection == Read) {
    // test if the file is already opened
    if (itsInFile == NULL) {
      LOG_TRACE_COND( "Input file not open yet" );
      itsInFile = fopen(itsFileName.c_str(), "r");
      if (!itsInFile) {
	LOG_ERROR_STR( "Error opening input file " << itsFileName );
	result = false;
      } else {
	LOG_TRACE_COND_STR( "Input file " << itsFileName << " opened" );
      }
    }
    // read data from file
    if (itsInFile != NULL) {
      int n_read;
      n_read = fread(buf, 1, nbytes, itsInFile);
      ASSERTSTR (n_read == nbytes, "TH_File::recvBlocking not enough bytes have been read");
      LOG_TRACE_CALC_STR( "Read main block " << n_read ); 
      char mySep[255];
      n_read = fread(mySep,
		     1,
		     itsSepLen,
		     itsInFile);
      LOG_TRACE_CALC_STR( "Read separator block " << n_read << " : " << mySep ); 
      ASSERT((n_read == itsSepLen) && (strncmp(itsSeparator,mySep, itsSepLen)== 0));

      LOG_TRACE_COND_STR( "Read from File " << itsFileName );
    } else {
      LOG_ERROR_STR( "Error reading from file " << itsFileName );
      result = false;	
    }
  } else {
      LOG_TRACE_COND( "Skip read from file" );
  }
  return result;
}

int32 TH_File::recvNonBlocking(void* buf, int32 nbytes, int tag, 
			      int32 offset, DataHolder* dh)
{
  LOG_WARN("TH_File::recvNonBlocking() is not implemented. The recvBlocking() method is used instead."); 
  return (recvBlocking(buf, nbytes, tag, offset, dh) ? nbytes : 0);
}

bool TH_File::sendBlocking(void* buf, int nbytes, int, DataHolder*)
{
  LOG_TRACE_RTTI("TH_File sendBlocking()");
  bool result = true;
  if (itsDirection == Write) {
    // test if the file is already opened
    if (itsOutFile == NULL) {
      LOG_TRACE_COND( "Output file not open yet" );
      itsOutFile = fopen(itsFileName.c_str(), "w");
      if (!itsOutFile) {
	LOG_ERROR_STR( "Error opening output file " << itsFileName );
	result = false;
      } else {
	LOG_TRACE_COND_STR( "Output file " << itsFileName << " opened" );
      }
    }
    // write data to file
    if (itsOutFile) {
      int n_written;
      n_written = fwrite(buf, 1, nbytes, itsOutFile);
      ASSERTSTR(n_written == nbytes, "TH_File::sendBlocking not enough bytes have been written");
      n_written = fwrite(itsSeparator,
			 1,
			 itsSepLen,
			 itsOutFile);
      ASSERT(n_written == itsSepLen);
      LOG_TRACE_COND_STR ( "Write to File " << itsFileName 
			   << " (" << nbytes 
			   << "," << itsSepLen 
			   << ") bytes" );
    } else {
      LOG_ERROR_STR( "Error writing to file" << itsFileName);
      result = false;	
    }
  } else {
    LOG_TRACE_COND( "Skip Write to File" );
  }

  return result;
}

bool TH_File::sendNonBlocking(void* buf, int nbytes, int, DataHolder*)
{
  LOG_WARN("TH_File::sendNonBlocking() is not implemented. The sendBlocking() method is used instead.");
  return sendBlocking(buf, nbytes, 0);
}

void TH_File::waitForSent(void*, int, int)
{
  LOG_TRACE_RTTI("TH_File waitForSent()");
}

void TH_File::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI("TH_File waitForReceived()");
}

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
  LOG_TRACE_RTTI("TH_File finalize()");
}

}
