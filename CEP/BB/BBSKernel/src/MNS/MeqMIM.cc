//# MeqMIM.cc: phaseshift for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id: MeqMIM.cc $

#include <lofar_config.h>

#include <BBSKernel/MNS/MeqMIM.h>
#include <BBSKernel/MNS/MeqPP.h>
#include <BBSKernel/MNS/MeqAzEl.h>
#include <BBSKernel/MNS/MeqSource.h>
#include <BBSKernel/MNS/MeqStation.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

  MeqMIM::MeqMIM(MeqExpr &pp, vector<MeqExpr> &MIMParms, MeqExpr &ref_pp)
  {
    ASSERTSTR(MIMParms.size()==NPARMS,"Wrong number of MIMparms defined!");
    for(uint parmIt=0;parmIt!=MIMParms.size();parmIt++)
      addChild (MIMParms[parmIt]);
    addChild(pp);
    addChild(ref_pp);
  }

  MeqMIM::~MeqMIM()
  {}


  MeqJonesResult MeqMIM::getJResult (const MeqRequest& request)
  {
    MeqJonesResult res(0);
    //get Parmresults:
    vector<MeqResult> parm_res(NPARMS);
    MeqResultVec pp_res;
    MeqResultVec pp_res2;
    vector<const MeqResult*> parms(NPARMS);
    vector<const MeqMatrix*> parmvalues(NPARMS);
        
    for(uint i=0;i<NPARMS;i++){
      parms[i] = &(getChild(i).getResultSynced(request,parm_res[i]));
      parmvalues[i]=&(parms[i]->getValue());
    }
    const MeqResultVec & pp=getChild(NPARMS).getResultVecSynced(request,pp_res);
    const MeqResultVec & ref_pp=getChild(NPARMS+1).getResultVecSynced(request,pp_res2);
    
    // Evaluate main value.
    evaluate(request,
	     pp[0].getValue(), pp[1].getValue(),
	     pp[2].getValue(), pp[3].getValue(),
	     parmvalues,
	     ref_pp[0].getValue(),ref_pp[1].getValue(),ref_pp[2].getValue(),
	     res.result11().getValueRW(),
	     res.result22().getValueRW());

    // Evaluate perturbed values.  
    const MeqParmFunklet *perturbedParm;
    for(int i = 0; i < request.nspid(); ++i)
    {
      
      // Find out if this perturbed value needs to be computed.
      bool gotit=false;
      for(uint iparm=0;iparm<NPARMS;iparm++)
	{
	  if(parms[iparm]->isDefined(i))
	    {
	      perturbedParm = parms[iparm]->getPerturbedParm(i);
	      gotit=true;
	      break;
	    }
	}
      if (!gotit)
	{
	if(pp[0].isDefined(i))
	  {
    	    perturbedParm = pp[0].getPerturbedParm(i);
	  }
    	else if(pp[1].isDefined(i))
	  {
    	    perturbedParm = pp[1].getPerturbedParm(i);
	  }
    	else if(pp[2].isDefined(i))
	  {
    	    perturbedParm = pp[2].getPerturbedParm(i);
	  }
    	else if(pp[3].isDefined(i))
	  {
    	    perturbedParm = pp[3].getPerturbedParm(i);
	  }
    	else if(ref_pp[0].isDefined(i))
	  {
    	    perturbedParm = ref_pp[1].getPerturbedParm(i);
	  }
    	else if(ref_pp[1].isDefined(i))
	  {
    	    perturbedParm = ref_pp[2].getPerturbedParm(i);
	  }
    	else if(ref_pp[2].isDefined(i))
	  {
    	    perturbedParm = ref_pp[3].getPerturbedParm(i);
	  }
    	else
	  {
    	    continue;
	  }
	}
	for(uint iparm=0;iparm<NPARMS;iparm++){
	  parmvalues[iparm]=&(parms[iparm]->getPerturbedValue(i));
	}
        evaluate(request,
		 pp[0].getPerturbedValue(i), pp[1].getPerturbedValue(i),
		 pp[2].getPerturbedValue(i), pp[3].getPerturbedValue(i),
		 parmvalues,
		 ref_pp[0].getPerturbedValue(i),ref_pp[1].getPerturbedValue(i),ref_pp[2].getPerturbedValue(i),
		 res.result11().getPerturbedValueRW(i),
		 res.result22().getPerturbedValueRW(i));

        res.result11().setPerturbedParm(i, perturbedParm);
        res.result22().setPerturbedParm(i, perturbedParm);
    }
    res.result12().setValue (MeqMatrix(0.));
    res.result21().setValue (MeqMatrix(0.));
    
    return res;
  }


