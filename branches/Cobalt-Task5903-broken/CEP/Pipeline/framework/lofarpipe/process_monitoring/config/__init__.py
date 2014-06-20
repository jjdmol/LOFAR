from __future__ import with_statement
from os.path import dirname

class Config(object):
    """This class takes care of reading a configuration file. This class should 
    also ship with a defaults.cfg file in the same folder which defines 
    all default values."""
    def __init__(self, filename = None):
        self.default_file = "defaults.cfg"
        self.path = dirname(__file__)
        self.__valsdict = dict()
        self.__defdict = dict()
        self.__read_defaults()
        if filename is not None:
            self.__read_configfile(filename)
    
    def add_item(self, it, val):
        """add item with name it and value it to config"""
        self.__valsdict[it] = val

    def get_value(self, option):
        """Get the value of option."""
        return self.__valsdict[option]
    
    def set_value(self, option, value):
        """Set configuration option 'option' to value, overriding any previously
        set value."""
        self.__valsdict[option] = value
    
    def set_to_default(self, itm):
        """Set value of itm to default value"""
        self.__valsdict[itm] = self.__defdict[itm]

    def __read_defaults(self):
        """Read the defaults. The file should be situated in the directory in 
        which the module resides."""
        dfile = self.path+"/"+self.default_file
        if dfile[0] != "/":
            dfile = "./" + dfile
        self.__read_configfile(dfile)
        self.__defdict = self.__valsdict.copy()
    
    def __read_configfile(self, filename):
        """Read config options from the file with name filename. The file
        should contain lines with option=value pairs"""
        with open(filename) as f:
            for lin in f:
                datlis = lin.split("=")
#                try:
#                    val = float(datlis[1].strip())
#                except ValueError:
#                    val = str(datlis[1].strip())
                if "," in datlis[1]:
                    val = datlis[1].strip().strip(",")
                else:
                    val = str(datlis[1].strip())
                    self.__valsdict[datlis[0].strip()] = val
        
    def __getitem__(self, itm):
        """Allow item access via cfval = cfg[itm]""" 
        return self.get_value(itm)
    
    def __call__(self, itm):
        """Allow item access via cfval = cfg(itm)"""
        return self.get_value(itm)
    
    def __setitem__(self, itm, val):
        """Allow item changing via cfg[itm] = val""" 
        return self.set_value(itm, val)
    
    def __delitem__(self, itm):
        """The del(cfg[itm]) construct makes the value of tm fall back to the 
        default value""" 
        return self.set_to_default(itm)
    
    def __str__(self):
        return str(self.__valsdict)

if __name__ == "__main__":
    print __doc__
    