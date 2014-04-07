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
import sys
import os
from datetime import *
from math import *
import time
import re
import atexit

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
import lofar.parameterset
import pyrap.tables as pt
from mpfit import *
from error import *
import readms
import io

import parmdbmain
import tables


###############################################################################


# Without the following statement python sometimes throws an exception on exit
atexit.register(client.rit.stop)

class IonosphericModel:
   """IonosphericModel class is the main interface to the functions impelmented in the ionosphere module"""

   def __init__( self, gdsfiles, clusterdesc = '', sky_name = 'sky', instrument_name = 'instrument', 
         sources = [], stations = [], polarizations = [], GainEnable = False, DirectionalGainEnable = False,
         PhasorsEnable = False, estimate_gradient = True,
         remove_gradient = True, equal_weights = False, normalize_weights = True, save_pierce_points = True,
         globaldb = 'globaldb', movie_file_name = '', format = 'mpg', plot_gradient = True, xy_range = [ - 0.5, 0.5 ],
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
      self.DirectionalGainEnable = DirectionalGainEnable
      
      if len(gdsfiles) == 1 and os.path.isdir( gdsfiles[0] ):
         self.load_globaldb( gdsfiles[0] )
      else:
         self.GainEnable = GainEnable
         self.DirectionalGainEnable = DirectionalGainEnable
         self.PhasorsEnable = PhasorsEnable
         self.polarizations = polarizations
         self.N_pol = len(polarizations)
         self.load_gds(gdsfiles, clusterdesc, globaldb, sky_name, instrument_name, stations, sources)

   def load_globaldb ( self, globaldb ) :
      self.globaldb = globaldb
      self.hdf5 = tables.openFile(os.path.join(globaldb , 'ionmodel.hdf5'), 'r+')

      self.stations = self.hdf5.root.stations.cols.name
      self.station_positions = self.hdf5.root.stations.cols.position
      self.array_center = self.hdf5.root.array_center
      self.N_stations = len(self.stations)
      
      
      #ionospheredbname = os.path.join(globaldb, 'ionosphere')
      #ionospheredb = lofar.parmdb.parmdb( ionospheredbname )
      
      #self.stations = get_station_list_from_ionospheredb( ionospheredb )

      #antenna_table = pt.table( globaldb + "/ANTENNA")
      #name_col = antenna_table.getcol('NAME')
      #position_col = antenna_table.getcol( 'POSITION' )
      #self.station_positions = [position_col[name_col.index(station_name)] for station_name in self.stations]
      #antenna_table.close()

      #field_table = pt.table( globaldb + "/FIELD")
      #phase_dir_col = field_table.getcol('PHASE_DIR')
      #self.pointing = phase_dir_col[0,0,:]
      #field_table.close()

      #self.sources = get_source_list_from_ionospheredb( ionospheredb )      # source positions

      self.sources = self.hdf5.root.sources[:]['name']
      self.source_positions = self.hdf5.root.sources[:]['position']
      self.N_sources = len(self.sources)
      self.N_piercepoints = self.N_sources * self.N_stations
      
      self.pointing = self.hdf5.root.pointing

      self.freqs = self.hdf5.root.freqs
      self.polarizations = self.hdf5.root.polarizations
      self.N_pol = len(self.polarizations)

      self.phases = self.hdf5.root.phases
      self.flags = self.hdf5.root.flags

      for varname in ['amplitudes', 'Clock', 'TEC', 'TECfit', 'TECfit_white', 'offsets', \
                      'times', 'timewidths', 'piercepoints', 'facets', 'facet_piercepoints', 'n_list', \
                      'STEC_facets'] :
         if varname in self.hdf5.root:
            self.__dict__.update( [(varname, self.hdf5.getNode(self.hdf5.root, varname))] )
            
      #if 'Clock' in self.hdf5.root : self.Clock = self.hdf5.root.Clock
      #if 'TEC' in self.hdf5.root: self.TEC = self.hdf5.root.TEC
      #if 'TECfit' in self.hdf5.root: self.TEC = self.hdf5.root.TECfit
      #if 'TECfit_white' in self.hdf5.root: self.TEC = self.hdf5.root.TECfit_white
      #if 'offsets' in self.hdf5.root: self.TEC = self.hdf5.root.offsets
      #if 'piercepoints' in self.hdf5.root: self.TEC = self.hdf5.root.piercepoints
         
      self.N_stations = len(self.stations)
      self.N_sources = len(self.sources)

   def load_gds( self, gdsfiles, clusterdesc, globaldb, sky_name, instrument_name, stations, sources ):

      self.gdsfiles = gdsfiles
      self.instrument_name = instrument_name
      self.globaldb = globaldb

      try:
         os.mkdir(globaldb)
      except OSError:
         pass

      self.instrumentdb_name_list = []
      for gdsfile in gdsfiles:
         instrumentdb_name = os.path.splitext(gdsfile)[0] + os.path.extsep + instrument_name
         if not os.path.exists(instrumentdb_name):
            instrumentdb_name = make_instrumentdb( gdsfile, instrument_name, globaldb )
         self.instrumentdb_name_list.append(instrumentdb_name)
         
      gdsfile = gdsfiles[0]
      instrumentdb = lofar.parmdb.parmdb( self.instrumentdb_name_list[0] )
      
      self.hdf5 = tables.openFile(os.path.join(globaldb , 'ionmodel.hdf5'), 'w')

      gdsparset = lofar.parameterset.parameterset( gdsfile )
      
      skydbfilename = os.path.join(gdsparset.getString( "Part0.FileName" ), sky_name)
      skydbhostname = gdsparset.getString( "Part0.FileSys" ).split(':')[0]
      os.system( "scp -r %s:%s %s/sky" % ( skydbhostname, skydbfilename, globaldb ) )
      skydbname = globaldb + "/sky"
      skydb = lofar.parmdb.parmdb( skydbname )

      gdsparset = lofar.parameterset.parameterset( gdsfile )
      msname = gdsparset.getString( "Part0.FileName" )
      mshostname = gdsparset.getString( "Part0.FileSys" ).split(':')[0]
      os.system( "scp -r %s:%s/ANTENNA %s" % ( mshostname, msname, globaldb ) )
      os.system( "scp -r %s:%s/FIELD %s" % ( mshostname, msname, globaldb ) )

      if len( stations ) == 0 :
         stations = ["*"]
      self.stations = get_station_list( instrumentdb, stations, self.DirectionalGainEnable )
      self.N_stations = len(self.stations)
      
      antenna_table = pt.table( globaldb + "/ANTENNA")
      name_col = antenna_table.getcol('NAME')
      position_col = antenna_table.getcol( 'POSITION' )
      self.station_positions = [position_col[name_col.index(station_name)] for station_name in self.stations]
      antenna_table.close()
      
      station_table = self.hdf5.createTable(self.hdf5.root, 'stations', {'name': tables.StringCol(40), 'position':tables.Float64Col(3)})
      row = station_table.row
      for (station, position) in zip(self.stations, self.station_positions) : 
         row['name'] = station
         row['position'] = position
         row.append()
      station_table.flush()
      
      self.array_center = array( self.station_positions ).mean(axis=0).tolist()
      self.hdf5.createArray(self.hdf5.root, 'array_center', self.array_center)

      field_table = pt.table( globaldb + "/FIELD")
      phase_dir_col = field_table.getcol('PHASE_DIR')
      self.pointing = phase_dir_col[0,0,:]
      field_table.close()
      self.hdf5.createArray(self.hdf5.root, 'pointing', self.pointing)

      if self.DirectionalGainEnable:
         if len( sources ) == 0:
            sources = ["*"]
         self.sources = get_source_list( instrumentdb, sources )
         self.source_positions = []
         for source in self.sources :
            try:
               RA = skydb.getDefValues( 'Ra:' + source )['Ra:' + source][0][0]
               dec = skydb.getDefValues( 'Dec:' + source )['Dec:' + source][0][0]
            except KeyError:
               # Source not found in skymodel parmdb, try to find components
               RA = numpy.array(skydb.getDefValues( 'Ra:' + source + '.*' ).values()).mean()
               dec = numpy.array(skydb.getDefValues( 'Dec:' + source + '.*' ).values()).mean()
            self.source_positions.append([RA, dec])
      else:
         self.sources = ["Pointing"]
         self.source_positions = [list(self.pointing)]
      self.N_sources = len(self.sources)

      source_table = self.hdf5.createTable(self.hdf5.root, 'sources', {'name': tables.StringCol(40), 'position':tables.Float64Col(2)})
      row = source_table.row
      for (source, position) in zip(self.sources, self.source_positions) :
         row['name'] = source
         row['position'] = position
         row.append()
      source_table.flush()

      if self.PhasorsEnable:
         infix = ('Ampl', 'Phase')
      else:
         infix = ('Real', 'Imag')

      if self.GainEnable :
         parmname0 = ':'.join(['Gain', str(self.polarizations[0]), str(self.polarizations[0]), infix[1], self.stations[0]])
         v0 = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]
      if self.DirectionalGainEnable :
         parmname0 = ':'.join(['DirectionalGain', str(self.polarizations[0]), str(self.polarizations[0]), infix[1], self.stations[0], self.sources[0]])
         v0 = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]

      self.N_freqs = 0
      for gdsfile, gdsfile_idx in zip(gdsfiles, range(len(gdsfiles))) :
         parset = lofar.parameterset.parameterset( gdsfile )
         self.N_freqs += numpy.sum(parset.getIntVector( 'NChan' ))

      freqs = v0['freqs']
      N_freqs = len(freqs)
      self.freqs = freqs
      self.freqwidths = v0['freqwidths']
      
      self.times = v0['times']
      self.timewidths = v0['timewidths']
      self.hdf5.createArray(self.hdf5.root, 'times', self.times)
      self.hdf5.createArray(self.hdf5.root, 'timewidths', self.timewidths)
      self.N_times = len( self.times )

      self.hdf5.createArray(self.hdf5.root, 'polarizations', self.polarizations)
      
      self.phases = self.hdf5.createCArray(self.hdf5.root, 'phases', tables.Float32Atom(), shape=(self.N_times, self.N_freqs, self.N_stations, self.N_sources, self.N_pol))
      self.amplitudes = self.hdf5.createCArray(self.hdf5.root, 'amplitudes', tables.Float32Atom(), shape=(self.N_times, self.N_freqs, self.N_stations, self.N_sources, self.N_pol))
      self.amplitudes[:] = 1
      self.flags = self.hdf5.createCArray(self.hdf5.root, 'flags', tables.Float32Atom(), shape=(self.N_times, self.N_freqs))

      freq_idx = 0
      for gdsfile, instrumentdb_name, gdsfile_idx in zip(gdsfiles, self.instrumentdb_name_list, range(len(gdsfiles))) :
         print 'Reading %s (%i/%i)' % (gdsfile, gdsfile_idx+1, len(gdsfiles))
         if gdsfile_idx > 0 :
            del instrumentdb
            # First give lofar.parmdb time to close processes on remote hosts an release sockets
            time.sleep(5)
            instrumentdb = lofar.parmdb.parmdb( instrumentdb_name )
            v0 = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]
            freqs = v0['freqs']
            N_freqs = len(freqs)
            self.freqs = numpy.concatenate([self.freqs, freqs])
            self.freqwidths = numpy.concatenate(self,freqs, v0['freqs'])
            
         try:
            self.flags[:, freq_idx:freq_idx+N_freqs] = instrumentdb.getValuesGrid('flags')['flags']['values']
         except KeyError:
            pass
         
         for pol, pol_idx in zip(self.polarizations, range(len(self.polarizations))):
            for station, station_idx in zip(self.stations, range(len(self.stations))):
               if self.GainEnable:
                  parmname0 = ':'.join(['Gain', str(pol), str(pol), infix[0], station])
                  parmname1 = ':'.join(['Gain', str(pol), str(pol), infix[1], station])
                  if self.PhasorsEnable:
                     gain_phase = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']
                     self.phases[:, freq_idx:freq_idx+N_freqs, station_idx, :, pol_idx] = resize(gain_phase, (self.N_sources, N_freqs, self.N_times)).T
                     try:
                        gain_amplitude = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                     except KeyError:
                        self.amplitudes[:, freq_idx:freq_idx+N_freqs, station_idx, :, pol_idx] = numpy.ones((self.N_times, N_freqs, self.N_sources))
                     else:
                        self.amplitudes[:, freq_idx:freq_idx+N_freqs, station_idx, :, pol_idx] = numpy.resize(gain_amplitudes, (self.N_sources, N_freqs, self.N_times)).T
                  else:
                     gain_real = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]
                     gain_imag = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]
                     self.phases[:, freq_idx:freq_idx+N_freqs, station_idx, :, pol_idx] = numpy.resize(numpy.arctan2(gain_imag['values'], gain_real['values']),(self.N_sources, N_freqs, self.N_times)).T
                     self.amplitudes[:, freq_idx:freq_idx+N_freqs, station_idx, :, pol_idx] = numpy.resize(numpy.sqrt(gain_imag['values']**2 + gain_real['values']**2),(self.N_sources, N_freqs, self.N_times)).T
               if self.DirectionalGainEnable:
                  for source, source_idx in zip(self.sources, range(len(self.sources))) :
                     parmname0 = ':'.join(['DirectionalGain', str(pol), str(pol), infix[0], station, source])
                     parmname1 = ':'.join(['DirectionalGain', str(pol), str(pol), infix[1], station, source])
                     if self.PhasorsEnable:
                        gain_phase = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']
                        self.phases[:, freq_idx:freq_idx+N_freqs, station_idx, source_idx, pol_idx] += gain_phase
                        try:
                           gain_amplitude = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                        except KeyError:
                           pass
                        else:
                           self.amplitudes[:, freq_idx:freq_idx+N_freqs, station_idx, source_idx, pol_idx] *= gain_amplitude
                     else:
                        gain_real = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                        gain_imag = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']
                        self.phases[:, freq_idx:freq_idx+N_freqs, station_idx, source_idx, pol_idx]  += numpy.arctan2(gain_imag, gain_real)
                        self.amplitudes[:, freq_idx:freq_idx+N_freqs, station_idx, source_idx, pol_idx] *= numpy.sqrt(gain_real**2 + gain_imag**2)
      freq_idx += N_freqs

      if self.flags.shape <>  self.phases.shape[0:2] :
         self.flags = numpy.zeros(self.phases.shape[0:2])
         
      self.hdf5.createArray(self.hdf5.root, 'freqs', self.freqs)
         
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
      self.globaldb = str( self. globaldb )
      
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
      
      self.offsets = zeros((N_pol, len(self.n_list)))
      p = ProgressBar(len(self.n_list), "Fitting phase screen: ")
      for i in range(len(self.n_list)) :
         p.update( i )
         U = self.U_list[i]
         S = self.S_list[i]
         for pol in range(N_pol) :
            TEC = self.TEC[ pol, self.n_list[i], :, :].reshape( (N_sources * N_stations, 1) )
            TECfit = dot(U, dot(inv(dot(U.T, dot(G, U))), dot(U.T, dot(G, TEC))))
            TECfit_white = dot(U, dot(diag(1/S), dot(U.T, TECfit)))
            self.offsets[pol, i] = TECfit[0] - dot(self.C_list[i][0,:], TECfit_white)
            self.TECfit[ pol, i, :, : ] = reshape( TECfit, (N_sources, N_stations) )
            self.TECfit_white[ pol, i, :, : ] = reshape( TECfit_white, (N_sources, N_stations) )
      p.finished()      

      self.TECfit_white.attrs.r_0 = self.r_0
      self.TECfit_white.attrs.beta = self.beta
      
      if 'offsets' in self.hdf5.root: self.hdf5.root.offsets.remove()
      self.hdf5.createArray(self.hdf5.root, 'offsets', self.offsets)

   def make_movie( self, extent = 0, npixels = 100, vmin = 0, vmax = 0 ):
      """
      """

      multiengine_furl =  os.environ['HOME'] + '/ipcluster/multiengine.furl'
      mec = client.MultiEngineClient( multiengine_furl )
      task_furl =  os.environ['HOME'] + '/ipcluster/task.furl'
      tc = client.TaskClient( task_furl )
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
         savefig('tmpfig%3.3i.png' % i)
      p.finished()
      os.system("mencoder mf://tmpfig???.png -o movie.mpeg -mf type=png:fps=3  -ovc lavc -ffourcc DX50 -noskip -oac copy")

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

   def ClockTEC( self, ClockEnable = True, TECEnable = True) :
      """
      Estimate Clock and ionospheric TEC from phase information 
      """

      if not ClockEnable and not TECEnable: return
         
      N_stations = len(self.stations)
      N_baselines = N_stations * (N_stations - 1) / 2
      N_times = len(self.times)
      N_sources = len(self.sources)
      N_pol = len(self.polarizations)
      
      if N_sources == 0:
         N_sources = 1

      if ClockEnable:
         if 'Clock' in self.hdf5.root: self.hdf5.root.Clock.remove()
         self.Clock = self.hdf5.createCArray(self.hdf5.root, 'Clock', tables.Float32Atom(), shape = (N_pol, N_times, N_stations))
      if TECEnable:
         if 'TEC' in self.hdf5.root: self.hdf5.root.TEC.remove()
         self.TEC = self.hdf5.createCArray(self.hdf5.root, 'TEC', tables.Float32Atom(), shape = (N_pol, N_times,  N_sources, N_stations))
      
      
      for pol in range(N_pol):
         for source_no in range(N_sources):
            if ClockEnable and TECEnable:
               (Clock, TEC) = fit_Clock_and_TEC( squeeze( self.phases[0:N_times, :, :, source_no, pol] ),
                                            self.freqs[:], self.flags[0:N_times, :] )
               self.Clock[ pol, :, 1: ] = Clock
               self.TEC[ pol, :, source_no, 1: ] = TEC
            else : 
               v = fit_Clock_or_TEC( squeeze( self.phases[:, :, :, source_no, pol] ), self.freqs[:], self.flags[:, :], ClockEnable )
               if ClockEnable :
                  self.Clock[ pol, :, source_no, 1: ] = v
               else:
                  self.TEC[ pol, :, source_no, 1: ] = v
            
   def write_to_parmdb( self ) : 
      """
      Write Clock and TEC to a parmdb
      """

      N_sources = len(self.sources)
      
      if N_sources == 0:
         N_sources = 1

      parms = {}
      parm = {}
      parm[ 'freqs' ] = numpy.array( [ .5e9 ] )
      parm[ 'freqwidths' ] = numpy.array( [ 1.0e9 ] )
      parm[ 'times' ] = self.times[:].ravel()
      parm[ 'timewidths' ] = self.timewidths[:].ravel()

      for n_pol in range(len(self.polarizations)):
         pol = self.polarizations[n_pol]

         for n_station in range(len(self.stations)):
            station = self.stations[n_station]
            
            # Clock
            if 'Clock' in self.__dict__ :
               Clock_parm = parm.copy()
               parmname = ':'.join(['Clock', str(pol), station])
               Clock_parm[ 'values' ] = self.Clock[n_pol, :, n_station]
               parms[ parmname ] = Clock_parm

            for n_source in range(N_sources):
               if self.DirectionalGainEnable:
                  source = self.sources[n_source]
                  identifier = ':'.join([str(pol), station, source])
               else:
                  identifier = ':'.join([str(pol), station])
               
               # TEC
               TEC_parm = parm.copy()
               parmname = ':'.join(['TEC', identifier])
               TEC_parm[ 'values' ] = self.TEC[n_pol, :, n_source, n_station]
               parms[ parmname ] = TEC_parm
               
               #TECfit
               TECfit_parm = parm.copy()
               parmname = ':'.join(['TECfit', identifier])
               TECfit_parm[ 'values' ] = self.TECfit[n_pol, :, n_source, n_station]
               parms[ parmname ] = TECfit_parm
               
               #TECfit_white
               TECfit_white_parm = parm.copy()
               parmname = ':'.join(['TECfit_white', identifier])
               TECfit_white_parm[ 'values' ] = self.TECfit_white[n_pol, :, n_source, n_station]
               parms[ parmname ] = TECfit_white_parm
               
      #Piercepoints
      
      for n_station in range(len(self.stations)):
         station = self.stations[n_station]
         for n_source in range(N_sources):
            if self.DirectionalGainEnable:
               source = self.sources[n_source]
               identifier = ':'.join([station, source])
            else:
               identifier = station
            PiercepointX_parm = parm.copy()
            parmname = ':'.join(['Piercepoint', 'X', identifier])
            print n_source, n_station
            x = self.piercepoints[:]['positions_xyz'][:,n_source, n_station,0]
            PiercepointX_parm['values'] = x
            parms[ parmname ] = PiercepointX_parm

            PiercepointY_parm = parm.copy()
            parmname = ':'.join(['Piercepoint', 'Y', identifier])
            y = self.piercepoints[:]['positions_xyz'][:,n_source, n_station,1]
            PiercepointY_parm['values'] = array(y)
            parms[ parmname ] = PiercepointY_parm

            PiercepointZ_parm = parm.copy()
            parmname = ':'.join(['Piercepoint', 'Z', identifier])
            z = self.piercepoints[:]['positions_xyz'][:,n_source, n_station,2]
            PiercepointZ_parm['values'] = z
            parms[ parmname ] = PiercepointZ_parm

            Piercepoint_zenithangle_parm = parm.copy()
            parmname = ':'.join(['Piercepoint', 'zenithangle', identifier])
            za = self.piercepoints[:]['zenith_angles'][:,n_source, n_station]
            Piercepoint_zenithangle_parm['values'] = za
            parms[ parmname ] = Piercepoint_zenithangle_parm

      time_start = self.times[0] - self.timewidths[0]/2
      time_end = self.times[-1] + self.timewidths[-1]/2
      
      
      parm[ 'times' ] = numpy.array([(time_start + time_end) / 2])
      parm[ 'timewidths' ] = numpy.array([time_end - time_start])

      height_parm = parm.copy()
      height_parm[ 'values' ] = numpy.array( self.piercepoints.attrs.height )
      parms[ 'height' ] = height_parm

      beta_parm = parm.copy()
      beta_parm[ 'values' ] = numpy.array( self.TECfit_white.attrs.beta )
      parms[ 'beta' ] = beta_parm
      
      r_0_parm = parm.copy()
      r_0_parm[ 'values' ] = numpy.array(  self.TECfit_white.attrs.r_0 )
      parms[ 'r_0' ] = r_0_parm
      
      parmdbmain.store_parms( self.globaldb + '/ionosphere', parms, create_new = True)

   def write_phases_to_parmdb( self, gdsfile, phases_name = 'ionosphere') :
      gdsparset = lofar.parameterset.parameterset( gdsfile )
      N_parts = gdsparset.getInt("NParts")
      parm = {}
      N_times = len(self.times)
      parm[ 'times' ] = self.times[:].ravel()
      parm[ 'timewidths' ] = self.timewidths[:].ravel()
      for i in [0]: # range(N_parts):
         parms = {}
         msname = gdsparset.getString( "Part%i.FileName" % i )
         mshostname = gdsparset.getString( "Part%i.FileSys" % i).split(':')[0]
         os.system("scp -r %s:%s/SPECTRAL_WINDOW %s" % ( mshostname, msname, self.globaldb ))
         spectral_table = pt.table( self.globaldb + "/SPECTRAL_WINDOW")
         freqs = spectral_table[0]['CHAN_FREQ']
         N_freqs = len(freqs)
         parm[ 'freqs' ] = freqs
         parm[ 'freqwidths' ] = spectral_table[0]['CHAN_WIDTH']
         spectral_table.close()

         for (pol, pol_idx) in zip(self.polarizations, range(len(self.polarizations))):
            for n_station in range(len(self.stations)):
               station = self.stations[n_station]
               
               ## Clock
               #if 'Clock' in self.__dict__ :
                  #Clock_parm = parm.copy()
                  #parmname = ':'.join(['Clock', str(pol), station])
                  #Clock_parm[ 'values' ] = self.Clock[n_pol, :, n_station]
                  #parms[ parmname ] = Clock_parm

               for (facet, facet_idx) in zip(self.facets[:]['name'], range(len(self.facets))):
                  v = exp(1j * self.STEC_facets[pol_idx, :, facet_idx, n_station].reshape((N_times,1)) * \
                      8.44797245e9 / freqs.reshape((1, N_freqs)))
                  identifier = ':'.join([str(pol), str(pol), 'Real', station, facet])
                  DirectionalGain_parm = parm.copy()
                  parmname = ':'.join(['DirectionalGain', identifier])
                  DirectionalGain_parm[ 'values' ] = real(v)
                  parms[ parmname ] = DirectionalGain_parm

                  identifier = ':'.join([str(pol), str(pol), 'Imag', station, facet])
                  DirectionalGain_parm = parm.copy()
                  parmname = ':'.join(['DirectionalGain', identifier])
                  DirectionalGain_parm[ 'values' ] = imag(v)
                  parms[ parmname ] = DirectionalGain_parm
                  
         parmdbmain.store_parms( self.globaldb + '/facetgains', parms, create_new = True)
         
   #def write_phases_to_parmdb( self, gdsfiles = [], phases_name = 'ionosphere') :
      #if len(gdsfiles) == 0:
         #gdsfiles = self.gdsfiles
      #for gdsfile in gdsfiles :
         #instrument_parmdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + str(self.instrument_name)
         #phases_parmdbname = os.path.splitext(gdsfile)[0] + os.path.extsep + phases_name
         #os.system( "rundist -wd %s parmdbwriter.py %s %s %s" % (os.environ['PWD'], instrument_parmdbname, self.globaldb, phases_name) )

         #p = re.compile('(^Part\\d*.FileName\\s*=\\s*\\S*)(%s$)' % str(self.instrument_name))
         #print repr(instrument_parmdbname)
         #file_instrument_parmdb = open(instrument_parmdbname)
         #file_phases_parmdb = open(phases_parmdbname, 'w')
         #file_phases_parmdb.writelines([p.sub('\\1%s' % phases_name, l) for l in file_instrument_parmdb.readlines()])
         #file_instrument_parmdb.close()
         #file_phases_parmdb.close()

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

