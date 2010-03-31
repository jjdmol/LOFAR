#!/usr/bin/python
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
from sys import *
import os
from datetime import *
from math import *
import time
import re

# import 3rd party modules
from IPython.kernel import client
import numpy
from pylab import *
import scipy.optimize

# import user modules
#from files import *
from acalc import *
import sphere
import lofar.parmdb
from mpfit import *
from error import *
import readms
import io

import parmdbmain

###############################################################################

class IonosphericModel:
   """IonosphericModel class is the main interface to the functions impelmented in the ionosphere module"""

   def __init__( self, gdsfiles, clusterdesc = '', sky_name = 'sky', instrument_name = 'instrument', 
         sources = [], stations = [], DirectionalGainEnable = True, estimate_gradient = True, 
         remove_gradient = True, equal_weights = False, normalize_weights = True, save_pierce_points = True,
         movie_file_name = '', format = 'mpg', plot_gradient = True, xy_range = [ - 0.5, 0.5 ],
         e_steps = 4, print_info = False, estimate_offsets = False,
         include_airmass = True, solution_version = 0 ):

      """
      gdsfiles can be either a list of gdsfiles or a single string that will be interpreted as the name
         of a file that was created by the save method
      clusterdesc is the name of the cluster description file
      sky_name 
      instrument_name
      sources
      stations
      
      """
      
      # Check whether to open a list of parmdbs or a previously stored IonosphericModel object
      if gdsfiles.__class__ == list :
         self.DirectionalGainEnable = DirectionalGainEnable
         self.load_gds(gdsfiles, clusterdesc, sky_name, instrument_name, stations, sources)
      else:
         self.load( gdsfiles )

   def load_gds( self, gdsfiles, clusterdesc, sky_name, instrument_name, stations, sources ):

      self.gdsfiles = gdsfiles
      self.instrument_name = instrument_name
      
      gdsfile = gdsfiles[0]
      instrumentdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + instrument_name
      skydbname = os.path.splitext(gdsfile)[0] + os.path.extsep + sky_name
      instrumentdb = lofar.parmdb.parmdb( instrumentdbname )
      skydb = lofar.parmdb.parmdb( skydbname )

      if len( stations ) == 0 :
         stations = ["*"]
      self.stations = get_station_list( instrumentdb, stations, self.DirectionalGainEnable )
      
      if self.DirectionalGainEnable:
         if len( sources ) == 0:
            sources = ["*"]
         self.sources = get_source_list( instrumentdb, sources )
      else:
         self.sources = []

      
      self.source_positions = []
      for source in self.sources :
         RA = skydb.getDefValues( 'Ra:' + source )['Ra:' + source][0][0]
         dec = skydb.getDefValues( 'Dec:' + source )['Dec:' + source][0][0]
         self.source_positions.append([RA, dec])

      (station_list, positions, pointing) = readms.readms(gdsfiles[0], clusterdesc)
      self.station_positions = [positions[station_list.index(station_name)] for station_name in self.stations]
      self.pointing = numpy.array(pointing[0][0])

      phase_list = []
      amplitude_list = []
      freqs_list = []
      flags_list = []
      firstrun = True
      for gdsfile in gdsfiles : 
         if firstrun:
            firstrun = False
         else:
            # First give lofar.parmdb time to close processes on remote hosts an release sockets
            time.sleep(5)
            instrumentdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + instrument_name
            skydbname = os.path.splitext(gdsfile)[0] + os.path.extsep + sky_name
            instrumentdb = lofar.parmdb.parmdb( instrumentdbname )
            skydb = lofar.parmdb.parmdb( skydbname )
         try:
            flags_list.append( instrumentdb.getValuesGrid('flags')['flags']['values'] )
         except KeyError:
            pass
            
         v1 = []
         amplitude1 = []
         if self.DirectionalGainEnable :
            source_pattern_list = [':' + source for source in self.sources]
            parmbasename = 'DirectionalGain'
         else :
            source_pattern_list = ['']
            parmbasename = 'Gain'
         for pol in range(2):
            v2 = []
            amplitude2 = []
            for source_pattern in source_pattern_list :
               v3 = []
               amplitude3 = []
               for station in self.stations:
                  parmname = parmbasename + ':' + str(pol) + ':' + str(pol) + ':Real:' + station + source_pattern
                  parm_real = instrumentdb.getValuesGrid( parmname )[ parmname ]
                  parmname = parmbasename + ':' + str(pol) + ':' + str(pol) + ':Imag:' + station + source_pattern
                  parm_imag = instrumentdb.getValuesGrid( parmname )[ parmname ]
                  v2.append(numpy.arctan2(parm_imag['values'], parm_real['values']))
                  amplitude2.append(numpy.sqrt(parm_imag['values']**2 + parm_real['values']**2))
            v1.append(v2)
            amplitude1.append(amplitude2)
         phase_list.append(numpy.array(v1).T)
         amplitude_list.append(numpy.array(amplitude1).T)
         freqs_list.append(numpy.array([parm_real['freqs']]).T)
         times = numpy.array([parm_real['times']]).T
         timewidths = numpy.array([parm_real['timewidths']]).T
         del instrumentdb
         del skydb
      self.phases = numpy.concatenate( phase_list )
      self.amplitudes = numpy.concatenate( amplitude_list )
      self.freqs = numpy.concatenate( freqs_list )
      self.times = times
      self.timewidths = timewidths
      self.time_count = len( self.times )

      if len(flags_list) > 0:
         self.flags = numpy.concatenate(flags_list, axis=1).T
      else:
         self.flags = numpy.zeros(self.phases.shape[0:2])
      self.array_center = array( self.station_positions ).mean(axis=0).tolist()
         
   def save( self, outfile ) :
      """
         Save IonModel object to a file named outfile
      """
      
      #attributes = ['phases', 'amplitudes', 'DirectionalGainEnable', 'pointing', 'stations', 'station_positions', 'ClockEnable',
       #'times', 'sources', 'flags', 'time_count', 'source_positions', 'freqs', 'array_center']
      #numpy.savez( outfile, **dict( [ (attribute, self.__dict__[attribute]) for attribute in attributes ] ) )
      io.savez( outfile, **self.__dict__ )
      
   def load( self, infile ) :
      """
      Load IonModel from file created previously by the save method
      """
      data = io.load( infile )
      self.__dict__.update( [(varname, data[varname]) for varname in data.files] )
      
   def calculate_piercepoints(self, time_steps = [], height = 200.e3):
      if ( len( time_steps ) == 0 ):
         n_list = range( self.times.shape[0] )
      else:
         n_list = time_steps
      self.n_list = n_list
      self.height = height
      self.piercepoints_list = [ PiercePoints( self.times[ n ], self.pointing, self.array_center, self.source_positions, self.station_positions, height = self.height ) for n in n_list ]
   
   def calculate_basevectors(self, order = 15, beta = 5. / 3., r_0 = 1.):
      self.order = order
      self.beta = beta
      self.r_0 = r_0
      p_count = self.antenna_count * self.source_count
      self.U_table_list = []
      for piercepoints in self.piercepoints_list:
         Xp_table = reshape(piercepoints.positions, (p_count, 2) )
         
         # calculate structure matrix
         D_table = resize( Xp_table, ( p_count, p_count, 2 ) )
         D_table = transpose( D_table, ( 1, 0, 2 ) ) - D_table
         D_table = add.reduce( D_table**2, 2 )
         D_table = ( D_table / ( r_0**2 ) )**( beta / 2. )

         # calculate covariance matrix C
         # calculate partial product for interpolation B
         # reforce symmetry
         C_table = - D_table / 2.
         C_table = transpose( C_table - ( add.reduce( C_table, 0 ) / float( p_count ) ) )
         B_table = add.reduce( C_table, 0 ) / float( p_count )
         C_table = C_table - B_table
         C_table = ( C_table + transpose( C_table ) ) / 2.

         # eigenvalue decomposition
         # reforce symmetry
         # select subset of base vectors
         [ U_table, S, Ut_table ] = linalg.svd( C_table )
         U_table = ( U_table + transpose( Ut_table ) ) / 2.
   #     Ut_table = transpose( U_table )
         U_table = U_table[ :, 0 : order ]
         S = S[ 0 : order ]
         Si = 1. / S
         self.U_table_list.append(U_table) 

   def ClockTEC( self, ClockEnable = True, Nodes = [] ) : 
      """
      Estimate Clock and ionospheric TEC from phase information
      """

      N_stations = len(self.stations)
      N_baselines = N_stations * (N_stations - 1) / 2
      N_times = len(self.times)
      #N_times = 1000
      print self.stations
      self.Clock = zeros((2, N_times, N_stations))
      self.TEC = zeros((2, N_times, N_stations))

      for pol in range(1):
         if ClockEnable :
            (Clock, TEC) = fit_ClockTEC( squeeze( self.phases[:,0:N_times,:,pol] ), self.freqs[:], self.flags[:, 0:N_times], Nodes )
            self.Clock[ pol, :, 1:] = Clock
            self.TEC[ pol, :, 1:] = TEC
         else : 
            TEC = fit_TEC( squeeze( self.phases[:,:,:,pol] ), self.freqs[:], self.flags[:, :] )
            self.TEC[ pol, :, 1:] = TEC
            
   def write_ClockTEC_to_parmdb( self, parmdbname ) : 
      """
      Write Clock and TEC to a parmdb
      """
      parms = {}
      for pol in range(2):
         n_station = 0
         for station in self.stations:
            parm = {}
            parm[ 'freqs' ] = numpy.array( [ 1.0 ] )
            parm[ 'freqwidths' ] = numpy.array( [ 1.0 ] )
            parm[ 'times' ] = self.times.ravel()
            parm[ 'timewidths' ] = self.timewidths.ravel()
            
            Clock_parm = parm.copy()
            parmname = ':'.join(['Clock', str(pol), station])
            Clock_parm[ 'values' ] = self.Clock[pol, :, n_station]
            parms[ parmname ] = Clock_parm
            
            TEC_parm = parm.copy()
            parmname = ':'.join(['TEC', str(pol), station])
            TEC_parm[ 'values' ] = self.TEC[pol, :, n_station]
            parms[ parmname ] = TEC_parm
            
            n_station += 1
            
      parmdbmain.store_parms( parmdbname, parms, create_new = True)
      
   def write_phases_to_parmdb( self, ClockTEC_parmdbname, phases_name ) :
      for gdsfile in self.gdsfiles :
         instrument_parmdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + str(self.instrument_name)
         phases_parmdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + phases_name
         os.system( "rundist -wd %s write_phases_to_parmdb.py %s %s %s" % (os.environ['PWD'], instrument_parmdbname, ClockTEC_parmdbname, phases_name) )

         p = re.compile('(^Part\\d*.FileName\\s*=\\s*\\S*)(%s$)' % str(self.instrument_name))
         print repr(instrument_parmdbname)
         file_instrument_parmdb = open(instrument_parmdbname)
         file_phases_parmdb = open(phases_parmdbname, 'w')
         file_phases_parmdb.writelines([p.sub('\\1%s' % phases_name, l) for l in file_instrument_parmdb.readlines()])
         file_instrument_parmdb.close()
         file_phases_parmdb.close()


