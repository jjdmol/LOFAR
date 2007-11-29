//#  StrategyController.h: A base class for all calibration strategies
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBS3_STRATEGYCONTROLLER_H
#define LOFAR_BBS3_STRATEGYCONTROLLER_H

// \file
// A base class for all calibration strategies

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <BBS3/Quality.h>
#include <BBS3/ParmWriter.h>
#include <Transport/Connection.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

//# Forward Declarations
class DH_Solution;
class DH_WOPrediff;
class DH_WOSolve;

/* This is an abstract base class for all calibration strategy controllers
*/

class StrategyController
{
public:
  // Creation of a StrategyController with its DataHolders to read and write.
  StrategyController(Connection* inSolConn, 
		     Connection* outWOPDConn, 
		     Connection* outWOSolveConn,
		     int nrPrediffers);

  // Destructor
  virtual ~StrategyController();

  /// Execute the strategy
  virtual bool execute() = 0;
   
  /// Postprocessing, can be used to do some clean-up, saving etc.
  virtual void postprocess() = 0;

  /// Get strategy implementation type
  virtual string getType() const = 0;

  /// Get and set in/output dataholders
  DH_Solution* getSolution() const;

  DH_WOPrediff* getPrediffWorkOrder() const;

  DH_WOSolve* getSolveWorkOrder() const;

  int getID() const;

  int getNewWorkOrderID();

  int getNumberOfPrediffers() const;

  ParmWriter& getParmWriter();

protected:
  Connection*   itsInSolConn;
  Connection*   itsOutWOPDConn;
  Connection*   itsOutWOSolveConn;
  int           itsNrPrediffers;
 
private:
  int           itsID;
  ParmWriter    itsParmWriter;

  static int    theirNextSCID;   // Unique ID for next Strategy Controller instance
  static int  theirNextWOID;     // Unique ID for next workorder
};

inline DH_Solution* StrategyController::getSolution() const
{ return (DH_Solution*)itsInSolConn->getDataHolder(true); }

inline DH_WOPrediff* StrategyController::getPrediffWorkOrder() const
{ return (DH_WOPrediff*)itsOutWOPDConn->getDataHolder(); }

inline DH_WOSolve* StrategyController::getSolveWorkOrder() const
{ return (DH_WOSolve*)itsOutWOSolveConn->getDataHolder(); }

inline int StrategyController::getID() const
{ return itsID; }

inline int StrategyController::getNewWorkOrderID()
{ return theirNextWOID++; }

inline ParmWriter& StrategyController::getParmWriter()
{ return itsParmWriter; }

inline int StrategyController::getNumberOfPrediffers() const
{ return itsNrPrediffers; }

// @}

} // namespace LOFAR

#endif