def fit_Clock_or_TEC( phase, freqs, flags, ClockEnable ):
   
   if ClockEnable :
      v = freqs.reshape((freqs.size,1))*2*pi
   else :
      v = 8.44797245e9/freqs.reshape((freqs.size,1))
      
   A1 = numpy.concatenate([v, 2*pi*numpy.ones(v.shape)], axis=1)
   A2 = v

   p22 = []
   rr = []
   
   multiengine_furl =  os.environ['HOME'] + '/ipcluster/multiengine.furl'
   mec = client.MultiEngineClient( multiengine_furl )
   mec.execute('import numpy, scipy.optimize')

   task_furl =  os.environ['HOME'] + '/ipcluster/task.furl'
   tc = client.TaskClient( task_furl )

   taskids = []
   for i in range(0,phase.shape[0]):
      print i+1, '/', phase.shape[0]
      maptask = client.MapTask(fit_Clock_or_TEC_worker, ( phase[i, :, :], flags[i, :], A1,A2))
      taskids.append(tc.run(maptask))
   i = 0
   for taskid in taskids :
      i += 1
      print i, '/', len(taskids)
      (residual_std, p) = tc.get_task_result(taskid, block = True)
      rr.append(residual_std)
      p22.append(p)      
      
   Clock_or_TEC = numpy.array(p22)

   tc.clear()
   del tc
   del mec
   
   return Clock_or_TEC


