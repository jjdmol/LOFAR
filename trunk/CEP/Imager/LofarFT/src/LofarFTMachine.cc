//# LofarFTMachine.cc: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#include <lofar_config.h>
#include <LofarFT/LofarFTMachine.h>

#include <Common/OpenMP.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{

  LofarFTMachine::LofarFTMachine (int nwPlanes, int64 cachesize)
    : AWProjectFT(nwPlanes, 0,
                  CountedPtr<CFCache>(),
                  CountedPtr<ConvolutionFunction>(),
                  CountedPtr<VisibilityResamplerBase>(),
                  False, True)
  {
    // Make as many Gridder objects as there are threads.
    ///    itsGridders.resize (OpenMP::maxThreads());
    LOG_INFO (OpenMP::maxThreads() << " gridder threads will be used");
    // Cast the convolution function to have the LOFAR one.
    itsConvFunc = dynamic_cast<LOFARConvolutionFunction*>(cf.operator->());
    ASSERTSTR (itsConvFunc, "No LofarConvolutionFunction given");
  }

  LofarFTMachine::~LofarFTMachine()
  {}

  LofarFTMachine* LofarFTMachine::clone() const
  {
    return new LofarFTMachine (*this);
  }

  // The following functions are adapted versions of AWProjectFT in
  // casa/synthesis/MeasurementComponents.
  ///#include "synthesis/MeasureComponents/AWProjectFT.FORTRANSTUFF"

  void LofarFTMachine::put (const VisBuffer& vb, Int row, Bool dopsf,
                            FTMachine::Type type)
  {
    // Take care of translation of Bools to Integer
    makingPSF = dopsf;
    // Determine the baselines in the VisBuffer.
    const Vector<Int>& ant1 = vb.antenna1();
    const Vector<Int>& ant2 = vb.antenna2();
    int nrant = 1 + max(max(ant1), max(ant2));
    // Sort on baseline (use a baseline nr which is faster to sort).
    Vector<Int> blnr(nrant*ant1);
    blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
    Vector<uInt> blIndex;
    GenSortIndirect<Int>::sort (blIndex, blnr);
    // Now determine nr of unique baselines and their start index.
    vector<int> blStart, blEnd;
    blStart.reserve (nrant*(nrant+1)/2);
    blEnd.reserve (nrant*(nrant+1)/2);
    Int lastbl = -1;
    bool usebl = false;
    for (uint i=0; i<blnr.size(); ++i) {
      Int bl = blnr[blIndex[i]];
      if (bl != lastbl) {
        // New baseline. Write the previous end index if applicable.
        if (usebl) {
          blEnd.push_back (i);
        }
        // Skip auto-correlations and high W-values and flagged rows
        usebl = false;
        if (ant1[blIndex[i]] != ant2[blIndex[i]]) {
          usebl = true;
          blStart.push_back (i);
        }
      }
    }
    // Write the last end index if applicable.
    if (usebl) {
      blEnd.push_back (blnr.size());
    }
    // Determine the time center of this data chunk.
    const Vector<Double>& times = vb.time();
    double time = 0.5 * (times[times.size()-1] - times[0]);

#pragma omp parallel
    {
      // Thread-private variables.
      Vector<uInt> rownrs;
      // The for loop can be parallellized. This must be done dynamically,
      // because the execution times of iterations can vary.
#pragma omp for schedule(dynamic)
      for (uint i=0; i<blStart.size(); ++i) {
        // Get the convolution function.
        
        // Create the vector of rows to use (reference to index vector part).
        Vector<uInt> rowsel(blIndex(Slice(blStart[i], blEnd[i] - blStart[i])));
        rownrs.reference (rowsel);
        // Grid the data.
        /////itsGridder[OpenMP::threadNum()].do();
      }
    } // end omp parallel

    /*
    const Matrix<Float> *imagingweight;
    imagingweight=&(vb.imagingWeight());

    Cube<Complex> data;
    //Fortran gridder need the flag as ints 
    Cube<Int> flags;
    Matrix<Float> elWeight;
    interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);


    // Cube<Int> flags(vb.flagCube().shape());
    // flags=0;
    // flags(vb.flagCube())=True;
    

    // if(type==FTMachine::MODEL)          data=&(vb.modelVisCube());
    // else if(type==FTMachine::CORRECTED) data=&(vb.correctedVisCube());
    // else                                data=&(vb.visCube());
    
    Int NAnt;
    if (doPointing) NAnt = findPointingOffsets(vb,l_offsets,m_offsets,True);
    
    //
    // If row is -1 then we pass through all rows
    //
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
      } 
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
      }
    //    
    // Get the uvws in a form that Fortran can use and do that
    // necessary phase rotation. On a Pentium Pro 200 MHz when null,
    // this step takes about 50us per uvw point. This is just barely
    // noticeable for Stokes I continuum and irrelevant for other
    // cases.
    //
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING u,v to correct for an image inversion problem
    for (Int i=startRow;i<endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    // Vector<Int> rowFlags(vb.nRow());
    // rowFlags=0;
    // rowFlags(vb.flagRow())=True;
    // if(!usezero_p) 
    //   for (Int rownr=startRow; rownr<=endRow; rownr++) 
    // 	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);  
    
    //Here we redo the match or use previous match
    
    //Channel matching for the actual spectral window of buffer
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }

      // {//Non-tiled version
      // 	Vector<Int> rowFlags(vb.nRow());
      // 	rowFlags=0;
      // 	rowFlags(vb.flagRow())=True;
      // 	if(!usezero_p) 
      // 	  for (Int rownr=startRow; rownr<=endRow; rownr++) 
      // 	    if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;

      // 	IPosition s(flags.shape());
	
      // 	Int Conj=0,doPSF=0;
      // 	Int ScanNo=0,doGrad=0;Double area=1.0;
	
      // 	if (dopsf) doPSF=1;
	
      // 	Int tmpPAI=1;
      // 	runFortranPut(uvw,dphase,*datStorage,s,Conj,flags,rowFlags,
      // 		      *imagingweight,
      // 		      row,uvOffset,
      // 		      griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
      // 		      l_offsets,m_offsets,sumWeight,area,doGrad,doPSF,tmpPAI);
      // }
    VBStore vbs;
    setupVBStore(vbs,vb, elWeight,data,uvw,flags, dphase);

    // visResampler_p->setParams(uvScale,uvOffset,dphase);
    // visResampler_p->setMaps(chanMap, polMap);
    resampleDataToGrid(griddedData, vbs, vb, dopsf);//, *imagingweight, *data, uvw,flags,dphase,dopsf);
    
  //Double or single precision gridding.
  // if(useDoubleGrid_p) 
  //   visResampler_p->DataToGrid(griddedData2, vbs, sumWeight, dopsf);
  // else
  //    visResampler_p->DataToGrid(griddedData, vbs, sumWeight, dopsf); 

  */
  }

} //# end namespace
