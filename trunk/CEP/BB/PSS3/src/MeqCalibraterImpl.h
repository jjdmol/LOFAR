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

#ifndef BB_PSS3_MEQCALIBRATER_H
#define BB_PSS3_MEQCALIBRATER_H

#include <casa/Arrays/Matrix.h>
#include <scimath/Fitting/LSQaips.h>
#include <casa/Quanta/MVBaseline.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <casa/BasicSL/String.h>
#include <casa/aips.h>
//#include <tasking/Glish/GlishRecord.h>

#include <PSS3/MNS/MeqDomain.h>
#include <PSS3/MNS/MeqHist.h>
#include <PSS3/MNS/MeqJonesExpr.h>
#include <PSS3/MNS/MeqMatrix.h>
#include <PSS3/MNS/MeqParm.h>
#include <PSS3/MNS/MeqPhaseRef.h>
#include <PSS3/MNS/MeqSourceList.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqStation.h>
#include <PSS3/MNS/MeqStatSources.h>
#include <PSS3/MNS/MeqLofarStatSources.h>
#include <PSS3/MNS/MeqStatUVW.h>
#include <PSS3/MNS/ParmTable.h>
#include <PSS3/Quality.h>

#include <Common/LofarTypes.h>

namespace LOFAR
{

  //# Forward Declarations
  class MMap;

/*!
 * Class to perform self-calibration on a casa::MeasurementSet using the
 * MeqTree approach.
 */
class MeqCalibrater
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
  MeqCalibrater (const casa::String& msName,
		 const casa::String& meqModel,
		 const casa::String& skyModel,
		 const casa::String& dbType,
		 const casa::String& dbName,
		 const casa::String& dbHost,
		 const casa::String& dbPwd,
		 casa::uInt ddid,
		 const vector<int>& ant,
		 const casa::String& modelType,
		 casa::Bool calcUVW,
		 bool lockMappedMem);

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
  bool nextInterval (bool callReadPolcs = true);

  //! Make all parameters non-solvable
  void clearSolvableParms();

  /*!
   * Make specific parameters solvable (isSolvable = True) or
   * non-solvable (False).
   */
  void setSolvableParms (vector<string>& parms, 
			 vector<string>& excludePatterns,
			 bool isSolvable);
 
  /*!
   * Solve for the data in the current domain.
   * \returns Returns fit value to indicate fitness of the solution and
   * updates the parameters for which to solve.
   */
  ///  casa::GlishRecord solve (casa::Bool useSVD);

  /*! Solve which returns solved parameter values in a vector and fit value 
   * in Quality object.
   */
  void solve (casa::Bool useSVD,
	      vector<string>& resultParmNames, 
	      vector<double>& resultParmValues,
	      Quality& resultQuality);

  //! Save solved parameters to the MEP database.
  void saveParms();

  //! Save all solvable parameters to the MEP database.
  void saveAllSolvableParms();

  /*!
   * Save residual data in the itsResColName column.
   * It does a predict for the sources to be peeled off and subtracts
   * the results from the itsSolveColName column.
   */
  void saveResidualData();

  /*!
   * Get info about the parameters whose name matches one of the parameter
   * patterns in a casa::GlishRecord, exclude parameters matching one of the
   * exclude pattterns.
   * isSolvable < 0  all matching parms
   *            = 0  only non-solvable parms
   *            > 0  only solvable parms
   */
  ///  casa::GlishRecord getParms (casa::Vector<casa::String>& parmPatterns,
  ///			      casa::Vector<casa::String>& excludePatterns,
  ///			      int isSolvable, bool denormalize);

  /*!
   * Get the names of the parameters whose name matches the parmPatterns,
   * but does not match the excludePatterns.
   * E.g. getParmNames("*") returns all parameter names.
   */
  ///  casa::GlishArray getParmNames(casa::Vector<casa::String>& parmPatterns,
  ///				casa::Vector<casa::String>& excludePatterns);

  /*!
   * Get a description of the current solve domain, which changes
   * after each call to nextTimeIteration.
   */
  ///  casa::GlishRecord getSolveDomain();

  /*!
   * Set the source numbers to use in this peel step.
   */
  bool peel (const vector<int>& peelSources,
	     const vector<int>& extraSources);

