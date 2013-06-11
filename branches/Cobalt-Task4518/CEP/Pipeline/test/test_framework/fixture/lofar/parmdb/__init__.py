import re
import copy
import os

class parmdb(object):
    """
    Much parmdb interface:
    Allows basic checking of called function and parameters
    """
    def __init__ (self, dbname, create = True, names = None):
        self._basename = dbname
        if not names == None:
            self.names = names
        elif os.path.basename(dbname) == "fullName":
            self.names = [
                      "Gain:1:1:Real:test",
                      "Gain:1:1:Imag:test",
                      "Gain:0:0:Real:test",
                      "Gain:0:0:Imag:test"]

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

    def getValuesGrid(self, *args):
        self.called_functions_and_parameters.append(
                            ['getValuesGrid', [arg for arg in args]])


        return {args[0]: {"values":[[1., 1., 1., 1., 100., 100.], [1., 1., 1., 1., 100., 100.]],
                          'freqs':[2],
                          'freqwidths':[2],
                          'times':[2],
                          'timewidths':[2]} }



