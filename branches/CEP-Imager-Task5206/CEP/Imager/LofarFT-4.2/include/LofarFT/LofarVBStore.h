// -*- C++ -*-
//# VBStore.h: Definition of the VBStore class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#ifndef LOFARFT_LOFARVBSTORE_H
#define LOFARFT_LOFARVBSTORE_H
#include <synthesis/TransformMachines/Utils.h>

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
  class LofarVBStore
  {
  public:
    LofarVBStore():dopsf_p(casa::False) {};
    ~LofarVBStore() {};
    inline casa::Int nRow()              {return nRow_p;};
    inline casa::Int beginRow()          {return beginRow_p;}
    inline casa::Int endRow()            {return endRow_p;}
    inline casa::Bool dopsf()            {return dopsf_p;}
    inline casa::Bool useCorrected()     {return useCorrected_p;};
    casa::Vector<casa::uInt>& selection()      {return selection_p;};
    casa::Matrix<casa::Double>& uvw()          {return uvw_p;};
    casa::Vector<casa::Bool>& rowFlag()        {return rowFlag_p;};
    casa::Cube<casa::Bool>& flagCube()         {return flagCube_p;};
    casa::Matrix<casa::Float>& imagingWeight() {return imagingWeight_p;};
    casa::Cube<casa::Complex>& visCube()       {return visCube_p;};
    casa::Vector<casa::Double>& freq()         {return freq_p;};
    casa::Cube<casa::Complex>& modelCube()     {return modelCube_p;};
    casa::Cube<casa::Complex>& correctedCube() {return correctedCube_p;};

    void reference(const LofarVBStore& other)
    {
      nRow_p=other.nRow_p;  beginRow_p=other.beginRow_p; endRow_p=other.endRow_p;
      dopsf_p = other.dopsf_p;
      useCorrected_p = other.useCorrected_p;

      selection_p.reference(other.selection_p);
      uvw_p.reference(other.uvw_p);
      rowFlag_p.reference(other.rowFlag_p);
      flagCube_p.reference(other.flagCube_p);
      imagingWeight_p.reference(other.imagingWeight_p);
      freq_p.reference(other.freq_p);
      // if (useCorrected_p) correctedCube_p.reference(other.correctedCube_p);
      // else visCube_p.reference(other.visCube_p);
      // if (useCorrected_p) 
      // 	{
      // 	  correctedCube_p.reference(other.correctedCube_p);
      // 	  visCube_p.reference(other.correctedCube_p);
      // 	}
      // else visCube_p.reference(other.visCube_p);
      correctedCube_p.reference(other.correctedCube_p);
      visCube_p.reference(other.visCube_p);
      modelCube_p.reference(other.modelCube_p);

      // uvw_p.assign(other.uvw_p);
      // rowFlag_p.assign(other.rowFlag_p);
      // flagCube_p.assign(other.flagCube_p);
      // imagingWeight_p.assign(other.imagingWeight_p);
      // freq_p.assign(other.freq_p);
      // visCube_p.assign(other.visCube_p);
      // modelCube_p.assign(other.modelCube_p);
      // correctedCube_p.assign(other.correctedCube_p);
    }

    casa::Int nRow_p, beginRow_p, endRow_p;
    casa::Matrix<casa::Double> uvw_p;
    casa::Vector<casa::uInt> selection_p;
    casa::Vector<casa::Bool> rowFlag_p;
    casa::Cube<casa::Bool> flagCube_p;
    casa::Matrix<casa::Float> imagingWeight_p;
    casa::Cube<casa::Complex> visCube_p, modelCube_p, correctedCube_p;
    casa::Vector<casa::Double> freq_p;
    casa::Bool dopsf_p,useCorrected_p;
  };

} //# NAMESPACE LOFAR - END
#endif