def fit_TEC( phase, freqs, flags ):
   
   def costfunction(p, A, y, flags = 0) : 
      N_stations = y.shape[1]
      TEC = numpy.concatenate( ( numpy.zeros(1), p ) )
      e = []
      for station0 in range(0, N_stations):
         for station1 in range(station0 + 1, N_stations):
            p1 = TEC[station1] - TEC[station0]
            dphase = y[:, [station1]] - y[:, [station0]]
            e.append( (mod( numpy.dot(A, p1) - dphase + pi, 2*pi ) - pi) )
      e = numpy.concatenate(e, axis=0)
      e = e[:,0] * (1-flags)
      return e
  
   A1 = numpy.concatenate([8e9/freqs, 2*pi*numpy.ones(freqs.shape)], axis=1)
   S1 = numpy.dot(numpy.linalg.inv(numpy.dot(A1.T, A1)), A1.T)

   A2 = numpy.concatenate([8e9/freqs], axis=1)
   S2 = numpy.dot(numpy.linalg.inv(numpy.dot(A2.T, A2)), A2.T)

   p22 = []
   residual_std1 = []
   
   N_stations = phase.shape[2]
   
   rr = []
   for i in range(0,phase.shape[1]):
      print i
      A = []
      dphase = []
      flags1 = []
      not_flagged_idx = find(1-flags[:,i])
      for station0 in range(0, N_stations):
         for station1 in range(station0 + 1, N_stations):
            v = zeros(N_stations)
            v[station1] = 1
            v[station0] = -1
            A.append(v)
            dphase1 = zeros(phase.shape[0])
            dphase1[not_flagged_idx] = unwrap(phase[not_flagged_idx, i, station1] - phase[not_flagged_idx, i, station0])
            dphase.append(dphase1)
            flags1.append(flags[:,i])
      A = numpy.array(A)
      dphase = numpy.concatenate(dphase)
      flags1 = numpy.concatenate(flags1)
      
      A3 = kron(A[:,1:], A1)
      S3 = numpy.dot(numpy.linalg.inv(numpy.dot(A3.T, A3)), A3.T)
      p =  numpy.dot(S3, dphase)
      
      p[0::2] = 0
      p[1::2] = numpy.round(p[1::2])
      dphase = dphase - numpy.dot(A3, p)
      
      A4 = kron(A[:,1:], A2)
      S4 = numpy.dot(numpy.linalg.inv(numpy.dot(A4.T, A4)), A4.T)
      p = numpy.dot(S4, dphase)
      
      p, returncode = scipy.optimize.leastsq(costfunction, p, (A2, numpy.squeeze( phase[:, i, :] ), flags1))
      
      while True:
         dphase_fit = numpy.dot(A4, p)
         residual = (mod(dphase - dphase_fit + pi,2*pi) - pi)
         residual_std = sqrt(mean(residual[flags1==0]**2))
         new_outlier_idx = find( (abs(residual) > (3*residual_std)) & (flags1 == 0))
         if len(new_outlier_idx) == 0:
            break
         flags1[new_outlier_idx] = 1
         p, returncode = scipy.optimize.leastsq(costfunction, p, (A2, numpy.squeeze( phase[:, i, :] ), flags1))
         p_init = p
      rr.append(residual_std)
      p22.append(p.copy())      
   
   rr = numpy.array(rr)
   p22 = numpy.array(p22)

   TEC = p22
   
   return TEC