void MeqMIM::evaluate(const MeqRequest& request, const MeqMatrix &in_x, const MeqMatrix &in_y,
		      const MeqMatrix &in_z, const MeqMatrix &in_alpha, const vector<const MeqMatrix *> Mimparms,
		      const MeqMatrix &in_refx, const MeqMatrix &in_refy,const MeqMatrix &in_refz,
		      MeqMatrix &out_11,MeqMatrix &out_22)
{
  //make sure input depends on t only!
  // Check preconditions.
  ASSERT(in_x.nelements() == request.ny());
  ASSERT(in_y.nelements() == request.ny());
  ASSERT(in_z.nelements() == request.ny());
  ASSERT(in_refx.nelements() == request.ny());
  ASSERT(in_refy.nelements() == request.ny());
  ASSERT(in_refz.nelements() == request.ny());
  ASSERT(in_alpha.nelements() == request.ny());
  
  double *E11_re, *E11_im;
  out_11.setDCMat(request.nx(), request.ny());
  out_11.dcomplexStorage(E11_re, E11_im);
  double *E22_re, *E22_im;
  out_22.setDCMat(request.nx(), request.ny());
  out_22.dcomplexStorage(E22_re, E22_im);
  double phase;
  vector<double> parms(NPARMS);
  
  // Evaluate beam.
  for(int t = 0; t < request.ny(); ++t)
    {
      for(int ip=0;ip<NPARMS;ip++)
	{
	  if(Mimparms[ip]->nelements() == 1)
	    parms[ip]=Mimparms[ip]->getDouble();
	  else
	    parms[ip]=Mimparms[ip]->getDouble(0,t);
	}
      double x=in_x.getDouble(0,t);
      double y=in_y.getDouble(0,t);
      double z=in_z.getDouble(0,t);
      double refx=in_refx.getDouble(0,t);
      double refy=in_refy.getDouble(0,t);
      double refz=in_refz.getDouble(0,t);
      double alpha=in_alpha.getDouble(0,t);
      for(int f = 0; f < request.nx(); ++f)
        {
	  double freq=request.domain().startX() + f*request.stepX()+request.stepX()/2.0;
	  phase = calculate_mim_function(parms,x,y,z,alpha,freq,refx,refy,refz);
	  *E11_re=std::sin(phase);
	  *E22_re=std::sin(phase);
	  *E11_im=std::cos(phase);
	  *E22_im=std::cos(phase);
	  E11_re++;E22_re++;E11_im++;E22_im++;
	}
    }
}

  double MeqMIM::calculate_mim_function(const vector<double> & parms,double x,double y,double z,double alpha,double freq,double ref_x,double ref_y,double ref_z){
    //dummy
    //calculate rotation matrix
    double lon = std::atan2(ref_y,ref_x);
    double lat = std::atan2( ref_z, std::sqrt( ref_x*ref_x + ref_y*ref_y ) );
    double rot_x = -1*std::sin(lon)*x+std::cos(lon)*y;
    double rot_y = -1*std::sin(lat)*std::cos(lon)*x-std::sin(lat)*std::sin(lon)*y+std::cos(lat)*z;


    return (parms[0]*rot_x/1000.+parms[1]*rot_y/1000.)/std::cos(alpha);
  }
#ifdef EXPR_GRAPH
  std::string MeqMIM::getLabel()
  {
    return std::string("MeqMIM\\nIonospheric disturbance of a source/station combination.");
  }
#endif
  
} //# namespace BBS
} //# namespace LOFAR

