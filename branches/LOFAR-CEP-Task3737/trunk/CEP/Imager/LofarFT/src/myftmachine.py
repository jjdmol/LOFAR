import lofar.imager
from lofar.imager.lofarftmachine import LofarFTMachine
import pyrap.tables

from pylab import *

class MyFTMachine(LofarFTMachine) :
  
  def __init__(self, lofarftmachine) :
    print "\033[94minitializing MyFTMachine\033[0m"
    # If we need to use the C++ methods of the base class LofarFTMachine, 
    # we need to call the constructor of the base class first
    LofarFTMachine.__init__(self, lofarftmachine)
  
  def initializeToSky(self, image, weight, vb):
    print "\033[94mMyFTMachine::initializeToSky\033[0m"
    #show that we have access to the image by printing its shape
    print image.shape()
    self.image = image
    # no python implementation yet, so call the C++ implementation instead
    LofarFTMachine.initializeToSky(self, image, weight, vb)
    
  def finalizeToSky(self):
    print "\033[94mMyFTMachine::finalizeToSky\033[0m"
    LofarFTMachine.finalizeToSky(self)
    
  def initializeToVis(self, image, vb):
    print "\033[94mMyFTMachine::initializeToVis\033[0m"
    LofarFTMachine.initializeToVis(self, image, vb)
    
  def finalizeToVis(self):
    print "\033[94mMyFTMachine::finalizeToVis\033[0m"
    LofarFTMachine.finalizeToVis()

  def put(self, vb, row, dopsf, ft_type) :
    print "\033[94mMyFTMachine::put\033[0m", vb.nRow, row, dopsf
    # show that we have access to the visbuffer 
    print mean(vb.timeCentroid)
    # no python implementation yet, so call the C++ implementation instead
    LofarFTMachine.put(self, vb, row, dopsf, ft_type)
    
  def makeImage(self, ft_type, vis, image, weight):
    print "\033[94mMyFTMachine::makeImage\033[0m"
    if not isinstance( vis, lofar.imager.ROVisibilityIterator):
      LofarFTMachine.makeImage(self, ft_type, vis, image, weight)
      return
    if ft_type == lofar.imager.FTMachineType.OBSERVED :
      print "ft_type = OBSERVED"
    print len(vis.ms)
    # Attach a visbuffer to to visibility iterator
    vb = lofar.imager.VisBuffer(vis)
    if (vb.polFrame==lofar.imager.PolFrame.Linear) :
      lofar.imager.changeCStokesRep(image, lofar.imager.PolRep.LINEAR)
    else :
      lofar.imager.changeCStokesRep(image, lofar.imager.PolRep.CIRCULAR)
    self.initializeToSky(image, weight, vb)
    print vb.nChannel
    # Iterate over the data    
    for i in vis :
      print [format(t, '15f') for t in unique(vb.timeCentroid)]
      self.put(vb, -1, False, ft_type)
    self.finalizeToSky()
    self.getImage(weight, True)
    data = image.getdata()
    imshow(real(squeeze(data[0,0,:,:])))    
    show()
    
  def getImage(self, weights, normalize):
    print "\033[94mMyFTMachine::getImage\033[0m"
    LofarFTMachine.getImage(self, weights, normalize)
    

  