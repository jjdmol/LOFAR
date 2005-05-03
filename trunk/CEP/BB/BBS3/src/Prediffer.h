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

#ifndef LOFAR_BB_BBS3_PREDIFFER_H
#define LOFAR_BB_BBS3_PREDIFFER_H

// \file Prediffer.h
// Read and predict read visibilities

#include <casa/Arrays/Matrix.h>
#include <casa/Quanta/MVBaseline.h>
#include <scimath/Fitting/LSQFit.h>

#include <BBS3/ParmData.h>
#include <BBS3/MNS/MeqDomain.h>
#include <BBS3/MNS/MeqHist.h>
#include <BBS3/MNS/MeqJonesExpr.h>
#include <BBS3/MNS/MeqJonesNode.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqParm.h>
#include <BBS3/MNS/MeqPhaseRef.h>
#include <BBS3/MNS/MeqSourceList.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqStation.h>
#include <BBS3/MNS/MeqStatUVW.h>
#include <BBS3/MNS/MeqLMN.h>
#include <BBS3/MNS/MeqDFTPS.h>
#include <BBS3/MNS/ParmTable.h>

#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <list>

namespace LOFAR
{

// \addtogroup BBS3
// @{

//# Forward Declarations
class MMap;
class FlagsMap;

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
  // Currently model types LOFAR.RI and LOFAR.AP are recognized.
  // The UVW coordinates can be recalculated or taken from the MS.
  Prediffer (const string& msName,
	     const string& meqModel,
	     const string& skyModel,
	     const string& dbType,
	     const string& dbName,
	     const string& dbHost,
	     const string& dbPwd,
	     const vector<int>& ant,
	     const string& modelType,
	     const vector<vector<int> >& sourceGroups,
	     bool calcUVW,
	     bool lockMappedMem);

  // Destructor
  ~Prediffer();

  // Make a selection of the MS to be used in the domain iteration.
  void select (const vector<int>& ant1, const vector<int>& ant2,
	       bool useAutoCorrelations, const vector<int>& corr);

  // Set the domain (in frequency and time).
  // Hereafter getSolvableParmData can be called.
  // It returns the maximum length of the buffer needed to marshall
  // the LSQFit object filled by fillFitter.
  // The length is 0 if the domain is outside the observation domain.
  // If needed the given domain is adjusted to the observation domain.
  int setDomain (double startFreq, double lengthFreq,
		 double startTime, double lengthTime);

  // Get the actual domain.
  const MeqDomain& getDomain() const
    { return itsDomain; }

  // Return the solvable parms.
  // The parms are in ascending order of spidnr.
  const vector<ParmData>& getSolvableParmData() const
    { return itsParmData; }

  // Make all parameters non-solvable.
  void clearSolvableParms();

  // Make specific parameters solvable (isSolvable = True) or
  // non-solvable (False).
  void setSolvableParms (const vector<string>& parms, 
			 const vector<string>& excludePatterns,
			 bool isSolvable);

  // Get the equations for all selected baselines and fill the
  // fitter object with them.
  // The fitter object gets initialized before being filled.
  void fillFitter (casa::LSQFit&);

  // Set the source numbers to use in this peel step.
  bool setPeelSources (const vector<int>& peelSources,
		       const vector<int>& extraSources);

  // Subtract the peel source(s) from the data.
  // If write=true, the data are written into a new file.
  void subtractPeelSources (bool write=false);

  // There are three ways to update the solvable parms after the solver
  // found a new solution.
  // <group>
  // Update the values of solvable parms.
  // Using its spid index each parm takes its values from the vector.
  void updateSolvableParms (const vector<double>& values);

  // Update the given values (for the current domain) of solvable parms
  // matching the corresponding parm name in the vector.
  // Vector values with a name matching no solvable parm are ignored.
  void updateSolvableParms (const vector<ParmData>& parmData);

  // Update the solvable parm values (reread from table).
  void updateSolvableParms();

  // Show the settings of the Prediffer.
  void showSettings() const;

  // Get the results instead of the equations.
  // This is mainly used for test purposes.
  vector<MeqResult> getResults (bool calcDeriv=true);

  // Write the solved parms.
  void writeParms();