####################################################################
def fit_Clock_or_TEC_worker( phase, flags, A1, A2 ):
   
   def costfunction(p, A, y, flags = 0) : 
      N_stations = y.shape[1]
      TEC = numpy.concatenate( ( numpy.zeros(1), p ) )
      e = []
      for station0 in range(0, N_stations):
         for station1 in range(station0 + 1, N_stations):
            p1 = TEC[station1] - TEC[station0]
            dphase = y[:, [station1]] - y[:, [station0]]
            e.append( (numpy.mod( numpy.dot(A, p1) - dphase + numpy.pi, 2*numpy.pi ) - numpy.pi) )
      e = numpy.concatenate(e, axis=0)
      e = e[:,0] * (1-flags)
      return e
  
   N_stations = phase.shape[1]
   
   rr = []
   A = []
   dphase = []
   flags1 = []
   not_flagged_idx, = numpy.nonzero(1-flags)
   for station0 in range(0, N_stations):
      for station1 in range(station0 + 1, N_stations):
         v = numpy.zeros(N_stations)
         v[station1] = 1
         v[station0] = -1
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
   
   p[0::2] = 0
   p[1::2] = numpy.round(p[1::2])
   dphase = dphase - numpy.dot(A3, p)
   
   A4 = numpy.kron(A[:,1:], A2)
   S4 = numpy.dot(numpy.linalg.inv(numpy.dot(A4.T, A4)), A4.T)
   p = numpy.dot(S4, dphase)
   
   p, returncode = scipy.optimize.leastsq(costfunction, p, (A2, phase, flags1))
   
   while True:
      dphase_fit = numpy.dot(A4, p)
      residual = (numpy.mod(dphase - dphase_fit + numpy.pi,2*numpy.pi) - numpy.pi)
      residual_std = numpy.sqrt(numpy.mean(residual[flags1==0]**2))
      new_outlier_idx, = numpy.nonzero( (numpy.abs(residual) > (3*residual_std)) & (flags1 == 0))
      if len(new_outlier_idx) == 0:
         break
      flags1[new_outlier_idx] = 1
      p, returncode = scipy.optimize.leastsq(costfunction, p, (A2, phase, flags1))
      p_init = p
      
   return residual_std, p

