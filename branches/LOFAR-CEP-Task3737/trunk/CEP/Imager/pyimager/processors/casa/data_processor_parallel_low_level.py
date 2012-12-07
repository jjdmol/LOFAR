import numpy
import lofar.casaimwrap
import time
import os
import socket
import subprocess
import sys
import IPython.parallel
import itertools
from ...processors import ImageWeight, Normalization

dataprocessor_id = 0
def get_dataprocessor_id() :
  global dataprocessor_id
  dataprocessor_id += 1
  return dataprocessor_id-1

class DataProcessorParallelLowLevel:
    def __init__(self, datadescriptor, options):
        self._profile = "%s-%s-%i-%i" % (os.path.basename(sys.argv[0]), socket.gethostname(), os.getpid(), get_dataprocessor_id())
        fnull = open(os.devnull, "w")
        # start ipcontroller
        self._ipcontroller = subprocess.Popen(["ipcontroller", "--ip=*", "--profile=" + self._profile], stdin=fnull, stdout=fnull, stderr=fnull)
        self._timeout = 60
        print "Waiting for ipcontroller to come online...",
        sys.stdout.flush()
        while True :
            try :
              self._rc = IPython.parallel.Client(profile=self._profile)
            except :
              pass
            else:
              break
            time.sleep(1)
        print 'ok'
        # start ipengines
        self.hostnames = datadescriptor.keys()
        self._pid = {}
        engineid = 0
        self.measurements = {}
        for hostname in self.hostnames :
            self._pid[hostname] = []
            measurements = datadescriptor[hostname]
            if isinstance(measurements, str):
                measurements = [measurements]
            shell = subprocess.Popen(["ssh", hostname, "/bin/sh"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=fnull)
            shell.stdin.write("export PYTHONPATH=" + os.environ['PYTHONPATH'] + "\n")
            shell.stdin.write("export LD_LIBRARY_PATH=" + os.environ['LD_LIBRARY_PATH'] +"\n")
            shell.stdin.write("export PATH=" + os.environ['PATH'] +"\n")
            shell.stdin.write("export LOFARROOT=" + os.environ['LOFARROOT'] +"\n")
            shell.stdin.write("export LOFARDATAROOT=" + os.environ['LOFARDATAROOT'] +"\n")
            for measurement in measurements:
                shell.stdin.write("ipengine --profile=" + self._profile + \
                                        ' --work-dir=' + os.getcwd() + \
                                        ' --c="global engineid;engineid=' + str(engineid) + '" > /dev/null &\n')
                shell.stdin.write('echo $!\n')
                pid = shell.stdout.readline()[:-1]
                self._pid[hostname].append(pid)
                self.measurements[engineid] = measurement
            shell.stdin.write("exit\n")
        print "Waiting for engines to register...",
        sys.stdout.flush()
        while len(self._rc) < len(self.measurements) :
            time.sleep(1)
        print 'ok'
        for dview in self._rc :
            engineid = dview['engineid']
            dview['msname'] = self.measurements[engineid]
        self._dview = self._rc[:]
        self._dview['options'] = options
        self._dview.execute('from lofar.pyimager.processors.casa.data_processor_low_level import DataProcessorLowLevel')
        self._dview.execute('localdataprocessor = DataProcessorLowLevel(msname, options)')
        self._remoteprocessor = IPython.parallel.Reference('localdataprocessor')
        self._connected = True

    def capabilities(self):
        results = self._rc[:].apply_sync(lambda processor : processor.capabilities(), self._remoteprocessor)
        capabilities = results[0]
        for result in results[1:] :
            assert(capabilities == result)
        return capabilities

    def phase_reference(self):
        results = self._rc[:].apply(lambda processor : processor.phase_reference(), self._remoteprocessor).get(timeout=self._timeout)
        phase_reference = results[0]
        for result in results[1:] :
            assert((phase_reference == result).all())
        return phase_reference

    def channels(self):
        freqs = self._rc[:].apply_sync(lambda processor :
            processor.channel_frequency(), self._remoteprocessor)
        widths = self._rc[:].apply_sync(lambda processor :
            processor.channel_width(), self._remoteprocessor)
        channels = list(set(itertools.izip(itertools.chain.from_iterable(freqs),
            itertools.chain.from_iterable(widths))))
        return channels

    def maximum_baseline_length(self):
        results = self._rc[:].apply_sync(lambda processor : processor.maximum_baseline_length(), self._remoteprocessor)
        maximum_baseline_length = max(results)
        return maximum_baseline_length

    def point_spread_function(self, coordinates, shape, density, as_grid):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.point_spread_function(*args), self._remoteprocessor, \
            coordinates, shape, density, as_grid)
        psf, weight = results[0]
        psf = psf.copy()
        weight = weight.copy()
        for result in results[1:] :
            psf1, weight1 = result
            psf += psf1
            weight += weight1
        return (psf, weight)

    def grid(self, coordinates, shape, normalization, density, as_grid):
        image, weight = self._processor.grid(coordinates, shape, \
            density, as_grid)

        return (numpy.where(weight > 0.0, image / weight, 0.0), weight)

    def degrid(self, coordinates, model, normalization):
        self._update_image_configuration(coordinates, model.shape)

        self._processor.degrid(self._coordinates, model, False)

    def residual(self, coordinates, model, density, as_grid):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.residual(*args), self._remoteprocessor, coordinates,
            model, density, as_grid)
        residual, weight = results[0]
        residual = residual.copy()
        weight = weight.copy()
        for result in results[1:] :
            residual1, weight1 = result
            residual += residual1
            weight += weight1
        return (residual, weight)

    def density(self, coordinates, shape):
        #results = self._dview.apply_sync(lambda processor, *args :
            #processor.density(*args), self._remoteprocessor, \
            #coordinates, shape)
        #density = results[0]
        #for result in results[1:] :
            #density += result
        #return density
        return 1.0

    def response(self, coordinates, shape, density):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.response(*args), self._remoteprocessor, \
            coordinates, shape, density)
        response = results[0].copy()
        for result in results[1:] :
            response += result
        return response

    def close(self):
        if self._connected :
            self._rc.shutdown(hub=True)
            self._connected = False

    def __del__(self) :
        self.close()