  // Marshall the fitter object into a buffer.
  static void marshall (const casa::LSQFit&, void* buffer, int bufferSize);

  // Demarshall the fitter object from a buffer.
  static void demarshall (casa::LSQFit&, const void* buffer, int bufferSize);


private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Prediffer (const Prediffer& other);
  Prediffer& operator= (const Prediffer& other);
  // </group>

  // Get measurement set description from file
  void readDescriptiveData (const string& fileName);

  // Initialize all parameters in the MeqExpr tree for the current domain
  void initParms (const MeqDomain& domain);

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
  // The EJones (per source/station) can be expressed as real/imag
  // or ampl/phase.
  // The station parameters are optionally taken into account.
  void makeLOFARExpr (bool useEJ, bool asAP, bool useStatParm);

  // Fill the fitter with the equations for the given baseline.
  void fillEquation (casa::LSQFit& fitter, int nresult, int nrval,
		     double* result, char* flagResult,
		     const fcomplex* data, const bool* flags,
		     const MeqRequest& request,
		     int blindex, int ant1, int ant2);

  // Get equations for a single time and baseline.
  void getEquation (double* result, char* flagResult,
		    const fcomplex* data, const bool* flags,
		    const MeqRequest& request,
		    int blindex, int ant1, int ant2);

  // Reset the loop variables for the getEquations loop.
  void resetEqLoop();


  string                itsMSName;      //# Measurement set name
  string                itsMEPName;     //# Common parmtable name
  ParmTable             itsMEP;         //# Common parmtable
  string                itsGSMMEPName;  //# GSM parameters parmtable name
  ParmTable             itsGSMMEP;      //# parmtable for GSM parameters
  MeqParmGroup          itsParmGroup;   //# container for all parms
  bool                  itsCalcUVW;

  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000

  MeqSourceList         itsSources;
  vector<vector<int> >  itsSrcGrp;      //# sources in each group
  vector<int>           itsSrcNrMap;    //# map of all srcnr to used srcnr
  casa::Vector<int>     itsPeelSourceNrs;
  vector<MeqLMN*>       itsLMN;         //# LMN for sources used
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;     //# UVW values per station
  vector<casa::MVBaseline> itsBaselines;
  vector<MeqJonesExpr>  itsExpr;        //# solve expression tree per baseline
  vector<MeqJonesExpr>  itsResExpr;     //# residual expr tree per baseline

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

  MeqDomain    itsDomain;             //# Current domain
  int          itsNrScid;             //# Nr of solvable parameter coeff.
  vector<ParmData> itsParmData;       //# solvable parm info. 

  bool                 itsCorr[4];     //# Correlations to use
  vector<int>          itsAnt1;        //# Antenna1 antenna numbers
  vector<int>          itsAnt2;        //# Antenna2 antenna numbers
  int                  itsNCorr;       //# Number of correlations (XX, etc.)
  int                  itsNSelCorr;    //# Number of correlations selected
  casa::Vector<double> itsTimes;       //# All times in MS
  casa::Vector<double> itsIntervals;   //# All intervals in MS
  casa::Matrix<double> itsAntPos;      //# All antenna positions
  unsigned int         itsNrBl;        //# Total number of unique baselines
  casa::Matrix<bool>   itsBLSelection; //# true = baseline is selected
  casa::Matrix<int>    itsBLIndex;     //# baseline index of antenna pair
                                       //# -1 is baseline is absent
  unsigned int   itsNrSelBl;       //# nr of selected baselines
  unsigned int   itsTimeIndex;     //# The index of the current time
  unsigned int   itsNrTimes;       //# The number of times in the time domain
  unsigned int   itsNrBufTB;       //# Nr of times/baselines fitting in buffer
  unsigned int   itsNrTimesDone;   //# The number of times done in time domain
  unsigned int   itsBlNext;        //# Next baseline to do in time domain

  MMap*          itsDataMap;       //# Data file to map
  FlagsMap*      itsFlagsMap;      //# Flags file to map
  bool           itsLockMappedMem; //# Lock memory immediately after mapping?

  NSTimer itsPredTimer;
  NSTimer itsEqTimer;

};

// @}

} // namespace LOFAR

#endif
