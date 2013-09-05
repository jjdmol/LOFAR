#!/usr/bin/env python
#coding: iso-8859-15
import re,sys
from scipy import *
from scipy.linalg import *

#
# getCoordLines
#
def getCoordLines(filename):
    """
    Returns a list containing all lines with coordinates
    """
    pattern=re.compile("^[0-9]+;.*", re.IGNORECASE | re.MULTILINE)
    return [ line for line in pattern.findall(open(filename).read())]

#
# calcDiffForZ
#
def calcDiffForZ(XYZmeas, solution):
    resultZ = []
    a,b,c = solution
    for X,Y,Z in XYZmeas:
        resultZ.append(Z - (a*X + b*Y + c))
    return resultZ

#
# fitPlane(originalCoordinated, selectedCoordinates)
#
def fitPlane(orgXYZ, selectedXYZ):
    # fit using the selected coordinates
    M=[]
    b=[]
    print
    print "fitting plane with", len(selectedXYZ), "points"
    for X,Y,Z in selectedXYZ:
        M.append([X,Y,1])
        b.append(Z)
    solution=lstsq(M,b)
    # using the solution calculate the stddev from Z
    stddev = std(calcDiffForZ(selectedXYZ, solution[0]))
    print "solution= ", solution[0]
    print "standard deviation= ", stddev

    # evaluate each point in the originalXYZ set and skip points with stddev > 3 sigma
    diffZ  = calcDiffForZ(orgXYZ, solution[0])
    newXYZ = []
    i      = 0
    for Z in diffZ:
        if (abs(Z) < 3.0*stddev):
            newXYZ.append([orgXYZ[i][0], orgXYZ[i][1], orgXYZ[i][2]])
        else:
            print "Discarding point ", i, abs(Z)
        i+=1

    if (len(selectedXYZ) == len(newXYZ)):
        return (solution[0])
    else:
        return fitPlane(orgXYZ, newXYZ)

#
# MAIN
#
if __name__ == '__main__':    

    # check syntax of invocation
    # Expected syntax: load_measurement stationname objecttypes datafile
    #
    if (len(sys.argv) != 2):
        print "Syntax: %s datafile" % sys.argv[0]
        sys.exit(1)

    orgXYZ=[]
    for cline in getCoordLines(sys.argv[1]):
        ( number, X, Y, Z, sX, sY, sZ ) = cline.split(';')
        orgXYZ.append([float(X), float(Y), float(Z)])
    [a,b,c] = fitPlane(orgXYZ, orgXYZ)
    print "Plane equation= ", a, b, c
    print

    normVect = array([-a, -b, 1]) / sqrt (a*a + b*b + 1)
    print "Normal vector=", normVect

    X0 = average(array_split(array(orgXYZ),3,axis=1)[0])
    Y0 = average(array_split(array(orgXYZ),3,axis=1)[1])
    print "(X0, Y0)= ", X0, Y0

    MeridianPlane = array([Y0, -X0, 0]) / sqrt(X0*X0 + Y0*Y0)
    print "MeridianPlane = ", MeridianPlane

    Qvect = cross(MeridianPlane, normVect)
    print "Qvect=", Qvect

    Pvect = cross(Qvect, normVect)
    print "Pvect=", Pvect

    Qcore = array([-0.791954, -0.095419, 0.603078]) / (1 - 5e-7)
    print "Qcore=", Qcore

    print "rotationMatrix=", array([[Pvect[0], Qvect[0], normVect[0]],[Pvect[1], Qvect[1], normVect[1]],[Pvect[2], Qvect[2], normVect[2]]]) 

