#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2007
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
###############################################################################

# import Python modules
import sys
import os
from datetime import *
from math import *
import time
import re
import atexit

# import 3rd party modules
from IPython.parallel import client
import numpy
from pylab import *
import scipy.optimize

# import user modules
#from files import *
from acalc import *
import sphere
from error import *
import tables


###############################################################################

class IonosphericModel:
   """IonosphericModel class is the main interface to the functions impelmented in the ionosphere module"""

   def __init__( self, ionmodel):

      """
      ionmodel is the hdf5 file
      """

      self.load_globaldb(ionmodel)

   def load_globaldb ( self, ionmodel) :
      self.hdf5 = tables.openFile(ionmodel, 'r+')
      for varname in self.hdf5.root:
         #print varname,"name:",varname.name
         self.__dict__.update( [(varname.name, self.hdf5.getNode(self.hdf5.root, varname.name))] )

      self.stations = self.hdf5.root.stations.cols.name
      self.station_positions = self.hdf5.root.stations.cols.position
      self.array_center = array( self.station_positions ).mean(axis=0).tolist()
      if not 'array_center' in self.hdf5.root:
         a_center=self.hdf5.createArray(self.hdf5.root, 'array_center', self.array_center)
         a_center.flush()
      self.array_center = self.hdf5.root.array_center
      self.N_stations = len(self.stations)
      self.pointing = self.hdf5.root.pointing
      if not 'sources' in self.hdf5.root:
         source_table = self.hdf5.createTable(self.hdf5.root, 'sources', {'name': tables.StringCol(40), 'position':tables.Float64Col(2)})
         row = source_table.row
         row['name'] = "Pointing"
         row['position'] = list(self.pointing)
         row.append()
         source_table.flush()
    
      self.sources = self.hdf5.root.sources[:]['name']
      self.source_positions = self.hdf5.root.sources[:]['position']
      self.N_sources = len(self.sources)
      self.N_piercepoints = self.N_sources * self.N_stations
      


      self.freqs = self.hdf5.root.freqs
      self.polarizations = self.hdf5.root.polarizations
      self.N_pol = len(self.polarizations)

      self.phases = self.hdf5.root.phases
      self.flags = self.hdf5.root.flags

      #      for varname in ['amplitudes', 'Clock', 'TEC', 'TECfit', 'TECfit_white', 'offsets', \
          #                         'rotation','times', 'timewidths', 'piercepoints', 'facets', 'facet_piercepoints', 'n_list', \
      #    'STEC_facets'] :
      #         if varname in self.hdf5.root:
      #            self.__dict__.update( [(varname, self.hdf5.getNode(self.hdf5.root, varname))] )
  
            
      
   def calculate_piercepoints(self, time_steps = [], height = 200.e3):
      if ( len( time_steps ) == 0 ):
         n_list = range( self.times.shape[0] )
      else:
         n_list = time_steps
      self.n_list = n_list
      if 'n_list' in self.hdf5.root: self.hdf5.root.n_list.remove()
      self.hdf5.createArray(self.hdf5.root, 'n_list', self.n_list)

      self.height = height

      if 'piercepoints' in self.hdf5.root: self.hdf5.root.piercepoints.remove()
      description = {'positions':tables.Float64Col((self.N_sources, self.N_stations,2)), \
                     'positions_xyz':tables.Float64Col((self.N_sources, self.N_stations,3)), \
                     'zenith_angles':tables.Float64Col((self.N_sources, self.N_stations))}
      self.piercepoints = self.hdf5.createTable(self.hdf5.root, 'piercepoints', description)
      self.piercepoints.attrs.height = self.height
      piercepoints_row = self.piercepoints.row
      p = ProgressBar(len(n_list), "Calculating piercepoints: ")
      for (n, counter) in zip(n_list, range(len(n_list))):
         p.update(counter)
         piercepoints =  PiercePoints( self.times[ n ], self.pointing, self.array_center, self.source_positions, self.station_positions, height = self.height )
         piercepoints_row['positions'] = piercepoints.positions
         piercepoints_row['positions_xyz'] = piercepoints.positions_xyz
         piercepoints_row['zenith_angles'] = piercepoints.zenith_angles
         piercepoints_row.append()
      self.piercepoints.flush()
      p.finished()
         
   def calculate_basevectors(self, order = 15, beta = 5. / 3., r_0 = 1.):
      self.order = order
      self.beta = beta
      self.r_0 = r_0
      
      N_stations = len(self.stations)
      N_sources = len(self.sources)
      
      N_piercepoints = N_stations * N_sources
      P = eye(N_piercepoints) - ones((N_piercepoints, N_piercepoints)) / N_piercepoints
      
      self.C_list = []
      self.U_list = []
      self.S_list = []
      p = ProgressBar(len(self.piercepoints), "Calculating base vectors: ")
      for (piercepoints, counter) in zip(self.piercepoints, range(len(self.piercepoints))):
         p.update( counter )
         Xp_table = reshape(piercepoints['positions_xyz'], (N_piercepoints, 3) )
         
         # calculate structure matrix
         D = resize( Xp_table, ( N_piercepoints, N_piercepoints, 3 ) )
         D = transpose( D, ( 1, 0, 2 ) ) - D
         D2 = sum( D**2, 2 )
         C = -(D2 / ( r_0**2 ) )**( beta / 2.0 )/2.0
         self.C_list.append(C)

         # calculate covariance matrix C
         # calculate partial product for interpolation B
         # reforce symmetry
         
         C = dot(dot(P, C ), P)

         # eigenvalue decomposition
         # reforce symmetry
         # select subset of base vectors
         [ U, S, V ] = linalg.svd( C )
         U = U[ :, 0 : order ]
         S = S[ 0 : order ]
         self.U_list.append(U) 
         self.S_list.append(S)
      p.finished()

   def fit_model  ( self ) :
      N_stations = len(self.stations)
      N_times = len(self.times)
      N_sources = len(self.sources)
      N_pol = len(self.polarizations)
      G = kron(eye( N_sources ), ( eye( N_stations ) - ones((N_stations, N_stations)) / N_stations))

      if 'TECfit' in self.hdf5.root: self.hdf5.root.TECfit.remove()
      self.TECfit = self.hdf5.createArray(self.hdf5.root, 'TECfit', zeros(self.TEC.shape))
      
      if 'TECfit_white' in self.hdf5.root: self.hdf5.root.TECfit_white.remove()
      self.TECfit_white = self.hdf5.createArray(self.hdf5.root, 'TECfit_white', zeros(self.TEC.shape))
      
      self.offsets = zeros((len(self.n_list),N_pol))
      p = ProgressBar(len(self.n_list), "Fitting phase screen: ")
      for i in range(len(self.n_list)) :
         p.update( i )
         U = self.U_list[i]
         S = self.S_list[i]
         for pol in range(N_pol) :
            TEC = self.TEC[ self.n_list[i], :, :,pol].reshape( (N_sources * N_stations, 1) )
            TECfit = dot(U, dot(inv(dot(U.T, dot(G, U))), dot(U.T, dot(G, TEC))))
            TECfit_white = dot(U, dot(diag(1/S), dot(U.T, TECfit)))
            self.offsets[i,pol] = TECfit[0] - dot(self.C_list[i][0,:], TECfit_white)
            self.TECfit[ i, :, : ,pol] = reshape( TECfit,  (N_sources,N_stations) )
            self.TECfit_white[ i, :, :,pol ] = reshape( TECfit_white,(N_sources,N_stations)  )
      p.finished()      

      self.TECfit_white.attrs.r_0 = self.r_0
      self.TECfit_white.attrs.beta = self.beta
      
      if 'offsets' in self.hdf5.root: self.hdf5.root.offsets.remove()
      self.hdf5.createArray(self.hdf5.root, 'offsets', self.offsets)

   def make_movie( self, extent = 0, npixels = 100, vmin = 0, vmax = 0 ):
      """
      """

      multiengine_furl =  os.environ['HOME'] + '/ipcluster/multiengine.furl'
