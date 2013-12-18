#!/usr/bin/env python

## read all normal_vectors and rotation_matrices from lisp files and
## store in normal_vector.dat and rotation_matrices.dat file 

import os
import array
import numpy as np
import string

def formatVector(vec):
    fm = ''
    if len(vec) != 3:
        return (fm)
    for n in vec:
        fm += "%8.6f, " %(float(n))
    return(fm[:-2])

def formatMatrix(mat):
    fm = ''
    if len(mat) != 9:
        return (fm)
    for n in mat:
        fm += "%12.10f, " %(float(n))
    return(fm[:-2])

## get vector or matrix from lisp file
def getSet(filename):
    f = open(filename,'r')
    s = f.read()
    f.close()
    # op = number of open paranthesis
    # cp = number of closed paranthesis
    op = cp = 0
    # mah = matrix header
    # ma  = matrix
    mah = ma = ''
    vector = matrix = []
    for ch in s:
        if ch == '(': op += 1
        if ch == ')': cp += 1

        if op == 1 and ch != '(': mah += ch
        if op >= 2 and op != cp : ma += ch
        if op > 0 and op == cp:
            if mah.split()[0].strip() == 'normal-vector':
                vector = ma.replace('d','e').replace('(',' ').replace(')',' ')
                vector = vector.split()
            
            elif mah.split()[0].strip() == 'station-pqr-to-etrs-matrix':    
                matrix = ma.replace('d','e').replace('(',' ').replace(')',' ')
                matrix = matrix.split()
            
            # clear all and look for next
            op = cp = 0
            mah = ma = ''
    return(formatVector(vector),formatMatrix(matrix))


def main():    
    fmat = open('data/rotation_matrices.dat','w')
    fmat.write("#\n# station, type, station-pqr-to-etrs-matrix(9x float)\n#\n")
    fvec = open('data/normal_vectors.dat','w')
    fvec.write("#\n# station, type, normal-vector(3x float)\n#\n")
    
    for dirname in os.listdir('vectors-and-matrices/'):
        if str(dirname).find('svn') != -1: # skip svn dir
            continue
        if os.path.isdir('vectors-and-matrices/'+dirname) == False:
            continue
        lba = hba = hba0 = hba1 = 0
        strlba_m = strlba_v = ''
        strhba_m = strhba_v = ''
        strhba0_m = strhba0_v = ''
        strhba1_m = strhba1_v = ''
        
        for rf in os.listdir('vectors-and-matrices/'+dirname):
            if rf[-4:] != 'lisp': # check if lisp file
                continue
                
            #print 'Reading %s and extract normal-vector and rotation-matrix' %(rf)
            filename = 'vectors-and-matrices/'+dirname+'/'+rf
            vector, matrix = getSet(filename)
            if vector == '' or matrix == '':
                print "Error, wrong vector or matrix format in: ", filename
                continue
                 
            stationtype = rf[0:2]
    
            if rf.find('lba') > -1:
                if lba:
                    print "Warning, lba file exists %d times" %(lba+1)
                lba += 1
                strlba_m = "%s, lba , %s\n" %(rf[0:5],matrix)
                strlba_v = "%s, lba , %s\n" %(rf[0:5],vector)
            elif rf.find('hba0') > -1:
                if hba0:
                    print "Warning, hba0 file exists %d times" %(hba0+1)
                hba0 += 1
                strhba0_m = "%s, hba0, %s\n" %(rf[0:5],matrix)
                strhba0_v = "%s, hba0, %s\n" %(rf[0:5],vector)
            elif rf.find('hba1') > -1:
                if hba1:
                    print "Warning, hba1 file exists %d times" %(hba1+1)
                hba1 += 1
                strhba1_m = "%s, hba1, %s\n" %(rf[0:5],matrix)
                strhba1_v = "%s, hba1, %s\n" %(rf[0:5],vector)
            elif rf.find('hba') > -1:
                if hba:
                    print "Warning, hba file exists %d times" %(hba+1)
                hba += 1
                strhba_m = "%s, hba , %s\n" %(rf[0:5],matrix)
                strhba_v = "%s, hba , %s\n" %(rf[0:5],vector)
            else:
                if lba or hba0 or hba1 or hba:
                    print "Warning, double lisp files for same antenna" 
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


if __name__ == '__main__':
    main()
