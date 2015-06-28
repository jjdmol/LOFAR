"""Fit basefunctions to phases using all baselines

Application: Clock-TEC separation


The phase can be described by a linear model

.. math::
   phases = A p
   
where the columns of matrix A contain the basefunctions and p are the parameters

The same equation holds for the phases of multiple stations
The dimension of phases is then N_freq x N_station and 
the dimension of p is N_freq x N_param

The cost function that will be minimized is 

.. math::
   \\sum_{i=1}^{N_{station}-1}\\sum_{j=0}^{i-1} \\sum_{k=0}^{N_{freq}} \\| \\exp(\imath(\\Delta phase_{ijk} - \\Delta modelphase_{ijk}))  \\|^2
where
.. math::
   \\Delta phase_{ijk} =
   \\Delta modelphase_{ijk} = 



"""
  
 
import _baselinefitting
 
def fit(phases, A, p_0 = None, flags = None, constant_parms = None):
    """see module description for detailed info"""
    return _baselinefitting.fit(phases, A, p_0, flags, constant_parms)