#      mec = client.MultiEngineClient( multiengine_furl )
      mec = client.MultiEngineClient( )
      task_furl =  os.environ['HOME'] + '/ipcluster/task.furl'
      #tc = client.TaskClient( task_furl )
      tc = client.TaskClient( )
      N_stations = len(self.stations)
      N_times = self.TECfit.shape[1]
      N_sources = len(self.sources)
      N_piercepoints = N_stations * N_sources
      N_pol = len(self.polarizations)
      R = 6378137
      taskids = []
      
      print "Making movie..."
      p = ProgressBar( len( self.n_list ), 'Submitting jobs: ' )
      for i in range(len(self.n_list)) :
         p.update(i)
         Xp_table = reshape(self.piercepoints[i]['positions'], (N_piercepoints, 2) )
         if extent > 0 :
            w = extent/R/2
         else :
            w = 1.1*abs(Xp_table).max()
         for pol in range(N_pol) :
            v = self.TECfit_white[ pol, i, :, : ].reshape((N_piercepoints,1))
            maptask = client.MapTask(calculate_frame, (Xp_table, v, self.beta, self.r_0, npixels, w) )
            taskids.append(tc.run(maptask))
      p.finished()
      
      if vmin == 0 and vmax == 0 :
         vmin = self.TECfit.min()
         vmax = self.TECfit.max()
         vdiff = vmax - vmin
         vmin = vmin - 0.1*vdiff
         vmax = vmax + 0.1*vdiff

      p = ProgressBar( len( self.n_list ), 'Fetch results: ' )
      for i in range(len(self.n_list)) :
         p.update(i)
         clf()
         for pol in range(N_pol) :
            (phi,w) = tc.get_task_result(taskids.pop(0), block = True)
            phi = phi + self.offsets[pol, i]
            subplot(1, N_pol, pol+1)
            w = w*R*1e-3
            h_im = imshow(phi, interpolation = 'nearest', origin = 'lower', extent = (-w, w, -w, w), vmin = vmin, vmax = vmax )
            h_axes = gca()
            cl = h_im.get_clim()
            TEC =  reshape( self.TEC[ pol, i, :, : ], N_piercepoints )
            for j in range(N_piercepoints):
               color = h_im.cmap(int(round((TEC[j]-cl[0])/(cl[1]-cl[0])*(h_im.cmap.N-1))))
               plot(R*1e-3*Xp_table[j,0], R*1e-3*Xp_table[j,1], marker = 'o', markeredgecolor = 'k', markerfacecolor = color)
            colorbar()
            xlim(-w, w)
            ylim(-w, w)
         savefig('tmpfig%4.4i.png' % i)
      p.finished()
      os.system("mencoder mf://tmpfig????.png -o movie.mpeg -mf type=png:fps=3  -ovc lavc -ffourcc DX50 -noskip -oac copy")

   def interpolate( self, facetlistfile ) :
      
      """
      """
      
      #facetdbname = os.path.join(self.globaldb, 'facets')
      #os.system( 'makesourcedb in=%s out=%s append=False' % (facetlistfile, facetdbname) )
      
      #patch_table = pt.table( os.path.join(facetdbname, 'SOURCES', 'PATCHES' ) )
      
      #if 'facets' in self.hdf5.root: self.hdf5.root.facets.remove()
      #description = {'name': tables.StringCol(40), 'position':tables.Float64Col(2)}
      #self.facets = self.hdf5.createTable(self.hdf5.root, 'facets', description)

      #facet = self.facets.row
      #for patch in patch_table :
         #facet['name'] = patch['PATCHNAME']
         #facet['position'] = array([patch['RA'], patch['DEC']])
         #facet.append()
      #self.facets.flush()
      self.N_facets = len(self.facets)
      
      self.facet_names = self.facets[:]['name']
      self.facet_positions = self.facets[:]['position']

      print self.n_list
      if 'STEC_facets' in self.hdf5.root: self.hdf5.root.STEC_facets.remove()
      self.STEC_facets = self.hdf5.createCArray(self.hdf5.root, 'STEC_facets', tables.Float32Atom(), shape = (self.N_pol, self.n_list.shape[0],  self.N_facets, self.N_stations))

      #if 'facet_piercepoints' in self.hdf5.root: self.hdf5.root.facet_piercepoints.remove()
      #description = {'positions':tables.Float64Col((self.N_facets, self.N_stations,2)), \
                     #'positions_xyz':tables.Float64Col((self.N_facets, self.N_stations,3)), \
                     #'zenith_angles':tables.Float64Col((self.N_facets, self.N_stations))}
      #self.facet_piercepoints = self.hdf5.createTable(self.hdf5.root, 'facet_piercepoints', description)
      #height = self.piercepoints.attrs.height
      #facet_piercepoints_row = self.facet_piercepoints.row
      #print "Calculating facet piercepoints..."
      #for n in self.n_list:
         #piercepoints = PiercePoints( self.times[ n ], self.pointing, self.array_center, self.facet_positions, self.station_positions, height = height )
         #facet_piercepoints_row['positions'] = piercepoints.positions
         #facet_piercepoints_row['positions_xyz'] = piercepoints.positions_xyz
         #facet_piercepoints_row['zenith_angles'] = piercepoints.zenith_angles
         #facet_piercepoints_row.append()
      #self.facet_piercepoints.flush()

      r_0 = self.TECfit_white.attrs.r_0
      beta = self.TECfit_white.attrs.beta
      
      for facet_idx in range(self.N_facets) :
         for station_idx in range(self.N_stations):
            for pol_idx in range(self.N_pol) :
               TEC_list = []
               for n in range(len(self.n_list)):
                  p = self.facet_piercepoints[n]['positions_xyz'][facet_idx, station_idx,:]
                  za = self.facet_piercepoints[n]['zenith_angles'][facet_idx, station_idx]
                  Xp_table = reshape(self.piercepoints[n]['positions_xyz'], (self.N_piercepoints, 3) )
                  v = self.TECfit_white[ pol_idx, n, :, : ].reshape((self.N_piercepoints,1))
                  D2 = sum((Xp_table - p)**2,1)
                  C = (D2 / ( r_0**2 ) )**( beta / 2. ) / -2.
                  self.STEC_facets[pol_idx, n,  facet_idx, station_idx] = dot(C, v)/cos(za)

