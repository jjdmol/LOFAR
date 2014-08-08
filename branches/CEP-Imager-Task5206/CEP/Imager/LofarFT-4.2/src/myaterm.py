import lofar.parameterset

class MyATerm :
  def __init__(self, parameters) :
    print "\033[94mPython MyATerm constructor\033[0m"
    print parameters.keys()

    
  def evaluate(self, t, dirmap, idStation, freq, reference, normalize):
    print "\033[94m", t, dirmap, idStation, freq, reference, normalize, "\033[0m"
    print "from myaterm..."
   
  def setDirection(self, coordinates, shape):
    print "\033[94m", coordinates, shape, "\033[0m"

  def setEpoch(self, epoch):
    print "\033[94m", epoch, "\033[0m"

