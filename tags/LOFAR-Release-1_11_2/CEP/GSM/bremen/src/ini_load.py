#!/usr/bin/python
try:
    # Try loading LOFAR parset support, fallback to ConfigObj.
    from lofar.parameterset import parameterset
    LOFAR_PARAMETERSET = True
except ImportError:
    from configobj import ConfigObj
    LOFAR_PARAMETERSET = False

def load_parameters(filename):
    """
    Load parameters from file and return them as hash.
    """
    if LOFAR_PARAMETERSET:
        data = parameterset(filename).dict()
    else:
        data = ConfigObj(filename, raise_errors=True, file_error=True)
    return data
