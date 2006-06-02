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

#ifndef LOFAR_BB_BBS_PREDIFFER_H
#define LOFAR_BB_BBS_PREDIFFER_H

// \file
// Read and predict read visibilities

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Fitting/LSQFit.h>

#include <BBS/ParmData.h>
#include <BBS/MMapMSInfo.h>
#include <BBS/MNS/MeqDomain.h>
#include <BBS/MNS/MeqJonesExpr.h>
#include <BBS/MNS/MeqJonesNode.h>
#include <BBS/MNS/MeqMatrix.h>
#include <BBS/MNS/MeqParm.h>
#include <BBS/MNS/MeqPhaseRef.h>
#include <BBS/MNS/MeqSourceList.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqStation.h>
#include <BBS/MNS/MeqStatUVW.h>
#include <BBS/MNS/MeqLMN.h>
#include <BBS/MNS/MeqDFTPS.h>
#include <ParmDB/ParmDB.h>

#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace casa {
  class Table;
}

namespace LOFAR
{

// \addtogroup BBS
// @{

//# Forward Declarations
class MMap;
class FlagsMap;
class MeqJonesMMap;

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
  // The UVW coordinates can be recalculated or taken from the MS.
  Prediffer (const string& msName,
	     const ParmDB::ParmDBMeta& meqPtd,
	     const ParmDB::ParmDBMeta& skyPtd,
	     const vector<int>& ant,
	     const string& modelType,
	     const vector<vector<int> >& sourceGroups,
	     bool calcUVW);

  // Destructor
  ~Prediffer();

  // Lock and unlock the parm database tables.
  // The user does not need to lock/unlock, but it can increase performance
  // if many small accesses have to be done.
  // <group>
  void lock (bool lockForWrite = true)
    { itsMEP.lock (lockForWrite); itsGSMMEP.lock (lockForWrite); }
  void unlock()
    { itsMEP.unlock(); itsGSMMEP.unlock(); }

  // Make a selection of the MS to be used in the domain iteration.
  // The size of the antenna vectors is the number of baselines to be used.
  // They give the first and second antenna (i.e. station) of each baseline.
  // <br>The correlation vector tells which correlations to use from the data.
  // 0=XX, 1=XY, 2=YX, 3=YY. An empty vector means all correlations.
  void select (const vector<int>& ant1, const vector<int>& ant2,
	       bool useAutoCorrelations, const vector<int>& corr);

  // Set the work domain (in frequency and time).
  // If needed the given domain is adjusted to the observation domain.
  // It reads all parms in the given domain.
  void setWorkDomain (int startChan, int endChan,
		      double startTime, double lengthTime);

  // Execute the strategy step which defines the tree to be used,
  // the solvable parameters, the solve domain size,
  // the data subset to be used, etc.
  // Hereafter getSolvableParmData can be called.
  ///  void setStrategyStep (const BBSStep&);

  // Get the actual domain.
  const MeqDomain& getWorkDomain() const
    { return itsWorkDomain; }

  // Return the solvable parms.
  // The parms are in ascending order of spidnr.
  const ParmDataInfo& getSolvableParmData() const
    { return itsParmData; }

  // Make all parameters non-solvable.
  void clearSolvableParms();

  // Make specific parameters solvable (isSolvable = True) or
  // non-solvable (False).
  void setSolvableParms (const vector<string>& parms, 
			 const vector<string>& excludePatterns);

  // Initialize all solvable parameters in the MeqExpr tree for the
  // given solve domains.
  // It sets the scids of the solvable parms and fills itsParmData.
  void initSolvableParms (const vector<MeqDomain>& solveDomains);

  // Get the equations for all selected baselines and fill the
  // fitter vector with them.
  // The fitter vector is resized as needed.
  // All fitter objects are initialized before being filled.
  void fillFitters (vector<casa::LSQFit>& fitters,
		    const string& dataColumnName);

  // Apply corrections to the data.
  void correctData (const string& inColumnName,
		    const string& outColumnName, bool flush=false);

  // Subtract the corrupted peel source(s) from the data.
  void subtractData (const string& inColumnName,
		     const string& outColumnName, bool flush=false);

  // Write the predicted data into the .res or .dat file.
  void writePredictedData (const string& outColumnName);

  // Set the source numbers to use in this peel step.
  bool setPeelGroups (const vector<int>& peelGroups,
		      const vector<int>& extraGroups);

  // There are three ways to update the solvable parms after the solver
  // found a new solution.
  // <group>
  // Update the values of solvable parms.
  // Using its spid index each parm takes its values from the vector.
  void updateSolvableParms (const vector<double>& values);

  // Update the given values (for the current domain) of solvable parms
  // matching the corresponding parm name in the vector.
  // Vector values with a name matching no solvable parm are ignored.
  void updateSolvableParms (const ParmDataInfo& parmData);

