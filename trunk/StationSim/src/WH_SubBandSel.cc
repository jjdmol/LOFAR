//  WH_SubBandSel.cc:
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

#include "StationSim/WH_SubBandSel.h"
#include "BaseSim/ParamBlock.h"
#include "Common/Debug.h"


WH_SubBandSel::WH_SubBandSel (const string& name,
			      unsigned int nsubband,
			      const string& fileName)
: WorkHolder    (0, 1, name, "WH_SubBandSel"),
  itsOutHolder  ("out", nsubband),
  itsNsubband   (nsubband),
  itsFileName   (fileName),
  itsFile       (fileName.c_str())
{
  AssertStr (itsFile, "Failed to open file " << fileName);
}

WH_SubBandSel::~WH_SubBandSel()
{
}

WorkHolder* WH_SubBandSel::construct (const string& name,
				      int ninput, int noutput,
				      const ParamBlock& params)
{
  Assert (ninput == 0);
  Assert (noutput == 1);
  return new WH_SubBandSel (name,
			    params.getInt ("nsubband", 10),
			    params.getString ("selfilename", ""));
}

WH_SubBandSel* WH_SubBandSel::make (const string& name) const
{
  return new WH_SubBandSel (name, itsNsubband, itsFileName);
}

void WH_SubBandSel::process()
{
  int* buf = itsOutHolder.getBuffer();
  for (int i=0; i<itsNsubband; i++) {
    itsFile >> buf[i];
    if (itsFile.eof()) {
      itsFile.close();
      itsFile.open (itsFileName.c_str());
      itsFile >> buf[i];
      AssertStr (!itsFile.eof(), "File " << itsFileName << " is empty");
    }
    AssertStr (!itsFile.fail(), "File " << itsFileName << " has wrong format");
  }
}

void WH_SubBandSel::dump() const
{
  cout << "WH_SubBandSel " << getName() << " Buffers:" << endl;
  cout << itsOutHolder.getBuffer()[0] << ','
       << itsOutHolder.getBuffer()[itsNsubband-1] << endl;
}


DataHolder* WH_SubBandSel::getInHolder (int channel)
{
  AssertStr (channel < 0,
	     "input channel too high");
  return 0;
}
DH_SubBandSel* WH_SubBandSel::getOutHolder (int channel)
{
  AssertStr (channel < 1,
	     "output channel too high");
  return &itsOutHolder;
}
