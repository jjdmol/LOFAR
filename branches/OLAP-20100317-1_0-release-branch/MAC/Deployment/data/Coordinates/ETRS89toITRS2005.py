#!/usr/bin/env python
#coding: iso-8859-15
#import sys,pgdb
import sys
from copy import deepcopy
from math import *
#from numpy import *
from numarray import *


INTRO=""" 
Conversion between ETRS89 and ITRS2000 coordinates based on
 Memo : Specifications for reference frame fixing in the analysis of a
        EUREF GPS campaign
 By Claude Boucher and Zuheir Altamimi

 which is available from EUREF

 In this utility I use the translational coefficients obtained by method "A" in
 section 4 and the rotational coefficients in section 5, both for the 2000 (00)
 reference frame.
""" 

def print_help():
    print "Usage: calc_coordinates <stationname> <objecttype> date"
    print "    <objecttype>: LBA|HBA|marker"
    print "    <date>      : yyyy.yy e.g. 2008.75 for Oct 1st 2008"

def subtract(a,b):
    return [x-y for x,y in zip(a,b)]


def rad_from_mas(mas):
    """
    Convert milli arc seconds to radians.
    1 as = pi / (60 * 60 * 180) radians
    """
    return pi*mas/3.6e+6/180.0


def solve(M,y):
    """
    solve Mx=y. The algorithm is Gauss-Jordan elimination
    without pivoting, which is allowed in this case as M is
    dominated by the diagonal. 
    """
    dim  = len(y)
    A    = deepcopy(M)
    sol  = deepcopy(y)
    if (len(A) != len(A[0])) or len(A[0]) != len(y):
        raise 'Incompatible dimensions'
    for row in range(dim):
        scale     = 1/float(A[row][row])
        A[row]    = [x*scale for x in A[row]]
        sol[row]  = scale*float(sol[row])
        for ix in range(dim):
            if ix != row:
               factor     = float(A[ix][row])
               A[ix]      = subtract( A[ix], [factor*float(x) for x in A[row]])
               A[ix][row] = 0.0
               sol[ix]     -= factor*float(sol[row])
    return sol


def convert(XEtrs, date_years):
    """
    Solve equation:
     /X\Etrs   /T0\  = [[  1     , -R2*dt,  R1*dt]  /X\Itrs2000
     |Y|     - |T1|     [  R2*dt ,  1    , -R0*dt]  |Y|
     \Z/       \T2/     [ -R1*dt , R0*dt ,  1]]     \Z/
    """
    T00    = [0.054, 0.051, -0.048] # meters
    Rdot00 = [rad_from_mas(mas) for mas in [0.081, 0.490, -0.792]] # mas
    dt     = date_years-1989.0
    Matrix = [[ 1           , -Rdot00[2]*dt, Rdot00[1]*dt],
              [ Rdot00[2]*dt, 1            , -Rdot00[0]*dt],
              [-Rdot00[1]*dt, Rdot00[0]*dt , 1]]
    XShifted = subtract(XEtrs,T00)
    return solve(Matrix, XShifted)


def latlonhgt2XYZ(lat, lon, height):
    """
    Convert the latitude,longitude,height arguments to a X,Y,Z triple
    The arguments must be in degrees and meters.
    """
    # wgs84 ellips constants
    wgs84a  = 6378137.0
    wgs84df = 298.25722356
    wgs84b  = (1.0-(1.0/wgs84df))*wgs84a
    e2      =  1.0-((wgs84b*wgs84b)/(wgs84a*wgs84a))

    latrad = radians(lat)
    lonrad = radians(lon)
    N = wgs84a / sqrt(1.0-(e2 * pow(sin(latrad),2)))
    X = (N+height) * cos(latrad) * cos(lonrad)
    Y = (N+height) * cos(latrad) * sin(lonrad)
    Z = (N*(1-e2) + height) * sin(latrad)

    return ( X, Y, Z )
    

def I89toI2005(XEtrs89, date_years):
    """
    Convert the given Etrs89 coordinates to I2005 coordinates for the given date
    """
    # T in cm, S in 10e-9, R in 0.001''
    # 2005->2000 according to http://itrf.ensg.ign.fr/ITRF_solutions/2005/tp_05-00.php
    #  ITRF2005   0.1   -0.8    -5.8   0.40    0.000    0.000    0.000
    #  rates	 -0.2    0.1    -1.8   0.08    0.000    0.000    0.000
    # 2000->1989 according to  ftp://itrf.ensg.ign.fr/pub/itrf/ITRF.TP
    #  ITRF89       2.97  4.75 -7.39    5.85    0.00    0.00   -0.18   1988.0    6
    # (rates        0.00 -0.06 -0.14    0.01    0.00    0.00    0.02)

    T2005to2000 = array([  0.1, -0.8 , -5.8  ]) # ITRS2005 to ITRS2000
    T2000to1989 = array([ 2.97,  4.75, -7.39 ]) # ITRS2000 to ITRS89 = ETRS89
    Tdot2005    = array([ -0.2,  0.1,  -1.8  ]) # shift per year for I2005
    S2005to2000 = 0.4
    S2000to1989 = 5.85
    Sdot2005    = 0.08
    R2005to2000 = array([ 0.0, 0.0,  0.0  ])
    R2000to1989 = array([ 0.0, 0.0, -0.18 ])
    Rdot2005    = array([ 0.0, 0.0,  0.0  ])

    Tfixed = T2005to2000 + T2000to1989
    Rfixed = R2005to2000 + R2000to1989
    Sfixed = S2005to2000 + S2000to1989
    Ttot   = (Tfixed + (Tdot2005 * (date_years - 2005.0))) / 100.0     # meters
    Rtot   = rad_from_mas(Rfixed + (Rdot2005 * (date_years - 2005.0))) # rad
    Stot   = (Sfixed + (Sdot2005 * (date_years - 2005.0))) / 1.0e9
    print "Ttot:", Ttot
    print "Rtot:", Rtot
    print "Stot:", Stot

    Matrix = array([[        1,  Rtot[2], -Rtot[1]],
                    [ -Rtot[2],        1,  Rtot[0]],
                    [  Rtot[1], -Rtot[0],        1]])

    Xnow = Ttot + (1 + Stot) * Matrix * XEtrs89
    return (Xnow[0][0], Xnow[1][1], Xnow[2][2])

#
# MAIN
#
if __name__ == '__main__':
    if len(sys.argv) != 4:
        print_help()
        sys.exit(0)

    (X, Y, Z) = latlonhgt2XYZ(52.9129392, 6.8690294, 54.1)
    print X, Y, Z
    (Xn, Yn, Zn) = I89toI2005([X, Y, Z], 2007.775342466)
    print Xn, Yn, Zn
    sys.exit(0)

    date_years = float(sys.argv[3]) 
    db = pgdb.connect(user="postgres", host="dop50", database="coordtest")
    cursor = db.cursor()
    cursor.execute("select * from get_ref_objects(%s, %s)", (sys.argv[1],sys.argv[2]))
    while (1):
        record = cursor.fetchone()
        if record == None:
            break
        XEtrs = [float(record[3]),
                 float(record[4]),
                 float(record[5])]
             
        XItrs2000 = convert(XEtrs, date_years)
        print record[2],'	',XItrs2000[0],'    ', XItrs2000[1],'    ', XItrs2000[2]
    db.close()
    sys.exit(1)

