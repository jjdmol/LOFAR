#!/usr/bin/env python
#coding: iso-8859-15
#import sys,pgdb,pg
#from copy import deepcopy
import sys,pgdb,pg
from math import *
import numpy as np
import string

dbName="donker"
dbHost="10.87.2.185"
#dbHost="dop50"

db1 = pgdb.connect(user="postgres", host=dbHost, database=dbName)
cursor = db1.cursor()

# calling stored procedures only works from the pg module for some reason.
db2 = pg.connect(user="postgres", host=dbHost, dbname=dbName)


##
def getRotation(station, anttype):
    cursor.execute("select * from get_field_rotation(%s, %s)", (station, anttype))
    record = cursor.fetchone()
    if record != None:
        rotation = float(record[2])
        return(rotation)
    return(0.0)

##
def getRotationMatrix(station, anttype):
    matrix = np.zeros((3,3))
    cursor.execute("select * from get_rotation_matrix(%s, %s)", (station, anttype))
    record = cursor.fetchone()
    if record != None:
        record = str(record[2]).replace('{','').replace('}','').split(',')
        print record
        cnt = 0
        for row in range(3):
            for col in range(3):
                matrix[row][col] = float(record[cnt])
                cnt += 1
    return(matrix)

##
def getStations(anttype):
    stations = []
    query = "SELECT o.stationname FROM object o INNER JOIN rotation_matrices r ON r.id = o.id WHERE o.type='%s'" %(anttype)
    print query
    cursor.execute(query)
    stations = cursor.fetchall()
    print stations
    return(stations)

##
def makeDBmatrix(matrix):
    shape = np.shape(matrix)
    # make db matrix [16][3]
    dbmatrix = "ARRAY["
    for row in range(shape[0]):
        dbmatrix += "["
        for col in range(shape[1]):
            dbmatrix += "%f" %(float(matrix[row][col]))
            if (col + 1) < shape[1]:
                dbmatrix += ","
        dbmatrix += "]"
        if (row + 1) < shape[0]:
                dbmatrix += ","
    dbmatrix += "]"
    return(dbmatrix)


##
def rotate_pqr(coord,rad=0):
    matrix = np.array([[cos(rad) ,sin(rad),0],
                       [-sin(rad),cos(rad),0],
                       [0        ,0       ,1]])
    return(np.inner(matrix,coord))

##
def rotate_pqr2etrf(coord, matrix):
    return(np.inner(matrix,coord))

    
##    
if __name__ == "__main__":
    ideltas = np.zeros((16,3))    

    deltas = np.array([[-1.875,  1.875, 0.0],
                       [-0.625,  1.875, 0.0],
                       [ 0.625,  1.875, 0.0],
                       [ 1.875,  1.875, 0.0],
                       [-1.875,  0.625, 0.0],
                       [-0.625,  0.625, 0.0],
                       [ 0.625,  0.625, 0.0],
                       [ 1.875,  0.625, 0.0],
                       [-1.875, -0.625, 0.0],
                       [-0.625, -0.625, 0.0],
                       [ 0.625, -0.625, 0.0],
                       [ 1.875, -0.625, 0.0],
                       [-1.875, -1.875, 0.0],
                       [-0.625, -1.875, 0.0],
                       [ 0.625, -1.875, 0.0],
                       [ 1.875, -1.875, 0.0]], float)

    for anttype in ('HBA','HBA0','HBA1'):
        print anttype
        for station in getStations(anttype):
            print station[0]
            rad = getRotation(station,anttype)
            matrix = getRotationMatrix(station,anttype)
            inr = 0
            for d in deltas:
                pqr = rotate_pqr(d,rad) 
                etrf = rotate_pqr2etrf(pqr,matrix)
                ideltas[inr] = etrf
                inr += 1
            matrix = makeDBmatrix(ideltas)
            db2.query("select * from add_hba_deltas('%s','%s',%s)" %(station[0], anttype, matrix))
