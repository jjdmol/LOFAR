//  WH_TargetTrack.cc:
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//
//////////////////////////////////////////////////////////////////////

#include "StationSim/WH_TargetTrack.h"
#include "BaseSim/ParamBlock.h"
#include "Common/Debug.h"
#include "Common/lofar_strstream.h"


WH_TargetTrack::WH_TargetTrack (const string& name,
				unsigned int maxNtrack,
				const string& fileName)
: WorkHolder    (0, 1, name, "WH_TargetTrack"),
  itsOutHolder  ("out", maxNtrack),
  itsMaxNtrack  (maxNtrack),
  itsFileName   (fileName),
  itsFile       (fileName.c_str()),
  itsLineNr     (0)
{
  AssertStr (itsFile, "Failed to open file " << fileName);
}

WH_TargetTrack::~WH_TargetTrack()
{
}

WorkHolder* WH_TargetTrack::construct (const string& name,
				       int ninput, int noutput,
				       const ParamBlock& params)
{
  Assert (ninput == 0);
  Assert (noutput == 1);
  return new WH_TargetTrack (name,
			     params.getInt ("maxntargettrack", 10),
			     params.getString ("targetfilename", ""));
}

WH_TargetTrack* WH_TargetTrack::make (const string& name) const
{
  return new WH_TargetTrack (name, itsMaxNtrack, itsFileName);
}

void WH_TargetTrack::process()
{
  DH_TargetTrack::BufferType* buf = itsOutHolder.getBuffer();
  char line[4096];
  // Read until a proper line is found.
  // Restart at the beginning if at end-of-file.
  bool eof = false;
  while (true) {
    itsFile.getline (line, sizeof(line)-1);
    if (itsFile.eof()) {
      itsFile.close();
      // Error if at eof without having read a value.
      AssertStr (!eof, "File " << itsFileName << " is empty");
      eof = true;
      itsFile.open (itsFileName.c_str());
      itsLineNr = 0;
      itsFile.getline (line, sizeof(line)-1);
    }
    itsLineNr++;
    AssertStr (!itsFile.fail(), "Read error on line " << itsLineNr
	       << " in file " << itsFileName);
    int nr = itsFile.gcount();
    // Skip empty lines and comment lines.
    // Note that \n also counts as a character read.
    if (nr > 1  &&  line[0] != '#') {
      nr--;
      line[nr] = 0;
      istrstream istr(line, nr);
      DH_TargetTrack::BufferType val;
      int nrval = 0;
      while (true) {
	istr >> val;
	if (istr.eof()) {
	  break;
	}
	AssertStr (!istr.fail(), "Line " << itsLineNr << " in file "
		   << itsFileName << " has incorrect data");
	AssertStr (nrval < itsMaxNtrack, "too many track data in line "
		   << itsLineNr << " in file " << itsFileName
		   << "(max=" << itsMaxNtrack << ')');
	buf[nrval++] = val;
      }
      if (nrval > 0) {
	// Some values have been found, so break the main loop.
	itsOutHolder.setNtrack (nrval);
	break;
      }
    }
  }
}

void WH_TargetTrack::dump() const
{
  cout << "WH_TargetTrack " << getName() << " Buffers:" << endl;
  cout << itsOutHolder.getBuffer()[0] << ','
       << itsOutHolder.getBuffer()[itsOutHolder.ntrack()-1] << endl;
}


DataHolder* WH_TargetTrack::getInHolder (int channel)
{
  AssertStr (channel < 0,
	     "input channel too high");
  return 0;
}
DH_TargetTrack* WH_TargetTrack::getOutHolder (int channel)
{
  AssertStr (channel < 1,
	     "output channel too high");
  return &itsOutHolder;
}
