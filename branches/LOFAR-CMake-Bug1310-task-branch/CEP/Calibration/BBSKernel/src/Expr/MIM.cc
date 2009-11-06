//# MIM.cc: Ionospheric disturbance of a (source,station) combination.
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
//# $Id$

#include <lofar_config.h>

#include <BBSKernel/Expr/MIM.h>
#include <BBSKernel/Expr/PValueIterator.h>

//using namespace casa;

namespace LOFAR
{
namespace BBS
{

MIM::MIM(const Expr &pp, const vector<Expr> &MIMParms, const Expr &ref_pp)
{
    ASSERTSTR(MIMParms.size() == NPARMS, "Wrong number of MIMparms defined!");
    for(uint i = 0; i != MIMParms.size(); ++i)
    {
        addChild(MIMParms[i]);
    }
    
    addChild(pp);
    addChild(ref_pp);
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

    ResultVec ref_pp_res;
    const ResultVec &ref_pp =
        getChild(NPARMS+1).getResultVecSynced(request,ref_pp_res);

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
         ref_pp[0].getValue(),ref_pp[1].getValue(),ref_pp[2].getValue(),
         res.result11().getValueRW(),
         res.result22().getValueRW());

    // Compute perturbed values.  
    vector<const Result*> pvSet(parms);
    pvSet.push_back(&(pp[0]));
    pvSet.push_back(&(pp[1]));
    pvSet.push_back(&(pp[2]));
    pvSet.push_back(&(pp[3]));
    pvSet.push_back(&(ref_pp[0]));
    pvSet.push_back(&(ref_pp[1]));
    pvSet.push_back(&(ref_pp[2]));

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
            pvIter.value(NPARMS + 4), pvIter.value(NPARMS + 5),
            pvIter.value(NPARMS + 6),
            res.result11().getPerturbedValueRW(pvIter.key()),
            res.result22().getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }

    return res;
}


void MIM::evaluate(const Request &request, const Matrix &in_x,
    const Matrix &in_y, const Matrix &in_z, const Matrix &in_alpha,
    const vector<const Matrix*> &MIMParms, const Matrix &in_refx,
    const Matrix &in_refy,const Matrix &in_refz, Matrix &out_11, Matrix &out_22)
{
    const size_t nChannels = request.getChannelCount();
    const size_t nTimeslots = request.getTimeslotCount();

    //make sure input depends on t only!
    // Check preconditions.
    ASSERT(static_cast<size_t>(in_x.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_y.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_z.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_refx.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_refy.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_refz.nelements()) == nTimeslots);
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
        double refx=in_refx.getDouble(0,t);
        double refy=in_refy.getDouble(0,t);
        double refz=in_refz.getDouble(0,t);
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
    //calculate rotation matrix
    double lon = std::atan2(ref_y,ref_x);
    double lat = std::atan2(ref_z, std::sqrt(ref_x*ref_x + ref_y*ref_y));
    double rot_x = -1*std::sin(lon)*x+std::cos(lon)*y;
    double rot_y = -1*std::sin(lat)*std::cos(lon)*x-std::sin(lat)*std::sin(lon)
      *y+std::cos(lat)*z;

    return (parms[0]*rot_x/1000.+parms[1]*rot_y/1000.)
        /std::cos(alpha);
}

#ifdef EXPR_GRAPH
std::string MIM::getLabel()
{
    return std::string("MIM\\nIonospheric disturbance of a source/station"
        " combination.");
}
#endif
  
} //# namespace BBS
} //# namespace LOFAR

