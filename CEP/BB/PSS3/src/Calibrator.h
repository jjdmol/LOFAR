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

#include <Common/lofar_string.h>
#include <Common/Debug.h>

namespace LOFAR
{

// Parameter prefices used by MeqCalImpl
#define BB_PARM_STOKES "StokesI.CP"
#define BB_PARM_RA     "RA.CP"
#define BB_PARM_DEC    "DEC.CP"


// Note: The term MeqCalImpl object is used to denote the object 
// imported from PSS3. 

class MeqCalibrater;

class Calibrator 
{
public:
  // Initialize all Calibrator members to their default values. 
  // Note: This method does not affect the MeqCalImpl object. 
  Calibrator (const string & ObservationData, const string & CelestialBodies, 
    const string & DBName = "test", const string & MEPName = "meqmodel", 
    const string & DBType = "postgres", const string & DBPasswd = "");

  // Destroys the MeqCalImpl object.
  virtual ~Calibrator ();

  // Change the size of the time slot in seconds.
  void setTimeSlot (double secs);

  // Create the MeqCalImpl object.
  void initialize (void);

  // Display all the settings for the MeqCalImpl object on cout.
  void showSettings (void);

  // Clear the Calibrator list of solvable parameters.
  // Note: This method does not affect the MeqCalImpl object. 
  void clearSolvableParms (void);

  // Add a new source to the Calibrator list of solvable parameters.
  // Note: This method does not affect the MeqCalImpl object. 
  void addSolvableParm (string parmName, int srcNo);

  // Add a new source specification to the Calibrator list of solvable 
  // parameters.Note: This method does not affect the MeqCalImpl object. 
  void addSolvableParm (string parmName);

  // Commit the Calibrator list of solvable parameters to the MeqCalImpl
  // object. The MeqCalImplObject must already be initialized using
  // Initialize ();
  void commitSolvableParms (void);

  // Resets the time interval iterator in the MeqCalImpl object. This 
  // method must be called prior to looping over all time intervals.
  void selectFirstTimeSlot (void);

  // Advance the time interval iterator in the MeqCalImpl object by
  // the unit specified in setTimeInterval (). This method returns 
  // true as long as the MeqCalImpl object is able to process the
  // next interval. Therefore, this method can best be used in the
  // while conidition for the loop over all time intervals.
  bool advanceTimeIntervalIterator (bool callInitParms = true);

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
  void run (void);

  void run (vector<string>& resultParmNames, vector<double>& resultParmValues, 
            Quality& resultQuality);

  void showCurrentParms (void);

  // After optimization, subtracts the calculated sources from the model
  // as optimized during the previous call to Run ().
  void subtractOptimizedSources (void);

  // After optimization (and possibly after source subtraction from the
  // model), commits the parameters to the internal storage of MeqCalImpl.
  void commitOptimizedParameters (void);


  void getParmValues (vector<string>& names,
		      vector<double>& values);
  void setParmValues (const vector<string>& names,
		      const vector<double>& values);

private:
  // The Calibrator members contain values which are used to initialize
  // and control the MeqCalImpl object:
  string itsObservationData;
  string itsCelestialSources;

  string itsMEPName;
  string itsDBType;
  string itsDBName;
  string itsDBPasswd;

  string itsModelType;
  uint   itsDDID;
  string itsScenario;

  string itsSolvParms;
  int    itsStChan;
  int    itsEndChan;
  string itsSelStr;
  bool   itsCalcUVW;
  float  itsTimeSlot;
  string itsDataColumn;
  string itsCorrectedDataColumn;

  // The following members are for administration purposes at the Calibrator
  // object level. These affect the MeqCalImpl object indirectly:
  vector<int>    itsPeelSources;
  vector<int>    itsPeelMasks;
  vector<string> itsSolvableParms;
  MeqCalibrater  * itsPSS3CalibratorImpl;
};

} // end namespace LOFAR

#endif







