from __future__ import with_statement

class Output(dict):
    """This class is the output class. 
    It essentially behaves as a dict but is extended
    with functions that write the output to an output 
    file"""
    
    def tofile(self, log_fname, err_fname):
        """Write output to file fname."""
        with open(log_fname,"a") as f:
            for val in self.keys():
                if self[val][0] != "":
                    f.write("{0} {1}\n".format(val, self[val][0]))
        with open(err_fname, "a") as f:
            for val in self.keys():
                if self[val][1] != "":
                    f.write("{0} {1}\n".format(val, self[val][1]))