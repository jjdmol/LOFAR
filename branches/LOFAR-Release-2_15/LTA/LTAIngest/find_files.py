import os, sys
path = sys.argv[1]
if os.path.isdir(path):
  files = os.listdir(path)
  for f in files:
    if f[-5:] == '.dppp' or f[-3:] == '.MS' or f[-4:] == '.dp3':
      print f
else:
  exit(1)
exit(0)