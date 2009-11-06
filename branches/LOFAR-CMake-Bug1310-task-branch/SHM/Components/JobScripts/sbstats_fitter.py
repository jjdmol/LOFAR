#! /usr/bin/env python

# import sys
import scipy
from scipy import optimize
from scipy import signal
import scipy.stats
from numpy import * 
import datetime


class SST_fit:
    "classify SST (Subband STatistics = passband) data"
    def __init__(self):
        self.x = scipy.linspace(0,1,len(arange(512)))  #necessary for spline model template
        # self.g0 = array([1.0e0, 1.0e-2, 1.001e0])      #guess model parameters
        self.g0 = array([1.0e0, 1.0e-2, 0.0])      #guess model parameters
        # self.g = array([scipy.NaN,scipy.NaN,scipy.NaN])
        self.g = self.g0.copy()
        # self.success = scipy.NaN
        self.success = 0.01
        
    def cost(self,gain,spectrum):
        return sqrt(abs(self.passband_profile(gain) - spectrum[self.bins]))

    def fit(self,spectrum):
        # (g,cov_x,infodict,mesg,success) = optimize.leastsq(self.cost, self.g0, args = spectrum,full_output=1)
        (g,success) = optimize.leastsq(self.cost, self.g0, args = spectrum,full_output=0)

        # for convenient return values
        self.g = g
        self.success = success
        # self.fitness = scipy.stats.median(self.cost(g,spectrum))**2
        self.fitness = scipy.stats.median((self.cost(g,spectrum)**2)/spectrum[self.bins])

        # return (cov_x,infodict,mesg)
        #return (0,0,'')
        return (self.g,self.fitness)

class rcumode03_fit(SST_fit):
    "fit spline template function to RCU mode 03 (LBH 10-80MHz) passband"

    def __init__(self):
        SST_fit.__init__(self)
        self.name = 'RCUMODE03'
        self.bins = arange(100,512-100)

    def passband_profile(self,gain):
            # tck is a spline fit to a representative LB spectrum (happend to be LST=1.836)
            # tck,fp,ier,msg = scipy.interpolate.splrep(x[xbins],s[xbins],full_output=1,s=1.0e0)
            tck = (array([ 0.19569472,  0.19569472,  0.19569472,  0.19569472,  0.34833659,
                           0.42465753,  0.50097847,  0.54011742,  0.54990215,  0.55577299,
                           0.55968689,  0.57729941,  0.58708415,  0.59686888,  0.61643836,
                           0.62622309,  0.63600783,  0.65362035,  0.80430528,  0.80430528,
                           0.80430528,  0.80430528]),
                   array([  8.96984273e-01,   9.68459150e-01,   8.83629654e-01,
                            1.49433570e+00,   2.48006932e+00,   6.13538548e+00,
                            9.81612736e+00,   1.17097536e+01,   1.27884287e+01,
                            1.19211082e+01,   1.09229797e+01,   6.28624231e+00,
                            4.91826221e+00,   5.25217792e+00,   3.07852571e+00,
                            1.13611380e-02,   8.57777528e-01,   2.46572479e-01]),
                   3)
            # _x = (self.x - 0.5) * gain[2] + 0.5
            _x = (self.x - 0.01*gain[2])
            model = gain[0] * 1.0e7 * (scipy.interpolate.splev(_x[self.bins],tck)**(1.0+ gain[1]))
            # model = gain[0] * 1.0e7 * (scipy.interpolate.splev(self.x[self.bins],tck)**(1.0+ gain[1]))
            return model

    def inlimits(self):
        '''empirical limits to fit parameters from examination of SHM db'''
        # The gains g[0] & g[1] are actually almost perfectly correlated
        #  and together follow a well defined curve parametrised by LST;
        # Could do this check wrt LST.
        if ( ( (self.g)[0] > 0.8)   and ( (self.g)[0] < 1.8)  and
             ( (self.g)[1] > -0.08) and ( (self.g)[1] < 0.08) and
             (  self.fitness < 0.1)):
            return True
        else:
            return False