####################################################################

def fit_Clock_and_TEC( phase, freqs, flags ):

   A1 = zeros((len(freqs),3))
   A1[:,0] = freqs*2*pi
   A1[:,1] = 8.44797245e9/freqs
   A1[:,2] = 2*pi*numpy.ones(freqs.shape)

   A2 = A1[:, 0:2]
   S2 = numpy.dot(numpy.linalg.inv(numpy.dot(A2.T, A2)), A2.T)

   dp = 2*pi*numpy.dot(S2, ones(phase.shape[1]))

   p22 = []
   residual_std1 = []

   rr = []
   multiengine_furl =  os.environ['HOME'] + '/ipcluster/multiengine.furl'
   mec = client.MultiEngineClient( multiengine_furl )
   mec.execute('import numpy, scipy.optimize')

   
   task_furl =  os.environ['HOME'] + '/ipcluster/task.furl'
   tc = client.TaskClient( task_furl )

   taskids = []
   for i in range(0,phase.shape[0]):
      print i+1, '/', phase.shape[0]
      maptask = client.MapTask(fit_Clock_and_TEC_worker, ( phase[i, :, :], flags[i, :], A1,A2,dp))
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

   tc.clear()
   del tc
   del mec

   Clock = p22[:,0::2]
   TEC = p22[:,1::2]

   return (Clock, TEC)

