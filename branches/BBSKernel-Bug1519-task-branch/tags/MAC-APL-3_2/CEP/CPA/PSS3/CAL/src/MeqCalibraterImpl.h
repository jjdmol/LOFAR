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

#include <casa/Arrays/Matrix.h>
#include <scimath/Fitting/LSQaips.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Quanta/MVBaseline.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <casa/BasicSL/String.h>
#include <casa/aips.h>
#include <tasking/Glish/GlishArray.h>
#include <tasking/Glish/GlishRecord.h>
#include <tasking/Glish/GlishValue.h>
#include <tasking/Tasking/ApplicationObject.h>

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
#include <Common/lofar_complex.h>

using namespace casa;

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
   * for the specified data descriptor (i.e. spectral window) and antennas.
   * The database type (aips or postgres) has to be given.
   * For postgres the database name has to be given as well.
   * Currently model types WSRT and LOFAR are recognized.
   * The UVW coordinates can be recalculated or taken from the MS.
   */ 
  MeqCalibrater (const String& msName,
		 const String& meqModel,
		 const String& skyModel,
		 const String& dbType,
		 const String& dbName,
		 const String& dbPwd,
		 uInt ddid,
		 const Vector<Int>& ant1,
		 const Vector<Int>& ant2,
		 const String& modelType,
		 Bool calcUVW,
		 const String& dataColName,
		 const String& residualColName);

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
   * Solve for the data in the itsSolveColName column in the current domain.
   * \returns Returns fit value to indicate fitness of the solution and
   * updates the parameters for which to solve.
   */
  GlishRecord solve (Bool useSVD);

  //! Save solved parameters to the MEP database.
  void saveParms();

  /*!
   * Save residual data in the itsResColName column.
   * It does a predict for the sources to be peeled off and subtracts
   * the results from the itsSolveColName column.
   */
  void saveResidualData();

  /*!
   * Get the residual data.
   * It does a predict for the sources to be peeled off and subtracts
   * the results from the itsSolveColName column.
   */
  GlishRecord getResidualData();

  /*!
   * Get info about the parameters whose name matches one of the parameter
   * patterns in a GlishRecord, exclude parameters matching one of the
   * exclude pattterns.
   * isSolvable < 0  all matching parms
   *            = 0  only non-solvable parms
   *            > 0  only solvable parms
   */
  GlishRecord getParms (Vector<String>& parmPatterns,
			Vector<String>& excludePatterns,
			int isSolvable, bool denormalize);

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
   * Set the source numbers to use in this peel step.
   */
  Bool peel (const Vector<int>& peelSourceNrs,
	     const Vector<Int>& extraSourceNrs);

  /*!
   * Make a selection of the MS to be used in the domain iteration.
   * The 'where' string selects the rows.
   * itsLastChan<0 means until the last channel.
   */
  Int select(const String& where, int itsFirstChan, int itsLastChan);

  /*!
   * Make a selection of the MS to be used in the solve.
   * The 'where' string selects the rows.
   */
  Int solveselect(const String& where);

  /*!
   * Return some statistics (optionally detailed (i.e. per baseline)).
   * If clear is true, the statistics are cleared thereafter.
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

  /*!
   * Get nr of channels.
   */
  int getNrChan() const
    { return itsNrChan; }

private:
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
  void fillStations (const Vector<int>& ant1, const Vector<int>& ant2);

  //! Get all baseline info.
  void fillBaselines(const Vector<int>& ant1, const Vector<int>& ant2);

  //! Fill all UVW coordinates if they are not calculated.
  void fillUVW();

  //! Create the WSRT expressions for each baseline.
  void makeWSRTExpr ();

  //! Create the LOFAR expressions for each baseline.
  // The EJones can be expressed as real/imag or ampl/phase.
  void makeLOFARExpr (Bool asAP);

  //! Append the current value of the parameters (as MeqMatrix) to rec
  void MeqCalibrater::addParm(const MeqParm& parm, bool denormalize,
			      GlishRecord& rec);

  /**
   * \defgroup PrivVariable Private variables
   */
  /*@{*/
  MeasurementSet        itsMS;          //# MS as given
  ROMSMainColumns       itsMSCol;
  Table                 itsSelMS;       //# Selected rows from MS
  ParmTable             itsMEP;         //# Common parmtable
  ParmTable             itsGSMMEP;      //# parmtable for GSM parameters
  bool                  itsCalcUVW;

  Vector<uInt>          itsCurRows;     //# Rows in the current iter step
  Vector<uInt>          itsSolveRows;   //# Rows to use in solve function
  TableIterator         itsIter;        //# Iterator on selected part of MS
  int                   itsFirstChan;   //# first channel selected
  int                   itsLastChan;    //# last channel selected

  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000
  MeqDomain             itsSolveDomain;

  Matrix<int>           itsBLIndex;     //# baseline index of antenna pair
  MeqSourceList         itsSources;
  Vector<Int>           itsPeelSourceNrs;
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

  double itsTimeInterval;

  double itsStartFreq;
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;

  String itsDataColName;                //# column containing data
  String itsResColName;                 //# column to store residuals in
  String itsSolveColName;               //# column to be used in solve
                                        //# (is dataColName or resColName)

  LSQaips      itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  vector<bool> itsIsParmSolvable;       //# is corresponding parmlist solvable?
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<LOFAR::dcomplex> itsDeriv;     //# derivatives of predict
  
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
