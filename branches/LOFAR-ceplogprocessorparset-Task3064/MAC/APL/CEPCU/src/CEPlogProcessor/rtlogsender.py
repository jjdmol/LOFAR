import datetime
import sys
import time

# Replays a RTCP logfile at the rate at which it was
# produced (according to the timestamps)#
#
# Provide the logfile on stdin, and it will be
# replayed on stdout.


now = datetime.datetime.utcnow
totalseconds = lambda td: (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / 10**6

def extractDateTime( logline ):
  p = logline.split(" ")
  try:
    return (p[1],p[2])
  except:
    return None

def convertDateTime( dt ):
  if dt is None:
    return now()

  y,m,d = dt[0].split("-")
  H,M,S = dt[1].split(":")
  try:
    S,F = S.split(".")
  except:
    F = 0.0

  return datetime.datetime( int(y), int(m), int(d), int(H), int(M), int(S), int(float(F)*1000) )

def sleepUntil( dt ):
  sleeptime = totalseconds(dt-now())
  if sleeptime > 0:
    time.sleep( sleeptime )

offset = None
starttime = None

for l in sys.stdin:
  dt = convertDateTime( extractDateTime( l ) )
  if offset is None:
    offset = dt
    starttime = now()
    wait = datetime.timedelta()
  else:
    wait = dt - offset

  if totalseconds( wait ) > 0:  
    sleepUntil( starttime + wait )

  print l.rstrip()
