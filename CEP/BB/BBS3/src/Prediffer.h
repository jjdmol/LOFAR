//# Prediffer.h: Read and predict read visibilities
//#
//# Copyright (C) 2004
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

#ifndef BB_BBS3_PREDIFFER_H
#define BB_BBS3_PREDIFFER_H

#include <casa/Arrays/Matrix.h>
#include <casa/Quanta/MVBaseline.h>

#include <BBS3/ParmData.h>
#include <BBS3/MNS/MeqDomain.h>
#include <BBS3/MNS/MeqHist.h>
#include <BBS3/MNS/MeqJonesExpr.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqParm.h>
#include <BBS3/MNS/MeqPhaseRef.h>
#include <BBS3/MNS/MeqSourceList.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqStation.h>
#include <BBS3/MNS/MeqStatSources.h>
#include <BBS3/MNS/MeqLofarStatSources.h>
#include <BBS3/MNS/MeqStatUVW.h>
#include <BBS3/MNS/ParmTable.h>

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <list>

namespace LOFAR
{

//# Forward Declarations
class MMap;

// Prediffer calculates the equations for the solver.
// It reads the measured data and predicts the data from the model.
// It subtracts them from each other and calculates the derivatives.
// These results can be sent to the solver to find better parameter values.

class Prediffer
{

public:
  // Create Prediffer object for a specific
  // MeaurementSet, MEQ model (with associated MEP database) and skymodel
  // for the specified data descriptor (i.e. spectral window) and antennas.
  // The database type (aips or postgres) has to be given.
  // For postgres the database name has to be given as well.
  // Currently model types WSRT, LOFAR.RI and LOFAR.AP are recognized.
  // The UVW coordinates can be recalculated or taken from the MS.
  Prediffer (const string& msName,
	     const string& meqModel,
	     const string& skyModel,
	     const string& dbType,
	     const string& dbName,
	     const string& dbHost,
	     const string& dbPwd,
	     uint ddid,
	     const vector<int>& ant,
	     const string& modelType,
	     bool calcUVW,
	     bool lockMappedMem);

  // Destructor
  ~Prediffer();

  // Make a selection of the MS to be used in the domain iteration.
  void select (const vector<int>& ant1, const vector<int>& ant2,
	       int itsFirstChan, int itsLastChan);

  // Start another time interval.
  // Hereafter getSolvableParmData can be called.
  // It returns false if the start value is outside the observation domain.
  // Length is trimmed if beyond end of observation.
  bool nextInterval (double start, double length, bool callReadPolcs = true);

  // Update the solvable parm values (reread from table).
  void updateSolvableParms();

  // It returns the solvable parms.
  // The parms are in ascending order of spidnr.
  const vector<ParmData>& getSolvableParmData() const
    { return itsParmData; }

  // Make all parameters non-solvable.
  void clearSolvableParms();

  // Make specific parameters solvable (isSolvable = True) or
  // non-solvable (False).
  void setSolvableParms (vector<string>& parms, 
			 vector<string>& excludePatterns,
			 bool isSolvable);

  // Get the equations for all selected baselines.
  std::list<MeqResult> getEquations ();

  // Save residual data in the itsResColName column.
  // It does a predict for the sources to be peeled off and subtracts
  // the results from the itsSolveColName column.
  void saveResidualData();

  // Set the source numbers to use in this peel step.
  bool peel (const vector<int>& peelSources,
	     const vector<int>& extraSources);

  // Set the names and values of all solvable parms for the current domain.
  // The double version can only be used if all parms are 0th-order
  // polynomials.
  // <group>
  void getParmValues (vector<string>& names,
		      vector<double>& values);
  void getParmValues (vector<string>& names,
		      vector<MeqMatrix>& values);
  // </group>

  // Set the given values (for the current domain) of parms matching
  // the corresponding name.
  // Values with a name matching no parm, are ignored.
  // <group>
  void setParmValues (const vector<string>& names,
		      const vector<double>& values);
  void setParmValues (const vector<string>& names,
		      const vector<MeqMatrix>& values);
  // </group>