####################################################################

def fit_ClockTEC1( phase, freqs, flags ):
   
   def costfunction(p, A, y, flags = 0) : 
      N_stations = y.shape[1]
      Clock = numpy.concatenate( ( numpy.zeros(1), p[0::2] ) )
      TEC = numpy.concatenate( ( numpy.zeros(1), p[1::2] ) )
      e = []
      for station0 in range(0, N_stations):
         for station1 in range(station0 + 1, N_stations):
            dClock = Clock[station1] - Clock[station0]
            dTEC = TEC[station1] - TEC[station0]
            p1 = numpy.array( [ dClock, dTEC ] )
            dphase = y[:, station1] - y[:, station0]
            e.append( (numpy.mod( numpy.dot(A, p1) - dphase + numpy.pi, 2*numpy.pi ) - numpy.pi) )
      e = numpy.concatenate(e) * (1-flags)
      return e
   
   N_stations = phase.shape[1]
   p_init = numpy.zeros( 2*N_stations -2 )
   A = []
   dphase = []
   flags1 = []
   not_flagged_idx, = numpy.nonzero(1-flags)
   for station0 in range(0, N_stations):
      for station1 in range(station0 + 1, N_stations):
         v = numpy.zeros(N_stations)
         v[ station1 ] = 1
         v[ station0 ] = -1
         A.append(v)
         dphase1 = numpy.zeros(phase.shape[0])
         dphase1[not_flagged_idx] = numpy.unwrap(phase[not_flagged_idx, station1] - phase[not_flagged_idx, station0])
         dphase.append(dphase1)
         flags1.append(flags)
   A = numpy.array(A)
   dphase = numpy.concatenate(dphase)
   flags1 = numpy.concatenate(flags1)
   
   A3 = numpy.kron(A[:,1:], A1)
   S3 = numpy.dot(numpy.linalg.inv(numpy.dot(A3.T, A3)), A3.T)
   p =  numpy.dot(S3, dphase)
   
   p[0::3] = 0
   p[1::3] = 0
   p[2::3] = numpy.round(p[2::3])
   dphase = dphase - numpy.dot(A3, p)
   
   
   A4 = numpy.kron(A[:,1:], A2)
   S4 = numpy.dot(numpy.linalg.inv(numpy.dot(A4.T, A4)), A4.T)
   p = numpy.dot(S4, dphase)
   p = p - numpy.kron(numpy.round(p[1::2] / dp[1]), dp)
      
   p, returncode = scipy.optimize.leastsq(costfunction, p, (A2, phase, flags1))
   p_init = p
   while True:
      dphase_fit = numpy.dot(A4, p)
      residual = numpy.mod(dphase - dphase_fit + numpy.pi,2*numpy.pi) - numpy.pi
      residual_std = numpy.sqrt( numpy.mean ( residual[flags1==0]**2 ) )
      new_outlier_idx, = numpy.nonzero( (numpy.abs(residual) > (3*residual_std)) & (flags1 == 0))
      if len( new_outlier_idx ) == 0:
         break
      flags1[new_outlier_idx] = 1
      p, returncode = scipy.optimize.leastsq(costfunction, p_init, (A2, phase, flags1))
      p_init = p
      
   #print p
   #dphase_fit = numpy.dot(A4, p)
   #print "Shape", dphase_fit.shape
   #a = numpy.reshape(dphase_fit, (248,-1), order='F')
   #b = numpy.reshape(numpy.mod(dphase - dphase_fit + numpy.pi, 2*numpy.pi) + dphase_fit - numpy.pi, (248,-1), order='F')
   #plot(a[:,0:14])
   #plot(b[:,0:14], 'x')
   #exit()
   
   return (residual_std, p.copy())



