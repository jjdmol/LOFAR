//  WH_Dump.cc:
//
//  Copyright (C) 2000, 2001
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
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include <tinyOnlineProto/WH_Dump.h>
#include <tinyOnlineProto/DH_Vis.h>
#include <tinyOnlineProto/DH_Empty.h>


// Print interval
#define INTERVAL 1
#define PLOTSIZE 1000

namespace LOFAR
{

WH_Dump::WH_Dump (const string& name,
		  unsigned int  nin,
		  int           rank)
  : WorkHolder    (nin, 1, name,"WH_Dump"),
    itsIndex   (1),
    itsCounter (0),
    itsRank    (rank)
{
  char str[8];
  // create the dummy input dataholder
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_Vis (string("in_") + str));
  }

  // create the dummy output dataholder
  sprintf (str, "%d", 1);
  getDataManager().addOutDataHolder(0, 
				    new DH_Empty (string("out_") + str));
  
  //  itsBuffer.resize(PLOTSIZE);
  //  itsBuffer = complex<float> (0,0);
}


WH_Dump::~WH_Dump()
{
}

WorkHolder* WH_Dump::construct (const string& name, 
				int nin, 
				int rank)
{
  return new WH_Dump (name, nin, rank);
}

WH_Dump* WH_Dump::make (const string& name)
{
  return new WH_Dump (name, getDataManager().getInputs(), itsRank);
}

void WH_Dump::process()
{
  complex<float> corr[NSTATIONS][NSTATIONS];
  //  corr = complex<float> (0,0);

//   TRACER4("WH_Dump::Process()");
  
  itsCounter++;
  DH_Vis *InDHptr = (DH_Vis*)getDataManager().getInHolder(0);
  
  if (( itsCounter % INTERVAL == 0 ) && ( 2 == itsRank )) {

    memcpy (corr, 
	    InDHptr->getBuffer(), 
	    NSTATIONS*NSTATIONS*sizeof(DH_Vis::BufferType));
    for (int i=0; i<5; i++) {
      for (int j=0; j<5; j++) {
	cout << corr[i][j] / (complex<float>)TSIZE;
      }
      cout << endl;
    }
    cout << endl;
  }
  getDataManager().readyWithInHolder(0);
}
void WH_Dump::dump()
{
}

}// namespace LOFAR