class rcumode04_fit(SST_fit):
    "fit spline template function to RCU mode 04 (LBH 30-80MHz) passband"

    def __init__(self):
        SST_fit.__init__(self)
        self.name = 'RCUMODE04'
        self.bins = arange(100,512-100)

    def passband_profile(self,gain):
        # tck,fp,ier,msg = scipy.interpolate.splrep(x[xbins],s[xbins],full_output=1,s=5.0e-1)
        # this model is most detailed because of low rfi template; wings detailed;
        #  don't have to fit full span in practice
        tck = (array([ 0.09393346,  0.09393346,  0.09393346,  0.09393346,  0.18590998,
                       0.23091977,  0.24266145,  0.25440313,  0.26614481,  0.27592955,
                       0.32289628,  0.36790607,  0.41291585,  0.45792564,  0.481409  ,
                       0.50489237,  0.52837573,  0.53424658,  0.54011742,  0.54990215,
                       0.5518591 ,  0.55381605,  0.55577299,  0.55772994,  0.55968689,
                       0.56164384,  0.56751468,  0.57142857,  0.57338552,  0.57534247,
                       0.57729941,  0.57925636,  0.58317025,  0.5851272 ,  0.5890411 ,
                       0.59099804,  0.59491194,  0.59882583,  0.60078278,  0.60665362,
                       0.61252446,  0.6183953 ,  0.62426614,  0.63013699,  0.63992172,
                       0.65166341,  0.66340509,  0.68493151,  0.72994129,  0.81996086,
                       0.81996086,  0.81996086,  0.81996086]),
               array([  0.49419084,   0.70902827,   0.63042392,   0.64247414,
                        0.87857545,   0.96119004,   1.3133642 ,   1.44227703,
                        1.43644923,   1.92768973,   2.30381448,   3.09664806,
                        4.6490922 ,   6.09464831,   8.88993676,  11.94071685,
                        14.16023748,  16.06291426,  17.7472498 ,  19.34733097,
                        19.44842678,  19.90988532,  19.69906503,  21.29215077,
                        21.33143344,  20.89211216,  20.97379891,  21.02803228,
                        20.31568386,  19.13131575,  18.06117994,  17.45281504,
                        16.38181542,  14.61967262,  13.55586438,  12.83092652,
                        10.66424895,   9.93016419,   8.56173155,   8.72624551,
                        8.57641651,   7.01989367,   5.29288074,   4.1100342 ,
                        3.04211097,   1.74058146,   0.89419475,   0.75981934,   0.40956083]),
               3)
        # _x = (self.x - 0.5) * gain[2] + 0.5
        _x = (self.x - 0.01*gain[2])
        model = gain[0] * 1.0e7 * (scipy.interpolate.splev(_x[self.bins],tck)**(1.0+ gain[1]))
        # model = gain[0] * 1.0e7 * (scipy.interpolate.splev(self.x[self.bins],tck)**(1.0+gain[1]))
        return model

    def inlimits(self):
        '''empirical limits to fit parameters from examination of SHM db'''
        if ( ( (self.g)[0] > 0.50)   and ( (self.g)[0] < 1.20)  and
             ( (self.g)[1] > -0.07)  and ( (self.g)[1] < 0.10) and
             (  self.fitness < 0.1)):
            return True
        else:
            return False
    
class rcumode05_fit(SST_fit):
    "fit spline template function to RCU mode 05 (HB 110-190MHz) passband"

    def __init__(self):
        SST_fit.__init__(self)
        self.name = 'RCUMODE05'
        self.bins = arange(32,512-32)

    def passband_profile(self,gain):
        # appr for 110-190 MHz
        # clean example spectrum, so model covers large bin range (32:512-32);
        #   don't need to fit such large range
        # tck,fp,ier,msg = scipy.interpolate.splrep(x[idx],s[idx],full_output=1,s=5.0e-2)
        tck = (array([ 0.06262231,  0.06262231,  0.06262231,  0.06262231,  0.11741683,
                       0.17221135,  0.19960861,  0.22700587,  0.25440313,  0.28180039,
                       0.39138943,  0.50097847,  0.55577299,  0.61056751,  0.66536204,
                       0.72015656,  0.8297456 ,  0.93737769,  0.93737769,  0.93737769,
                       0.93737769]),
               array([ 0.71437508,  1.06758007,  1.41973097,  1.37215385,  2.08257292,
                       2.58576608,  2.51740134,  1.50299689,  1.08885794,  0.91703928,
                       0.94893394,  1.203528  ,  1.26062858,  0.7153864 ,  0.19364445,
                       0.25003272,  0.09605682]),
               3)
        # _x = (self.x - 0.5) * gain[2] + 0.5
        _x = (self.x - 0.01*gain[2])
        model = gain[0] * 1.0e7 * (scipy.interpolate.splev(_x[self.bins],tck)**(1.0+ gain[1]))
        # model = gain[0] * 1.0e7 * (scipy.interpolate.splev(self.x[self.bins],tck)**(1.0+ gain[1]))
        return model

    def inlimits(self):
        '''empirical limits to fit parameters from examination of SHM db'''
        # if ( ( (self.g)[0] > 2.80)   and ( (self.g)[0] < 4.00)  and
        #      ( (self.g)[1] > -0.02)  and ( (self.g)[1] < 0.08) and
        #      (  self.fitness < 0.1)):
        if ( ( (self.g)[0] > 0.25)   and ( (self.g)[0] < 0.85)  and
             ( (self.g)[1] > -0.5)  and ( (self.g)[1] < 0.10) and
             (  self.fitness < 0.1)):
            return True
        else:
            return False
    
