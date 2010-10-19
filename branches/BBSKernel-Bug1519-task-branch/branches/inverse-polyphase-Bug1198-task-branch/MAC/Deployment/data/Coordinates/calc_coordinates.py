#!/usr/bin/env python
#coding: iso-8859-15
import sys,pgdb,pg
from copy import deepcopy
from math import *
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

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

db1 = pgdb.connect(user="postgres", host=dbHost, database=dbName)
cursor = db1.cursor()

# calling stored procedures only works from the pg module for some reason.
db2 = pg.connect(user="postgres", host=dbHost, dbname=dbName)

def print_help():
    print "Usage: calc_coordinates <stationname> <objecttype> date"
    print "    <objecttype>: LBA|HBA|HBA0|HBA1|marker"
    print "    <date>      : yyyy.yy e.g. 2008.75 for Oct 1st 2008"

def subtract(a,b):
    return [x-y for x,y in zip(a,b)]

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
        scale     = 1./float(A[row][row])
        A[row]    = [x*scale for x in A[row]]
        sol[row]  = scale*float(sol[row])
        for ix in range(dim):
            if ix != row:
               factor     = float(A[ix][row])
               A[ix]      = subtract( A[ix], [factor*float(x) for x in A[row]])
               A[ix][row] = 0.0
               sol[ix]   -= factor*float(sol[row])
    return sol


def convert(XEtrs, date_years, trans):
    """
    Solve equation:
     /X\Etrs   /T0\  = [[  1     , -R2*dt,  R1*dt]  /X\Itrs2000
     |Y|     - |T1|     [  R2*dt ,  1    , -R0*dt]  |Y|
     \Z/       \T2/     [ -R1*dt , R0*dt ,  1]]     \Z/
    """
    #
    # get translate parameters from database
    # ref-frame    = trans[0]
    # TOO          = trans[1:4]   = Tx,Ty,Tz
    # mas          = trans[5:8]   = Rx,Ry,Rz
    # diagonal(sf) = trans[4] + 1 = sf
    #
   
    T00    = [float(t) for t in trans[1:4]] # meters
    Rdot00 = [float(t) for t in trans[5:8]] # mas
    #print "T00=[%e %e %e]    Rdot00=[%e %e %e]" %(T00[0],T00[1],T00[2],Rdot00[0],Rdot00[1],Rdot00[2])
    
    dt = date_years - 1989.0
    #print 'date_years=%f  dt=%f' %(date_years, dt)
    sf = float(trans[4]) + 1.
    #print 'sf=',sf
    Matrix = [[ sf          , -Rdot00[2]*dt, Rdot00[1]*dt ],
              [ Rdot00[2]*dt, sf           , -Rdot00[0]*dt],
              [-Rdot00[1]*dt, Rdot00[0]*dt , sf           ]]
    XShifted = subtract(XEtrs,T00)
    #print "Matrix=",Matrix
    return solve(Matrix, XShifted)

#
# MAIN
#
if __name__ == '__main__':
    #print sys.argv
    if len(sys.argv) != 4:
        print_help()
        sys.exit(0)
    
    #trans=[]
    
    date_years = float(sys.argv[3]) 
    
    cursor.execute("select * from get_transformation_info('ITRF2005')")
    trans = cursor.fetchone()
    
    #record = ['CS001','LBA','0','d',3828736.156, 443304.7520, 5064884.523]
     
    cursor.execute("select * from get_ref_objects(%s, %s)", (str(sys.argv[1]).upper(), str(sys.argv[2]).upper()))
    
    print "\n%s    %s    %8.3f" %(str(sys.argv[1]).upper(), str(sys.argv[2]).upper(),float(sys.argv[3]))
    while (1):
	record = cursor.fetchone()
        if record == None:
            print 'record even = None'
            break
        XEtrs = [float(record[4]),
                 float(record[5]),
                 float(record[6])]
        #print 'XEtrs=',XEtrs
        XItrs2000 = convert(XEtrs, date_years, trans)
        
        # write output to generated_coord ??
        print "%d    %14.6f    %14.6f    %14.6f" %(record[2], XItrs2000[0], XItrs2000[1],XItrs2000[2])
        db2.query("select * from add_gen_coord('%s','%s',%s,%s,%s,%s,%s,'%s')" %\
                 (record[0], record[1], record[2], XItrs2000[0], XItrs2000[1], XItrs2000[2], date_years, 'ITRF2005'))
	#record = None
    
    db1.close()
    db2.close()
    sys.exit(1)

