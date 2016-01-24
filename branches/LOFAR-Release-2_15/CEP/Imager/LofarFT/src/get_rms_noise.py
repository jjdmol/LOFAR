# get_rms_noise.py: Python function to get image rms noise per stokes
# Copyright (C) 2012
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
#
# @author Mike Bell

# edited by Adriaan Renting and Ger van Diepen

import numpy as np
import pyrap.images as pim

f = 16

def myfit(x,y, fn):
  # Find the width of a Gaussian distribution by computing the second moment of 
  # the data (y) on a given axis (x)
  
  w = np.sqrt(abs(sum(x**2*y)/sum(y)))
  
  # Or something like this. Probably a Gauss plus a power-law with a lower cutoff is necessary
  #func = lambda x, a, b: np.exp(0.5*x**2/a**2) + x**(-1.*b)	 

  #[popt, pvar] = curve_fit(func, x, y)
  # Need a newer version of scipy for this to work...
  return w


def get_rms_noise (imageName):
  image = pim.image(imageName)
  nfo = image.info()
  d = image.getdata()
  nstokes = d.shape[1]
  nra = d.shape[2]
  ndec = d.shape[3]

#  bmaj = nfo['imageinfo']['restoringbeam']['major']['value']
#  bmin = nfo['imageinfo']['restoringbeam']['minor']['value']
#  barea = 2.*np.pi*bmaj*bmin/(2.3548**2)

  noises = []

  Id = d[0,0, (nra/2 - nra/f):(nra/2 + nra/f)].flatten()
  if nstokes==4:
    Qd = d[0,1, (nra/2 - nra/f):(nra/2 + nra/f)].flatten()
    Ud = d[0,2, (nra/2 - nra/f):(nra/2 + nra/f)].flatten()
    Vd = d[0,3, (nra/2 - nra/f):(nra/2 + nra/f)].flatten()

  hrange = (-1,1)
  Ih = np.histogram(Id, bins=100, range=hrange) # 0 = values, 1 = bin edges
  Ix = Ih[1][:-1] + 0.5*(Ih[1][1] - Ih[1][0])
  Iv = Ih[0]/float(max(Ih[0]))

# stupid fitting method
  Inoise = myfit(Ix, Iv, imageName+'_histI.png')
  noises.append (('I', Inoise))

  if nstokes==4:
    hrange = (-0.1, 0.1)
    Qh = np.histogram(Qd, bins=100,range=hrange) # 0 = values, 1 = left bin edges
    Qx = Qh[1][:-1] + 0.5*(Qh[1][1] - Qh[1][0])
    Qv = Qh[0]/float(max(Qh[0]))
    Uh = np.histogram(Ud, bins=100, range=hrange) # 0 = values, 1 = left bin edges
    Ux = Uh[1][:-1] + 0.5*(Uh[1][1] - Uh[1][0])
    Uv = Uh[0]/float(max(Uh[0]))
    Vh = np.histogram(Vd, bins=100, range=hrange) # 0 = values, 1 = left bin edges
    Vx = Vh[1][:-1] + 0.5*(Vh[1][1] - Vh[1][0])
    Vv = Vh[0]/float(max(Vh[0]))
  
    Qnoise = myfit(Qx, Qv, imageName+'_histQ.png')
    Unoise = myfit(Ux, Uv, imageName+'_histU.png')
    Vnoise = myfit(Vx, Vv, imageName+'_histV.png')
    noises.append (('Q', Qnoise))
    noises.append (('U', Unoise))
    noises.append (('V', Vnoise))

  return noises