  // Update the solvable parm values (reread from table).
  void updateSolvableParms();

  // Show the settings of the Prediffer.
  void showSettings() const;

  // Get the results instead of the equations.
  // This is mainly used for test purposes.
  vector<MeqResult> getResults (bool calcDeriv=true);

  // Get the data and flags for the time domain.
  // This is mainly used for test purposes.
  void getData (const string& columnName, bool useTree,
		casa::Array<casa::Complex>& data,
		casa::Array<casa::Bool>& flags);

  // Write the solved parms.
  void writeParms();

private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Prediffer (const Prediffer& other);
  Prediffer& operator= (const Prediffer& other);
  // </group>

  // Get measurement set description from file
  void readDescriptiveData (const string& fileName);

  // Get the phase reference position of the first field.
  void getPhaseRef (double ra, double dec, double startTime);

  // Get the station info (position and name).
  void fillStations (const vector<int>& antnrs);

  // Get all baseline info.
  void fillBaselines (const vector<int>& antnrs);

  // Count nr of baselines and correlations selected.
  void countBaseCorr();

  // Fill all UVW coordinates if they are not calculated.
  void fillUVW();

  // Get all sources from the sky model table.
  // Also check the source groups.
  void getSources();

  // Create the LOFAR expressions for each baseline.
  // The EJones (per patch/station) can be expressed as real/imag
  // or ampl/phase.
  // The station parameters are optionally taken into account.
  void makeLOFARExpr (bool usePatchEJ, bool useTotalEJ, bool asAP,
		      bool useStatParm);

  // Find all nodes to be precalculated.
  void setPrecalcNodes (vector<MeqJonesExpr>& nodes);

  // Fill the fitter with the equations for the given baseline.
  void fillEquation (int threadnr, void* arg,
		     const fcomplex* dataIn, fcomplex* dummy,
		     const bool* flags,
		     const MeqRequest& request, int blindex,
		     bool showd=false);

  // Reset the loop variables for the getEquations loop.
  void resetEqLoop();

  // Map the correct data files (if not mapped yet).
  // If a string is empty, the file is not mapped.
  void mapDataFiles (const string& inFile, const string& outFile);

  // Add a data column to the table and create a symlink for it.
  void addDataColumn (casa::Table& tab, const string& columnName,
		      const string& symlinkName);

  // Define the signature of a function processing a baseline.
  typedef void (Prediffer::*ProcessFuncBL) (int threadnr, void* arg,
					    const fcomplex* dataIn,
					    fcomplex* dataOut,
					    const bool* flags,
					    const MeqRequest& request,
					    int blindex, bool showd);

  // Loop through all data and process each baseline by ProcessFuncBL.
  void processData (const string& inFile, const string& outFile,
		    bool useFlags, bool preCalc, bool calcDeriv,
		    ProcessFuncBL func, void* arg);

  // Subtract the data of a baseline.
  void subtractBL (int threadnr, void* arg,
		   const fcomplex* dataIn, fcomplex* dataOut,
		   const bool* flags,
		   const MeqRequest& request, int blindex,
		   bool showd);

  // Correct the data of a baseline.
  void correctBL (int threadnr, void* arg,
		  const fcomplex* dataIn, fcomplex* dataOut,
		  const bool* flags,
		  const MeqRequest& request, int blindex,
		  bool showd);

  // Write the predicted data of a baseline.
  void predictBL (int threadnr, void* arg,
		  const fcomplex* dummy, fcomplex* dataOut,
		  const bool* flags,
		  const MeqRequest& request, int blindex,
		  bool showd);

  // Get the data of a single baseline.
  // <group>
  void getBL (int threadnr, void* arg,
	      const fcomplex* data, fcomplex*,
	      const bool* flags,
	      const MeqRequest&, int blindex,
	      bool);
  void getMapBL (int threadnr, void* arg,
		 const fcomplex* data, fcomplex*,
		 const bool* flags,
		 const MeqRequest& request, int blindex,
		 bool);
  // </group>

  // Read the polcs for all parameters for the current work domain.
  void readParms();

  // Get access to the next data chunk and fill in all pointers.
  // The data pointers are filled in the MMapMSInfo object.
  bool nextDataChunk (bool useFitters);

  // Do the precalculations for all lower level nodes.
  void precalcNodes (const MeqRequest& request);

  // Show the data on cout.
  void showData (int corr, int sdch, int inc, int nrchan,
		 const bool* flags, const fcomplex* data,
		 const double* realData, const double* imagData);


