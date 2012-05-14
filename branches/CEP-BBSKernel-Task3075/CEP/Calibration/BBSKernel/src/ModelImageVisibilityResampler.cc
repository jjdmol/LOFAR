//# ModelImageVisibilityResampler.cc: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: ModelImageVisibilityResampler.cc 20029 2012-04-13 15:08:23Z duscha $


#include <BBSKernel/ModelImageVisibilityResampler.h>

using namespace LOFAR::BBS;
using namespace casa;

ModelImageVisibilityResampler::ModelImageVisibilityResampler()
{
}

ModelImageVisibilityResampler::ModelImageVisibilityResampler(const vector<double> &frequencies)
{
  
}

void ModelImageVisibilityResampler::DataToGrid(Array<DComplex>& grid)
{
  Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
  Int achan, apol, rbeg, rend;
  Vector<Float> sampling(2);
  Vector<Int> support(2),loc(2), off(2), iloc(2);
  Vector<Double> pos(2);

  //    IPosition grdpos(4);
  Vector<Int> igrdpos(4);

  Double norm=0, wt, imgWt;
  Complex phasor, nvalue;

/*
  rbeg = vbs.beginRow_p;
  rend = vbs.endRow_p;
  //    cerr << rbeg << " " << rend << " " << vbs.nRow() << endl;
*/
  nx       = grid.shape()[0]; ny        = grid.shape()[1];
  nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

//  nDataPol  = vbs.flagCube_p.shape()[0];
//  nDataChan = vbs.flagCube_p.shape()[1];
  nDataPol = polMap_p.size();
  nDataChan = itsFrequencies.size();

  sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
  support(0) = convFuncStore_p.xSupport[0];
  support(1) = convFuncStore_p.ySupport[0];
/*
  Bool Dummy, gDummy;
  T __restrict__ *gridStore = grid.getStorage(gDummy);
  Int * __restrict__ iPosPtr = igrdpos.getStorage(Dummy);
  const Int * __restrict__ iPosPtrConst = iPosPtr;
  Double *__restrict__ convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
  Double * __restrict__ freq=vbs.freq_p.getStorage(Dummy);
  Bool * __restrict__ rowFlag=vbs.rowFlag_p.getStorage(Dummy);

  Float * __restrict__ imagingWeight = vbs.imagingWeight_p.getStorage(Dummy);
  Double * __restrict__ uvw = vbs.uvw_p.getStorage(Dummy);
  Bool * __restrict__ flagCube = vbs.flagCube_p.getStorage(Dummy);
  Complex * __restrict__ visCube = vbs.visCube_p.getStorage(Dummy);
  Double * __restrict__ scale = uvwScale_p.getStorage(Dummy);
  Double * __restrict__ offset = offset_p.getStorage(Dummy);
  Float * __restrict__ samplingPtr = sampling.getStorage(Dummy);
  Double * __restrict__ posPtr=pos.getStorage(Dummy);
  Int * __restrict__ locPtr=loc.getStorage(Dummy);
  Int * __restrict__ offPtr=off.getStorage(Dummy);
  Double * __restrict__ sumwtPtr = sumwt.getStorage(Dummy);
  Int * __restrict__ ilocPtr=iloc.getStorage(Dummy);
  Int * __restrict__ supportPtr = support.getStorage(Dummy);
  Int nDim = vbs.uvw_p.shape()[0];

  //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
  cacheAxisIncrements(grid.shape().asVector());
*/
  Int nDim=;   // nTimeslots*3 ?

  for(Int irow=rbeg; irow < rend; irow++)         // For all rows
  {  
// We don't have flags  
//    if(!rowFlag[irow]){                         // If the row is not flagged

for(Int ichan=0; ichan< nDataChan; ichan++)       // For all channels
{
  
  //	  if (vbs.imagingWeight(ichan,irow)!=0.0) {  // If weights are not zero
//  if (imagingWeight[ichan+irow*nDataChan]!=0.0)     // If weights are not zero
//  {
    achan=chanMap_p(ichan);
    
    if((achan>=0) && (achan<nGridChan))      // If selected channels are valid
    {  
      // sgrid(pos,loc,off, phasor, irow, 
      // 	    vbs.uvw,dphase_p[irow], vbs.freq[ichan], 
      // 	    uvwScale_p, offset_p, sampling);
      sgrid(nDim,posPtr,locPtr,offPtr, phasor, irow, 
      uvw,dphase_p[irow], freq[ichan], 
      scale, offset, samplingPtr);

      if (onGrid(nx, ny, loc, support))    // If the data co-ords. are with-in the grid
      {
  /*
  for(Int ipol=0; ipol< nDataPol; ipol++)  // For all polarizations
  {
    // if((!vbs.flagCube(ipol,ichan,irow)))   // If the pol. & chan. specific
    if((!flagCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol]))
    {
      apol=polMap_p(ipol);
      if ((apol>=0) && (apol<nGridPol)) 
      {
        //	      igrdpos(2)=apol; igrdpos(3)=achan;
        iPosPtr[2]=apol; iPosPtr[3]=achan;
        norm=0.0;

        imgWt=imagingWeight[ichan+irow*nDataChan];
        if(dopsf)  nvalue=Complex(imgWt);
        else	 nvalue=imgWt*
         // (vbs.visCube(ipol,ichan,irow)*phasor);
         (visCube[ipol+ichan*nDataPol+irow*nDataPol*nDataChan]*phasor);

        for(Int iy=-supportPtr[1]; iy <= supportPtr[1]; iy++) 
        {
      ilocPtr[1]=abs((int)(samplingPtr[1]*iy+offPtr[1]));
      //			  igrdpos(1)=loc(1)+iy;
      iPosPtr[1]=locPtr[1]+iy;
      //		  wt = convFunc[ilocPtr[1]];
      for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
        {
          ilocPtr[0]=abs((int)(samplingPtr[0]*ix+offPtr[0]));
          wt = convFunc[iloc[0]]*convFunc[iloc[1]];
          //wt *= convFunc[ilocPtr[0]];

          //igrdpos(0)=loc(0)+ix;
          iPosPtr[0]=locPtr[0]+ix;


          // grid(grdpos) += nvalue*wt;

          // The following uses raw index on the 4D grid
          addTo4DArray(gridStore,iPosPtr,nvalue,wt);
          norm+=wt;
        }
    }
        //		      sumwtPtr[apol+achan*nGridPol]+=imgWt*norm;
        sumwt(apol,achan)+=imgWt*norm;
      }
    }
  }
      }
    }
    */
  }
}
    }
  }
  /*
  T *tt=(T *)gridStore;
  grid.putStorage(tt,gDummy);
  */
}

