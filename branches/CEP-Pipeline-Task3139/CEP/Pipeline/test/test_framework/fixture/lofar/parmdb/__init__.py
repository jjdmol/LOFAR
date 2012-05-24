import re
import copy

class parmdb(object):
    """
    Much parmdb interface:
    Allows basic checking of called function and parameters
    """
    def __init__ (self, dbname, create = True, names = None):
        self._basename = dbname
        if not names == None:
            self.names = names
        else:
            self.names = ["1:1:Real:name1",
                      "1:1:Real:name2",
                      "1:1:Real:name3",
                      "1:1:Real:name4",
                      "Gain:1:2:Real:station1"]
        self.called_functions_and_parameters = []

    def getNames(self, parmnamepattern = ''):
        if parmnamepattern == '':
            return self.names
        #convert the pattern to regexp
        listed_pattern = parmnamepattern.split("*")
        parmnamepattern = ".*{0}.*".format(".*".join(listed_pattern))
        #create regexp matcher!
        prog = re.compile(parmnamepattern)
        # only return regexp matches the pattern in the string
        return [s for s in self.names if prog.match(s)]

    def deleteValues(self, *args):
        self.called_functions_and_parameters.append(['deleteValues',
                                                     [arg for arg in args]])

    def addValues(self, *args):
        self.called_functions_and_parameters.append(
                            ['addValues', [arg for arg in args]])