###############################################################################

def fit_Clock_and_TEC_worker( phase, flags, A1, A2, dp ):
   #import numpy
   #import scipy.optimize
   
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
   p_init = p - numpy.kron(numpy.round(p[1::2] / dp[1]), dp)
   while True:
      dphase_fit = numpy.dot(A4, p)
      residual = numpy.mod(dphase - dphase_fit + numpy.pi,2*numpy.pi) - numpy.pi
      residual_std = numpy.sqrt( numpy.mean ( residual[flags1==0]**2 ) )
      new_outlier_idx, = numpy.nonzero( (numpy.abs(residual) > (3*residual_std)) & (flags1 == 0))
      if len( new_outlier_idx ) == 0:
         break
      flags1[new_outlier_idx] = 1
      p, returncode = scipy.optimize.leastsq(costfunction, p_init, (A2, phase, flags1))
      p_init = p - numpy.kron(numpy.round(p[1::2] / dp[1]), dp)
  
   return (residual_std, p)

#####################################################################


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
      source_list.extend([n.split(':')[-1] for n in parmname_list])
   return sorted(set(source_list))

def get_source_list_from_ionospheredb( pdb ):
   parmname_list = pdb.getNames( 'TEC:?:*:*:*' )
   source_list = [n.split(':')[-1] for n in parmname_list]
   return sorted(set(source_list))