  // Get nr of channels.
  int getNrChan() const
    { return itsNrChan; }

  // Show the settings of the Prediffer.
  void showSettings() const;

private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Prediffer (const Prediffer& other);
  Prediffer& operator= (const Prediffer& other);
  // </group>

  // Get measurement set description from file
  void readDescriptiveData (const string& fileName);

  // initialize all parameters in the MeqExpr tree for the current domain
  void initParms (const MeqDomain& domain, bool callReadPocs);

  // Get the phase reference position of the first field.
  void getPhaseRef (double ra, double dec, double startTime);

  // Get the station info (position and name).
  void fillStations (const vector<unsigned int>& ant1,
		     const vector<unsigned int>& ant2);

  // Get all baseline info.
  void fillBaselines (const vector<unsigned int>& ant1,
		      const vector<unsigned int>& ant2);

  // Fill all UVW coordinates if they are not calculated.
  void fillUVW();

  // Create the WSRT expressions for each baseline.
  void makeWSRTExpr();

  // Create the LOFAR expressions for each baseline.
  // The EJones can be expressed as real/imag or ampl/phase.
  void makeLOFARExpr (casa::Bool asAP);

  MeqResult getEquation (const MeqRequest& request,
			 int bl, int ant1, int ant2,
			 bool showd);


  string                itsMSName;      // Measurement set name
  string                itsMEPName;     // Common parmtable name
  ParmTable             itsMEP;         //# Common parmtable
  string                itsGSMMEPName;  // GSM parameters parmtable name
  ParmTable             itsGSMMEP;      //# parmtable for GSM parameters
  bool                  itsCalcUVW;

  int                   itsFirstChan;   //# first channel selected
  int                   itsLastChan;    //# last channel selected

  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000
  MeqDomain             itsSolveDomain;

  casa::Matrix<int>           itsBLIndex;     //# baseline index of antenna pair
  MeqSourceList         itsSources;
  casa::Vector<int>     itsPeelSourceNrs;
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;
  vector<MeqStatSources*> itsStatSrc;
  vector<MeqLofarStatSources*> itsLSSExpr; //# Lofar sources per station
  vector<MeqJonesExpr*> itsStatExpr;    //# Expression per station
  vector<casa::MVBaseline>     itsBaselines;
  vector<MeqHist>       itsCelltHist;   //# Histogram of #cells in time
  vector<MeqHist>       itsCellfHist;   //# Histogram of #cells in freq
  vector<MeqJonesExpr*> itsExpr;        //# solve expression tree per baseline
  vector<MeqJonesExpr*> itsResExpr;     //# residual expr tree per baseline

  double itsStartFreq;
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;

  string itsSolveFileName;            //# Data file used (.dat or .res)

  int          itsNrScid;             //# Nr of solvable parameter coeff.
  vector<bool> itsIsParmSolvable;     //# is corresponding parmlist solvable?
  casa::Vector<casa::String> itsSolvableParms;    // Solvable parameters
  vector<ParmData> itsParmData;       // solvable parm info. 

  casa::Vector<int>    itsAnt1Data;         // Antenna 1 data
  casa::Vector<int>    itsAnt2Data;         // Antenna 2 data
  int                  itsNPol;             // Number of polarisations
  casa::Vector<double> itsTimes;            // All times in MS
  casa::Vector<double> itsIntervals;        // All intervals in MS
  casa::Matrix<double> itsAntPos;           // All antenna positions
  unsigned int         itsNrBl;             // Total number of unique baselines
  casa::Matrix<bool>   itsBLSelection;      // Matrix to indicate which baselines are selected
  vector<int>    itsSelAnt;         // The selected antennas

  unsigned int   itsTimeIndex;      // The index of the current time
  unsigned int   itsNrTimes;        // The number of times in the time interval
  MMap*          itsDataMap;        // Data file to map
  bool           itsLockMappedMem;  // Lock memory immediately after mapping?
};

} // namespace LOFAR

#endif
