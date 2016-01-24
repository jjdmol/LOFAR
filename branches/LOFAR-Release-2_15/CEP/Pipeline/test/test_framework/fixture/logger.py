#test fixture for pipeline framework

class logger():
    """
    muck logger object, allows logging of info, debug and error function
    new entries are appended to a list with the type of logger error and 
    the suplied input
    """
    def __init__(self):
        self._log = ["logger_started"]
        self.name = "muck_logger"

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

