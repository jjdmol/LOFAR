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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/WorkHolder.h"
#include <aips/Arrays/Vector.h>


class Calibrator;

/**
   This workholder acts as a knowledge source in the black board.
   It contains a Calibrator object for calibration. 
*/

class WH_PSS3: public LOFAR::WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// Note: antenna numbers arguments must currently be passed as an 
  /// aips++ vector!
  explicit WH_PSS3(const string& name, const String& msName, 
		   const String& meqModel, const String& skyModel,
		   uInt ddid, const Vector<int>& ant1, 
		   const Vector<int>& ant2, const String& modelType, 
		   bool calcUVW, const String& dataColName, 
		   const String& residualColName);

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
  
  Calibrator* itsCal;
  String itsMSName;
  String itsMeqModel;
  String itsSkyModel;
  unsigned int itsDDID;
  Vector<int> itsAnt1;        // Aips vector type for antenna numbers
  Vector<int> itsAnt2;        // Aips vector type for antenna numbers
  String itsModelType;
  bool itsCalcUVW;
  String itsDataColName;
  String itsResidualColName;

};


#endif
