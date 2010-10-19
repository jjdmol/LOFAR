#!/usr/bin/env python

from util.Hosts import ropen
from Locations import Locations

# do not modify any files if DRYRUN is True
DRYRUN = False

"""
  The following files exist to aid the creation of observation IDs:

  nextMSnumber	contains the next free observation ID (integer)
  MSList	contains a list of existing measurements and their start time

  Their locations are stored in the Locations dictionary.
"""

# the comment to add to the measurement database for each measurement
MSLISTCOMMENTMASK = "${YEAR} ${MONTH} ${DAY}"

class ObservationID:
  def __init__( self ):
    self.obsid = 0

  def generateID( self, parset ):
    """ Returns an unique observation ID to use and reserve it. """

    if self.obsid:
      # already reserved an observation ID
      return self.obsid

    # read the next ms number
    f  = ropen( Locations.files["nextmsnumber"], "r" )
    obsid = int(f.readline())
    f.close()

    # update the parset in order to parse the masks properly
    parset.setObsID( obsid )

    if not DRYRUN:
      # increment it and save
      f = ropen( Locations.files["nextmsnumber"], "w" )
      print >>f, "%s" % (obsid+1)
      f.close()

      # Add this measurement to the database. 
      f = ropen( Locations.files["mslist"], "a")
      print >>f, "%s\t%s" % ( parset.parseMask(), parset.parseMask(MSLISTCOMMENTMASK) )
      f.close()

    self.obsid = obsid

    return self.obsid

ObservationID = ObservationID()
