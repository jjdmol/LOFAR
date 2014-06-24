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

#ifndef LOFAR_LOFARFT_FTMACHINESPLITBEAMWSTACKWB_H
#define LOFAR_LOFARFT_FTMACHINESPLITBEAMWSTACKWB_H

#include <LofarFT/FTMachine.h>
#include <LofarFT/VisResamplerWB.h>

namespace LOFAR {
namespace LofarFT {

class VisBuffer;  
  
class FTMachineSplitBeamWStackWB : public FTMachine {
public:
  static const casa::String theirName;

  FTMachineSplitBeamWStackWB(
    const casa::MeasurementSet& ms, 
    const ParameterSet& parset);

  virtual ~FTMachineSplitBeamWStackWB();
  
  // Copy constructor
  FTMachineSplitBeamWStackWB(const FTMachineSplitBeamWStackWB &other);

  // Assignment operator
  FTMachineSplitBeamWStackWB &operator=(const FTMachineSplitBeamWStackWB &other);

  // Clone
  FTMachineSplitBeamWStackWB* clone() const;

  virtual casa::String name() const { return theirName;}
  
  // Get actual coherence from grid by degridding
  virtual void get(casa::VisBuffer& vb, casa::Int row=-1);
  virtual void get(VisBuffer& vb, casa::Int row=-1);


  // Put coherence to grid by gridding.
  using casa::FTMachine::put;

  virtual void put(
    const VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
protected:

  virtual void initialize_model_grids(casa::Bool normalize);
  
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
  casa::Bool itsSplitBeam;
  int itsNThread;
  casa::Int itsRefFreq;
  casa::Float itsTimeWindow;
  
  casa::CountedPtr<VisResamplerWB> itsVisResampler;
  virtual VisResampler* visresampler() {return &*itsVisResampler;}

private:
  
  struct Chunk
  {
    int start;
    int end;
    double time;
    double w;
    casa::Matrix<casa::Float> sum_weight;
    vector<int> wplane_map;
  };
  
  struct VisibilityMap
  {
    VisibilityMap() : max_w_plane(0) {}
    vector<Chunk> chunks;
    casa::Vector<casa::uInt> baseline_index_map;
    int max_w_plane;
  };

  VisibilityMap make_mapping(
    const VisBuffer& vb, 
    const casa::Vector< casa::Double > &frequency_list_CF,
    double dtime,
    double w_step);

  bool put_on_w_plane(
    const VisBuffer &vb,
    const VBStore &vbs,
    const casa::Vector<casa::Double> &lsr_frequency,
    vector< casa::Array<casa::Complex> >  &w_plane_grids,
    const VisibilityMap &v,
    int w_plane,
    double w_offset, 
    bool dopsf);
  
};

} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
