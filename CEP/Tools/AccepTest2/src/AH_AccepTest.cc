//  AH_AccepTest.cc: Application holder for acceptance test 2
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
#include <Common/LofarLogger.h>

namespace LOFAR
{
  AH_AccepTest::AH_AccepTest()
  {};
  ~AH_AccepTest::AH_AccepTest()
  {};

  void AH_AccepTest::define(const KeyValueMap& kvm)
  {
    undefine();

    int NoRndNodes = kvm.getInt("RandomNodes", 3);
    int NoHeatLines = kvm.getInt("HeatLines", 2);
    int NoSplitters = kvm.getInt("Splitters", 1);
    int NoHeatsPerSplit = kvm.getInt("HeatsPerSplit", 2);
    int MatrixX = kvm.getInt("RandomMatrixX", 5);
    int MatrixY = kvm.getInt("RandomMatrixY", 5);
    ASSERT_MSG(NoSplitters<NoHeatLines, "There should be more heat lines than splitted heat lines.");
    
    vector<WH_Random*> RndNodes;
    vector<WH_Transpose*> TransposeNodes;
    vector<WH_Heat1*> Heat1Nodes;
    vector<WH_Dump*> DumpNodes;
    vector<WH_Split*> SplitNodes;
    vector<WH_Heat2*> Heat2Nodes;
    vector<WH_Join*> JoinNodes;

    //    itsNoWHs = NoRndNodes + 
    //      NoHeatLines * ( 3 ) + // each heatline has a transpose, heat and dump
    //      NoSplitters * 2 + NoSplitters * NoHeatsPerSplit; // the splitted heatline has a split, join and a number of splits

    // create workholders
    char WH_DH_Name[128];
    for (int i=0; i<NoRndNodes; i++) {
      // random nodes
      sprintf(WH_DH_Name, "Random_generator_%d_of_%d", i, NoRndNodes);
      RndNodes.push_back(new WH_Random(WH_DH_Name,
					  NoHeatLines,
					  MatrixX,
					  MatrixY));
      itsWHs.push_back((WorkHolder*) RndNodes[i]);
    };
    for (int i=0; i<NoHeatLines; i++) {
      // transpose node
      sprintf(WH_DH_Name, "Transpose_%d_of_%d", i, NoHeatLines);
      TransposeNodes.push_back(new WH_Transpose(WH_DH_Name,
						NoRndNodes,
						MatrixX,
						MatrixY));
      itsWHs.push_back((WorkHolder*) TransposeNodes[i]);

      // heat1 nodes
      sprintf(WH_DH_Name, "Heat1_%d_of_%d", i, NoHeatLines);
      Heat1Nodes.push_back(new WH_Heat1(WH_DH_Name,
					NoRndNodes*MatrixY,
					MatrixX));
      itsWHs.push_back((WorkHolder*) Heat1Nodes[i]);

      // dump nodes
      sprintf(WH_DH_Name, "Dump_%d_of_%d", i, NoHeatLines);
      DumpNodes.push_back(new WH_Dump(WH_DH_Name,
				      NoRndNodes*MatrixY,
				      MatrixY));
      itsWHs.push_back((WorkHolder*) DumpNodes[i]);
    };
    // create the (extra) workholders of the splitted heat lines
    for (int i=0; i<NoSplitters; i++) {
      // split node
      sprintf(WH_DH_Name, "Split_%d_of_%d", i, NoSplitters);
      SplitNodes.push_back(new WH_Split(WH_DH_Name,
					NoHeatsPerSplit,
					NoRndNodes*MatrixY/NoHeatsPerSplit, // there is no check to see if this is an integer
					MatrixX));
      itsWHs.push_back((WorkHolder*) SplitNodes[i]);

      for (int j=0; j<NoHeatPerSplit; j++) {
	// heat2 node
	sprintf(WH_DH_Name, "Heat_%d_of_%d_in_line_%d_of_%d", j, NoHeatsPerSplit, i, NoHeatLines);
	Heat2Nodes.push_back(new WH_Heat1(WH_DH_Name,
					      NoRndNodes*MatrixY/NoHeatsPerSplit, // there is no check to see if this is an integer
					      MatrixX));
	itsWHs.push_back((WorkHolder*) Heat2Nodes[i]);
      }

      // join node
      sprintf(WH_DH_Name, "Join_%d_of_%d", i, NoHeatLines);
      JoinNodes.push_back(new WH_Transpose(WH_DH_Name,
					   NoHeatsPerSplit,
					   NoRndNodes*MatrixY/NoHeatsPerSplit, // there is no check to see if this is an integer
					   MatrixX));
      itsWHs.push_back((WorkHolder*) JoinNodes[i]);
    };


    // connect workholders
    for (int Hi = 0; Hi < NoHeatLines; Hi++) {
      // transpose nodes to random nodes
      for (int Ri = 0; Ri < NoRndNodes; Ri ++) {
	TransposeNodes[Hi]->getDataManager().getInHolder(Ri)->connectTo
	  ( RandomNodes[Ri]->getDataManager().getOutHolder(Hi), ??TH);
      }
      // heat1 nodes to transpose nodes
      Heat1Nodes[Hi]->getDataManager().getInHolder(0)->connectTo
	  ( TransposeNodes[Hi]->getDataManager().getOutHolder(0), ??TH);
    }
    for (int Si = 0; Si < NoSplitters; Si++) {
      // split nodes to heat1 nodes
      SplitNodes[Si]->getDataManager().getInHolder(0)->connectTo
	  ( Heat1Nodes[Si]->getDataManager().getOutHolder(0), ??TH);
      // heat2 nodes to split nodes
      for (int Hi = 0; Hi < NoHeatsPerSplit; Hi++) {
	Heat2Nodes[Si*NoHeatsPerSplit + Hi]->getDataManager().getInHolder(0)->connectTo
	  ( SplitNodes[Si]->getDataManager().getOutHolder(Hi), ??TH);
      }
      // join to heat2
      for (int Hi = 0; Hi < NoHeatsPerSplit; Hi++) {
	JoinNodes[Si]->getDataManager().getInHolder(Hi)->connectTo
	  ( Heat2Nodes[Si*NoHeatsPerSplit + Hi]->getDataManager().getOutHolder(0), ??TH);
      }
      // dump to join
      DumpNodes[Si]->getDataManager().getInHolder(0)->connectTo
	( JoinNodes[Si]->getDataManager().getOutHolder(0), ??TH);
    }
    for (int Hi = NoSplitters; Hi < NoHeatLines; Hi++) {
      // dump to heat1
      DumpNodes[Hi]->getDataManager().getInHolder(0)->connectTo
	( Heat1Nodes[Hi]->getDataManager().getOutHolder(0), ??TH);
    }      
  };

  void AH_AccepTest::undefine()
  {
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it!=itsWHs.end(); it++) {
      delete *it;
    }
    itsWHs.clear();
  }

  void AH_AccepTest::init() {
    // call preprocess method on all WH's
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it!=itsWHs.end(); it++) {
      (*it)->basePreprocess();
    }
  }
  
  void AH_AccepTest::run(int nsteps) {
    // call process method on all WH's
    int NoWHs = itsWHs.size();
    for (int s = 0; s < nsteps; s++) {
      for (int i = 0; i < NoWHs; i++) {
	itsWHs[i]->baseProcess();
      }
    }
  }
  
  void AH_AccepTest::quit() {
  }
}
