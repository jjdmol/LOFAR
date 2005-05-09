//#  AH_TestFilter.cc:
//#
//#  Copyright (C) 2002-2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AH_TestFilter.h>
#include <Transport/TH_Mem.h>

#include <ACC/ParameterSet.h>

namespace LOFAR
{

  AH_TestFilter::AH_TestFilter() :
  itsRank (0)
  {
#ifdef HAVE_MPI
    itsRank = TH_MPI::getCurrentRank();
#endif

  }

  AH_TestFilter::~AH_TestFilter() {
  }

  void AH_TestFilter::define(const KeyValueMap& /*params*/) {

    ACC::ParameterSet myPS("TFlopCorrelator.cfg");
    char str[32];

    itsWHs.push_back((WorkHolder*) new WH_FilterInput("input"));
    itsWHs.push_back((WorkHolder*) new WH_SubBand("filter", 0));

    int outputs = myPS.getInt("Corr_per_Filter");
    itsWHs.push_back((WorkHolder*) new WH_FilterOutput(str,outputs));

    // we only need to run the test on a single node
    itsWHs[0]->runOnNode(0);
    itsWHs[1]->runOnNode(0);
    itsWHs[2]->runOnNode(0);

    // connect the WorkHolders using TH_Mem()
    itsWHs[0]->getDataManager().getOutHolder(0)->connectTo
      ( *(itsWHs[1]->getDataManager().getInHolder(0)), 
	  TH_Mem(),
	  false);
    for (int c = 0; c < outputs; c++) {
      itsWHs[1]->getDataManager().getOutHolder(c)->connectTo
	( *(itsWHs[2]->getDataManager().getInHolder(c)), 
	  TH_Mem(),
	  false);
    }
  }

  void AH_TestFilter::undefine() {
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it!=itsWHs.end(); it++) {
      delete *it;
    }
    itsWHs.clear();
  }
  
  void AH_TestFilter::init() {
      vector<WorkHolder*>::iterator it = itsWHs.begin();
      for (; it != itsWHs.end(); it++) {
       	(*it)->basePreprocess();
      }
  }

  void AH_TestFilter::run(int nsteps) {
    
    for (int i = 0; i < nsteps; i++) {
      vector<WorkHolder*>::iterator it = itsWHs.begin();
      for (; it != itsWHs.end(); it++) {
	(*it)->baseProcess();
      }
    }
    
  }

  void AH_TestFilter::dump() {
  }

  void AH_TestFilter::postrun() {
  }

  void AH_TestFilter::quit() {
  }

} // namespace LOFAR