  string                itsMSName;      //# Measurement set name
  string                itsMEPName;     //# Common parmtable name
  ParmDB::ParmDB        itsMEP;         //# Common parmtable
  string                itsGSMMEPName;  //# GSM parameters parmtable name
  ParmDB::ParmDB        itsGSMMEP;      //# parmtable for GSM parameters
  MeqParmGroup          itsParmGroup;   //# container for all parms
  bool                  itsCalcUVW;

  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000

  MeqSourceList         itsSources;
  vector<vector<int> >  itsSrcGrp;      //# sources in each group
  vector<int>           itsSrcNrMap;    //# map of all srcnr to used srcnr
  vector<int>           itsPeelSourceNrs;
  vector<MeqExpr>       itsLMN;         //# LMN for sources used
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;     //# UVW values per station
  vector<MeqJonesExpr>  itsExpr;        //# solve expression tree per baseline
  vector<vector<MeqExprRep*> > itsPrecalcNodes;  //# nodes to be precalculated
  vector<MeqJonesExpr>  itsCorrStat;    //# Correct per station
  vector<MeqJonesMMap*> itsCorrMMap;    //# MMap for each baseline
  vector<MeqJonesExpr>  itsCorrExpr;    //# Ampl/phase expressions (to correct)

  string itsInDataColumn;
  string itsOutDataColumn;

  double itsStartFreq;                //# start frequency of observation
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;                   //# nr of channels in observation
  int    itsFirstChan;                //# first channel of selected domain
  int    itsLastChan;                 //# last channel of selected domain
  bool   itsReverseChan;              //# Channels are in reversed order
  int    itsDataFirstChan;            //# First channel to use in data
                                      //# (can be different if reversed order)

  string itsSolveFileName;            //# Data file used (.dat or .res)

  MeqDomain    itsWorkDomain;         //# Current work domain
  vector<int>  itsNrScids;            //# Nr of solvable coeff per solve domain
  int          itsNrPert;             //# Nr of perturbed values in result
  ParmDataInfo itsParmData;           //# solvable parm info. 

  bool                 itsCorr[4];     //# Correlations to use
  int                  itsNCorr;       //# Number of correlations (XX, etc.)
  int                  itsNSelCorr;    //# Number of correlations selected
  casa::Vector<double> itsTimes;       //# All times in MS
  casa::Vector<double> itsIntervals;   //# All intervals in MS
  casa::Matrix<double> itsAntPos;      //# All antenna positions

  //# All bselines in the MS are numbered 0 to itsNrBl-1.
  unsigned int         itsNrBl;        //# Total number of baselines in MS
  int                  itsNrUsedBl;    //# nr of used baselines
  vector<int>          itsAnt1;        //# Baseline antenna1 numbers in MS
  vector<int>          itsAnt2;        //# Baseline antenna2 numbers in MS
  //# Define the baselines that can be used (thus selected in constructor).
  //# The baseline index is a sequence number in the itsExpr vector.
  vector<int>          itsBLUsedInx;   //# map of basel index to basel number
  casa::Matrix<int>    itsBLIndex;     //# baseline index of antenna pair
                                       //# -1 means will never be used
  //# Define which baselines are selected in the select function.
  casa::Matrix<bool>   itsBLSelection; //# true = antenna pair is selected
  vector<int>          itsBLSelInx;    //# indices of selected baselines
  unsigned int   itsTimeIndex;     //# The index of the current time
  unsigned int   itsNrTimes;       //# The number of times in the time domain
  unsigned int   itsNrTimesDone;   //# Nr of times done after a setDomain
  vector<double> itsChunkTimes;    //# Times in current data chunk

  MMapMSInfo     itsMSMapInfo;     //# Info about mapped input and output file
  MMap*          itsInDataMap;     //# Input data file mapped
  MMap*          itsOutDataMap;    //# Output data file mapped (can same as in)
  FlagsMap*      itsFlagsMap;      //# Flags file mapped
  MMap*          itsWeightMap;     //# Weights file mapped
  bool           itsIsWeightSpec;  //# true = weight per channel

  //# Fitter info set by initSolvableParms.
  vector<int> itsFreqFitInx;       //# Fitter for each freq in work domain
                                   //# for the data in the MS resolution
  vector<int> itsTimeFitInx;       //# Fitter for each time in work domain
  uint        itsFreqNrFit;        //# Nr of fitters in freq direction

  //# Thread private buffers.
  int itsNthread;
  vector<casa::Block<bool> >      itsFlagVecs;
  vector<vector<const double*> >  itsResultVecs;
  vector<vector<double> >         itsDiffVecs;
  vector<vector<uint> >           itsIndexVecs;
  ///  vector<casa::Block<bool> >     itsOrdFlagVecs;

  //# Timers.
  NSTimer itsPredTimer;
  NSTimer itsEqTimer;
};

// @}

} // namespace LOFAR

#endif
