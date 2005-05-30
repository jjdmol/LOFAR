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

// \file StrategyController.h
// A base class for all calibration strategies

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <BBS3/Quality.h>
#include <BBS3/ParmWriter.h>

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
  StrategyController(int id, DH_Solution* inDH, 
		     DH_WOPrediff* outWOPD, 
		     DH_WOSolve* outWOSolve,
		     int nrPrediffers,
		     const int DBMasterPort);

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
  void setSolution(DH_Solution* dhPtr);

  DH_WOPrediff* getPrediffWorkOrder() const;
  void setPrediffWorkOrder(DH_WOPrediff* dhPtr);

  DH_WOSolve* getSolveWorkOrder() const;
  void setSolveWorkOrder(DH_WOSolve* dhPtr); 

  int getID() const;

  int getNumberOfPrediffers() const;

  ParmWriter& getParmWriter();

 protected:
  DH_Solution*  itsInDH;
  DH_WOPrediff* itsWOPD;
  DH_WOSolve*   itsWOSolve;
  int           itsNrPrediffers;
 
private:
  int           itsID;
  ParmWriter    itsParmWriter;
};

inline DH_Solution* StrategyController::getSolution() const
{ return itsInDH; }

inline void StrategyController::setSolution(DH_Solution* dhPtr)
{ itsInDH = dhPtr; }

inline DH_WOPrediff* StrategyController::getPrediffWorkOrder() const
{ return itsWOPD; }

inline void StrategyController::setPrediffWorkOrder(DH_WOPrediff* dhPtr)
{ itsWOPD = dhPtr; }

inline DH_WOSolve* StrategyController::getSolveWorkOrder() const
{ return itsWOSolve; }

inline void StrategyController::setSolveWorkOrder(DH_WOSolve* dhPtr)
{ itsWOSolve = dhPtr; }

inline int StrategyController::getID() const
{ return itsID; }

inline ParmWriter& StrategyController::getParmWriter()
{ return itsParmWriter; }

inline int StrategyController::getNumberOfPrediffers() const
{ return itsNrPrediffers; }

// @}

} // namespace LOFAR

#endif
