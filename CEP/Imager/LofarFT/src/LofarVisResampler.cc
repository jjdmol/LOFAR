//# LofarVisResampler.cc: Implementation of the LofarVisResampler class
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

#include <LofarFT/LofarVisResampler.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

namespace LOFAR {

  template
  void LofarVisResampler::DataToGridImpl_p(Array<DComplex>& grid, LofarVBStore& vbs, 
                                           const Vector<uInt>& rows,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs) __restrict__;
  template
  void LofarVisResampler::DataToGridImpl_p(Array<Complex>& grid, LofarVBStore& vbs, 
                                           const Vector<uInt>& rows,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs) __restrict__;

  template <class T>
  void LofarVisResampler::DataToGridImpl_p(Array<T>& grid,  LofarVBStore& vbs, 
                                           const Vector<uInt>& rows,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs)
  {
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4),scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(2);

    //    IPosition grdpos(4);
    Vector<Int> igrdpos(4);
    //    Float sampling[2];
    //    Double pos[2]
    //    Int support[2], loc[3], off[3], iloc[4], tiloc[4];
    //    Int scaledSampling[2], scledSupport[2];
    //    Int igrdpos[4];

    Double norm=0.0;
    Complex phasor, nvalue, wt;
    // A conv. fucntion per pol.
    // For time being only one channel.
    Vector<Int> cfShape(4,1);
    cfShape[0] = (*(cfs.vdata))[0][0][0].shape()[0];
    cfShape[1] = (*(cfs.vdata))[0][0][0].shape()[1];
    Vector<Int> convOrigin = (cfShape-1)/2;

    Int rbeg = vbs.beginRow_p;
    Int rend = vbs.endRow_p;
    
    Int nx = grid.shape()[0];
    Int ny = grid.shape()[1];
    Int nw = 1;
    Int nGridPol = grid.shape()[2];
    Int nGridChan = grid.shape()[3];

    Int nDataPol  = vbs.flagCube_p.shape()[0];
    Int nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = cfs.sampling[0];
    support[0] = cfs.xSupport[0];
    support[1] = cfs.ySupport[1];

    T* __restrict__ gridStore = grid.data();
    const Int * __restrict__ iPosPtr = igrdpos.data();
    const Complex* __restrict__ convFuncV[4];

    //Bool Dummy, gDummy;
    //T* __restrict__ gridStore = grid.getStorage(gDummy);
    //const Int * __restrict__ iPosPtr = igrdpos.getStorage(Dummy);
    //const Complex* __restrict__ convFuncV=cfs.vdata->getStorage(Dummy);


    for (int i=0; i<4; ++i) {
      convFuncV[i] = (*(cfs.vdata))[0][i][i].data();
      //Matrix<Complex> im((*(cfs.vdata))[0][i][i]);
      //store2(im,"Aterm-ch"+String::toString(i)+".img");
    }
      
    const Double *freq  = vbs.freq_p.data();

    // Cache increment values for adding to grid in gridInc.  This is
    // supplied to addTo4DArray later.
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    // Cache the CF related increments internally in
    // VisibilityResamplerBase for use in getFrom4DArray later.
    cacheAxisIncrements(cfShape, cfInc_p);

    const Bool * __restrict__ flagCube_ptr=vbs.flagCube_p.data();
    const Float * __restrict__ imgWts_ptr = vbs.imagingWeight_p.data();
    const Complex * __restrict__ visCube_ptr = vbs.visCube_p.data();
    Double * __restrict__ sumWt_ptr = sumwt.data();

    // {
    //   IPosition tt(4);
    //   for(tt(0)=0;tt(0)<grid.shape()(0);tt(0)++)
    // 	for(tt(1)=0;tt(1)<grid.shape()(1);tt(1)++)
    // 	  for(tt(2)=0;tt(2)<grid.shape()(2);tt(2)++)
    // 	    for(tt(3)=0;tt(3)<grid.shape()(3);tt(3)++)
    // 	      grid(tt)*=1.0;
    // }

