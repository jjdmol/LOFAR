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
import os
import pwd
from ConfigParser import SafeConfigParser, NoSectionError, DuplicateSectionError
from optparse import OptionGroup
from os import stat, path, chmod
import logging

logger = logging.getLogger(__name__)

__all__ = ["Credentials", "DBCredentials", "options_group", "parse_options"]

# obtain the environment, and add USER and HOME if needed (since supervisord does not)
environ = os.environ
user_info = pwd.getpwuid(os.getuid())
environ.setdefault("HOME", user_info.pw_dir)
environ.setdefault("USER", user_info.pw_name)


def findfiles(pattern):
  """ Returns a list of files matched by `pattern'.
      The pattern can include environment variables using the
      {VAR} notation.
  """
  try:
    return glob(pattern.format(**environ))
  except KeyError:
    return []


class Credentials:
  def __init__(self):
    # Flavour of database (postgres, mysql, oracle, sqlite)
    self.type = "postgres"

    # Connection information (port 0 = use default)
    self.host = "localhost"
    self.port = 0

    # Authentication
    self.user = environ["USER"]
    self.password = ""

    # Database selection
    self.database = ""

  def __str__(self):
    return "type={type} addr={host}:{port} auth={user}:{password} db={database}".format(**self.__dict__)

  def pg_connect_options(self):
    """
      Returns a dict of options to provide to PyGreSQL's pg.connect function. Use:

      conn = pg.connect(**dbcreds.pg_connect_options())
    """
    return {
      "host": self.host,
      "port": self.port or -1,

      "user": self.user,
      "passwd": self.password,

      "dbname": self.database,
    }


  def mysql_connect_options(self):
    """
      Returns a dict of options to provide to python's mysql.connector.connect function. Use:

      from mysql import connector
      conn = connector.connect(**dbcreds.mysql_connect_options())
    """
    options = { "host": self.host,
                "user": self.user,
                "passwd": self.password,
                "database": self.database }

    if self.port:
        options["port"] = self.port

    return options

class DBCredentials:
  def __init__(self, filepatterns=None):
    """
      Read database credentials from all configuration files matched by any of the patterns.

      By default, the following files are read:

        $LOFARROOT/etc/dbcredentials/*.ini
        ~/.lofar/dbcredentials/*.ini

      The configuration files allow for any number of database sections:

        [database:OTDB]
        type = postgres     # postgres, mysql, oracle, sqlite
        host = localhost
        port = 0            # 0 = use default port
        user = paulus
        password = boskabouter
        database = LOFAR_4

      These database credentials can subsequently be queried under their
      symbolic name ("OTDB" in the example).
    """
    if filepatterns is None:
      filepatterns = [
        "{LOFARROOT}/etc/dbcredentials/*.ini",
        "{HOME}/.lofar/dbcredentials/*.ini",
        ]

    self.files = sum([findfiles(p) for p in filepatterns],[])

    # make sure the files are mode 600 to hide passwords
    for file in self.files:
        if oct(stat(file).st_mode & 0777) != '0600':
            logger.info('Changing permissions of %s to 600' % file)
            try:
                chmod(file, 0600)
            except Exception as e:
                logger.error('Error: Could not change permissions on %s: %s' % (file, str(e)))

    #read the files into config
    self.config = SafeConfigParser()
    self.config.read(self.files)

  def get(self, database):
    """
      Return credentials for a given database.
    """
    # create default credentials
    creds = Credentials()

    # read configuration
    try:
      d = dict(self.config.items(self._section(database)))
    except NoSectionError:
      return creds

    # parse and convert config information
    if "host" in d:     creds.host = d["host"]
    if "port" in d:     creds.port = int(d["port"] or 0)

    if "user" in d:     creds.user = d["user"]
    if "password" in d: creds.password = d["password"]

    if "database" in d: creds.database = d["database"]

    if "type" in d:     creds.type = d["type"]

    return creds


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
    self.config.set(section, "type", credentials.type)
    self.config.set(section, "host", credentials.host)
    self.config.set(section, "port", str(credentials.port))
    self.config.set(section, "user", credentials.user)
    self.config.set(section, "password", credentials.password)
    self.config.set(section, "database", credentials.database)

  def list(self):
    """
      Return a list of databases for which credentials are available.
    """
    sections = self.config.sections()
    return [s[9:] for s in sections if s.startswith("database:")]


  def _section(self, database):
    return "database:%s" % (database,)


def options_group(parser):
  """
    Return an optparse.OptionGroup containing command-line parameters
    for database connections and authentication.
  """
  group = OptionGroup(parser, "Database Credentials")
  group.add_option("-D", "--database", dest="dbName", type="string", default="",
                   help="Name of the database")
  group.add_option("-H", "--host", dest="dbHost", type="string", default="",
                   help="Hostname of the database server")
  group.add_option("-p", "--port", dest="dbPort", type="string", default="",
                   help="Port number of the database server")
  group.add_option("-U", "--user", dest="dbUser", type="string", default="",
                   help="User of the database server")
  group.add_option("-P", "--password", dest="dbPassword", type="string", default="",
                   help="Password of the database server")
  group.add_option("-C", "--dbcredentials", dest="dbcredentials", type="string", default="",
                   help="Name of database credential set to use [default=%default]")

  return group


def parse_options(options, filepatterns=None):
  """
    Parses command-line parameters provided through options_group()
    and returns a credentials dictionary.

    `filepatterns' can be used to override the patterns used to find configuration
    files.
  """

  dbc = DBCredentials(filepatterns)

  # get default values
  creds = dbc.get(options.dbcredentials)

  # process supplied overrides
  if options.dbHost:     creds.host     = options.dbHost
  if options.dbPort:     creds.port     = options.dbPort
  if options.dbUser:     creds.user     = options.dbUser
  if options.dbPassword: creds.password = options.dbPassword
  if options.dbName:     creds.database = options.dbName

  return creds


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
  print str(dbc.get(options.database))

