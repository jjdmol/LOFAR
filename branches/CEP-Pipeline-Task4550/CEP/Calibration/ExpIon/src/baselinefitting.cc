//# fitting.cc: Clock and TEC fitting using cascore 
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
//# $Id: $

#include <lofar_config.h>
#include <Common/OpenMP.h>

#include <casa/Containers/ValueHolder.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/CountedPtr.h>
#include <scimath/Fitting/LSQFit.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>
#include <pyrap/Converters/PycRecord.h>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

using namespace casa;
using namespace casa::pyrap;
using namespace boost::python;

namespace LOFAR
{
namespace ExpIon
{
    
ValueHolder fit(const ValueHolder &phases_vh, const ValueHolder &A_vh, const ValueHolder &init_vh, 
                const ValueHolder &flags_vh, const ValueHolder &constant_parm_vh ) 
{
    // Arrays are passed as ValueHolder
    // They are Array is extracted by the asArray<Type> methods
    // They pointer to the actual data is obtained by the Array.data() method.
    
    // Get the phases 
    Array<Float> phases = phases_vh.asArrayFloat();
    Float *phases_data = phases.data();

    // Get the flags
    Array<Bool> flags(flags_vh.asArrayBool());
    Bool noflags = (flags.ndim() == 0);
    Bool *flags_data;
    if (!noflags)
    {
        flags_data = flags.data();
    }
    
    IPosition s = phases.shape();
    int N_station = s[0];
    int N_freq = s[1];
    
    // Get the matrix with basis functions
    Array<Float> A = A_vh.asArrayFloat();
    Float *A_data = A.data();
    
    IPosition s1 = A.shape();
    
    int N_coeff = s1[0];
    int N_parm = N_station * N_coeff;
    Float sol[N_parm];
    
    Array<Float> init(init_vh.asArrayFloat());
    if (init.ndim() == 0)
    {
        for(int i = 0; i < N_parm; i++) sol[i] = 0.0;
    }
    else    
    {
        for(int i = 0; i < N_parm; i++) sol[i] = init.data()[i];
    }
    
    // Get the flags indicating which parameters are constant_parm
    // i.e. are not a free parameter in the minimization problem
    Array<Bool> constant_parm(constant_parm_vh.asArrayBool());
    Bool no_constant_parm = (constant_parm.ndim() == 0);
    Bool *constant_parm_data;
    if (!no_constant_parm)
    {
        constant_parm_data = constant_parm.data();
    }
    
    Float cEq[N_parm];
    for(int i=0; i<N_parm; ++i) cEq[i] = 0.0;
    
    int N_thread = OpenMP::maxThreads();
    LSQFit lnl[N_thread];
    
    uInt nr = 0;
    
    for (int iter = 0; iter<1000; iter++)
    {
        for(int i = 0; i<N_thread; i++) {
            lnl[i] = LSQFit(N_parm);
        }
        #pragma omp parallel
        {
            int threadNum = OpenMP::threadNum();
            Float derivatives_re[2*N_coeff];
            Float derivatives_im[2*N_coeff];
            uInt idx[2*N_coeff];
            #pragma omp for
            for(int k = 0; k<N_freq; k++)
            {
                Float *A_data_k = A_data + k*N_coeff;
                Float *phases_data_k = phases_data + k*N_station;
                Bool *flags_data_k = flags_data + k*N_station;
                for(int i = 1; i<N_station; i++) 
                {
                    Float phases_data_k_i = phases_data_k[i];
                    Bool flags_data_k_i = flags_data_k[i];
                    for(int j = 0; j<i; j++)
                    {
                        if (noflags || !(flags_data_k_i || flags_data_k[j]))
                        {
                            Float phase_ij_obs = phases_data_k_i - phases_data_k[j];
                            Float phase_ij_model = 0.0;
                            
                            for (int l = 0; l<N_coeff; l++) 
                            {
                                Float coeff = A_data_k[l];
                                phase_ij_model += (sol[i + l*N_station] - sol[j + l*N_station]) * coeff ;
                            }
                            Float sin_dphase, cos_dphase;
                            sincosf(phase_ij_obs - phase_ij_model, &sin_dphase, &cos_dphase);
                            Float residual_re = cos_dphase - 1.0;
                            Float residual_im = sin_dphase;
                            Float derivative_re = -sin_dphase;
                            Float derivative_im = cos_dphase;
 
                            
                            for (int l = 0; l<N_coeff; l++)
                            {
                                Float coeff = A_data_k[l];
                                Float a = derivative_re * coeff;
                                Float b = derivative_im * coeff;
                                derivatives_re[l] = a;
                                derivatives_re[N_coeff + l] = -a;
                                derivatives_im[l] = b;
                                derivatives_im[N_coeff + l] = -b;
                                idx[l] = i+l*N_station;
                                idx[N_coeff + l] = j+l*N_station;
                            }
                            lnl[threadNum].makeNorm(uInt(2*N_coeff), (uInt*) idx, (Float*) derivatives_re, Float(1.0), residual_re);
                            lnl[threadNum].makeNorm(uInt(2*N_coeff), (uInt*) idx, (Float*) derivatives_im, Float(1.0), residual_im);
                        }
                    }
                }
            }
        }
        
        for(int i = 1; i<N_thread; i++)
        {
            lnl[0].merge(lnl[i]);
        }
        

        Float eq[2];
        eq[0] = 1.0;
        eq[1] = -1.0;
        uInt idx[2];

        if ((!no_constant_parm) )
        {
            for (int i = 0; i<N_parm; i++) 
            {
                if (constant_parm_data[i])
                {
                    cEq[i] = 1.0;
                    lnl[0].addConstraint( (Float*) cEq, 0.0);
                    cEq[i] = 0.0;
                }
            }
        }
        
        if (!lnl[0].solveLoop(nr, sol, True)) 
        {
            cout << "Error in loop: " << nr << endl;
            break;
        }
        if (lnl[0].isReady())
        {
            break;
        }
    }
    
    Array<Float> solutions(IPosition(2, N_station, N_coeff), sol);
    ValueHolder result(solutions);
    return result;
};


} // namespace ExpIon
} // namespace LOFAR

BOOST_PYTHON_MODULE(_baselinefitting)
{
    casa::pyrap::register_convert_excp();
    casa::pyrap::register_convert_basicdata();
    casa::pyrap::register_convert_casa_valueholder();
    casa::pyrap::register_convert_casa_record();
    
    def("fit", LOFAR::ExpIon::fit);
}
