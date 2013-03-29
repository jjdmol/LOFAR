# $Id$

from data_processor import DataProcessor

def create_data_processor(measurement, options):
    """Factory function that can be used to switch to a specialized class
    depending on the options and the measurement(s) to be processed.
    A measurement can be a string or a list of strings.
    If measurement is a dict then a parallel dataprocessor object will be
    created. The measurement.keys() are the hosts on which the engines will run
    The measurement.values() are lists of measurements to be opened.
    For each entry of the list a separate engine will be started.
    Otherwise a local dataprocessor will be created.
    """

    assert(not isinstance(measurement, dict))
    return DataProcessor(measurement, options)
