import sys
from psycopg2 import psycopg1 as dbapi
import lofar.shm

# Check http://www.python.org/peps/pep-0249.html

#def encode_string(data):
#        return str.join("", map(lambda x: "\\%03o" % ord(x), data))

class DatabaseResult:
    def __init__(self, description, row):
        assert len(description) == len(row)
        self.description = description
        for (field_description, field_value) in zip(description, row):
            setattr(self, field_description[0], field_value)

class DatabaseResults(list):
    def __init__(self, description, results):
        self.description = description
        for result in results:
            self.append(result)

class Database:
    def __init__(self):
        self._connection = None
        self._cursor = None
        
            
    def _dbapi_delegate(self, callable, *args, **kwargs):
        try:
            return callable(*args, **kwargs)
        except (dbapi.InterfaceError, dbapi.DatabaseError, dbapi.DataError, dbapi.OperationalError, dbapi.IntegrityError, dbapi.InternalError, dbapi.ProgrammingError, dbapi.NotSupportedError), ex:
            # NOTE: psycopg non-conformance: all the exceptions listed above are
            # required to be subclasses of Error. however, psycopg does not adhere
            # to the DB API 2.0 standard at this point.
            raise lofar.shm.DatabaseError, "[dbapi]: %s" % (str(ex),)

    def open(self):
        if not lofar.shm.config.has("database.dsn"):
            raise lofar.shm.DatabaseError, "not enough information to connect to the database; please check the lofar-shm.conf config file."
        
        self._connection = self._dbapi_delegate(dbapi.connect, lofar.shm.config.get("database.dsn"))
        self._dbapi_delegate(self._connection.autocommit, True)
        self._cursor = self._dbapi_delegate(self._connection.cursor)


    def close(self):
        self._dbapi_delegate(self._cursor.close)
        self._cursor = None
        
        self._dbapi_delegate(self._connection.close)
        self._connection = None


    def begin_transaction(self):
        self._dbapi_delegate(self._cursor.execute, "BEGIN;")
        
        
    def commit(self):
        # _connection.commit() does not work when autocommit is on.
        self._dbapi_delegate(self._cursor.execute, "COMMIT;")


    def rollback(self):
        # _connection.rollback() does not work when autocommit is on.
        self._dbapi_delegate(self._cursor.execute, "ROLLBACK;")


    def perform_query(self, query, args = None):

        self._dbapi_delegate(self._cursor.execute, query, args)

        if self._cursor.description is None:
            return None
        else:
            results = [DatabaseResult(self._cursor.description, result) for result in self._dbapi_delegate(self._cursor.fetchall)]
            return DatabaseResults(self._cursor.description, results)

    def fetch(self, query, args = None):
        self._dbapi_delegate(self._cursor.execute, query, args)
            
        while True:
            row = self._dbapi_delegate(self._cursor.fetchone)

            if row is None:
                break

            yield self._row_wrapper(self._cursor.description, row)


class SysHealthDatabase(Database):
    def __init__(self):
        # call super class constructor
        Database.__init__(self)

    # the following methods are probably unused at this point. Investigate and remove if possible.

    #def GetTimesWhenNameIsValue(self, name, value, start_time_string, stop_time_string):
    #    """This method returns a list of time strings, denoting all time instances where
    #       variable 'name' has value 'value', in the time window (start_time_string, stop_time_string)"""
    #    query = "SELECT * FROM GetTimesWhenNameIsValue('%(name)s', '%(value)s', '%(start_time_string)s', '%(stop_time_string)s');" % vars()
    #    (description, results) = self.perform_query(query)
    #    return [x[0] for x in results]

    #def GetValuesOnTime(self, prefix, stable_time):
    #    query = "SELECT name, value FROM GetValuesOnTime('%(prefix)s', '%(stable_time)s');" % vars()
    #    (description, results) = self.perform_query(query)
    #    return results

    #def SaveBinaryData(self, data):
    #    str_data = str.join("", map(lambda x: "\\\\%03o" % ord(x), data))
    #    query = "INSERT INTO bdata(data) VALUES('%(str_data)s');" % vars()
    #    self.Perform_query(query, commit = True)