def fit_ClockTEC( phase, freqs, flags, Nodes ):
   
   A1 = numpy.concatenate([freqs*2*pi/1e9,  8e9/freqs, 2*pi*numpy.ones(freqs.shape)], axis=1)
   S1 = numpy.dot(numpy.linalg.inv(numpy.dot(A1.T, A1)), A1.T)

   A2 = numpy.concatenate([freqs*2*pi/1e9,  8e9/freqs], axis=1)
   S2 = numpy.dot(numpy.linalg.inv(numpy.dot(A2.T, A2)), A2.T)

   dp = 2*pi*numpy.dot(S2, ones(phase.shape[0]))

   p22 = []
   residual_std1 = []

   rr = []
   #job_server = pp.Server( ppservers=tuple(Nodes) )
   mec = client.MultiEngineClient()
   tc = client.TaskClient()

   mec.execute('import numpy, scipy.optimize')
   mec.push( { 'A1': A1, 'A2' : A2, 'dp' : dp } )
   mec.push_function( { 'fit_ClockTEC1': fit_ClockTEC1 } )

   taskids = []
   for i in range(0,phase.shape[1]):
      print i+1, '/', phase.shape[1]
      maptask = client.MapTask(fit_ClockTEC1, ( phase[:, i, :], freqs, flags[:,i]))
      taskids.append(tc.run(maptask))
   i = 0
   for taskid in taskids :
      i += 1
      print i, '/', len(taskids)
      (residual_std, p) = tc.get_task_result(taskid, block = True)
      rr.append(residual_std)
      p22.append(p.copy())      

   rr = numpy.array(rr)
   p22 = numpy.array(p22)

   Clock = p22[:,0::2]
   TEC = p22[:,1::2]

   return (Clock, TEC)