class rcumode06_fit(SST_fit):
    "fit spline template function to RCU mode 06 (HB 170-230 MHz) passband"

    def __init__(self):
        SST_fit.__init__(self)
        self.name = 'RCUMODE06'
        self.bins = arange(64,512-64)

    def passband_profile(self,gain):
        tck = (array([ 0.12524462,  0.12524462,  0.12524462,  0.12524462,  0.21917808,
                       0.31311155,  0.40704501,  0.50097847,  0.59491194,  0.64187867,
                       0.6888454 ,  0.73581213,  0.78277886,  0.87475538,  0.87475538,
                       0.87475538,  0.87475538]),
               array([ 0.64756739,  0.95663549,  2.18475486,  1.98772696,  1.15377759,
                       0.91197256,  1.11596855,  1.17076092,  1.01908458,  0.88536053,
                       0.7493797 ,  0.52499908,  0.28522369]),
               3)
        # _x = (self.x - 0.5) * gain[2] + 0.5
        _x = (self.x - 0.01*gain[2])
        model = gain[0] * 1.0e7 * (scipy.interpolate.splev(_x[self.bins],tck)**(1.0+ gain[1]))
        return model

    def inlimits(self):
        '''empirical limits to fit parameters from examination of SHM db'''
        if ( ( (self.g)[0] > 2.80)   and ( (self.g)[0] < 6.00)  and
             ( (self.g)[1] > -0.02)  and ( (self.g)[1] < 0.30) and
             (  self.fitness < 0.1)):
        # if ( ( (self.g)[0] > 0.010)   and ( (self.g)[0] < 10.00)  and
        #     ( (self.g)[1] > -0.6)  and ( (self.g)[1] < 0.50) and
        #     (  self.fitness < 0.2)):
            return True
        else:
            return False
    
class rcumode07_fit(SST_fit):
    "fit spline template function to RCU mode 07 (HB 210-270 MHz) passband"

    def __init__(self):
        SST_fit.__init__(self)
        self.name = 'RCUMODE07'
        self.bins = arange(58,308)

    def passband_profile(self,gain):
        # tck,fp,ier,msg = scipy.interpolate.splrep(x[idx],s[idx],full_output=1,s=5.0e-1)
        tck = (array([ 0.11350294,  0.11350294,  0.11350294,  0.11350294,  0.14481409,
                       0.17612524,  0.23679061,  0.29745597,  0.35812133,  0.47945205,
                       0.51076321,  0.54011742,  0.60078278,  0.60078278,  0.60078278,
                       0.60078278]),
               array([ 1.64193764,  2.23586444,  3.97529527,  5.14499283,  5.64402102,
                       4.89082124,  1.56625712,  1.95341818,  2.48234224,  1.68670441,
                       1.46402628,  1.08066387]),
               3)
        # _x = (self.x - 0.5) * gain[2] + 0.5
        _x = (self.x - 0.01*gain[2])
        model = gain[0] * 1.0e7 * (scipy.interpolate.splev(_x[self.bins],tck)**(1.0+ gain[1]))
        # model = gain[0] * 1.0e7 * (scipy.interpolate.splev(self.x[self.bins],tck)**(1.0+ gain[1]))
        return model

    def inlimits(self):
        '''limits copied from HB1 -- too few examples in SHM db'''
        # The gains g[0] & g[1] are actually almost perfectly correlated
        #  and together follow a well defined curve parametrised by LST;
        # Could do this check wrt LST.
        if ( ( (self.g)[0] > 3.00)   and ( (self.g)[0] < 6.00)  and
             ( (self.g)[1] > -0.02) and ( (self.g)[1] < 0.10) and
             (  self.fitness < 0.1)):
            return True
        else:
            return False
