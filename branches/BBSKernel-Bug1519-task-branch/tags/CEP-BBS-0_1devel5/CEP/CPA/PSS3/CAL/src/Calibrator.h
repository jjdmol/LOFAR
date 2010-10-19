// Calibrator.h: Wrapper for MeqCallibratorImpl class for BB
//
// Copyright (C) 2003
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Id: 

// The Calibrator class is a wrapper class for MeqCallibratorImpl.
// It hides all AIPS details in its implementation to blackboard.

#ifndef __CEP_BB_CALIBRATOR_H__
#define __CEP_BB_CALIBRATOR_H__

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/Debug.h>

// Parameter prefices used by MeqCalImpl
#define BB_PARM_STOKES "StokesI.CP"
#define BB_PARM_RA     "RA.CP"
#define BB_PARM_DEC    "DEC.CP"


// Note: The term MeqCalImpl object is used to denote the object 
// imported from PSS3. 

class MeqCalibrater;

// Usage:
// First insantiate Calibrator

class Calibrator 
{
public:
  LocalDebugContext

  // Initialize all Calibrator members to their default values. 
  // Note: This method does not affect the MeqCalImpl object. 
  Calibrator ();

  // Destroys the MeqCalImpl object.
  virtual ~Calibrator ();

  // Create the MeqCalImpl object and initialize it with the values in
  // the Calibrator members.
  void Initialize (void);

  // Clear the Calibrator list of solvable parameters.
  // Note: This method does not affect the MeqCalImpl object. 
  void clearSolvableParms (void);

  // Add a new source to the Calibrator list of solvable parameters.
  // Note: This method does not affect the MeqCalImpl object. 
  void addSolvableParm (string parmName, int srcNo);

  // Commit the Calibrator list of solvable parameters to the MeqCalImpl
  // object. The MeqCalImplObject must already be initialized using
  // Initialize ();
  void commitSolvableParms (void);

  // Resets the time interval iterator in the MeqCalImpl object. This 
  // method must be called prior to looping over all time intervals.
  void resetTimeIntervalIterator (void);

  // Advance the time interval iterator in the MeqCalImpl object by
  // the unit specified in setTimeInterval (). This method returns 
  // true as long as the MeqCalImpl object is able to process the
  // next interval. Therefore, this method can best be used in the
  // while conidition for the loop over all time intervals.
  bool advanceTimeIntervalIterator (void);

  // Clears the Calibrator list of sources which are to be taken into
  // account during calibration of a single iteration. 
  // Note: This method does not affect the MeqCalImpl object. 
  void clearPeelSources (void);

  // Clears the Calibrator list of sources which are to be ignored 
  // during calibration of a single iteration. 
  // Note: This method does not affect the MeqCalImpl object. 
  void clearPeelMasks (void);

  // Add a new source to the Claibrator list of sources which are to be
  // taken into account during a single iteration.
  // Note: This method does not affect the MeqCalImpl object. 
  void addPeelSource (int srcNo);

  // Add a new source to the Claibrator list of sources which are to be
  // ignored during a single iteration.
  // Note: This method does not affect the MeqCalImpl object. 
  void addPeelMask (int srcNo);

  // Commit both the Calibrator list of sources to be taken into account
  // and the Calibrator list of sources to be ignored to the MeqCalImpl 
  // object. 
  void commitPeelSourcesAndMasks (void);

  // Execute the PSS3 algorithm. Optimizes the sources specified using 
  // addSolvableParm () by taking into account those sources specified
  // using addPeelSource () and by ignoring those sources specified 
  // using addPeelMask (), for the current interval as advanced using
  // advanceTimeIntervalIterator (). The Run () method executes the
  // PSS3 algorithm for exactly one iteration.
  void Run (void);

  // After optimization, subtracts the calculated sources from the model
  // as optimized during the previous call to Run ().
  void SubtractOptimizedSources (void);

  // After optimization (and possibly after source subtraction from the
  // model), commits the parameters to the internal storage of MeqCalImpl.
  void CommitOptimizedParameters (void);

  // This method is an example of one of the ways in which the Calibrator
  // class can be used. It has the same effect as calibrating the first
  // three sources in twenty iterations each. The method demonstrates the
  // sequence of calls to the Calibrator class to obtain the same behaviour
  // as the original PSS3 algorithm.
  void ExamplePSS3Run (void);

  // Temporary place-holder. Will be removed.
  void ExperimentalOptimizeSource (int src);

  // Temporary place-holder. Will be removed.
  void OptimizeSource (int, int);

  // Temporary place-holder. Will be removed.
  void OptimizeSourceWOSaveRes (int, int);

private:
  // The Calibrator members contain values which are used to initialize
  // and control the MeqCalImpl object:

  vector<int> itsPrimaryAntennae;
  vector<int> itsSecondaryAntennae;

  int itsNrOfIterations;

  MeqCalibrater * itsPSS3CalibratorImpl;

  string itsTblMeasurementSet;
  string itsTblMeqModel;
  string itsTblSkyModel;

  uint   itsDDID;

  string itsModelType;

  bool   itsCalcUVW;

  string itsDataColumn;
  string itsCorrectedDataColumn;

  float  itsTimeInterval;

  vector<string> itsSolvableParms;

  vector<int> itsPeelSources;
  vector<int> itsPeelMasks;
};


#endif







