import subprocess
import re
import numpy
import sys

def grep_finished_lines_from_log(logfile, obs_id):
    """
    Start grep in a separate process on the provided logfile
    It will look for all lines containing the following regular expression:
    
    <obs_id>.*stream.*Finished
    
    It will return all the final log messages containing the information 
    with the amount of correctly written data   
    """
    # construct copy command
    command = ["grep", "-E", "{0}.*stream.*Finished".format(obs_id), logfile]
    #Spawn a subprocess and connect the pipes
    copy_process = subprocess.Popen(
        command,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdoutdata, stderrdata) = copy_process.communicate()

    exit_status = copy_process.returncode
    if exit_status != 0:
        print "Std out"
        print stdoutdata
        print "Error out:"
        print stderrdata
        raise RuntimeError("Grep return with a non zero exit status bailing out")

    return stdoutdata


def matcher_function(reg_ex_object, line):
    """
    Run the regular expression match object object on
    the provided line and return a possible matching number
    as a python type:
    Provides a wrapper performing error checking en casting
    Raises exceptions if no numbers are found    
    """

    matchobject = reg_ex_object.match(line)
    if matchobject == None:
        print line
        print "^^^^^This one failed ^^^^^"
        raise Exception("The line is not long enough to be a log line")

    try:
        return int(matchobject.group(1))
    except:
        try:
            return float(matchobject.group(1))
        except:
            print line
            print "^^^^^This one failed ^^^^^"
            raise Exception("Could not parse the number: {0}".format(matchobject.group(1)))


def get_formatted_data(lines_with_data):
    """
    Parse running stats from the logline provided.
    all parsing is done with regexpressions and entered in a dict
    together with the line parsed from
    """
    parsed_lines = []
    for line in lines_with_data.split("\n"):
        local_data = {}
        #append(line)
        if len(line) < 10:
            continue
        local_data['line'] = line
        local_data['locus'] = matcher_function(re.compile(".*locus([0-9]*) .*"), line)
        local_data['stream_id'] = matcher_function(re.compile(".*stream *([0-9]*) writer.*"), line)
        local_data['writer'] = matcher_function(re.compile(".*writer *([0-9]*)] .*"), line)
        local_data['blocks_written'] = matcher_function(re.compile(".*writing: ([0-9]*) .*"), line)
        local_data['blocks_dropped'] = matcher_function(re.compile(".*written, ([0-9]*) blocks.*") , line)
        local_data['percent_lost'] = matcher_function(re.compile(".*dropped: ([0-9]*\.*[0-9]*)\%.*"), line)
        parsed_lines.append(local_data)

    return parsed_lines


def get_statistics(parsed_lines, entry_name):
    """
    Get all named entries from the list of dicts in parsed_lines.
    Collect in array and return a dict with stats
    """
    number_of_entries = len(parsed_lines)
    list_of_entry_value = []
    for data_dict in parsed_lines:
        list_of_entry_value.append(data_dict[entry_name])
    stats = {}
    stats['name'] = entry_name
    stats['max'] = numpy.max(list_of_entry_value)
    stats['min'] = numpy.min(list_of_entry_value)
    stats['mean'] = numpy.mean(list_of_entry_value)
    stats['median'] = numpy.median(list_of_entry_value)
    stats['std'] = numpy.std(list_of_entry_value)

    return stats

def print_stats(stats):
    """
    Print state in a formatted line
    """
    formatted_line = \
      "{0},{1},{2},{3},{4},{5}".format(
         stats['name'], stats['min'], stats['max'], stats['mean'], stats['std'],
           stats['median'])

    print formatted_line


def calculate_and_print_stats(parsed_lines, obs_id, filename):
    """
    Collect and pretty print all preparsed dict in the list parsed_lines    
    """
    print ""
    print "Observation id: {0}    datasource: {1}".format(obs_id, filename)
    print "name,min,max,mean,std,median" # header
    print_stats(get_statistics(parsed_lines, 'blocks_written'))
    print_stats(get_statistics(parsed_lines, 'blocks_dropped'))
    print_stats(get_statistics(parsed_lines, 'percent_lost'))
    print ""
    print ""


def perform_analysis(filename, obs_id):
    lines_with_data = grep_finished_lines_from_log(filename, obs_id)
    parsed_lines = get_formatted_data(lines_with_data)
    calculate_and_print_stats(parsed_lines, obs_id, filename)

def usage():
    print \
""" usage: python performance_test_storage.py <logfile path> <obs_id>

    Parse the provided logfile for summary log lines of the writers part of
    obs_id. It collects the lines and parsed the percentage written dat.
    Calculates some statistics on them and displays this.  
       
    """


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    try:
        filename, obs_id = sys.argv[1:3]
        perform_analysis(filename, obs_id) # 60719)
    except:
        usage()

