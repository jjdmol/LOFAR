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

#ifndef MEQ_CALIBRATER_H
#define MEQ_CALIBRATER_H

#include <aips/aips.h>
#include <trial/Tasking/ApplicationObject.h>
#include <aips/Utilities/String.h>

#include <aips/MeasurementSets/MeasurementSet.h>
#include <MNS/ParmTable.h>

class MeqCalibrater : public ApplicationObject
{

 public:
  // MeqCalibrater specific methods
  MeqCalibrater(const String& msName,
		const String& meqModel,
		const String& skyModel,
		const String& mepDB,
		Int spw);
  ~MeqCalibrater();

  void setTimeIntervalSize(Int secInterval);
  void resetTimeIterator();
  Bool nextTimeInterval();

  void clearSolvableParms();
  void setSolvableParms(Vector<String>& parmPatterns, Bool isSolvable);

  void   predict(const String& modelColName);
  Double solve(); /* returns double fit, updates parameters */

  void saveParms();
  void saveData(const String& dataColName); /* save data, e.g. "MODEL" */
  void saveResidualData(const String& colAName, const String& colBName,
			const String& residualColName); /* save A-B in residualColName */

  GlishRecord getParms(Vector<String>& parmPatterns);

  // standard methods
  virtual String         className() const;
  virtual Vector<String> methods() const;
  virtual Vector<String> noTraceMethods() const;

  virtual MethodResult runMethod(uInt which,
				 ParameterSet& inputRecord,
				 Bool runMethod);

 private:

  MeqCalibrater(const MeqCalibrater& other);            // no copy constructor
  MeqCalibrater& operator=(const MeqCalibrater& other); // no assignment operator


  // variables
  MeasurementSet ms;
  ParmTable      mep;

  //
  Int timeIteration;
  Double fitValue;
};

class MeqCalibraterFactory : public ApplicationObjectFactory
{
  virtual MethodResult make(ApplicationObject*& newObject,
			    const String& whichConstructor,
			    ParameterSet& inputRecord,
			    Bool runConstructor);
};

#endif
