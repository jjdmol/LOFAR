//  WH_Random.cc: An empty WorkHolder (doing nothing)
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

#include <AccepTest2/WH_Random.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>

namespace LOFAR
{

  int WH_Random::theirBaseNumber = 0;

  WH_Random::WH_Random (const string& name, 
			int noutputs, 
			int matrixXsize, 
			int matrixYsize)
    : WorkHolder (0, noutputs, name, "WH_Random"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize),
      itsNOutputs(noutputs),
      itsLastTime(0)
  {
    //    getDataManager().addInDataHolder(0, new DH_Empty());
    char buffer[64];
    for (int i=0; i<noutputs; i++) {
      sprintf(buffer, "random_matrix_%d", i);
      LOG_TRACE_VAR("Adding out dataholder of wh_random");
      getDataManager().addOutDataHolder(i, new DH_CMatrix(buffer, 
							  matrixXsize, 
							  matrixYsize, 
							  "time", 
							  "frequency"));
    }
#ifdef USE_INCREMENTAL
    theirBaseNumber += 10000;
    number = theirBaseNumber;
    //number = 10000*((rand())/(RAND_MAX/10));
#endif
  }

  WH_Random::~WH_Random()
  {}

  WorkHolder* WH_Random::construct (const string& name, 
				    int noutputs, 
				    int matrixXsize, 
				    int matrixYsize)
  {
    return new WH_Random (name, noutputs, matrixXsize, matrixYsize);
  }

  WH_Random* WH_Random::make (const string& name)
  {
    return new WH_Random (name, itsNOutputs, itsMatrixXSize, itsMatrixYSize);
  }

  void WH_Random::process()
  {
    // this is the reference implementation
    // each point in the matrix is filled with a random complex number
    // this is expensive!  
    DH_CMatrix* dh_matrix;

    for (int outi=0; outi<itsNOutputs; outi++) {
      // set axis properties
      dh_matrix = (DH_CMatrix*)getDataManager().getOutHolder(outi);
      dh_matrix->getXaxis().setBegin(itsLastTime);
      dh_matrix->getXaxis().setEnd(itsLastTime + 1);
      dh_matrix->getYaxis().setBegin(outi);
      dh_matrix->getYaxis().setEnd(outi+1);

      // fill matrix
      for (int fi=0; fi<itsMatrixYSize; fi++) {
	for (int ti=0; ti<itsMatrixXSize; ti++) {
#ifdef USE_INCREMENTAL
	  DH_CMatrix::valueType c((float) number, (float) number+1);
	  number+=2;
#else
	  DH_CMatrix::valueType c((float) rand()/RAND_MAX, (float) rand()/RAND_MAX);
#endif
	  dh_matrix->value(ti, fi) = c;
	  //cout<<"xi, yi, c, matrix[xi,yi]: "<<xi<<", "<<yi<<", "<<c<<", "<<dh_matrix->value(xi,yi)<<endl;
	}
      }
      getDataManager().readyWithOutHolder(outi);
    }
    itsLastTime++;
  }

  void WH_Random::dump()
  {
  }
}
