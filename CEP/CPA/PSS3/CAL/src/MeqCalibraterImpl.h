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

#include <aips/Arrays/Matrix.h>
#include <aips/Fitting/LSQ.h>
#include <aips/MeasurementSets/MSMainColumns.h>
#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/TableIter.h>
#include <aips/Utilities/String.h>
#include <aips/aips.h>
#include <aips/Glish/GlishArray.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <trial/Tasking/ApplicationObject.h>

#include <MNS/MeqDomain.h>
#include <MNS/MeqHist.h>
#include <MNS/MeqJonesExpr.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqParm.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqUVWPolc.h>
#include <MNS/ParmTable.h>
#include <GSM/SkyModel.h>

/*!
 * Class to perform self-calibration on a MeasurementSet using the
 * MeqTree approach.
 */
class MeqCalibrater : public ApplicationObject
{

public:
  //! Constructor
  /*!
   * Create MeqCalibrater object for a specific
   * MeaurementSet, MEQ model (with associated MEP database) and skymodel
   * for the specified spectral window.
   */ 
  MeqCalibrater(const String& msName,
		const String& meqModel,
		const String& skyModel,
		uInt spw);

  //! Destructor
  ~MeqCalibrater();

  /*!
   * Set the time interval for which to solve.
   * \param secInterval The time interval in seconds.
   */
  void setTimeInterval (double secInterval);

  //! Reset the iterator.
  void resetIterator();

  /*!
   * Advance the iterator.
   * \returns false if at end of iteration.
   */
  bool nextInterval();

  //! Make all parameters non-solvable
  void clearSolvableParms();

  /*!
   * Make specific parameters solvable (isSolvable = True) or
   * non-solvable (False).
   */
  void setSolvableParms (Vector<String>& parmPatterns, 
			 Vector<String>& excludePatterns,
			 Bool isSolvable);

  //! Predict visibilities for the current domain and store in column
  void predict(const String& modelDataColName);
  
  /*!
   * Solve for the data in the given column in the current domain.
   * \returns Returns fit value to indicate fitness of the solution and
   * updates the parameters for which to solve.
   */
  Double solve (const String& colName);

  //! Save solved parameters to the MEP database.
  void saveParms();

  /*!
   * Save residual data in the named column (residualColName) by substracting
   * data in the first named column (colAName) from data in the second named
   * column (colBName).
   */
  void saveResidualData (const String& colAName, const String& colBName,
			 const String& residualColName);

  /*!
   * Get info about the parameters whose name matches one of the parameter
   * patterns in a GlishRecord, exclude parameters matching one of the
   * exclude pattterns.
   */
  GlishRecord getParms (Vector<String>& parmPatterns,
			Vector<String>& excludePatterns);

  /*!
   * Get the names of the parameters whose name matches the parmPatterns,
   * but does not match the excludePatterns.
   * E.g. getParmNames("*") returns all parameter names.
   */
  GlishArray getParmNames(Vector<String>& parmPatterns,
			  Vector<String>& excludePatterns);

  /*!
   * Get a description of the current solve domain, which changes
   * after each call to nextTimeIteration.
   */
  GlishRecord getSolveDomain();

  /*!
   * Make a selection of the MS to be used in the solve.
   * The 'where' string selects the rows.
   * itsLastChan<0 means until the last channel.
   */
  Bool select(const String& where, int itsFirstChan, int itsLastChan);

  /*!
   * Return some statistics (optionally detailed (i.e. per baseline)).
   * If cleat is true, the statistics are cleared thereafter.
   */
  GlishRecord getStatistics (bool detailed, bool clear);

  /**
   * \defgroup DOStandard Standard Distributed Object methods.
   */
  //@{
  virtual String         className() const;
  virtual Vector<String> methods() const;
  virtual Vector<String> noTraceMethods() const;

  virtual MethodResult runMethod(uInt which,
				 ParameterSet& inputRecord,
				 Bool runMethod);
  //@}

private:

  //! calculate the UVW polcs for all frequency domains per hour wide time domain
  void calcUVWPolc();

  /**
   * \defgroup DisallowedContructors Dissallowed constructors.
   */
  /*@{*/
  MeqCalibrater(const MeqCalibrater& other);
  MeqCalibrater& operator=(const MeqCalibrater& other);
  /*@}*/

  //! initialize all parameters in the MeqExpr tree for the current domain
  void initParms    (const MeqDomain& domain);

  //! Get the phase reference position of the first field.
  void getPhaseRef  ();

  //! Get the frequncy info of the given data desc (spectral window).
  void getFreq      (int ddid);

  //! Get the station info (position and name).
  void fillStations ();

  //! Get all baseline info.
  void fillBaselines(const Vector<int>& ant1, const Vector<int>& ant2);

  //! Create the expressions for each baseline.
  void makeWSRTExpr ();

  //! Calculate the UVW polynomial coefficients.
  void calcUVWPolc  (const Table& ms);

  //! Append the current value of the parameters (as MeqMatrix) to rec
  void MeqCalibrater::addParm(const MeqParm& parm, GlishRecord& rec);

  /**
   * \defgroup PrivVariable Private variables
   */
  /*@{*/
  MeasurementSet        itsMS;          //# MS as given
  ROMSMainColumns       itsMSCol;
  ParmTable             itsMEP;
  Table                 itsGSMTable;
  GSM::SkyModel         itsGSM;

  Vector<uInt>          itsCurRows;     //# Rows in the current iter step
  TableIterator         itsIter;        //# Iterator on selected part of MS
  int                   itsFirstChan;   //# first channel selected
  int                   itsLastChan;    //# last channel selected

  MDirection            itsPhaseRef;    //# Phase reference position in J2000
  MeqDomain             itsSolveDomain;

  Matrix<int>           itsBLIndex;     //# baseline index of antenna pair
  vector<MeqStation>    itsStations;
  vector<MeqJonesExpr*> itsStatExpr;    //# Expression per station
  vector<MVBaseline>    itsBaselines;
  vector<MeqUVWPolc*>   itsUVWPolc;     //# UVW polynomial per baseline
  vector<MeqHist>       itsCelltHist;   //# Histogram of #cells in time
  vector<MeqHist>       itsCellfHist;   //# Histogram of #cells in freq
  vector<MeqJonesExpr*> itsExpr;        //# expression tree per baseline

  double itsTimeInterval;

  double itsStartFreq;
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;

  LSQ          itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  vector<bool> itsIsParmSolvable;       //# is corresponding parmlist solvable?
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<complex<double> > itsDeriv;   //# derivatives of predict
  
  /*@}*/
};

/*!
 * Factory class to instantiate the MeqCalibrater object
 */
class MeqCalibraterFactory : public ApplicationObjectFactory
{
  //! instantiate MqCalibrater object (subclassed from ApplicationObject)
  virtual MethodResult make(ApplicationObject*& newObject,
			    const String& whichConstructor,
			    ParameterSet& inputRecord,
			    Bool runConstructor);
};

#endif
