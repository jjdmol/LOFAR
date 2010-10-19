#!/usr/bin/env python
#coding: iso-8859-15
import sys,pgdb
from copy import deepcopy
from math import *


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

    
    
#
# MAIN
#
if __name__ == '__main__':
    if len(sys.argv) != 4:
        print_help()
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

