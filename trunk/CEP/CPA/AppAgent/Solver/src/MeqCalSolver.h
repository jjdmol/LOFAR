//# MeqCalSolver.h: VDM implementation of the meqcalibrater
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef SOLVER_SRC_MEQCALSOLVER_H_HEADER_INCLUDED_D8FEA530
#define SOLVER_SRC_MEQCALSOLVER_H_HEADER_INCLUDED_D8FEA530


#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Fitting/FitLSQ.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/BasicSL/String.h>

#include <MNS/MeqDomain.h>
#include <MNS/MeqHist.h>
#include <MNS/MeqJonesExpr.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqParm.h>
#include <MNS/MeqPhaseRef.h>
#include <MNS/MeqSourceList.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqStatSources.h>
#include <MNS/MeqLofarStatSources.h>
#include <MNS/MeqStatUVW.h>
#include <MNS/ParmTable.h>

#include <Solver/BaseSolver.h>
#include <Solver/AID-Solver.h>

#pragma aidgroup Solver
#pragma aid SolvableParm SolvableFlag Peel Pred Index Apply
#pragma aid Ant1 Ant2 AntMode CorrSel Niter UseSVD
#pragma aid CalcUVW ModelType MEP GSM
#pragma aid Save Solvable Param Params Residuals
#pragma aid Names Values Rank Fit Errors CoVar Flag Mu StdDev Chi

#pragma aid Domain Solve Iter Num Intermediate Final 
#pragma aid Current Start End Time Tile Count

namespace SolverControl
{
  using namespace ApplicationVocabulary;
  
  // init, start-solution and end-solution record fields
  const HIID  FDomainSize         = AidDomain|AidSize;

  const HIID  FCalcUVW            = AidCalcUVW;
  const HIID  FModelType          = AidModelType;
  const HIID  FMEPName            = AidMEP|AidName;
  const HIID  FGSMName            = AidGSM|AidName;

  const HIID  SolvableParm        = AidSolvable|AidParams;
  const HIID  SolvableFlag        = AidSolvable|AidFlag;
  const HIID  PeelNrs             = AidPeel|AidIndex;
  const HIID  PredNrs             = AidPred|AidIndex;
  const HIID  Ant1                = AidAnt1;
  const HIID  Ant2                = AidAnt2;
  const HIID  AntMode             = AidAntMode;
  const HIID  CorrSel             = AidCorrSel;
  const HIID  UseSVD              = AidUseSVD;

  const HIID  SaveParms           = AidSave|AidParams;
  const HIID  SaveResiduals       = AidSave|AidResiduals;
  const HIID  ApplyPeel           = AidApply|AidPeel;
  
  // additional dataset header fields
  const HIID  FDomainStartTime    = AidDomain|AidStart|AidTime,
              FDomainEndTime      = AidDomain|AidEnd|AidTime,
              FSolveIterNum       = AidSolve|AidIter|AidNum,
              
              DataType_Intermediate = AidDomain|AidIntermediate|AidResiduals,
              DataType_Final        = AidDomain|AidFinal|AidResiduals;
  
  
  // status record fields
  const HIID  StTileCount          = AidTile|AidCount;
  const HIID  StSolutionTileCount  = AidSolution|AidTile|AidCount;
  const HIID  StDomainStart        = AidStart|AidTime;
  const HIID  StDomainEnd          = AidEnd|AidTime;
  const HIID  StDomainCurrent      = AidCurrent|AidTime;
  
  const HIID  StParamValues   = AidParam|AidValues;
  const HIID  StParamNames    = AidParam|AidNames;
  const HIID  StRank          = AidRank;
  const HIID  StFit           = AidFit;
  const HIID  StErrors        = AidErrors;
  const HIID  StCoVar         = AidCoVar;
  const HIID  StFlag          = AidFlag;
  const HIID  StMu            = AidMu;
  const HIID  StStdDev        = AidStdDev;
  const HIID  StChi           = AidChi;
};


//##ModelId=3EC9F6EB0396
class MeqCalSolver : public BaseSolver
{
public:
    //##ModelId=3EC9F6EC0231
  MeqCalSolver();

    //##ModelId=3EC9F6EC0232
  ~MeqCalSolver();

    //##ModelId=3EC9F6EC0233
  virtual void run();
    
//    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

private:
    //##ModelId=3EC9F6EC0236
  void processHeader (const DataRecord& initrec, const DataRecord& header);
    //##ModelId=3EC9F6EC0239
  void clear();
    //##ModelId=3EC9F6EC023D
  void clearDomain();
    //##ModelId=3EC9F6EC023F
  void clearExpr();
    //##ModelId=3EC9F6EC0241
  void makeExpr();
    //##ModelId=3EC9F6EC0243
  void makeStatExpr (int ant, bool wsrtModel, bool asAP);
    //##ModelId=3EC9F6EC0248
  void makeBLExpr (bool wsrtModel);
    //##ModelId=3EC9F6EC024A
  void initParms();
    //##ModelId=3EC9F6EC024C
  void setSolvable (const vector<string>& parms,
		    const vector<bool>& flags);
    //##ModelId=3EC9F6EC024F
  void fillUVW();

    //##ModelId=3EC9F6EC0252
  void setPeel (const vector<int>& peelSourceNrs,
		const vector<int>& extraSourceNrs);

    //##ModelId=3EC9F6EC0255
  void solveSelect (const vector<int>& ant1,
		    const vector<int>& ant2,
		    int antMode,
		    const vector<bool>& corrSel);

    //##ModelId=3EC9F6EC025A
  void solve (bool useSVD, DataRecord::Ref& header);

