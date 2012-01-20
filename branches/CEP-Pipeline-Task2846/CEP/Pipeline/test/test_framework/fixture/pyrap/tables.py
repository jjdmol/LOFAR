# *****************************************************************************
# mock pyrap class allowing testing of functionality without actual access to
# a working ms set
# *****************************************************************************
from numpy.ma import extras

class table():
    """
    Muck table object allowing assinging to class static variable dictionary
    and return keyword and cell functions
    A possible better implementation allowing for paralel testing is the 
    addition of a dict of variable dictionarys indexed by ms name 
    """
    variable_dictionary = {}

    def __init__(self, measurement_set):
        if measurement_set == 'except':
            self.exception = True
        else:
            self.exception = False
        self.measurement_set = measurement_set

    def getkeyword(self, keyword, default = None):
        if self.exception:
            raise Exception(table.variable_dictionary[keyword])
        if table.variable_dictionary.has_key(keyword):
            return table.variable_dictionary[keyword]

        return default

    def getcell(self, keyword, idx, default = None):
        if self.exception:
            raise Exception(table.variable_dictionary[keyword])
        if table.variable_dictionary.has_key(keyword):
            cell_data = table.variable_dictionary[keyword]
            if idx < len(cell_data):
                return cell_data[idx]

        return default

    def set_variable_dictionary(self, variable_dictionary):
        table.variable_dictionary = variable_dictionary

    def close(self):
        return

def taql(input_string):
    if input_string.strip() == 'CALC C()':
        return 299792458.0
    raise Exception("Unknow taql command, Add to tables.py")
    
