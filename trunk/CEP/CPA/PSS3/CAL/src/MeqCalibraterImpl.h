//# MeqCalibraterImpl.h: Implementation of the MeqCalibrater DO
//#
//# Copyright (C) 2002
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

#ifndef CAL_MEQCALIBRATER_H
#define CAL_MEQCALIBRATER_H

#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/TableIter.h>
#include <aips/Utilities/String.h>
#include <aips/aips.h>
#include <trial/MeasurementEquations/VisBuffer.h>
#include <trial/MeasurementEquations/VisibilityIterator.h>
#include <trial/Tasking/ApplicationObject.h>

#include <MNS/ParmTable.h>
#include <MNS/MeqDomain.h>
#include <MNS/MeqParm.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqJonesExpr.h>
#include <MNS/MeqUVW.h>
#include <MNS/MeqUVWPolc.h>
#include <GSM/SkyModel.h>

//
// Class to perform self-calibration on a MeasurementSet using the
// MeqTree approach.
//
class MeqCalibrater : public ApplicationObject
{

 public:
  //
  // Create MeqCalibrater object for a specific
  // MeaurementSet, MEQ model (with associated MEP database) and skymodel
  // for the specified spectral window.
  // 
  MeqCalibrater(const String& msName,
		const String& meqModel,
		const String& skyModel,
		uInt spw);

  // destructor
  ~MeqCalibrater();

  // set the time interval for which to solve
  void setTimeIntervalSize(uInt secInterval);

  // reset the time interval iterator
  void resetTimeIterator();

  // advance the time interval iterator
  void nextTimeInterval();

  // make all parameters non-solvable
  void clearSolvableParms();

  // make specific parameters solvable (isSolvable = True) or non-solvable (False)
  void setSolvableParms(Vector<String>& parmPatterns, Bool isSolvable);

  // predict visibilities for the current domain
  void predict(const String& modelColName);
  
  // Solve for the current domain.
  // Return fit value to indicate fitness of the solution and updates the
  // parameters for which to solve.
  Double solve();

  // Save modified parameters to the MEP database.
  void saveParms();

  // Save the predicted data to the named column of the MeasurementSet.
  void savePredictedData(const String& dataColName);

  // Save residual data in the named column (residualColName) by substracting
  // data in the first named column (colAName) from data in the second named
  // column (colBName).
  void saveResidualData(const String& colAName, const String& colBName,
			const String& residualColName);

  // Get info about the parameters whose name matches one of the parameter
  // patterns in a GlishRecord, exclude parameters matching one of the
  // exclude pattterns.
  GlishRecord getParms(Vector<String>& parmPatterns,
		       Vector<String>& excludePatterns);

  // Get a description of the current solve domain, which changes
  // after each call to nextTimeIteration.
  GlishRecord getSolveDomain();

  // Return True if iterator over time intervals is past the end
  Bool timeIteratorPastEnd();

  // standard DO methods
  virtual String         className() const;
  virtual Vector<String> methods() const;
  virtual Vector<String> noTraceMethods() const;

  virtual MethodResult runMethod(uInt which,
				 ParameterSet& inputRecord,
				 Bool runMethod);

 private:

  // calculate the UVW polcs for all frequency domains per hour wide time domain
  void calcUVWPolc();
  MeqCalibrater(const MeqCalibrater& other);            // no copy constructor
  MeqCalibrater& operator=(const MeqCalibrater& other); // no assignment operator

  // initialize all parameters in the MeqExpr tree for the current domain
  void initParms(MeqDomain& domain);

  //
  // variables
  //
  MeasurementSet     itsMS;
  ParmTable          itsMEP;
  Table              itsGSMTable;
  GSM::SkyModel      itsGSM;

  TableIterator      itsIter;

  MeqDomain          itsDomain;
  MeqRequest         itsRequest;

  vector<MVBaseline> itsBaselines;
  vector<MeqStation> itsStations;
  MeqUVWPolc         itsUVWPolc;
  MeqJonesExpr*      itsExprTree;

  double itsStartTime;

  double itsStartFreq, itsEndFreq;
  int    itsNrChan;

  Bool itsDataRead;

  Matrix<DComplex> its_xx, its_xy, its_yx, its_yy;

  //
  // variables used in the dummy implementation
  //
  Double itsFitValue;
};

//
// Factory class to instantiate the MeqCalibrater object
//
class MeqCalibraterFactory : public ApplicationObjectFactory
{
  virtual MethodResult make(ApplicationObject*& newObject,
			    const String& whichConstructor,
			    ParameterSet& inputRecord,
			    Bool runConstructor);
};

#endif
