//# DummySolver.h: VDM implementation of the meqcalibrater
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

#ifndef SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
#define SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530


#include <aips/Arrays/Vector.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Fitting/FitLSQ.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/Utilities/String.h>

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
#pragma aid SolvableParm SolvableFlag PeelNrs PredNrs
#pragma aid Ant1 Ant2 AntMode CorrSel Niter UseSVD
#pragma aid CalcUVW ModelType MEP GSM
#pragma aid Save Parms Residuals
#pragma aid SolvParams Rank Fit Errors CoVar Flag Mu StdDev Chi

namespace SolverControl
{
  using namespace ApplicationVocabulary;
  
  const HIID  FDomainSize         = AidDomain|AidSize;

  const HIID  FCalcUVW            = AidCalcUVW;
  const HIID  FModelType          = AidModelType;
  const HIID  FMEPName            = AidMEP|AidName;
  const HIID  FGSMName            = AidGSM|AidName;

  const HIID  SolvableParm        = AidSolvableParm;
  const HIID  SolvableFlag        = AidSolvableFlag;
  const HIID  PeelNrs             = AidPeelNrs;
  const HIID  PredNrs             = AidPredNrs;
  const HIID  Ant1                = AidAnt1;
  const HIID  Ant2                = AidAnt2;
  const HIID  AntMode             = AidAntMode;
  const HIID  CorrSel             = AidCorrSel;
  const HIID  Niter               = AidNiter;
  const HIID  UseSVD              = AidUseSVD;

  const HIID  SaveParms           = AidSave|AidParms;
  const HIID  SaveResiduals       = AidSave|AidResiduals;

  const HIID  StSolutionSolvParams= AidSolution|AidSolvParams;
  const HIID  StSolutionRank      = AidSolution|AidRank;
  const HIID  StSolutionFit       = AidSolution|AidFit;
  const HIID  StSolutionErrors    = AidSolution|AidErrors;
  const HIID  StSolutionCoVar     = AidSolution|AidCoVar;
  const HIID  StSolutionFlag      = AidSolution|AidFlag;
  const HIID  StSolutionMu        = AidSolution|AidMu;
  const HIID  StSolutionStdDev    = AidSolution|AidStdDev;
  const HIID  StSolutionChi       = AidSolution|AidChi;
};


class DummySolver : public BaseSolver
{
public:
  DummySolver();

  ~DummySolver();

  virtual void run();
    
//    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

private:
  void processHeader (const DataRecord& initrec, const DataRecord& header);
  void clear();
  void clearDomain();
  void clearExpr();
  void makeExpr();
  void makeStatExpr (int ant, bool wsrtModel);
  void makeBLExpr (bool wsrtModel);
  void initParms();
  void setSolvable (const vector<string>& parms,
		    const vector<bool>& flags);
  void fillUVW();

  void setPeel (const vector<int>& peelSourceNrs,
		const vector<int>& extraSourceNrs);

  void solveSelect (const vector<int>& ant1,
		    const vector<int>& ant2,
		    int antMode,
		    const vector<bool>& corrSel);

  void solve (bool useSVD);

  void saveParms();

  void saveResiduals (const DataRecord::Ref& header);


  void addTileToDomain (VisTile::Ref &tileref);
  void checkInputState (int instat);
  void endDomain ();
  void endSolution (const DataRecord& endrec, const DataRecord::Ref& header);

  double domain_start;
  double domain_end;
  double domain_size;
  bool in_domain;
    
  int ntiles;

  vector<VisTile::Ref>  itsVisTiles;

  ParmTable*            itsMEP;         //# Common parmtable
  ParmTable*            itsGSMMEP;      //# parmtable for GSM parameters
  bool                  itsCalcUVW;
  string                itsModelType;

  vector<bool>          itsCorrSel;     //# correlations to use in a solve
  vector<bool>          itsVisTilesSel; //# VisTiles to use in a solve

  vector<string>        itsSolvParms;
  vector<bool>          itsSolvFlags;

  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000
  MeqDomain             itsSolveDomain;
  LoMat_double          itsAntPos;

  vector<int>           itsBLIndex;     //# expr vector index of a baseline
  MeqSourceList         itsSources;
  vector<int>           itsPeelSourceNrs;
  vector<int>           itsExtraSourceNrs;
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;
  vector<MeqStatSources*> itsStatSrc;
  vector<MeqLofarStatSources*> itsLSSExpr; //# Lofar sources per station
  vector<MeqJonesExpr*> itsStatExpr;    //# Expression per station
  vector<MVBaseline>    itsBaselines;
  vector<MeqHist>       itsCelltHist;   //# Histogram of #cells in time
  vector<MeqHist>       itsCellfHist;   //# Histogram of #cells in freq
  vector<MeqJonesExpr*> itsExpr;        //# solve expression tree per baseline
  vector<MeqJonesExpr*> itsResExpr;     //# residual expr tree per baseline
  vector<MeqExpr*>      itsExprDel;     //# expressions to delete
  vector<MeqJonesExpr*> itsJExprDel;    //# jones expressions to delete
  double itsTimeInterval;

  double itsStartTime;
  double itsEndTime;
  double itsStartFreq;
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;

  String itsDataColName;                //# column containing data
  String itsResColName;                 //# column to store residuals in
  String itsSolveColName;               //# column to be used in solve
                                        //# (is dataColName or resColName)

  FitLSQ       itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<complex<double> > itsDeriv;    //# derivatives of predict
};

#endif /* SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530 */
