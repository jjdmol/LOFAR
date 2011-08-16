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

    // Instantiate both templates.
  template
  void LofarVisResampler::DataToGridImpl_p(Array<DComplex>& grid, LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs) __restrict__;
  template
  void LofarVisResampler::DataToGridImpl_p(Array<Complex>& grid, LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs) __restrict__;


  template <class T>
  void LofarVisResampler::DataToGridImpl_p(Array<T>& grid,  LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs)
  {
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4),scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(3);

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
    // const Complex* __restrict__ convFuncV[4];
    // for (int i=0; i<4; ++i) {
    //   convFuncV[i] = (*(cfs.vdata))[0][i][i].data();
    // }

    const Complex* __restrict__ convFuncV[4][4];
    for (int i=0; i<4; ++i) {
      for (int j=0; j<4; ++j) {
        const Array<Complex>& conv = (*(cfs.vdata))[0][j][i];
        if (conv.empty()) {
          convFuncV[j][i] = 0;
        } else {
          convFuncV[j][i] = conv.data();
        }
      //Matrix<Complex> im((*(cfs.vdata))[0][i][i]);
      //store2(im,"Aterm-ch"+String::toString(i)+".img");
      }
    }
    //cout<<"cfs.vdata= "<<&cfs<<endl;

      
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

	      //cout<<"  pos= ["<<pos[0]<<", "<<pos[1]<<"]"
	//	  <<", loc= ["<<loc[0]<<", "<<loc[1]<<", "<<loc[2]<<"]"
	//	  <<", off= ["<<off[0]<<", "<<off[1]<<", "<<off[2]<<"]"
	//	  <<endl;

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

		      iloc[3]=ipol; //PolnPlane;

		      //cout<<"Weight= "<<Complex(*(imgWts_ptr + ichan + irow*nDataChan))<<", Vis= "<<(*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor)<<endl;


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

                                for (int ic=0; ic<4; ++ic) {
                                  if (convFuncV[iloc[3]][ic]) {
				    //cout<<ic<<" "<<iloc[3]<<" "<<tiloc[0]<<endl;
                                    wt = convFuncV[iloc[3]][ic][tiloc[1]*cfInc_p[1]+tiloc[0]];
				    //cout<<wt<<endl;
				    //wt=convFuncV[iloc[3]][tiloc[1]*cfInc_p[1]+tiloc[0]];
				//wt = (*(cfs.vdata))[0][iloc[3]][iloc[3]](tiloc[0],tiloc[1]);
                                ///                              wt = getFrom4DArray(convFuncV, tiloc,cfInc_p);
				    igrdpos[0]=loc[0]+ix;
				//				  grid(igrdpos) += nvalue*wt;
				    //cout<<igrdpos[0]<<endl;
				    //cout<<"ipol="<<ipol<<", iloc[1]="<<iloc[1]<<", cfInc_p[1]="<<cfInc_p[1]<<", iloc[0]="<<iloc[0]<<", wt="<<wt<<", vis="<<nvalue<<endl;
                                //assert (wt > 1e-10  &&  wt < 1);
				// The following uses raw index on the 4D grid
				    addTo4DArray(gridStore,iPosPtr,gridInc_p, nvalue,wt);
				//				  norm+=real(wt);
				  }
				}
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

  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer from a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void LofarVisResampler::lofarGridToData(LofarVBStore& vbs,
                                          const Array<Complex>& grid,
                                          const Vector<uInt>& rows,
                                          Int rbeg, Int rend,
                                          LofarCFStore& cfs)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny,nw;
    Int achan, apol, PolnPlane, ConjPlane;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4), scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(3);

    IPosition grdpos(4);

    Complex phasor, nvalue, norm, wt;
    Vector<Int> cfShape(4,1);
    cfShape[0] = (*(cfs.vdata))[0][0][0].shape()[0];
    cfShape[1] = (*(cfs.vdata))[0][0][0].shape()[1];
    //    Vector<Int> convOrigin = (cfShape-1)/2;
    Vector<Int> convOrigin = (cfShape-1)/2;
    Double sinDPA=0.0, cosDPA=1.0;

    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nw       = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = cfs.sampling[0];
    support(0) = cfs.xSupport[0];
    support(1) = cfs.ySupport[0];
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

    // Take all Mueller elements into account.
    const Complex* __restrict__ convFuncV[4][4];
    for (int i=0; i<4; ++i) {
      for (int j=0; j<4; ++j) {
        const Array<Complex>& conv = (*(cfs.vdata))[0][j][i];
        if (conv.empty()) {
          convFuncV[j][i] = 0;
        } else {
          convFuncV[j][i] = conv.data();
        }
      //Matrix<Complex> im((*(cfs.vdata))[0][i][i]);
      //store2(im,"Aterm-ch"+String::toString(i)+".img");
      }
    }
    
    
    // Vector<Double> UVWSCALE_MOI(uvwScale_p*1.4);
    // Vector<Double> UVWOFF_MOI(offset_p);
    // UVWOFF_MOI(0)=320;
    // UVWOFF_MOI(1)=320;
    // UVWOFF_MOI(2)=0;
    
    Double *freq=vbs.freq_p.getStorage(Dummy);

    Matrix<Float>&  imagingWeight=vbs.imagingWeight_p;
    Matrix<Double>& uvw=vbs.uvw_p;
    Cube<Complex>&  visCube=vbs.visCube_p;
    Cube<Bool>&     flagCube=vbs.flagCube_p;

    Vector<Int> gridInc, cfInc;

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    cacheAxisIncrements(cfShape, cfInc_p);

    for(Int inx=rbeg; inx<rend; inx++) {
        Int irow = rows[inx];

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];

	  if((achan>=0) && (achan<nGridChan)) {

	    scaledSampling[0] = SynthesisUtils::nint(sampling[0]);
	    scaledSampling[1] = SynthesisUtils::nint(sampling[1]);
	    scaledSupport[0]  = support[0];
	    scaledSupport[1]  = support[1];

	    sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
	    uvwScale_p,offset_p,sampling);
	    //sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
	//	  UVWSCALE_MOI,UVWOFF_MOI,sampling);
	    


	    iloc[2]=max(0, min(nw-1, loc[2]));
	    //cout<<"-----------------------"<<endl;
	    if (onGrid(nx, ny, nw, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {
		//cout<<"ipol "<<ipol<<endl;
		if(!flagCube(ipol,ichan,irow)) {
		  apol=polMap_p[ipol];

		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
                    nvalue=0.0;
                    norm  =0.0;

		    //ConjPlane = cfMap_p(ipol);
		    //PolnPlane = conjCFMap_p(ipol);

		    iloc[3]=ipol;

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
                                // Use polarization leakage terms if defined.
                                for (int ic=0; ic<4; ++ic) {
                                  if (convFuncV[ic][iloc[3]]) {
				    //cout<<"iloc[3]<<ic "<<iloc[3]<<" "<<ic<<endl;
                                    wt = convFuncV[iloc[3]][ic][tiloc[1]*cfInc_p[1]+tiloc[0]];
                                    //wt = conj(convFuncV[ic][iloc[3]][tiloc[1]*cfInc_p[1]+tiloc[0]]);
                                    norm+=(wt);
				    //			    nvalue+=wt*grid(grdpos);
				// The following uses raw index on the 4D grid
				//				nvalue+=wt*getFrom4DArray(gridStore,iPosPtr,gridInc);
                                    igrdpos[2] = ic;
                                    //igrdpos[2] = iloc[3];//ic;
				    Complex wt2=getFrom4DArray(gridStore,igrdpos,gridInc_p);
				    //cout<<"Resampler : ic iloc cf sumcf vis: "<<ic<<" "<<iloc[3]<<" "<<wt<<" "<<nvalue<<" "<<wt2<<endl;
                                    nvalue+=wt*wt2;
				    //cout<<"wt wt2 wt*wt2 = "<<wt<<" "<<wt2<<" "<<wt*wt2<<endl;
                                  }
                                }
			      }
			  }
		      }
		    //cout<<"nvalue<<" "<<conj(phasor)<<" "<<norm= "<<nvalue<<" "<<conj(phasor)<<" "<<norm<<endl;
		    visCube(ipol,ichan,irow)=nvalue;//(nvalue*conj(phasor))/norm;
        	    //cout<<"Vis  "<<visCube(ipol,ichan,irow)<<"  "<<ipol<<"  "<<ichan<<"  "<<irow<<endl;
		    //modelCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm;
		    //cout<<"modelCube  = "<<modelCube(ipol,ichan,irow)<<endl;
		  }
		}
	      }
	    }
	  }
	}
    }
  }


  void LofarVisResampler::lofarComputeResiduals(LofarVBStore& vbs)
  {
    Int rbeg = vbs.beginRow_p, rend = vbs.endRow_p;
    IPosition vbDataShape=vbs.modelCube_p.shape();
    IPosition start(vbDataShape), last(vbDataShape);
    start=0; start(2)=rbeg;
    last(2)=rend; //last=last-1;

    //cout<<"//////////////////// Compuute residual!!!!!!"<<"vbs.useCorrected_p"<<vbs.useCorrected_p<<endl;

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
    	    for(uInt irow = start(2); irow < last(2); irow++){
    	      //cout<<"===="<<endl;
	      //if(!(abs(vbs.modelCube_p(ichan,ipol,irow))==0.)){cout<<ipol<<" "<<ichan<<" "<<irow<<" "<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;};
    	      
	      //if(!(abs(vbs.modelCube_p(ichan,ipol,irow))==0.)){cout<<"data "<<ipol<<" "<<ichan<<" "<<irow<<" "<<vbs.modelCube_p(ichan,ipol,irow)<<" "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;};
    	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.correctedCube_p(ichan,ipol,irow);
    	      //cout<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;
    	    };
	
      }

    // if (!vbs.useCorrected_p)
    //   {
    // 	for(uInt ichan = start(0); ichan < last(0); ichan++)
    // 	  for(uInt ipol = start(1); ipol < last(1); ipol++)
    // 	    for(uInt irow = start(2); irow < last(2); irow++)
    // 	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.visCube_p(ichan,ipol,irow);
	
    //   }
    // else
    //   {
    // 	for(uInt ichan = start(0); ichan < last(0); ichan++)
    // 	  for(uInt ipol = start(1); ipol < last(1); ipol++)
    // 	    for(uInt irow = start(2); irow < last(2); irow++){
    // 	      //cout<<"===="<<endl;
    // 	      //cout<<vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.modelCube_p(ichan,ipol,irow)<<endl;
    // 	      vbs.correctedCube_p(ichan,ipol,irow) = vbs.correctedCube_p(ichan,ipol,irow) - vbs.modelCube_p(ichan,ipol,irow);
    // 	      //cout<<vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.modelCube_p(ichan,ipol,irow)<<endl;
    // 	    };
	
    //   }


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

    pos(2)=0;//sqrt(abs(scale[2]*uvw_l(2)*freq/C::c))+offset[2];
    loc(2)=0;//SynthesisUtils::nint(pos[2]);
    off(2)=0;

    for(Int idim=0;idim<2;idim++)
      {
	pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+offset[idim];
	loc[idim]=SynthesisUtils::nint(pos[idim]);
	//off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]); // Cyr: I've added "+1" next line, and it solves a difficult problem, i don't know why
	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]+1);
      }

    //if (dphase != 0.0)
    //  {
//	phase=-2.0*C::pi*dphase*freq/C::c;
//	phasor=Complex(cos(phase), sin(phase));
    //  }
    //else
      phasor=Complex(1.0);
  }

} // end namespace
