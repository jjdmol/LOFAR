import numpy
import lofar.casaimwrap
import time
import os
import socket
import subprocess
import sys
import IPython.parallel
import itertools
import json
from ...processors import ImageWeight, Normalization

dataprocessor_id = 0
def get_dataprocessor_id() :
  global dataprocessor_id
  dataprocessor_id += 1
  return dataprocessor_id-1

class DataProcessorParallelLowLevel:
    def __init__(self, datadescriptor, options):
      
        if not isinstance(datadescriptor, dict) and os.path.isfile(datadescriptor) :
            f = open(datadescriptor)
            datadescriptor = json.load(f)

        if len(options['profile']) == 0:
            self._start_ipcluster()
        else:
            self._profile = options['profile']
            self.measurements = datadescriptor.values()
            
        self._rc = IPython.parallel.Client(profile=self._profile)
        for dview in self._rc :
            print dview
            engineid = dview['engineid']
            print engineid
            sys.stdout.flush()
            dview['msname'] = str(self.measurements[engineid])
            print dview['msname']
        self._dview = self._rc[:]
        self._dview['options'] = options
        self._dview.execute('from lofar.pyimager.processors.casa.data_processor_low_level import DataProcessorLowLevel')
        self._dview.execute('localdataprocessor = DataProcessorLowLevel(msname, options)', block = True)
        self._remoteprocessor = IPython.parallel.Reference('localdataprocessor')
        
    def _start_ipcluster(self) :
        self._profile = "%s-%s-%i-%i" % (os.path.basename(sys.argv[0]), socket.gethostname(), os.getpid(), get_dataprocessor_id())
        fnull = open(os.devnull, "w")
        # start ipcontroller
        self._ipcontroller = subprocess.Popen(["ipcontroller", "--ip=*", "--profile=" + self._profile, "--log-to-file", "--ping=1000"], stdin=fnull, stdout=fnull, stderr=fnull)
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
        self._started_ipcontroller = True
        # start ipengines
        self.hostnames = datadescriptor.keys()
        self._pid = {}
        engineid = 0
        self.measurements = {}
        for hostname in self.hostnames :
            self._pid[hostname] = []
            measurements = datadescriptor[hostname]
            if isinstance(measurements, basestring):
                measurements = [measurements]
            if hostname != "localhost" :
                shell = subprocess.Popen( ["ssh", hostname, "/bin/sh"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=fnull)
            else:
                shell = subprocess.Popen( ["/bin/sh"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=fnull)
            shell.stdin.write("PYTHONPATH=%s; export PYTHONPATH\n" % os.environ['PYTHONPATH'])
            shell.stdin.write("LD_LIBRARY_PATH=%s; export LD_LIBRARY_PATH\n" % os.environ['LD_LIBRARY_PATH'])
            shell.stdin.write("PATH=%s; export PATH\n" % os.environ['PATH'])
            shell.stdin.write("LOFARROOT=%s; export LOFARROOT\n" % os.environ['LOFARROOT'])
            shell.stdin.write("LOFARDATAROOT=%s; export LOFARDATAROOT\n" % os.environ['LOFARDATAROOT'])
            for measurement in measurements:
                shell.stdin.write("ipengine --profile=" + self._profile + \
                                        ' --work-dir=' + os.getcwd() + \
                                        ' --log-to-file' + \
                                        ' --EngineFactory.max_heartbeat_misses=0' + \
                                        ' --c="global engineid;engineid=' + str(engineid) + '" > /dev/null &\n')
                shell.stdin.write('echo $!\n')
                pid = shell.stdout.readline()[:-1]
                print "pid:", pid
                self._pid[hostname].append(pid)
                self.measurements[engineid] = measurement
                engineid += 1
            shell.stdin.write("exit\n")
            shell.wait()
        print "Waiting for engines to register...",
        sys.stdout.flush()
        while len(self._rc.ids) < len(self.measurements) :
            time.sleep(1)
        print 'ok'
        

    def clear(self):
        pass
        self._dview.purge_results(targets = None)
        self._rc.results.clear()
        self._rc.metadata.clear()
        self._dview.results.clear()

    def capabilities(self):
        results = self._dview.apply_sync(lambda processor : processor.capabilities(), self._remoteprocessor)
        capabilities = results[0]
        for result in results[1:] :
            assert(capabilities == result)
        self.clear()    
        return capabilities

    def phase_reference(self):
        results = self._rc[:].apply_sync(lambda processor : processor.phase_reference(), self._remoteprocessor)
        phase_reference = results[0]
        for result in results[1:] :
            assert((phase_reference == result).all())
        self.clear()    
        return phase_reference

    def channels(self):
        freqs = self._rc[:].apply_sync(lambda processor :
            processor.channel_frequency(), self._remoteprocessor)
        widths = self._rc[:].apply_sync(lambda processor :
            processor.channel_width(), self._remoteprocessor)
        channels = list(set(itertools.izip(itertools.chain.from_iterable(freqs),
            itertools.chain.from_iterable(widths))))
        self.clear()    
        return channels

    def maximum_baseline_length(self):
        results = self._rc[:].apply_sync(lambda processor : processor.maximum_baseline_length(), self._remoteprocessor)
        maximum_baseline_length = max(results)
        self.clear()    
        return maximum_baseline_length
        
    def set_density(self, density, coordinates):
        self._rc[:].apply_sync(lambda processor, *args : processor.set_density(*args), \
            self._remoteprocessor, density, coordinates)
        self.clear()    

    def point_spread_function(self, coordinates, shape, as_grid):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.point_spread_function(*args), self._remoteprocessor, \
            coordinates, shape, as_grid)
        psf, weight = results[0]
        psf = psf.copy()
        weight = weight.copy()
        for result in results[1:] :
            psf1, weight1 = result
            psf += psf1
            weight += weight1
        self.clear()    
        return (psf, weight)

    def grid(self, coordinates, shape, as_grid):
      
        print  "****************************************************************"
        print coordinates
        print shape
        print  "****************************************************************"
        
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.grid(*args), self._remoteprocessor, coordinates,
            shape, as_grid)
        image, weight = results[0]
        image = image.copy()
        weight = weight.copy()
        for result in results[1:] :
            image1, weight1 = result
            image += image1
            weight += weight1
        self.clear()    
        return (image, weight)

    def degrid(self, coordinates, model, as_grid):
        raise RuntimeError("degrid not implemented")

    def residual(self, coordinates, model, as_grid):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.residual(*args), self._remoteprocessor, coordinates,
            model, as_grid)
        residual, weight = results[0]
        residual = residual.copy()
        weight = weight.copy()
        for result in results[1:] :
            residual1, weight1 = result
            residual += residual1
            weight += weight1
        self.clear()    
        return (residual, weight)

    def density(self, coordinates, shape):
        results = self._dview.apply_sync(lambda processor, *args :
            processor.density(*args), self._remoteprocessor, \
            coordinates, shape)
        density = results[0].copy()
        for result in results[1:] :
            density += result
        return density

    def response(self, coordinates, shape):
        results = self._rc[:].apply_sync(lambda processor, *args :
            processor.response(*args), self._remoteprocessor, \
            coordinates, shape)
        response = results[0].copy()
        for result in results[1:] :
            response += result
        self.clear()    
        return response

    def close(self):
        try:
            if self._started_ipcontroller :
                self._rc.shutdown(hub=True)
                self._started_ipcontroller = False
        except AttributeError:
            pass

    def __del__(self) :
        self.close()
