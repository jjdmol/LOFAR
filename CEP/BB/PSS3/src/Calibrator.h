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

class MeqCalibrater;

class Calibrator 
{
 public:
  Calibrator ();
  virtual ~Calibrator ();

  void Initialize (void);

  void OptimizeSource (int, int);
  void OptimizeSourceWOSaveRes (int, int);

 private:
  // Data members for the configuration of the PSS3 Calibrater

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
};


#endif







