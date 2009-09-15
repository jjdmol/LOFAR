import os
import glob
import ConfigParser

def initialize():
    __config.clear()
    
    # set defaults
    __config["database.host"] = "sas001.lofar"
    __config["database.port"] = 5432
    __config["job_control.uid"] = os.getuid()
    __config["job_control.gid"] = os.getgid()
    __config["job_control.cycle_time"] = 1

        
    # parse config file
    parser = ConfigParser.ConfigParser()
    try:
#        parser.read(glob.glob(os.path.join(os.sep, "etc", "lofar-shm", "*.conf")))
#        parser.read(glob.glob(os.path.join(os.sep, "/dop54_0/guest1", "etc", "lofar-shm", "*.conf")))
        parser.read(glob.glob(os.path.join(os.sep, "/home/avruch/work/lofar/shm/versions/Components/LofarPython", "etc", "lofar-shm", "*.conf")))
    except (IOError, ConfigParser.ParsingError):
        print 'MAX was here\n'
        return
    
    # insert all option, value pairs into the config dictionary
    for section in parser.sections():
        for item in parser.items(section):
            __config["%s.%s" % (section, item[0])] = item[1]

    try:
        __config["database.dsn"] = "dbname=%s user=%s host=%s port=%s" % (__config["database.dbname"], __config["database.user"], __config["database.host"], __config["database.port"])
    except KeyError:
        pass


def has(key):
    return __config.has_key(key)


def get(key):
    return __config[key]


def set(key, value):
    __config[key] = value


# init config dictionary when this module
# is imported.
__config = {}
initialize()
