//#  WH_Random.cc: An empty WorkHolder (doing nothing)
//#
//#  Copyright (C) 2000, 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

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
          DH_CMatrix::valueType c(makedcomplex((float)number,(float)number+1));
	  number+=2;
#else
	  DH_CMatrix::valueType c(makedcomplex((float)rand()/RAND_MAX,(float)rand()/RAND_MAX));
#endif
	  dh_matrix->value(ti, fi) = c;
	  //cout<<"xi, yi, c, matrix[xi,yi]: "<<xi<<", "<<yi<<", "<<c<<", "<<dh_matrix->value(xi,yi)<<endl;
	}
      }
    }
    itsLastTime++;
    //getDataManager().readyWithOutHolder(outi);
  }

  void WH_Random::dump()
  {
  }
}
