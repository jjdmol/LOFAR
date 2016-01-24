import lofar.parameterset
import pyrap.images
import numpy

class MyATerm :
  def __init__(self, parameters) :
    print "\033[94mPython MyATerm constructor\033[0m"
    #print parameters.keys()
    
  def evaluate(self, idStation, freq, reference, normalize):
    print "\033[94m", idStation, freq, reference, normalize, "\033[0m"
    
    result = numpy.zeros((len(freq), 4, self.nx, self.ny), dtype = complex)
    result[:,0,:,:] = 1.0
    result[:,3,:,:] = 1.0

    return result
   
  def setDirection(self, coordinates, shape):
    print "\033[94m", coordinates, shape, "\033[0m"
    self.nx = shape[0]
    self.ny = shape[1]
    

  def setEpoch(self, time):
    print "\033[94m", time, "\033[0m"
    self.time = time
