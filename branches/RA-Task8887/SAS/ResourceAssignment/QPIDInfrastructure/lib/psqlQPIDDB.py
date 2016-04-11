#!/usr/bin/python

import psycopg2 as pg
import psycopg2.extras as pgdefs

class psqlQPIDDB:
    """ psqlQPIDDB class
    defines low level database interaction with the 
    postgres database that holds the QPID infra configuration.

    """
    def __init__(self, dbcreds=None):
	""" Init the class with the name of the database
	example: db = psqlQPIDDB(dbcreds)
        where `dbcreds' is an lofar.common.dbcredentials.Credentials object.
	"""
	self.dbcreds = dbcreds
	self.conn = None

	self.ensure_connect()

    def ensure_connect(self):
	""" ensure that the database is still connected.
	raises an exception "ERROR: Failed to connect to database XXX"
	if the reconnect failed.
	"""

        if self.conn and self.conn.status==1:
	    return

	self.conn = pg.connect(**self.dbcreds.pg_connect_options())

	if self.conn and self.conn.status==1:
	    return
	else:
	    raise Exception("ERROR: Failed to reconnect to database %s" % (self.dbcreds,))

    def doquery(self,query):
	""" execute a query on the database and return reult as a list of dicts.
	This assumes nothing needs to be committed and thus
	useful for fetching infromation from the database.
	usage: ret=doquery("select * from table;")
	"""

	self.ensure_connect()
	cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
        cur.execute(query)
	return cur.fetchall()

    def docommit(self,query):
	""" execute a database query that needs a commit to update the database.
	example: docommit("INSERT INTO table (one,two) VALUES ('one','two');")
	"""

	self.ensure_connect()
	cur = self.conn.cursor()
	cur.execute(query)
	print cur.statusmessage
	self.conn.commit()

    def getid(self,itemtype,itemname):
	""" retrieve an id from a table with assumptions on table layout.
	the query is done by substituting the table name with the itemtype with 's' appended.
	example:
	     id = getid('shoe','myshoe')
	the used query will be:
	        "select * from shoes where shoename='myshoe';"
	"""
	tmp=self.doquery("select * from %ss where %sname='%s';" %(itemtype,itemtype,itemname))
	if (tmp==[]):
	    return 0
	return tmp[0]["%sid" %(itemtype)]

    def delid(self,itemtype,itemid):
	""" delete a record from a table with assumptions on table layout.
	the query is done by substituting the table name with the itemtype with 's' appended.
	example:
	     id = getid('shoe',245)
	the used query will be:
	     "delete from shoes where shoeid=245;"
	"""
	if (id!=0):
	    self.docommit("delete from %ss where %sid=%d;"(itemtype,itemtype,itemid))

    def delname(self,itemtype,itemname, verbose=True):
	""" delete a record from a table with assumptions on table layout.
        the query is done by substituting the table name with the itemtype with 's' appended.
        example:
             id = getid('shoe','myshoe')
        the used query will be:
             "delete from shoes where shoename='myshoe';"
        """
	id= self.getid(itemtype,itemname)
	if (id):
	    if verbose:
		print("Deleting %s from table %ss." %(itemname,itemtype))
	    self.docommit("delete from %ss where %sid=%d and %sname='%s'" %(itemtype,itemtype,itemtype,itemname))
	else:
	    print("%s %s not found in database." %(itemtype,itemname))

    def getname(self,itemtype,itemid):
	""" retrieve name from database table for index.
	example:
	     name = getname('shoe',245);
	the used query will be:
	     "SELECT shoename FROM shoes WHERE shoeid=245;"
	"""
	res=self.doquery("select %sname from %ss where %sid=%d;" %(itemtype,itemtype,itemtype,itemid))
	if (res!=[]):
	    return res[0]["%sname" %(itemtype)]
	return 'NotAvailableInDatabase'

    def additem(self,itemtype,itemname,verbose=True):
	""" Insert a record in the database with assumptions on the table layout.
	example:
	     additem('shoe','myshoe',verbose=False)
	the used query will be:
	     "INSERT INTO shoes (shoename) VALUES ('myshoe');"
	"""
	id = self.getid(itemtype,itemname)
	if (id!=0):
	    if verbose:
		print("%s %s already available in database." %(itemtype,itemname))
	    return id
	self.docommit("insert into %ss (%sname) values ('%s');" %(itemtype,itemtype,itemname))
	if verbose:
	    print (" added %s %s to DB" %(itemtype,itemname))
	return self.getid(itemtype,itemname)

    def delitem(self,itemtype,itemname,verbose=True):
	""" Delete a record from the database with assumptions on the table layout.
	example:
	     delitem('shoe','myshoe',verbose=False)
	the used query will be:
	     "DELETE FROM shoes WHERE shoename='myshoe';"
	"""

	id = self.getid(itemtype,itemname)
	if (id!=0):
	    if verbose:
		print("Deleting from table %s the item %s." %(itemtype,itemname))
	    self.docommit("delete from %ss where %sid=%d and %sname='%s';" %(itemtype,itemtype,id,itemtype,itemname))
	    return 0;
	print("%s %s not found in the database" %(itemtype,itemname))

