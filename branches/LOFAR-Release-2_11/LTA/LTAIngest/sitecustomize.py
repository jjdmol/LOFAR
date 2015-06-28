import sys, os.path
dir = os.path.split(__file__)[0]
if os.path.isdir(dir + '/ClientForm-0.1.17'):
  sys.path.insert(1, dir + '/ClientForm-0.1.17')
if os.path.isdir(dir + '/SOAPpy-0.12.0'):
  sys.path.insert(1, dir + '/SOAPpy-0.12.0')
if os.path.isdir(dir + '/fpconst-0.7.0'):
  sys.path.insert(1, dir + '/fpconst-0.7.0')
#if os.path.isdir(dir + '/dav'):
#  sys.path.insert(1, dir + '/dav')
if os.path.isdir(dir + '/multiprocessing'):
  sys.path.insert(1, dir + '/multiprocessing')

