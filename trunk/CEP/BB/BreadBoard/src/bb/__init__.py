"""
Blackboard package.

$Id$

"""

__all__ = ["BlackBoard", "Workload", "Thread"];


Version_info = (0,0,1);
__version__ = "0.0.1";

db = None;

def init():
  import pg;
  global db;
  db = pg.DB(dbname="bb");

init();