    for(Int inx=rbeg; inx< rend; inx++){
        Int irow = rows[inx];
	for(Int ichan=0; ichan< nDataChan; ichan++){
	    Int achan=chanMap_p[ichan];
	    
	    if((achan>=0) && (achan<nGridChan)) {

	      scaledSampling[0] = SynthesisUtils::nint(sampling[0]);
	      scaledSampling[1] = SynthesisUtils::nint(sampling[1]);
	      scaledSupport[0]  = support[0];
	      scaledSupport[1]  = support[1];

	      sgrid(pos,loc,off, phasor, irow, 
		    vbs.uvw_p, dphase_p[irow], freq[ichan], 
		    uvwScale_p, offset_p, sampling);

	      iloc[2]=max(0, min(nw-1, loc[2]));

	      if (onGrid(nx, ny, nw, loc, support)) { 

		for(Int ipol=0; ipol< nDataPol; ipol++) { 
		  //		  if((!flagCube(ipol,ichan,irow))){  
		  if((!(*(flagCube_ptr + ipol + ichan*nDataPol + irow*nDataPol*nDataChan)))){  
		    Int apol=polMap_p(ipol);
		    if ((apol>=0) && (apol<nGridPol)) {
		      igrdpos[2]=apol; igrdpos[3]=achan;
		      
		      norm=0.0;
		      //int ConjPlane = cfMap_p[ipol];
		      //int PolnPlane = conjCFMap_p[ipol];

		      iloc[3]=ipol;//PolnPlane;

		      if(dopsf)  nvalue=Complex(*(imgWts_ptr + ichan + irow*nDataChan));
		      else	 nvalue= *(imgWts_ptr+ichan+irow*nDataChan)*
		      		   (*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor);
		      for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++) 
			{
			  iloc[1]=(Int)(scaledSampling[1]*iy+off[1]);
			  igrdpos[1]=loc[1]+iy;
			  for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
			    {
			      iloc[0]=(Int)(scaledSampling[0]*ix+off[0]);
			      tiloc=iloc;
                              if (reindex(iloc,tiloc, 0, 1, 
                                          convOrigin, cfShape)) {
                                wt = convFuncV[iloc[3]][iloc[1]*cfInc_p[1]+iloc[0]];
                                ///                              wt = getFrom4DArray(convFuncV, tiloc,cfInc_p);
                                igrdpos[0]=loc[0]+ix;
                              //				  grid(igrdpos) += nvalue*wt;
                                cout<<"ipol="<<ipol<<", iloc[1]="<<iloc[1]<<", cfInc_p[1]="<<cfInc_p[1]<<", iloc[0]="<<iloc[0]<<", wt="<<wt<<", vis="<<nvalue<<endl;
                              // The following uses raw index on the 4D grid
                                addTo4DArray(gridStore,iPosPtr,gridInc_p, nvalue,wt);
                              //				  norm+=real(wt);
                              }
			    }
			}
		      //		      sumwt(apol,achan)+=imagingWeight(ichan,irow);//*norm;
		      *(sumWt_ptr+apol+achan*nGridChan)+= *(imgWts_ptr+ichan+irow*nDataChan);
		    }
		  }
		}
	      }
	    }
        }
    }
  }

