//  WH_PSS3.h: This workholder acts as a knowledge source in the black board.
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PSS3_WH_PSS3_H
#define PSS3_WH_PSS3_H

#include <lofar_config.h>

#include <Common/lofar_vector.h>
#include "CEPFrame/WorkHolder.h"


class DH_Solution;
class CalibratorOld;

/**
   This workholder acts as a knowledge source in the black board.
   It contains a Calibrator object for calibration. 
*/

class WH_PSS3: public LOFAR::WorkHolder
{
public:
  /// Construct the work holder and give it a name.
/*   explicit WH_PSS3(const string& name, bool outputAllIter, int number); */

  explicit WH_PSS3(const string& name, const string& msName,
		   const string& meqModel, const string& skyModel,
		   const string& dbType, const string& dbName,
		   const string& dbPwd, unsigned int ddid, 
		   const string& modelType, bool calcUVW, 
		   const string& dataColName,
		   const string& residualColName, bool outputAllIter,
		   int number);

  virtual ~WH_PSS3();

  /// Make a fresh copy of the WH object.
  virtual WH_PSS3* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Preprocess.
  virtual void preprocess();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_PSS3 (const WH_PSS3&);

  /// Forbid assignment.
  WH_PSS3& operator= (const WH_PSS3&);

  // The following 2 methods translate data from a DH_Solution object to
  // vectors needed by the Calibrator and vice versa.
  // This will no longer be necessary when parameters in DH_Solution are
  // stored as vectors!
  void putVectorsIntoSolution(DH_Solution* dh, const vector<string>& pNames, 
			      const vector<double>& pValues);
  void putSolutionIntoVectors(DH_Solution* dh, vector<string>& pNames, 
			      vector<double>& pValues, vector<int>& source);
  
  CalibratorOld* itsCal;
  string itsMSName;
  string itsMeqModel;
  string itsSkyModel;
  string itsDbType;
  string itsDbName;
  string itsDbPwd;
  unsigned int itsDDID;
  vector<int> itsAnt1;        // Aips vector type for antenna numbers
  vector<int> itsAnt2;        // Aips vector type for antenna numbers
  string itsModelType;
  bool itsCalcUVW;
  string itsDataColName;
  string itsResidualColName;

  bool itsOutputAllIter;     // Send the results of all iterations to the
                             // database
  int itsNumber;      // Temporary: used for unique output

};


#endif
