import lofar.imager
import pyrap.tables


class LofarFTMachine :
  
  def __init__(self, lofarftmachine) :
    self._lofarftmachine = lofarftmachine
  
  def put(self, vb, row, dopsf, ft_type) :
    self._lofarftmachine.put(vb, row, dopsf, ft_type)
  
  def initializeToSky(self, image, weight, vb):
    self._lofarftmachine.initializeToSky(image, weight, vb)
    
  def finalizeToSky(self):
    self._lofarftmachine.finalizeToSky()
    
  def initializeToVis(self):
    self._lofarftmachine.initializeToVis(image, vb)
    
  def finalizeToVis(self):
    self._lofarftmachine.finalizeToVis()
    
  def makeImage(self, ft_type, vis, image, weights):
    if isinstance( vis, lofar.imager.ROVisibilityIterator):
      new_weights = self._lofarftmachine.makeImage_vi(ft_type, vis, image, weights)
    elif isinstance( vis, lofar.imager.VisSet):
      new_weights = self._lofarftmachine.makeImage_vs(ft_type, vis, image, weights)
    weights.resize(new_weights.shape, refcheck=False)
    weights[:] = new_weights[:]

  def getImage(self, weights, normalize):
    (img, new_weights) = self._lofarftmachine.getImage(weights, normalize)
    weights.resize(new_weights.shape, refcheck=False)
    weights[:] = new_weights[:]
    return img
  