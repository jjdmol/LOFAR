from lofarpipe.recipes.nodes.parmexportcal import ParmExportCal

class logger():
    """
    muck logger object, allows logging of info, debug and error function
    new entries are appended to a list with the type of logger error and 
    the suplied input
    """
    def __init__(self):
        self._log = []

    def info(self, input_string):
        self._log.append(("info", input_string))

    def debug(self, input_string):
        self._log.append(("debug", input_string))

    def error(self, input_string):
        self._log.append(("error", input_string))

    def warn(self, input_string):
        self._log.append(("warn", input_string))

    def last(self):
        """
        return that last error
        """
        return self._log[-1]


class ParmExportCalWrapper(ParmExportCal):
    """
    Wrapper inserting logger functionality
    """
    def __init__(self):
        """
        """
        self.logger = logger()


if __name__ == "__main__":

    infile, outfile, sigma = sys.argv[1:4]
    parmdb = ParmExportCalWrapper()
    parmdb._filter_stations_parmdb(infile, outfile, sigma)
    sys.exit()