/*
  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void AWVisResampler::GridToData(VBStore& vbs, const Array<Complex>& grid)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny,nw;
    Int achan, apol, rbeg, rend, PolnPlane, ConjPlane;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4), scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(2);

    IPosition grdpos(4);

    Vector<Complex> norm(4,0.0);
    Complex phasor, nvalue, wt;
    Vector<Int> cfShape(convFuncStore_p.data->shape().asVector());
    //    Vector<Int> convOrigin = (cfShape-1)/2;
    Vector<Int> convOrigin = (cfShape-1)/2;
    Double sinDPA=0.0, cosDPA=1.0;
    Double cfScale,lambda, cfRefFreq = convFuncStore_p.coordSys.
      spectralCoordinate(convFuncStore_p.coordSys.findCoordinate(Coordinate::SPECTRAL))
      .referenceValue()(0);

    rbeg=0;
    rend=vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nw       = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];
    //
    // The following code reduces most array accesses to the simplest
    // possible to improve performance.  However this made no
    // difference in the run-time performance compared to Vector,
    // Matrix and Cube indexing.
    //
    Bool Dummy;
    const Complex *gridStore = grid.getStorage(Dummy);
    Vector<Int> igrdpos(4);
    const Int *iPosPtr = igrdpos.getStorage(Dummy);
    Complex *convFunc=(*(convFuncStore_p.data)).getStorage(Dummy);
    const Complex* __restrict__ convFuncV=convFuncStore_p.data->getStorage(Dummy);
    Double *freq=vbs.freq_p.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    Matrix<Float>&  imagingWeight=vbs.imagingWeight_p;
    Matrix<Double>& uvw=vbs.uvw_p;
    Cube<Complex>&  visCube=vbs.visCube_p;
    Cube<Bool>&     flagCube=vbs.flagCube_p;

    Vector<Int> gridInc, cfInc;

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    cacheAxisIncrements(cfShape, cfInc_p);

    for(Int irow=rbeg; irow<rend; irow++) {
      if(!rowFlag[irow]) {

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];

	  if((achan>=0) && (achan<nGridChan)) {

	    lambda = C::c/freq[ichan];
	    cfScale = cfRefFreq/freq[ichan];

	    scaledSampling[0] = SynthesisUtils::nint(sampling[0]*cfScale);
	    scaledSampling[1] = SynthesisUtils::nint(sampling[1]*cfScale);
	    scaledSupport[0]  = SynthesisUtils::nint(support[0]/cfScale);
	    scaledSupport[1]  = SynthesisUtils::nint(support[1]/cfScale);


	    sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
		  uvwScale_p,offset_p,sampling);

	    iloc[2]=max(0, min(nw-1, loc[2]));

	    if (onGrid(nx, ny, nw, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {

		if(!flagCube(ipol,ichan,irow)) { 
		  apol=polMap_p[ipol];
		  
		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
		    nvalue=0.0;      norm(apol)=0.0;

		    ConjPlane = cfMap_p(ipol);
		    PolnPlane = conjCFMap_p(ipol);

		    iloc[3]=PolnPlane;

		    for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++) 
		      {
			iloc(1)=(Int)(scaledSampling[1]*iy+off[1]);
			igrdpos[1]=loc[1]+iy;
			
			for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
			  {
			    iloc(0)=(Int)(scaledSampling[0]*ix+off[0]);
			    igrdpos[0]=loc[0]+ix;
			    tiloc=iloc;
			    if (reindex(iloc,tiloc,sinDPA, cosDPA, 
			    		convOrigin, cfShape))
			      {
				wt=getFrom4DArray(convFuncV,tiloc,cfInc_p);
				norm(apol)+=(wt);
				//			    nvalue+=wt*grid(grdpos);
				// The following uses raw index on the 4D grid
				//				nvalue+=wt*getFrom4DArray(gridStore,iPosPtr,gridInc);
				nvalue+=wt*getFrom4DArray(gridStore,igrdpos,gridInc_p);
			      }
			  }
		      }
		    visCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm(apol);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
*/

  void LofarVisResampler::lofarComputeResiduals(LofarVBStore& vbs)
  {
    Int rbeg = vbs.beginRow_p, rend = vbs.endRow_p;
    IPosition vbDataShape=vbs.modelCube_p.shape();
    IPosition start(vbDataShape), last(vbDataShape);
    start=0; start(2)=rbeg;
    last(2)=rend; //last=last-1;

    if (!vbs.useCorrected_p)
      {
	for(uInt ichan = start(0); ichan < last(0); ichan++)
	  for(uInt ipol = start(1); ipol < last(1); ipol++)
	    for(uInt irow = start(2); irow < last(2); irow++)
	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.visCube_p(ichan,ipol,irow);
      }
    else
      {
	for(uInt ichan = start(0); ichan < last(0); ichan++)
	  for(uInt ipol = start(1); ipol < last(1); ipol++)
	    for(uInt irow = start(2); irow < last(2); irow++)
	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.correctedCube_p(ichan,ipol,irow);
      }
  }

  void LofarVisResampler::sgrid(Vector<Double>& pos, Vector<Int>& loc, 
			     Vector<Int>& off, Complex& phasor, 
			     const Int& irow, const Matrix<Double>& uvw, 
			     const Double& dphase, const Double& freq, 
			     const Vector<Double>& scale, 
			     const Vector<Double>& offset,
			     const Vector<Float>& sampling)
  {
    Double phase;
    Vector<Double> uvw_l(3,0); // This allows gridding of weights
			       // centered on the uv-origin
    if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw(i,irow);

    pos(2)=sqrt(abs(scale[2]*uvw_l(2)*freq/C::c))+offset[2];
    loc(2)=SynthesisUtils::nint(pos[2]);
    off(2)=0;

    for(Int idim=0;idim<2;idim++)
      {
	pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+offset[idim];
	loc[idim]=SynthesisUtils::nint(pos[idim]);
	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
      }

    if (dphase != 0.0)
      {
	phase=-2.0*C::pi*dphase*freq/C::c;
	phasor=Complex(cos(phase), sin(phase));
      }
    else
      phasor=Complex(1.0);
  }

} // end namespace
