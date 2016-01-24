import lofar.parameterset
import pyrap.images
import numpy

class ATermImages :
  def __init__(self, parameters) :
    #print "\033[94mPython MyATerm constructor\033[0m"
    #print parameters.keys()
    self.img = pyrap.images.image("beam")
    self.coordsys = self.img.coordinates().dict()
    
  def evaluate(self, idStation, freq, reference, normalize):
    #print "\033[94m", idStation, freq, reference, normalize, "\033[0m"
    
    ntime_in, nant_in, nfreq_in, npol_in, nrealimag_in, nx_in, ny_in = self.subimg.shape()
    
    img1 = self.subimg.subimage(blc=(0,idStation,0,0,0,0,0), trc=(ntime_in-1,idStation,nfreq_in-1,npol_in-1,1,nx_in-1,ny_in-1), dropdegenerate=False)
    
    cs1_dict = self.img.coordinates().dict()

    cs1_dict['tabular3']['pixelvalues'] = numpy.arange(len(freq), dtype=float)
    cs1_dict['tabular3']['worldvalues'] = freq

    cs1_dict['linear5']['crpix'] = numpy.array([0.0])
    cs1_dict['linear5']['crval'] = numpy.array([self.time])

    cs1 = pyrap.images.coordinates.coordinatesystem(cs1_dict)

    img2_shape = [1, 1, 1, npol_in, 2, self.nx, self.ny]
    
    axes = [5,6]
    if nfreq_in > 1:
      axes.insert(0,2)
    if ntime_in>1:
      axes.insert(0,0)

    img2 = img1.regrid(axes=axes, coordsys=cs1, outshape=img2_shape, decimate=0)
    
    d = img2.getdata()
    #d = numpy.ones(img2_shape)
    
    # axes of d are: time, station, freq, polarization, real/imag, dec, ra
    result = d[0, 0, :, :, 0, :,:] + 1j * d[0, 0, :, :, 1, :, :]
    # remaining axes in result are freq, polarization, dec, ra

    return result
   
  def setDirection(self, coordinates, shape):
    #print "\033[94m", coordinates, shape, "\033[0m"
    self.nx = shape[0]
    self.ny = shape[1]
    self.coordsys['direction0'] = coordinates
    

  def setEpoch(self, time):
    #print "\033[94m", time, "\033[0m"
    self.time = time
    ntime_in, nant_in, nfreq_in, npol_in, nrealimag_in, nx_in, ny_in = self.img.shape()
    if ntime_in > 1:
      w = self.img.toworld((0,0,0,0,0,0,0))
      w[0] = time
      t0 = int(self.img.topixel(w)[0])
      if t0 < 0 :
        t0 = 0
      elif t0 >= (ntime_in-1 ) :
        t0 = ntime_in-2
      
      self.subimg = self.img.subimage(blc=(t0,0,0,0,0,0,0), trc=(t0+1,nant_in-1,nfreq_in-1,npol_in-1,1,nx_in-1,ny_in-1), dropdegenerate=False)
    else:
      self.subimg = self.img