void ModelImageVisibilityResampler::sgrid(Int& uvwDim,
          Double* __restrict__ pos, 
          Int* __restrict__ loc, 
          Int* __restrict__ off, 
          Complex& phasor, const Int& irow,
          // const Matrix<Double>& __restrict__ uvw, 
          const Double* __restrict__ uvw, 
          const Double&  dphase, 
          const Double&  freq, 
          const Double* __restrict__ scale, 
          const Double* __restrict__ offset,
          const Float* __restrict__ sampling) __restrict__ 
{
  Double phase;
  Int ndim=2;

  for(Int idim=0;idim<ndim;idim++)
  {
    pos[idim]=scale[idim]*uvw[idim+irow*uvwDim]*freq/C::c+offset[idim];
    loc[idim]=(Int)std::floor(pos[idim]+0.5);
    off[idim]=(Int)std::floor(((loc[idim]-pos[idim])*sampling[idim])+0.5);
  }

  if (dphase != 0.0)
  {
    phase=-2.0*C::pi*dphase*freq/C::c;
    phasor=Complex(cos(phase), sin(phase));
  }
  else
  {
    phasor=Complex(1.0);
  }
}



/*
void VisibilityResampler::DataToGrid(Array<DComplex>& grid,  VBStore& vbs, const Bool& dopsf,
             Matrix<Double>& sumwt)
{
  Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
  Int achan, apol, rbeg, rend;
  Vector<Float> sampling(2);
  Vector<Int> support(2),loc(2), off(2), iloc(2);
  Vector<Double> pos(2);

  //    IPosition grdpos(4);
  Vector<Int> igrdpos(4);

  Double norm=0, wt, imgWt;
  Complex phasor, nvalue;

  rbeg = vbs.beginRow_p;
  rend = vbs.endRow_p;
  //    cerr << rbeg << " " << rend << " " << vbs.nRow() << endl;
  nx       = grid.shape()[0]; ny        = grid.shape()[1];
  nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

  nDataPol  = vbs.flagCube_p.shape()[0];
  nDataChan = vbs.flagCube_p.shape()[1];

  sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
  support(0) = convFuncStore_p.xSupport[0];
  support(1) = convFuncStore_p.ySupport[0];

  Bool Dummy, gDummy;
  T __restrict__ *gridStore = grid.getStorage(gDummy);
  Int * __restrict__ iPosPtr = igrdpos.getStorage(Dummy);
  const Int * __restrict__ iPosPtrConst = iPosPtr;
  Double *__restrict__ convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
  Double * __restrict__ freq=vbs.freq_p.getStorage(Dummy);
  Bool * __restrict__ rowFlag=vbs.rowFlag_p.getStorage(Dummy);

  Float * __restrict__ imagingWeight = vbs.imagingWeight_p.getStorage(Dummy);
  Double * __restrict__ uvw = vbs.uvw_p.getStorage(Dummy);
  Bool * __restrict__ flagCube = vbs.flagCube_p.getStorage(Dummy);
  Complex * __restrict__ visCube = vbs.visCube_p.getStorage(Dummy);
  Double * __restrict__ scale = uvwScale_p.getStorage(Dummy);
  Double * __restrict__ offset = offset_p.getStorage(Dummy);
  Float * __restrict__ samplingPtr = sampling.getStorage(Dummy);
  Double * __restrict__ posPtr=pos.getStorage(Dummy);
  Int * __restrict__ locPtr=loc.getStorage(Dummy);
  Int * __restrict__ offPtr=off.getStorage(Dummy);
  Double * __restrict__ sumwtPtr = sumwt.getStorage(Dummy);
  Int * __restrict__ ilocPtr=iloc.getStorage(Dummy);
  Int * __restrict__ supportPtr = support.getStorage(Dummy);
  Int nDim = vbs.uvw_p.shape()[0];

  //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
  cacheAxisIncrements(grid.shape().asVector());

  for(Int irow=rbeg; irow < rend; irow++){          // For all rows
    
    if(!rowFlag[irow]){                        // If the row is not flagged

for(Int ichan=0; ichan< nDataChan; ichan++){ // For all channels
  
  //	  if (vbs.imagingWeight(ichan,irow)!=0.0) {  // If weights are not zero
  if (imagingWeight[ichan+irow*nDataChan]!=0.0) {  // If weights are not zero
    achan=chanMap_p(ichan);
    
    if((achan>=0) && (achan<nGridChan)) {   // If selected channels are valid
      
      // sgrid(pos,loc,off, phasor, irow, 
      // 	    vbs.uvw,dphase_p[irow], vbs.freq[ichan], 
      // 	    uvwScale_p, offset_p, sampling);
      sgrid(nDim,posPtr,locPtr,offPtr, phasor, irow, 
      uvw,dphase_p[irow], freq[ichan], 
      scale, offset, samplingPtr);

      if (onGrid(nx, ny, loc, support)) {   // If the data co-ords. are with-in the grid
  
  for(Int ipol=0; ipol< nDataPol; ipol++) { // For all polarizations
    // if((!vbs.flagCube(ipol,ichan,irow))){   // If the pol. & chan. specific
    if((!flagCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol])){
      apol=polMap_p(ipol);
      if ((apol>=0) && (apol<nGridPol)) {
        //	      igrdpos(2)=apol; igrdpos(3)=achan;
        iPosPtr[2]=apol; iPosPtr[3]=achan;
        norm=0.0;

        imgWt=imagingWeight[ichan+irow*nDataChan];
        if(dopsf)  nvalue=Complex(imgWt);
        else	 nvalue=imgWt*
         // (vbs.visCube(ipol,ichan,irow)*phasor);
         (visCube[ipol+ichan*nDataPol+irow*nDataPol*nDataChan]*phasor);

        for(Int iy=-supportPtr[1]; iy <= supportPtr[1]; iy++) 
    {
      ilocPtr[1]=abs((int)(samplingPtr[1]*iy+offPtr[1]));
      //			  igrdpos(1)=loc(1)+iy;
      iPosPtr[1]=locPtr[1]+iy;
      //		  wt = convFunc[ilocPtr[1]];
      for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
        {
          ilocPtr[0]=abs((int)(samplingPtr[0]*ix+offPtr[0]));
          wt = convFunc[iloc[0]]*convFunc[iloc[1]];
          //wt *= convFunc[ilocPtr[0]];

          //igrdpos(0)=loc(0)+ix;
          iPosPtr[0]=locPtr[0]+ix;


          // grid(grdpos) += nvalue*wt;

          // The following uses raw index on the 4D grid
          addTo4DArray(gridStore,iPosPtr,nvalue,wt);
          norm+=wt;
        }
    }
        //		      sumwtPtr[apol+achan*nGridPol]+=imgWt*norm;
        sumwt(apol,achan)+=imgWt*norm;
      }
    }
  }
      }
    }
  }
}
    }
  }
  T *tt=(T *)gridStore;
  grid.putStorage(tt,gDummy);
}
*/