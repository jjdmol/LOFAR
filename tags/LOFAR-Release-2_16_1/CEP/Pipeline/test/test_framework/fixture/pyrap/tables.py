from numpy.ma import extras

class table():
    """
    Muck table object.
    A minimal implementation of the pyrapl.table object:
    It allows getting of keywords and cells from a previously initialed static
    dictionary of keyword -> value pairs. Currently a global dictionary is used

    """
    # TODO: global dict prevent parallen running of unit tests (fragile code smell)
    variable_dictionary = {}

    def __init__(self, measurement_set):
        """
        Initialize the table, give it a name. If the name is except all calls
        to getters will return an exception
        """
        if measurement_set == 'except':
            self.exception = True
        else:
            self.exception = False
        self.measurement_set = measurement_set

    def getkeyword(self, keyword, default = None):
        """
        Returns the value found in the variable dict.
        Returns exception if set       
        """
        if self.exception:
            raise Exception(table.variable_dictionary[keyword])
        if table.variable_dictionary.has_key(keyword):
            return table.variable_dictionary[keyword]

        return default

    def getcell(self, keyword, idx, default = None):
        """
        Returns the value found in the variable dict
        Returns exception if set       
        """
        if self.exception:
            raise Exception(table.variable_dictionary[keyword])
        if table.variable_dictionary.has_key(keyword):
            cell_data = table.variable_dictionary[keyword]
            if idx < len(cell_data):
                return cell_data[idx]

        return default

    def set_variable_dictionary(self, variable_dictionary):
        """
        Sets the class member variable_dictionary.
        """
        table.variable_dictionary = variable_dictionary

    def close(self):
        return

def taql(input_string):
    """
    Return taql commands:
    1. 'CALC C()' -> return c (float)
    """   
    if input_string.strip() == 'CALC C()':
        return 299792458.0
    raise Exception("command not know in this muck implementation!")
    
