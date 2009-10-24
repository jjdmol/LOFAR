//# MIM.cc: Ionospheric disturbance of a (source,station) combination.
//#
//# Copyright (C) 2007
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

#include <lofar_config.h>

#include <BBSKernel/Expr/MIM.h>
#include <BBSKernel/Expr/PValueIterator.h>

//using namespace casa;

namespace LOFAR
{
namespace BBS
{

  MIM::MIM(const Expr &pp, const vector<Expr> &MIMParms, const Station &ref_station)
    : itsRefStation(ref_station)
{
    for(uint i = 0; i != MIMParms.size(); ++i)
    {
        addChild(MIMParms[i]);
    }
    NPARMS=MIMParms.size();
    addChild(pp);
}

MIM::~MIM()
{
}

JonesResult MIM::getJResult(const Request &request)
{
    //get Parmresults:
    vector<Result> parm_res(NPARMS);
    vector<const Result*> parms(NPARMS);
    vector<const Matrix*> parmvalues(NPARMS);
    for(uint i = 0; i < NPARMS; i++)
    {
        parms[i] = &(getChild(i).getResultSynced(request,parm_res[i]));
        parmvalues[i] = &(parms[i]->getValue());
    }

    ResultVec pp_res;
    const ResultVec &pp = getChild(NPARMS).getResultVecSynced(request,pp_res);

    const casa::MVPosition &ref_pos = itsRefStation.position.getValue();

    
    // Allocate result.
    JonesResult res;
    res.init();

    // Zero off-diagonal elements.
    res.result12().setValue(Matrix(0.));
    res.result21().setValue(Matrix(0.));

    // Compute main value.
    evaluate(request,
	     pp[0].getValue(), pp[1].getValue(),
	     pp[2].getValue(), pp[3].getValue(),
	     parmvalues,
	     ref_pos(0),ref_pos(1),ref_pos(2),
	     res.result11().getValueRW(),
	     res.result22().getValueRW());

    // Compute perturbed values.  
    vector<const Result*> pvSet(parms);
    pvSet.push_back(&(pp[0]));
    pvSet.push_back(&(pp[1]));
    pvSet.push_back(&(pp[2]));
    pvSet.push_back(&(pp[3]));

    PValueSetIteratorDynamic pvIter(pvSet);    
    while(!pvIter.atEnd())
    {
        for(uint i=0;i<NPARMS;i++){
            parmvalues[i]=&(pvIter.value(i));
        }

        evaluate(request,
		 pvIter.value(NPARMS), pvIter.value(NPARMS + 1),
		 pvIter.value(NPARMS + 2), pvIter.value(NPARMS + 3),
		 parmvalues,
		 ref_pos(0),ref_pos(1),ref_pos(2),
		 res.result11().getPerturbedValueRW(pvIter.key()),
		 res.result22().getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }

    return res;
}


void MIM::evaluate(const Request &request, const Matrix &in_x,
    const Matrix &in_y, const Matrix &in_z, const Matrix &in_alpha,
    const vector<const Matrix*> &MIMParms, const double refx,
    const double refy,const double refz, Matrix &out_11, Matrix &out_22)
{
    const size_t nChannels = request.getChannelCount();
    const size_t nTimeslots = request.getTimeslotCount();

    //make sure input depends on t only!
    // Check preconditions.
    ASSERT(static_cast<size_t>(in_x.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_y.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_z.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_alpha.nelements()) == nTimeslots);

    double *E11_re, *E11_im;
    out_11.setDCMat(nChannels, nTimeslots);
    out_11.dcomplexStorage(E11_re, E11_im);
    double *E22_re, *E22_im;
    out_22.setDCMat(nChannels, nTimeslots);
    out_22.dcomplexStorage(E22_re, E22_im);

    double phase;
    vector<double> parms(NPARMS);

    // Evaluate beam.
    Axis::ShPtr freqAxis(request.getGrid()[FREQ]);    
    for(size_t t = 0; t < nTimeslots; ++t)
    {
        for(size_t ip=0;ip<NPARMS;ip++)
        {
            parms[ip]=MIMParms[ip]->getDouble(0,t);
        }

        double x=in_x.getDouble(0,t);
        double y=in_y.getDouble(0,t);
        double z=in_z.getDouble(0,t);
        double alpha=in_alpha.getDouble(0,t);
	double tec = calculate_mim_function(parms, x, y, z, alpha, refx,
					    refy, refz);

        for(size_t f = 0; f < nChannels; ++f)
        {
            const double freq = freqAxis->center(f);
            //convert tec-value to freq. dependent phase
	    phase =(75e8/freq)*tec; 
            *E11_re=std::cos(phase);
            *E22_re=std::cos(phase);
            *E11_im=std::sin(phase);
            *E22_im=std::sin(phase);
            
            ++E11_re; ++E22_re; ++E11_im; ++E22_im;
        }
    }
}

double MIM::calculate_mim_function(const vector<double> &parms, double x,
    double y, double z, double alpha, double ref_x, double ref_y,
    double ref_z)
{
    //dummy
    //calculate rotation matrix, actually we only need to do this once for all MIM nodes, 
    // some optimization could be done here
    double lon = std::atan2(ref_y,ref_x);
    double lat = std::atan2(ref_z, std::sqrt(ref_x*ref_x + ref_y*ref_y));
    double rot_x = -1*std::sin(lon)*x+std::cos(lon)*y;
    double rot_y = -1*std::sin(lat)*std::cos(lon)*x-std::sin(lat)*std::sin(lon)
      *y+std::cos(lat)*z;

    uint rank=uint(std::sqrt(parms.size()+1));
    double res=0;;
    for(int ilon=rank-1;ilon>=0;ilon--){
      res*=rot_x/1000.;
      double resy=0;
      for(int ilat=rank-1;ilat>=0;ilat--)
	{
	  resy*=rot_y/1000.;
	  if(ilon==0 && ilat==0) continue;
	  resy+=parms[ilon*rank+ilat-1];
	}
      res+=resy;
    }
    //    return (parms[0]*rot_x/1000.+parms[1]*rot_y/1000.)        /std::cos(alpha);
    return res/std::cos(alpha);

}

  
} //# namespace BBS
} //# namespace LOFAR