###############################################################################

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

def get_source_list( pdb, source_pattern_list ):
   source_list = []
   for pattern in source_pattern_list :
      parmname_list = pdb.getNames( 'DirectionalGain:?:?:*:*:' + pattern )
      source_list.extend(sorted(set([n.split(':')[-1] for n in parmname_list])))
   return  source_list

def get_station_list( pdb, station_pattern_list, DirectionalGainEnable ):
   station_list = []
   for pattern in station_pattern_list :
      parmname_list = pdb.getNames( { True : 'DirectionalGain:?:?:*:'+pattern+':*', False: 'Gain:?:?:*:' + pattern}[DirectionalGainEnable] )
      station_list.extend(sorted(set([n.split(':')[{True : -2, False : -1}[DirectionalGainEnable]] for n in parmname_list])))
   return station_list

def get_time_list( pdb ):
   parameter_names = pdb.getNames()
   time_list = pdb.getValuesGrid(parameter_names[0])[parameter_names[0]]['times']
   return time_list

###############################################################################

class PiercePoints:
   
   def __init__( self, time, pointing, array_center, source_positions, antenna_positions, height = 400.e3 ):
      # source table radecs at observing epoch

      # calculate Earth referenced coordinates of puncture points of array center towards pointing center
      [ center_pxyz, center_pza ] = sphere.calculate_puncture_point( array_center, pointing, time, height = height )
      self.center_p_geo_llh = sphere.xyz_to_geo_llh( center_pxyz )

      # loop over sources
      positions = []
      zenith_angles = []
      
      for k in range( len( source_positions ) ):
         positions1 = []
         zenith_angles1 = []

         # loop over antennas
         for i in range( len( antenna_positions ) ):
            # calculate Earth referenced coordinates of puncture points of antenna towards peeled source
            [ pxyz, pza ] = sphere.calculate_puncture_point( antenna_positions[ i ], source_positions[ k ], time, height = height )
            p_geo_llh = sphere.xyz_to_geo_llh( pxyz )

            # calculate local angular coordinates of antenna puncture point ( x = East, y = North )
            [ separation, angle ] = sphere.calculate_angular_separation( self.center_p_geo_llh[ 0 : 2 ], p_geo_llh[ 0 : 2 ] )
            X = [ separation * sin( angle ), separation * cos( angle ) ]

            # store model fit input data
            positions1.append(X)
            zenith_angles1.append( pza )
         positions.append(positions1)
         zenith_angles.append( zenith_angles1 )
      
      self.positions = array( positions ) 
      self.zenith_angles = array( zenith_angles )