def product(*args, **kwds):
    # product('ABCD', 'xy') --> Ax Ay Bx By Cx Cy Dx Dy
    # product(range(2), repeat=3) --> 000 001 010 011 100 101 110 111
    pools = map(tuple, args) * kwds.get('repeat', 1)
    result = [[]]
    for pool in pools:
        result = [x+[y] for x in result for y in pool]
    for prod in result:
        yield tuple(prod)

def fillarray( a, v ) :
   print a.shape, a.chunkshape
   for idx in product(*[xrange(0, s, c) for s, c in zip(a.shape, a.chunkshape)]) :
      s = tuple([slice(i,min(i+c,s)) for i,s,c in zip(idx, a.shape, a.chunkshape)])
      a[s] = v
   


def calculate_frame(Xp_table, v, beta, r_0, npixels, w):
   import numpy
   phi = numpy.zeros((npixels,npixels))
   N_piercepoints = Xp_table.shape[0]
   P = numpy.eye(N_piercepoints) - numpy.ones((N_piercepoints, N_piercepoints)) / N_piercepoints
   for x_idx in range(0, npixels):
      x = -w + 2*x_idx*w/( npixels-1 )  
      for y_idx in range(0, npixels):
         y = -w + 2*y_idx*w/(npixels-1)
         D2 = numpy.sum((Xp_table - numpy.array([ x, y ]))**2,1)
         C = (D2 / ( r_0**2 ) )**( beta / 2. ) / -2.
         phi[y_idx, x_idx] = numpy.dot(C, v)
   return phi, w





