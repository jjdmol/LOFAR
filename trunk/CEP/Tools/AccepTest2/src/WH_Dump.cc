//  WH_Dump.cc: Dump to output stream
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <AccepTest2/WH_Dump.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>
#include <iomanip>

namespace LOFAR
{

  using std::endl;
  using std::setw;

  WH_Dump::WH_Dump (const string& name, 
		    int matrixXsize, 
		    int matrixYsize,
		    ostream& os)
    : WorkHolder (1, 0, name, "WH_Dump"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize),
      itsOs(os)
  {
    LOG_TRACE_VAR("Adding in dataholder of wh_dump");
    getDataManager().addInDataHolder(0, new DH_CMatrix("dump_input", 
						       matrixXsize, 
						       matrixYsize, 
						       "x", 
						       "y"));
  }

  WH_Dump::~WH_Dump()
  {}

  WorkHolder* WH_Dump::construct (const string& name, 
				  int matrixXsize, 
				  int matrixYsize,
				  ostream& os)
  {
    return new WH_Dump (name, matrixXsize, matrixYsize, os);
  }

  WH_Dump* WH_Dump::make (const string& name)
  {
    return new WH_Dump (name, itsMatrixXSize, itsMatrixYSize, itsOs);
  }

  void WH_Dump::process()
  {
    DH_CMatrix* dh_matrix = (DH_CMatrix*)getDataManager().getInHolder(0);
    itsOs<<endl<<"Dumping output ..."<<endl;
    itsOs<<"X-axis: "<<setw(AXISNAMESIZE+2)<<dh_matrix->getXaxis().getName()
      <<" range: ["
      <<setw(10)<<dh_matrix->getXaxis().getBegin()<<", "
      <<setw(10)<<dh_matrix->getXaxis().getEnd()<<"]" <<endl;
    itsOs<<"Y-axis: "<<setw(AXISNAMESIZE+2)<<dh_matrix->getYaxis().getName()
      <<" range: ["
      <<setw(10)<<dh_matrix->getYaxis().getBegin()<<", "
      <<setw(10)<<dh_matrix->getYaxis().getEnd()<<"]" <<endl;
    // print matrix
    for (int yi=0; yi<itsMatrixYSize; yi++) {
      itsOs<<"[ | ";
      for (int xi=0; xi<itsMatrixXSize; xi++) {
	itsOs<<setw(10)<<dh_matrix->value(xi, yi)<<" | ";
      }
      itsOs<<" ]"<<endl;
    }
    //    getDataManager().readyWithInHolder(0);
  }

  void WH_Dump::dump()
  {
  }
}
