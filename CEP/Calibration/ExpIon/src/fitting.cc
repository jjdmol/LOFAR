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
// #include <omp.h>
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
    
ValueHolder fit(const ValueHolder &phases_vh, const ValueHolder &A_vh, const ValueHolder &init_vh = ValueHolder(), const ValueHolder &flags_vh = ValueHolder()) 
{
    Array<Float> phases = phases_vh.asArrayFloat();
    Float *phases_data = phases.data();
    Array<Bool> flags;
    Bool noflags = flags_vh.isNull();
    if (!noflags)
    {
      flags = flags_vh.asArrayBool();
    }
    Bool *flags_data = flags.data();
    Array<Float> A = A_vh.asArrayFloat();
    Float *A_data = A.data();
    IPosition s = phases.shape();
    IPosition s1 = A.shape();
    
    int N_station = s[0];
    int N_freq = s[1];
    int N_coeff = s1[0];
    int N_parm = N_station * N_coeff;
    Float sol[N_parm];
    
    int N_thread = OpenMP::maxThreads();
    LSQFit lnl[N_thread];
    lnl[0] = LSQFit(N_parm);
    lnl[0].setMaxIter(10000);
    uInt nr = 0;
    
    if (!init_vh.isNull())
    {
        Array<Float> init = init_vh.asArrayFloat();
        for(int i = 0; i < N_parm; i++) sol[i] = init.data()[i];
    }
    else    
    {
        for(int i = 0; i < N_parm; i++) sol[i] = 0.0;
    }
    int iter = 0;
    while (!lnl[0].isReady()) 
    {
        for(int i = 1; i<N_thread; i++) {
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
                            
                            int idx_i = i*N_coeff;
                            int idx_j = j*N_coeff;
                            
                            for (int l = 0; l<N_coeff; l++) 
                            {
                                Float coeff = A_data_k[l];
                                phase_ij_model += (sol[idx_i + l] - sol[idx_j + l]) * coeff ;
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
                                idx[l] = idx_i+l;
                                idx[N_coeff + l] = idx_j+l;
                            }
                            lnl[threadNum].makeNorm(uInt(2*N_coeff), (uInt*) idx, (Float*) derivatives_re, Float(1.0), residual_re);
                            lnl[threadNum].makeNorm(uInt(2*N_coeff), (uInt*) idx, (Float*) derivatives_im, Float(1.0), residual_im);
                        }
                    }
                }
            }
//             #pragma omp sections
//             {
//                 { lnl[0].merge(lnl[1]); }
//                 #pragma omp section
//                 { lnl[2].merge(lnl[3]); }
//             }
        }
//         lnl[0].merge(lnl[2]);
        
        for(int i = 1; i<N_thread; i++)
        {
            lnl[0].merge(lnl[i]);
        }
        
        if (!lnl[0].solveLoop(nr, sol, True)) 
        {
            cout << "Error in loop: " << nr << endl;
            break;
        }
        iter++;
    }
    cout << iter << endl;
    
    Array<Float> solutions(IPosition(2, N_coeff, N_station ), sol);
    ValueHolder result(solutions);
    return result;
};

ValueHolder fit2(const ValueHolder &phases_vh, const ValueHolder &A_vh) 
{
    return fit(phases_vh, A_vh);
}    


} // namespace ExpIon
} // namespace LOFAR

BOOST_PYTHON_MODULE(fitting)
{
    casa::pyrap::register_convert_excp();
    casa::pyrap::register_convert_basicdata();
    casa::pyrap::register_convert_casa_valueholder();
    casa::pyrap::register_convert_casa_record();
    
    def("fit", LOFAR::ExpIon::fit);
    def("fit", LOFAR::ExpIon::fit2);
}