def fit_phi_klmap_model( P, U_table = None, pza_table = None, phase_table = None, dojac = None):
# TODO: calculate penalty terms of MAP estimator

   # handle input parameters
   if ( ( U_table == None ) or
        ( pza_table == None ) or 
        ( phase_table == None ) ):
      return - 1, None, None

   (antenna_count, source_count) = phase_table.shape

   # calculate phases at puncture points
   phi_table = dot( U_table, P ) / cos( aradians( pza_table ) )
   phi_table = phi_table.reshape( (antenna_count, source_count) )

   # calculate chi2 terms
   chi_list = []
   for source in range(source_count):
      for i in range(antenna_count):
         for j in range(i, antenna_count):
            chi_list.append( mod( phi_table[i, source] - phi_table[j, source] - phase_table[i, source] + phase_table[j, source] + pi, 2*pi) - pi )

   # make (normalized) chi2 array
   chi_array = array( chi_list )

   return 0, chi_array, None

###############################################################################

# gradient

def phi_gradient_model( X, p ):
  phi = dot( X, p )
  return phi

###############################################################################

def phi_klmap_model( X, Xp_table, B_table, F_table, beta = 5. / 3., r_0 = 1. ):
# B_table = (1/m)(1T)(C_table)(A_table)
# F_table = ( Ci_table )( U_table )( P_table )

  # input check
  if ( len( shape( X ) ) == 1 ):
    X_table = array( [ X ] )
  elif ( len( shape( X ) ) == 2 ):
    X_table = X

  # calculate structure matrix
  x_count = len( X_table )
  p_count = len( Xp_table )
  D_table = transpose( resize( X_table, ( p_count, x_count, 2 ) ), ( 1, 0, 2 ) )
  D_table = D_table - resize( Xp_table, ( x_count, p_count, 2 ) )
  D_table = add.reduce( D_table**2, 2 )
  D_table = ( D_table / ( r_0**2 ) )**( beta / 2. )

  # calculate covariance matrix
  C_table = - D_table / 2.
  C_table = transpose( transpose( C_table ) - ( add.reduce( C_table, 1 ) / float( p_count ) ) )
  C_table = C_table - B_table

  phi = dot( C_table, F_table )
  phi = reshape( phi, ( x_count ) )
  if ( len( phi ) == 1 ):
    phi = phi[ 0 ]

  return phi