def get_station_list( pdb, station_pattern_list, DirectionalGainEnable ):
   station_list = []
   for pattern in station_pattern_list :
      parmname_list = pdb.getNames( { True : 'DirectionalGain:?:?:*:'+pattern+':*', False: 'Gain:?:?:*:' + pattern}[DirectionalGainEnable] )
      station_list.extend(sorted(set([n.split(':')[{True : -2, False : -1}[DirectionalGainEnable]] for n in parmname_list])))
   return station_list

def get_station_list_from_ionospheredb( pdb ):
   parmname_list = pdb.getNames( 'TEC:?:*:*:*' )
   station_list = [n.split(':')[-2] for n in parmname_list]
   return sorted(set(station_list))

def get_time_list_from_ionospheredb( pdb, pattern = '*' ):
   parameter_names = pdb.getNames( pattern )
   parm0 = pdb.getValuesGrid(parameter_names[0])[parameter_names[0]]
   time_list = parm0['times']
   time_width_list = parm0['timewidths']
   return (time_list, time_width_list)

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

def make_instrumentdb( gdsfilename, instrument_name, globaldb ):
   instrumentdb_name = os.path.join( globaldb, os.path.splitext(os.path.basename(gdsfilename))[0] + os.path.extsep + instrument_name)
   p = re.compile('(^Part\\d*.FileName\\s*=\\s*\\S*)')
   gdsfile = open(gdsfilename)
   instrumentdb_file = open(instrumentdb_name, 'w')
   instrumentdb_file.writelines([p.sub('\\1%s%s' % (os.path.sep, instrument_name), l) for l in gdsfile.readlines()])
   gdsfile.close()
   instrumentdb_file.close()
   return instrumentdb_name
  

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
      
