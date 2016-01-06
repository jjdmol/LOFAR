#!/usr/bin/python

# Copyright (C) 2012-2015    ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id$

from glob import glob
from os import environ
from ConfigParser import SafeConfigParser, NoSectionError, DuplicateSectionError

__all__ = ["DBCredentials"]

def findfiles(pattern):
  """ Returns a list of files matched by `pattern'.
      The pattern can include environment variables using the
      {VAR} notation.
  """
  try:
    return glob(pattern.format(**environ))
  except KeyError:
    return []

class DBCredentials:
  defaults = {
    # Flavour of database (postgres, mysql, oracle, sqlite)
    "type": "postgres",

    # Connection information
    "host": "localhost",
    "port": "",

    # Authentication
    "user": "{USER}".format(**environ),
    "password": "",

    # Database selection
    "database": "",
  }

  def __init__(self, filepatterns=None):
    """
      Read database credentials from all configuration files matched by any of the patterns 
    """
    if filepatterns is None:
      filepatterns = [
        "{LOFARROOT}/etc/dbcredentials/*.ini",
        "{HOME}/.lofar/dbcredentials/*.ini",
        ]

    self.config = SafeConfigParser(defaults=self.defaults)

    self.files = sum([findfiles(p) for p in filepatterns],[])
    self.config.read(self.files)


  def get(self, database):
    """
      Return credentials for a given database.
    """
    try:
      return dict(self.config.items(self._section(database)))
    except NoSectionError:
      return self.config.defaults()


  def set(self, database, credentials):
    """
      Add or overwrite credentials for a given database.
    """
    section = self._section(database)

    # create section if needed
    try:
      self.config.add_section(section)
    except DuplicateSectionError:
      pass

    # set or override credentials
    for k,v in credentials.iteritems():
      self.config.set(section, k, v)


  def list(self):
    """
      Return a list of databases for which credentials are available.
    """
    sections = self.config.sections()
    return [s[9:] for s in sections if s.startsWith("database:")]


  def _section(self, database):
    return "database:%s" % (database,)

if __name__ == "__main__":
  import sys
  from optparse import OptionParser

  parser = OptionParser("%prog [options]")
  parser.add_option("-D", "--database", dest="database", type="string", default="",
                    help="Name of the database")
  parser.add_option("-L", "--list", dest="list", action="store_true", default=False,
                    help="List known databases")
  parser.add_option("-F", "--files", dest="files", action="store_true", default=False,
                    help="List names of parsed configuration files")
  (options, args) = parser.parse_args()

  if not options.database and not options.list and not options.files:
    print "Missing database name"
    parser.print_help()
    sys.exit(1)

  dbc = DBCredentials()

  if options.files:
    """ Print list of configuration files that we've read. """
    if dbc.files:
      print "\n".join(dbc.files)
    sys.exit(0)

  if options.list:
    """ Print list of databases. """
    databases = dbc.list()
    if databases:
      print "\n".join(databases)
    sys.exit(0)

  """ Print credentials of a specific database. """
  print "\n".join(["%s %s" % (k,v) for k,v in dbc.get(options.database).iteritems()])

