from lofar.parmdb import parmdb

class WritableParmDB(parmdb):
    def __init__(self, name):
        super(WritableParmDB, self).__init__(name)
        self.name = name

