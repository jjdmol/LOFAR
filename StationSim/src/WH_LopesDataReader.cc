//#  WH_LopesDataReader.cc:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#include <stdio.h>             // for sprintf
#include <StationSim/WH_LopesDataReader.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_LopesDataReader::WH_LopesDataReader (const string& name,
										unsigned int nrcu,
										const string& dirName)
  : WorkHolder       (0, 1, name, "WH_LopesDataReader"),
	itsOutHolder     ("out", nrcu, 1),
	itsDirName       (dirName),
	itsDirPointer    (0),
	itsBlockPointer  (0),
	itsNrcu          (nrcu),
	itsLength        (1),
	itsCount         (0)
{
  // Initialize
  getDirEntries (itsDirName, entries);

  itsBuffer = new buftype* [itsNrcu];
}

WH_LopesDataReader::~WH_LopesDataReader()
{
  //  delete &itsOutHolder;
}

WorkHolder* WH_LopesDataReader::construct (const string& name,
										   int ninput, int noutput,
										   const ParamBlock& params)
{
  Assert (ninput == 0);
  return new WH_LopesDataReader (name, noutput,	params.getString ("lopesdirname", ""));
}

WH_LopesDataReader* WH_LopesDataReader::make (const string& name) const
{
  return new WH_LopesDataReader (name, itsNrcu, itsDirName);
}

void WH_LopesDataReader::process()
{
  char channid [4];

  if (itsBlockPointer == itsLength - 1) {
	// Open the next file
	itsCurrentFile = new ifstream ((itsDirName + string (entries[itsDirPointer]->d_name)).c_str (), ios::binary);
	itsDirPointer = ++itsDirPointer % entries.size ();
	AssertStr (itsCurrentFile->is_open (), "Failed to open file " << entries[itsDirPointer]->d_name);

	cout << "LOPES data reader : opened the " << entries[itsDirPointer - 1]->d_name << " file." << endl;
	
	// Read the header of the newly opened file
	readLopesHeader ();
	
	// Read in the data and put it in the buffer
	for (unsigned int r = 0; r < itsNrcu; r++) {
	  itsCurrentFile->read (channid, 4);
	  itsCurrentFile->read ((char*)&itsLength, 4);	  
	  itsBuffer[r] = new buftype[itsLength];
	  itsCurrentFile->read ((char*)&itsBuffer[r][0], itsLength * 2);
	}

	// close current file
	itsCurrentFile->close ();	  

	// Put block pointer at the beginning of the file
	itsBlockPointer = 0;
  }

  // Copy a part of the block to the ouput
  for (unsigned int r = 0; r < itsNrcu; r++) {
	itsOutHolder.getBuffer ()[r] = (DH_SampleC::BufferType)itsBuffer[r][itsBlockPointer];
  }
	
  // Increment the block pointer
  itsBlockPointer++;

  //  cout << "WH_LopesDataReader : " << itsCount++ << endl;
}

void WH_LopesDataReader::dump() const
{
}


DataHolder* WH_LopesDataReader::getInHolder (int channel)
{
  AssertStr (channel < 0, "input channel too high");
  return 0;
}

DH_SampleC* WH_LopesDataReader::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return &itsOutHolder;
}

void WH_LopesDataReader::getDirEntries (const string& dirname, vector<dirent*>& entries)
{
  dirent** darray;
  int entryCount = scandir (dirname.c_str (), &darray, 0, alphasort);
  int k;
  for (k = 0; k < entryCount; k++) {
	if (strncmp (darray[k]->d_name, ".", 1) != 0) {
	  entries.push_back (darray[k]);
	}
  }
}

void WH_LopesDataReader::readLopesHeader ()
{
  unsigned int length;
  unsigned int blocksize;
  char temp[20];

  // Read header data
  itsCurrentFile->read ((char*)&length, 4);
  itsCurrentFile->read ((char*)&temp, 20);
  itsCurrentFile->read ((char*)&blocksize, 4);
  itsCurrentFile->read ((char*)&temp, 20);
  
  // Determine number of channels from filesize
  AssertStr ((length - 48) / (blocksize * 2 + 8) == itsNrcu, "The LOPES channels don't correspond with the number of rcu's");
}
