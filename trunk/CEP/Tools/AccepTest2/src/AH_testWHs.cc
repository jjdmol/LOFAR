//  AH_testWHs.cc: Application holder for acceptance test 2
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
#include <AccepTest2/WH_Transpose.h>
#include <AccepTest2/WH_Dump.h>
#include <AccepTest2/WH_Heat.h>
#include <AccepTest2/WH_Split.h>
#include <AccepTest2/WH_Join.h>
#include <Common/LofarLogger.h>
#include <AH_testWHs.h>
#include <Transport/TH_Mem.h>

namespace LOFAR
{
  AH_testWHs::AH_testWHs() : itsFileOutput(0)
  {};
  AH_testWHs::~AH_testWHs()
  {};

  void AH_testWHs::define(const KeyValueMap& kvm)
  {
    undefine();

    // These variables can be chosen just the way you like them, EXCEPT:
    //     MatrixY*NoRndNodes must be a multiple of NoSplitsPerSplitter
    //     and of course NoSplitters <= NoHeatLines
    int NoRndNodes = 10;
    int NoHeatLines = 10;
    int NoSplitters = 5;
    int NoSplitsPerSplitter = 10;
    int MatrixX = 100;
    int MatrixY = 100;
    
    TH_Mem myTransProto;
    itsFileOutput = new fstream("/dev/null", fstream::out);
    // you can choose the output for the WH_Dump here
    // of course this can also be done per workholder
    //ostream& myOutput = cout;
    ostream& myOutput = *itsFileOutput;

    vector<WH_Random*> RandomNodes;
    vector<WH_Transpose*> TransposeNodes;
    vector<WH_Dump*> DumpNodes;
    vector<WH_Heat*> Heat1Nodes;
    vector<WH_Split*> SplitNodes;
    vector<WH_Heat*> Heat2Nodes;
    vector<WH_Join*> JoinNodes;

    // create workholders
    char WH_DH_Name[128];
    for (int i=0; i<NoRndNodes; i++) {
      // random nodes
      sprintf(WH_DH_Name, "Random_generator_%d_of_%d", i, NoRndNodes);
      RandomNodes.push_back(new WH_Random(WH_DH_Name,
					  NoHeatLines,
					  MatrixX,
					  MatrixY));
      itsWHs.push_back((WorkHolder*) RandomNodes[i]);
    };
    for (int i=0; i<NoHeatLines; i++) {
      // transpose nodes
      sprintf(WH_DH_Name, "Transpose_%d_of_%d", i, NoHeatLines);
      TransposeNodes.push_back(new WH_Transpose(WH_DH_Name,
						NoRndNodes,
						MatrixX,
						MatrixY));
      itsWHs.push_back((WorkHolder*) TransposeNodes[i]);

      // heat nodes
      sprintf(WH_DH_Name, "Heat_%d_of_%d", i, NoHeatLines);
      Heat1Nodes.push_back(new WH_Heat(WH_DH_Name,
				       MatrixY*NoRndNodes,
				       MatrixX));
      itsWHs.push_back((WorkHolder*) Heat1Nodes[i]);
    };

    for (int Si=0; Si<NoSplitters; Si++) {
      // split nodes
      sprintf(WH_DH_Name, "Split_%d_of_%d", Si, NoSplitters);
      SplitNodes.push_back(new WH_Split(WH_DH_Name,
					NoSplitsPerSplitter,
					MatrixY*NoRndNodes/NoSplitsPerSplitter,
					MatrixX));
      itsWHs.push_back((WorkHolder*) SplitNodes[Si]);

      for (int Di=0; Di<NoSplitsPerSplitter; Di++) {
	// heat2 nodes
	sprintf(WH_DH_Name, "Heat2_%d_of_%d_on_line_%d_of_%d", Di, NoSplitsPerSplitter, Si, NoSplitters);
	Heat2Nodes.push_back(new WH_Heat(WH_DH_Name,
					 MatrixY*NoRndNodes/NoSplitsPerSplitter,
					 MatrixX,
					 false));
	itsWHs.push_back((WorkHolder*) Heat2Nodes[Si*NoSplitsPerSplitter+Di]);
      }
      // join nodes
      sprintf(WH_DH_Name, "Join_%d_of_%d", Si, NoSplitters);
      JoinNodes.push_back(new WH_Join(WH_DH_Name,
				      NoSplitsPerSplitter,
				      MatrixY*NoRndNodes/NoSplitsPerSplitter,
				      MatrixX));
      itsWHs.push_back((WorkHolder*) JoinNodes[Si]);
    }
    // create dump nodes last so they process will be called on them last
    for (int i=0; i<NoHeatLines; i++) {
      // dump nodes
      sprintf(WH_DH_Name, "Dump_%d_of_%d", i, NoHeatLines);
      DumpNodes.push_back(new WH_Dump(WH_DH_Name,
				      NoRndNodes*MatrixY,
				      MatrixX,
				      myOutput));
      itsWHs.push_back((WorkHolder*) DumpNodes[i]);
    }



    //connect WorkHolders
    for (int Hi = 0; Hi < NoHeatLines; Hi++) {
      // random nodes to transpose
      for (int Ri = 0; Ri < NoRndNodes; Ri++) {
	RandomNodes[Ri]->getDataManager().getOutHolder(Hi)->connectTo
	  ( *(TransposeNodes[Hi]->getDataManager().getInHolder(Ri)), myTransProto, false);
      }
      // transpose nodes to heat1
      TransposeNodes[Hi]->getDataManager().getOutHolder(0)->connectTo
	( *(Heat1Nodes[Hi]->getDataManager().getInHolder(0)), myTransProto, false);
    }
    for (int Si = 0; Si < NoSplitters; Si++) {
      // heat nodes to split
      Heat1Nodes[Si]->getDataManager().getOutHolder(0)->connectTo
	( *(SplitNodes[Si]->getDataManager().getInHolder(0)), myTransProto, false);
      for (int Hi = 0; Hi < NoSplitsPerSplitter; Hi++) {
	// split nodes to heat2
	SplitNodes[Si]->getDataManager().getOutHolder(Hi)->connectTo
	  ( *(Heat2Nodes[Si*NoSplitsPerSplitter+Hi]->getDataManager().getInHolder(0)), myTransProto, false);
	// heat2 nodes to join
	Heat2Nodes[Si*NoSplitsPerSplitter+Hi]->getDataManager().getOutHolder(0)->connectTo
	  ( *(JoinNodes[Si]->getDataManager().getInHolder(Hi)), myTransProto, false);
      }
      // dump nodes to the join
      JoinNodes[Si]->getDataManager().getOutHolder(0)->connectTo
	( *(DumpNodes[Si]->getDataManager().getInHolder(0)), myTransProto, false);
    }
    // connect the remaining Dump nodes directly to Heat1
    for (int Di = NoSplitters; Di < NoHeatLines; Di++) {
      Heat1Nodes[Di]->getDataManager().getOutHolder(0)->connectTo
	( *(DumpNodes[Di]->getDataManager().getInHolder(0)), myTransProto, false);
    }
  };

  void AH_testWHs::undefine()
  {
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it!=itsWHs.end(); it++) {
      delete *it;
    }
    itsWHs.clear();
    delete itsFileOutput;
  }

  void AH_testWHs::init() {
    // call preprocess method on all WH's
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it!=itsWHs.end(); it++) {
      (*it)->basePreprocess();
    }
  }
  
  void AH_testWHs::run(int nsteps) {
    // call process method on all WH's
    int NoWHs = itsWHs.size();
    for (int s = 0; s < nsteps; s++) {
      for (int i = 0; i < NoWHs; i++) {
	WorkHolder* WH = itsWHs[i];
	WH->baseProcess();
	//itsWHs[i]->baseProcess();
      }
    }
  }
  
  void AH_testWHs::quit() {
  }
}