###############################################################################

class PiercePoints:
   
   def __init__( self, time, pointing, array_center, source_positions, antenna_positions, height = 400.e3 ):
      # source table radecs at observing epoch

      # calculate Earth referenced coordinates of puncture points of array center towards pointing center
      [ center_pxyz, center_pza ] = sphere.calculate_puncture_point_mevius( array_center, pointing, time, height = height )
      self.center_p_geo_llh = sphere.xyz_to_geo_llh( center_pxyz, time )

      # loop over sources
      positions = []
      positions_xyz = []
      zenith_angles = []
      
      for k in range( len( source_positions ) ):
         positions_xyz1 = []
         positions1 = []
         zenith_angles1 = []

         # loop over antennas
         for i in range( len( antenna_positions ) ):
            # calculate Earth referenced coordinates of puncture points of antenna towards peeled source
            [ pxyz, pza ] = sphere.calculate_puncture_point_mevius( antenna_positions[ i ], source_positions[ k ], time, height = height )
            p_geo_llh = sphere.xyz_to_geo_llh( pxyz, time )

            # calculate local angular coordinates of antenna puncture point ( x = East, y = North )
            [ separation, angle ] = sphere.calculate_angular_separation( self.center_p_geo_llh[ 0 : 2 ], p_geo_llh[ 0 : 2 ] )
            X = [ separation * sin( angle ), separation * cos( angle ) ]

            # store model fit input data
            positions1.append(X)
            positions_xyz1.append( pxyz )
            zenith_angles1.append( pza )
         positions.append(positions1)
         positions_xyz.append(positions_xyz1)
         zenith_angles.append( zenith_angles1 )
      
      self.positions = array( positions ) 
      self.positions_xyz = array( positions_xyz ) 
      self.zenith_angles = array( zenith_angles )


class ProgressBar:
   
   def __init__(self, length, message = ''):
      self.length = length
      self.current = 0
      sys.stdout.write(message + '0%')
      sys.stdout.flush()

   def update(self, value):
      while self.current < 2*int(50*value/self.length):
         self.current += 2
         if self.current % 10 == 0 :
            sys.stdout.write(str(self.current) + '%')
         else:
            sys.stdout.write('.')
         sys.stdout.flush()
            
   def finished(self):
      self.update(self.length)
      sys.stdout.write('\n')
      sys.stdout.flush()
      
