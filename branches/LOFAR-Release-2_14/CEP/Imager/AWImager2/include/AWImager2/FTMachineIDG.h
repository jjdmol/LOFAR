//# FTMachineSimpleWB.h: Definition for FTMachineSimple
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id: FTMachineSimpleWB.h 28512 2014-03-05 01:07:53Z vdtol $

#ifndef LOFAR_LOFARFT_FTMACHINEIDG_H
#define LOFAR_LOFARFT_FTMACHINEIDG_H

#include <AWImager2/FTMachine.h>

#include <idg/XEON/Proxies.h>

namespace LOFAR {
namespace LofarFT {

class VisBuffer;  
  
class FTMachineIDG : public FTMachine {
public:
  static const casa::String theirName;

  FTMachineIDG(
    const casa::MeasurementSet& ms, 
    const ParameterSet& parset);

  virtual ~FTMachineIDG();
  
  // Copy constructor
  FTMachineIDG(const FTMachineIDG &other);

  // Assignment operator
  FTMachineIDG &operator=(const FTMachineIDG &other);

  // Clone
  FTMachineIDG* clone() const;

  virtual casa::String name() const { return theirName;}

  virtual casa::Matrix<casa::Float> getAveragePB();
//   virtual casa::Matrix<casa::Float> getSpheroidal();
  
  // Get actual coherence from grid by degridding
  virtual void get(VisBuffer& vb, casa::Int row=-1);
  
  // Put coherence to grid by gridding.
  virtual void put(
    const VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);

  virtual void residual(
    VisBuffer& vb, 
    casa::Int row = -1, 
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
protected:

  virtual void initialize_model_grids(casa::Bool normalize);

  virtual void getput(
    VisBuffer& vb, 
    casa::Int row=-1, 
    casa::Bool doget = casa::True,
    casa::Bool doput = casa::True,    
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
    
  casa::CountedPtr<Xeon> itsProxy;


  // Get the appropriate data pointer
  casa::Array<casa::Complex>* getDataPointer(const casa::IPosition&, casa::Bool);

  // Gridder
  casa::String convType;

  casa::Float maxAbsData;

  // Useful IPositions
  casa::IPosition centerLoc;
  casa::IPosition offsetLoc;


  // Shape of the padded image
  casa::IPosition padded_shape;

  casa::Int convSampling;
  casa::Float pbLimit_p;
  int itsNThread;
  casa::Int itsRefFreq;
  casa::Float itsTimeWindow;

private:
  
  std::string itsCompiler;
  std::string itsCompilerFlags;

};

} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