  /*!
   * Make a selection of the MS to be used in the domain iteration.
   */
  void select(const vector<int>& ant1, const vector<int>& ant2,
	      int itsFirstChan, int itsLastChan);

  /*!
   * Return some statistics (optionally detailed (i.e. per baseline)).
   * If clear is true, the statistics are cleared thereafter.
   */
  ///  casa::GlishRecord getStatistics (bool detailed, bool clear);

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

  /*!
   * Get nr of channels.
   */
  int getNrChan() const
    { return itsNrChan; }

  void showSettings() const;

  void showParmValues();

private:
  /**
   * \defgroup DisallowedContructors Dissallowed constructors.
   */
  /*@{*/
  MeqCalibrater(const MeqCalibrater& other);
  MeqCalibrater& operator=(const MeqCalibrater& other);
  /*@}*/

  // Get measurement set description from file
  void readDescriptiveData(const string& fileName);

  //! initialize all parameters in the MeqExpr tree for the current domain
  void initParms    (const MeqDomain& domain, bool callReadPocs);

  //! Get the phase reference position of the first field.
  void getPhaseRef  (double ra, double dec, double startTime);

  //! Get the station info (position and name).
  void fillStations (const vector<unsigned int>& ant1, const vector<unsigned int>& ant2);

  //! Get all baseline info.
  void fillBaselines(const vector<unsigned int>& ant1, const vector<unsigned int>& ant2);

  //! Fill all UVW coordinates if they are not calculated.
  void fillUVW();

  //! Create the WSRT expressions for each baseline.
  void makeWSRTExpr ();

  //! Create the LOFAR expressions for each baseline.
  // The EJones can be expressed as real/imag or ampl/phase.
  void makeLOFARExpr (casa::Bool asAP);

  //! Append the current value of the parameters (as MeqMatrix) to rec
  ///  void MeqCalibrater::addParm(const MeqParm& parm, bool denormalize,
  ///			      casa::GlishRecord& rec);

  /**
   * \defgroup PrivVariable Private variables
   */
  /*@{*/
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
  casa::Vector<casa::Int>           itsPeelSourceNrs;
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;
  vector<MeqStatSources*> itsStatSrc;
  vector<MeqLofarStatSources*> itsLSSExpr; //# Lofar sources per station
  vector<MeqJonesExpr*> itsStatExpr;    //# Expression per station
  vector<casa::MVBaseline>    itsBaselines;
  vector<MeqHist>       itsCelltHist;   //# Histogram of #cells in time
  vector<MeqHist>       itsCellfHist;   //# Histogram of #cells in freq
  vector<MeqJonesExpr*> itsExpr;        //# solve expression tree per baseline
  vector<MeqJonesExpr*> itsResExpr;     //# residual expr tree per baseline

  double itsTimeInterval;

  double itsStartFreq;
  double itsEndFreq;
  double itsStepFreq;
  int    itsNrChan;

  string itsSolveFileName;              //# Data file used in solve (.dat or .res)

  casa::LSQaips      itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  vector<bool> itsIsParmSolvable;       //# is corresponding parmlist solvable?
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<complex<double> > itsDeriv;    //# derivatives of predict
  
  Quality itsSol;                       //# Solution quality

  casa::Vector<casa::String> itsSolvableParms;     // Solvable parameters

  casa::Vector<int>    itsAnt1Data;          // Antenna 1 data
  casa::Vector<int>    itsAnt2Data;          // Antenna 2 data
  int            itsNPol;              // Number of polarisations
  casa::Vector<double> itsTimes;             // All times in MS
  casa::Vector<double> itsIntervals;         // All intervals in MS
  casa::Matrix<double> itsAntPos;            // All antenna positions
  unsigned int   itsNrBl;              // Total number of unique baselines
  casa::Matrix<bool>   itsBLSelection;       // Matrix to indicate which baselines are selected
  vector<int>    itsSelAnt;            // The selected antennas


  unsigned int   itsTimeIndex;         // The index of the current time
  unsigned int   itsNrTimes;           // The number of times in the time interval
  MMap*          itsDataMap;           // Data file to map
  bool           itsLockMappedMem;     // Lock memory immediately after mapping? 

  /*@}*/
};

} // namespace LOFAR

#endif
