 # Compatability layer for IPython.client 0.10 and IPython.parallel >= 0.11

import IPython

if [int(v) for v in IPython.__version__.split('.')] < [0,11] :
  from IPython.kernel import client
  import atexit
# Without the following statement python sometimes throws an exception on exit
  atexit.register(client.rit.stop)
  MultiEngineClient = client.MultiEngineClient
  TaskClient = client.TaskClient
  MapTask = client.MapTask
else:
  from IPython.parallel import Client
  
  def MultiEngineClient() :
    rc = Client()
    dview = rc[:]
    return dview
    
  class TaskClient :
    def __init__(self) :
      self.rc = Client()
      self.dview = self.rc[:]
      self.lbview = self.rc.load_balanced_view()
    
    def run(self, maptask) :
      return self.lbview.apply(maptask.func, *maptask.args)
      
    def get_task_result(self, task, block = True) :
      return task.get()
    
    def clear(self):
      pass
    
  class MapTask :
    def __init__(self, func, args) :
      self.func = func
      self.args = args
      pass
    