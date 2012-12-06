 # Compatability layer for IPython.client 0.10 and IPython.parallel >= 0.11

import IPython

if float(IPython.__version__) < 0.11 :
  from IPython.kernel import client
  # Without the following statement python sometimes throws an exception on exit
  atexit.register(client.rit.stop)
  MultiEngineClient = client.MultiEngineClient
  TaskClient = client.TaskClient
  MapTask = client.MapTask
else:
  from IPython.parallel import Client
  
  rc = Client()
  dview = rc[:]
  
  def MultiEngineClient() :
    return dview
    
  class TaskClient :
    def __init__(self) :
      self.lbview = rc.load_balanced_view()
    
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
    