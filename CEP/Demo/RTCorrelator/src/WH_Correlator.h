//#  WH_Correlate.h: Round robin BG/L correlator. Using hard real-time
//#  property of BG/L so we don't have to synchronize.
//#
//#  Copyright (C) 2002-2004
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

#ifndef RTCORRELATOR_WH_CORRELATE_H
#define RTCORRELATOR_WH_CORRELATE_H

#include <stdlib.h>
#include <lofar_config.h>

#include <Common/lofar_complex.h>
#include <Transport/TH_MPI.h>
#include <tinyCEP/WorkHolder.h>

#include <DH_CorrCube.h>
#include <DH_Vis.h>

namespace LOFAR
{

class WH_Correlator: public WorkHolder {

 public:
  explicit WH_Correlator (const string& name, 
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nelements,
			  unsigned int nsamples,
			  unsigned int nchannels, 
			  unsigned int nruns);

  virtual ~WH_Correlator();

  static WorkHolder* construct(const string& name, 
			       unsigned int nin, 
			       unsigned int nout,
			       unsigned int nelements, 
			       unsigned int nsamples, 
			       unsigned int nchannels, 
			       unsigned int nruns);

  virtual WH_Correlator* make (const string& name);

  virtual void preprocess();
  virtual void process();
  virtual void postprocess();
  virtual void dump();
  
 private:
  const int itsNelements;
  const int itsNsamples;
  const int itsNchannels;
  const int itsNruns;

  int task_id;
  
  DH_CorrCube::BufferType *sig_buf;
  DH_Vis::BufferType      *cor_buf;

  /// Forbid copy constructor
  WH_Correlator(const WH_Correlator&);
  /// Forbid assignment
  WH_Correlator& operator= (const WH_Correlator&);
  
  /// Main correlator routine
  void correlator_core(DH_CorrCube::BufferType& sig, 
		       DH_Vis::BufferType&      cor);  
  /// Master and slave procedures
  void master();
  void slave();
};

} // namespace LOFAR

#endif
