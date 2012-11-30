#!/usr/bin/python

import dataprocessor
import IPython.parallel
import time
import json
import numpy

class NumpyAwareJSONEncoder(json.JSONEncoder):
  def default(self, obj):
    if isinstance(obj, numpy.ndarray) :
      return [self.default(x) for x in obj]
    if isinstance(obj, numpy.float64) :
      return float(obj)
    return json.JSONEncoder.default(self, obj)

d1 = dataprocessor.createDataProcessor(['localhost:/home/vdtol/pyimager/imagtest1.MS', 'localhost:/home/vdtol/pyimager/imagtest1.MS'])
#d2 = dataprocessor.createDataProcessor(['localhost:name1', 'localhost:name2'])

print d1.profile
print json.dumps(d1.get_info(), indent=2, cls=NumpyAwareJSONEncoder)
#print d2.profile
