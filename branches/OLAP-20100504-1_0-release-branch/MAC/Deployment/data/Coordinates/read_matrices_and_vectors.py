#!/usr/bin/env python

## read all rotation matrices from lisp files
## and store in rotation_matrices.dat file

import os
import array
import numpy as np
import string

def formatVector(vec):
    fm = ''
    m = vec.replace('(','').replace(')','')
    for n in m.split(','):
        fm += "%8.6f, " %(float(n.strip()))
    fm += 'E'
    fm = fm.replace(', E','')
    return(fm)

def formatMatrix(mat):
    fm = ''
    m = mat.replace('(','').replace(')','')
    for n in m.split(','):
        fm += "%12.10f, " %(float(n.strip()))
    fm += 'E'
    fm = fm.replace(', E','')
    return(fm)

## get vector or matrix from lisp file
def getSet(filename):
    f = open(filename,'r')
    s = f.read()
    f.close()
    op = cp = 0
    mah = ma = ''
    for ch in s:
        if ch == '(': op += 1
        if ch == ')': cp += 1

        if op == 1 and ch != '(': mah += ch
        if op >= 2 and op != cp : ma += ch
        if op > 0 and op == cp:
            if mah.find('normal-vector ') != -1:
                vector = ma
                vector = vector.replace(' ',', ')
                
            if mah.find('station-pqr-to-etrs-matrix') != -1:
                matrix = ma
                matrix = matrix.replace('d','e')
                matrix = matrix.replace('\n',',').replace('  ','')
                matrix = matrix.replace(' ',', ')
            op = cp = 0
            mah = ma = ''
    return(formatVector(vector),formatMatrix(matrix))


    
fmat=open('data/rotation_matrices.dat','w')
fmat.write("#\n# station, type, station-pqr-to-etrs-matrix(9x float)\n#\n")
fvec=open('data/normal_vectors.dat','w')
fvec.write("#\n# station, type, normal-vector(3x float)\n#\n")
for dirname in os.listdir('vectors-and-matrices/'):
    
    if os.path.isdir('vectors-and-matrices/'+dirname) == False:
        continue
    lba = hba = hba0 = hba1 = 0
    strlba_m = strlba_v = ''
    strhba_m = strhba_v = ''
    strhba0_m = strhba0_v = ''
    strhba1_m = strhba1_v = ''
    
    for rf in os.listdir('vectors-and-matrices/'+dirname):
        print 'Reading %s and extract normal-vector and rotation-matrix' %(rf)
        filename = 'vectors-and-matrices/'+dirname+'/'+rf
        vector, matrix = getSet(filename)
        
        stationtype = rf[0:2]

        if rf.find('lba') > -1:
            lba = 1
            strlba_m = "%s, lba , %s\n" %(rf[0:5],matrix)
            strlba_v = "%s, lba , %s\n" %(rf[0:5],vector)
        elif rf.find('hba0') > -1:
            hba0 = 1
            strhba0_m = "%s, hba0, %s\n" %(rf[0:5],matrix)
            strhba0_v = "%s, hba0, %s\n" %(rf[0:5],vector)
        elif rf.find('hba1') > -1:
            hba1 = 1
            strhba1_m = "%s, hba1, %s\n" %(rf[0:5],matrix)
            strhba1_v = "%s, hba1, %s\n" %(rf[0:5],vector)
        elif rf.find('hba') > -1:
            hba = 1
            strhba_m = "%s, hba , %s\n" %(rf[0:5],matrix)
            strhba_v = "%s, hba , %s\n" %(rf[0:5],vector)
        else:
            strlba_m = "%s, lba , %s\n" %(rf[0:5],matrix)
            strlba_v = "%s, lba , %s\n" %(rf[0:5],vector)
            strhba_m = "%s, hba , %s\n" %(rf[0:5],matrix)
            strhba_v = "%s, hba , %s\n" %(rf[0:5],vector)
            strhba0_m = "%s, hba0, %s\n" %(rf[0:5],matrix)
            strhba0_v = "%s, hba0, %s\n" %(rf[0:5],vector)
            strhba1_m = "%s, hba1, %s\n" %(rf[0:5],matrix)
            strhba1_v = "%s, hba1, %s\n" %(rf[0:5],vector)

    ## write vectors and matrices to file 
    if string.upper(stationtype) == 'CS':
        fmat.write(strlba_m)
        fmat.write(strhba0_m)
        fmat.write(strhba1_m)
        fvec.write(strlba_v)
        fvec.write(strhba0_v)
        fvec.write(strhba1_v)
    else:
        fmat.write(strlba_m)
        fmat.write(strhba_m)
        fvec.write(strlba_v)
        fvec.write(strhba_v)

fmat.close()
fvec.close()