    //##ModelId=3EC9F6EC0260
  void saveParms();

    //##ModelId=3EC9F6EC0262
  void saveResiduals (DataRecord::Ref& header,bool apply_peel );


    //##ModelId=3EC9F6EC0265
  void addTileToDomain (VisTile::Ref &tileref);
    //##ModelId=3EC9F6EC0267
  void checkInputState (int instat);
    //##ModelId=3EC9F6EC026A
  void endDomain ();
    //##ModelId=3EC9F6EC026C
  void endSolution (const DataRecord& endrec,DataRecord::Ref& header);

    //##ModelId=3EC9F6EC01DF
  double domain_start;
    //##ModelId=3EC9F6EC01E1
  double domain_end;
    //##ModelId=3EC9F6EC01E2
  double domain_size;
    //##ModelId=3EC9F6EC01E4
  bool in_domain;
  
    //##ModelId=3F5F436400AE
  VisTile::Format::Ref tileformat_;
    //##ModelId=3F5F436400EA
  int  dataset_seq_num_;
    //##ModelId=3F5F43640128
  DataRecord::Ref solution_;
    
    //##ModelId=3EC9F6EC01E6
  int ntiles_;

    //##ModelId=3EC9F6EC01E7
  vector<VisTile::Ref>  itsVisTiles;

    //##ModelId=3EC9F6EC01E9
  ParmTable*            itsMEP;         //# Common parmtable
    //##ModelId=3EC9F6EC01EB
  ParmTable*            itsGSMMEP;      //# parmtable for GSM parameters
    //##ModelId=3EC9F6EC01ED
  bool                  itsCalcUVW;
    //##ModelId=3EC9F6EC01EE
  string                itsModelType;

    //##ModelId=3EC9F6EC01F0
  vector<bool>          itsCorrSel;     //# correlations to use in a solve
    //##ModelId=3EC9F6EC01F2
  vector<bool>          itsVisTilesSel; //# VisTiles to use in a solve

    //##ModelId=3EC9F6EC01F3
  vector<string>        itsSolvParms;
    //##ModelId=3EC9F6EC01F5
  vector<bool>          itsSolvFlags;

    //##ModelId=3EC9F6EC01F7
  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000
    //##ModelId=3EC9F6EC01F8
  MeqDomain             itsSolveDomain;
    //##ModelId=3EC9F6EC01FA
  LoMat_double          itsAntPos;

    //##ModelId=3EC9F6EC01FC
  vector<int>           itsBLIndex;     //# expr vector index of a baseline
    //##ModelId=3EC9F6EC01FD
  MeqSourceList         itsSources;
    //##ModelId=3EC9F6EC01FF
  vector<int>           itsPeelSourceNrs;
    //##ModelId=3EC9F6EC0201
  vector<int>           itsExtraSourceNrs;
    //##ModelId=3EC9F6EC0202
  vector<MeqStation*>   itsStations;
    //##ModelId=3EC9F6EC0204
  vector<MeqStatUVW*>   itsStatUVW;
    //##ModelId=3EC9F6EC0206
  vector<MeqStatSources*> itsStatSrc;
    //##ModelId=3EC9F6EC0208
  vector<MeqLofarStatSources*> itsLSSExpr; //# Lofar sources per station
    //##ModelId=3EC9F6EC0209
  vector<MeqJonesExpr*> itsStatExpr;    //# Expression per station
    //##ModelId=3EC9F6EC020B
  vector<MVBaseline>    itsBaselines;
    //##ModelId=3EC9F6EC020D
  vector<MeqHist>       itsCelltHist;   //# Histogram of #cells in time
    //##ModelId=3EC9F6EC020E
  vector<MeqHist>       itsCellfHist;   //# Histogram of #cells in freq
    //##ModelId=3EC9F6EC0210
  vector<MeqJonesExpr*> itsExpr;        //# solve expression tree per baseline
    //##ModelId=3EC9F6EC0212
  vector<MeqJonesExpr*> itsResExpr;     //# residual expr tree per baseline
    //##ModelId=3EC9F6EC0213
  vector<MeqExpr*>      itsExprDel;     //# expressions to delete
    //##ModelId=3EC9F6EC0215
  vector<MeqJonesExpr*> itsJExprDel;    //# jones expressions to delete
    //##ModelId=3EC9F6EC0217
  double itsTimeInterval;

    //##ModelId=3EC9F6EC0219
  double itsStartTime;
    //##ModelId=3EC9F6EC021A
  double itsEndTime;
    //##ModelId=3EC9F6EC021C
  double itsStartFreq;
    //##ModelId=3EC9F6EC021E
  double itsEndFreq;
    //##ModelId=3EC9F6EC021F
  double itsStepFreq;
    //##ModelId=3EC9F6EC0221
  int    itsNrChan;

    //##ModelId=3EC9F6EC0223
  String itsDataColName;                //# column containing data
    //##ModelId=3EC9F6EC0224
  String itsResColName;                 //# column to store residuals in
    //##ModelId=3EC9F6EC0226
  String itsSolveColName;               //# column to be used in solve
                                        //# (is dataColName or resColName)

    //##ModelId=3EC9F6EC0228
  FitLSQ       itsSolver;
    //##ModelId=3EC9F6EC022A
  int          itsNrScid;               //# Nr of solvable parameter coeff.
    //##ModelId=3EC9F6EC022B
  MeqMatrix    itsSolution;             //# Solution as complex numbers
    //##ModelId=3EC9F6EC022D
  vector<double> itsFitME;
    //##ModelId=3EC9F6EC022F
  vector<complex<double> > itsDeriv;    //# derivatives of predict
};

#endif /* SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530 */
