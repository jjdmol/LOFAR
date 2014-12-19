#                                                      LOFAR PIPELINE FRAMEWORK
#
#                                                          aggregate stats 
#                                                           Wouter Klijn, 2014
#                                                               klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import xml.dom.minidom as xml

def usage():
    usage_string = """
    usage:  python aggregate_stats.py <pipeline_xml_stats_file>
    
    This program parses and aggregates pipelines statistics from a
    xml stat file generate by the Lofar automatic pipeline framework.
    
    It creates the following aggregate statistics, in both visual format
    and as csv file for further visualization/processing (optional):

    1. For the whole pipeline:
        - Running time   (total duration)
        - CPU seconds    (of all the node processing) (calculated)
        TODO: For each stat the min, max, median, mean and std 
    2. Per data product sets
        - Running time   ( max duration for this data product set)
        - CPU seconds
        - Max memory usage
        - Bytes read and write
    3. Per recipe for each data product
        - Running time
        - CPU second
        - memory usage
        - disk usage
    """

    print usage_string

    
def open_file_and_parse(xml_stats_path):
    """
    Performs the interaction with the xml_stats file.
    Opens it from disk, throwing an ImportError if it fails
    converts to an instantiated minidom xml three. Throwing a ImportError if
    it fails.    
    """    
    # Open the file raise on error
    raw_file_string = ""
    
    try:
        raw_file_string = open(xml_stats_path, 'r').read()
    except:
        # TODO: Failing to open a file is expected faulty behaviour,
        # should we do something else than an exception here?
        print "could not open file: {0}".format(xml_stats_path)
        raise ImportError("Could not open supplied file for parsing")
    
    # Parse to xml
    stats_xml = None
    
    try:        #, encoding='ascii'
        stats_xml = xml.parseString(raw_file_string).documentElement  # only need the data not the xml fluff
    except:
        # Parsing of xml should succeed if written by the pipeline framework
        # In this case an exception should be allowed
        
        print "Attempted to parse '{0}' as an xml file. This failed".format(xml_stats_path)

    # return as xml_minidom object
    return stats_xml


def collect_information_for_a_recipe(recipe_xml):
    """
    Collects all the information available in a recipe xml tree
    Again, this is tightly coupled with the implementation in the pipeline
    """

    
def convert_xml_attributes_to_dict(attributes):
    """
    helper function converting the xml.attributes return list
    to a ascii dict, where possible entries are cast to their 
    python representation
    """
    # put all the intries into a dict
    attribute_dict = {}
    for attribute in attributes.items():
        attribute_dict[attribute[0].encode("ascii", 'ignore')] = \
                attribute[1].encode("ascii", 'ignore')
    
    
    for key, value in attribute_dict.items():
        try:
            casted_value = eval(value)
            attribute_dict[key] = casted_value
        except:
            # eval is expected to fail swallow all exceptions
            pass
    return attribute_dict
 
 
def collect_toplevel_information(stats_xml):
    """
    Collects the information found at the top level of xml
    Is simply converts the xml element information into python
    dicts for ease of access
    """
    toplevel_dict = convert_xml_attributes_to_dict(stats_xml.attributes)
    return toplevel_dict

    
def create_timeline_dict(data_points_list):
    """
    receives a list of xml containing the actual resource traces
    
    Converts the data into python variables
    Attempts to clean this data of empty datapoints
    Then creates a dict of metric to list with the actual time trace    
    """
    # the list which will filled with the trance for the respective stat
    timestamp = []
    mem = []
    cpu = []
    read_bytes = []
    write_bytes = []
    cancelled_bytes = []
    
    for data_point in data_points_list:
        # convert to python variables
        single_data_point_dict =  convert_xml_attributes_to_dict(data_point.attributes)
        
        # if there is no valid data in the correct point
        if (single_data_point_dict['mem'] == ""):
            continue
            
        # assume that the entries are ordered when creating the time line
        # TODO: this assumption might be broken when other ppl add info to the xml stats
        timestamp.append(single_data_point_dict['timestamp'])
        mem.append(single_data_point_dict['mem'])
        cpu.append(single_data_point_dict['cpu'])
        read_bytes.append(single_data_point_dict['read_bytes'])
        write_bytes.append(single_data_point_dict['write_bytes'])
        cancelled_bytes.append(single_data_point_dict['cancelled_bytes'])
        
    # add the collected traces to a nice dict    
    resource_traces_dict = {}
    resource_traces_dict['timestamp']        = timestamp
    resource_traces_dict['mem']              = mem
    resource_traces_dict['cpu']              = cpu
    resource_traces_dict['read_bytes']       = read_bytes
    resource_traces_dict['write_bytes']      = write_bytes
    resource_traces_dict['cancelled_bytes']  = cancelled_bytes
        
    return resource_traces_dict
  
  
def collect_job_information(job_xml):
    """
    Collects all the information for an indivual jb run. Including
    possible executable information added
    """
    #first get the attribute information
    jobs_dict = convert_xml_attributes_to_dict(job_xml.attributes)
    
    # now collect the actual 'statistics'
    # statistics are burried one node deeper   
    resource_xml = job_xml.getElementsByTagName('resource_usage')
    if len(resource_xml) != 1:
        print "Encountered an error while parsing resource node"
        print "Continue parsing with other available nodes."
        print "information might be corrupted or incomplete"
    
    # get the attributes mainly needed for the pid of the job recipe 
    resource_dict = convert_xml_attributes_to_dict(resource_xml[0].attributes)

    # get the children, this is a list proces statistics
    processes_xml = job_xml.getElementsByTagName('process')
    for idx, single_process_xml in enumerate(processes_xml):
        # First grab information about the executable
        single_process_dict =  convert_xml_attributes_to_dict(single_process_xml.attributes)
        
        # Then create the time line
        data_points_list = single_process_xml.getElementsByTagName('data_point')
        resource_traces_dict = create_timeline_dict(data_points_list)
        single_process_dict['trace'] = resource_traces_dict
        
        # we have no idea what is the order of the indiv jobs, store them with an idx
        jobs_dict[idx] = single_process_dict
    
    jobs_dict['number_of_jobs'] = len(processes_xml)   
    return jobs_dict
 
 
def collect_recipe_information(node_xml):
    """
    Parses and collects information of a recipe step
    """   
    # get the name and duration for this specific step
    node_dict = convert_xml_attributes_to_dict(node_xml.attributes)
    node_dict['node_name'] = node_xml.nodeName.encode("ascii", 'ignore')
    
    # The actual node run information is stored 2 nodes deeper
    nodes = node_xml.getElementsByTagName('nodes')
    if len(nodes) == 0:   # if zero this node did not run on the compute nodes
        node_dict['info'] = "No node level information"
        return node_dict
    
    if len(nodes) > 1:    # there should only be a single node entry failure state otherwise
        print "Encountered an error while parsing node {0}".format(node_dict['node_name'])
        print "Continue parsing with other available nodes."
        print "information might be corrupted or incomplete"
        return node_dict
    
    # we have a single node as expected
    # grab the job (node level information)   
    jobs = nodes[0].getElementsByTagName('job')
    if len(jobs) == 0:
        print "Encountered an error while parsing node {0}".format(node_dict['node_name'])
        print "No job / node level information was found"
        print "Continue parsing with other available nodes."
        print "information might be corrupted or incomplete"
        return node_dict

    # now parse the individual nodes   
    jobs_dict = {}
    for job in jobs:
        single_job_dict = collect_job_information(job)
        jobs_dict[single_job_dict['job_id']] = single_job_dict
        
    # save the parsed information
    node_dict['jobs'] = jobs_dict

    return node_dict
    
    
def get_pipeline_information(stats_xml):
    """
    Collects all the information needed to create toplevel information about
    the pipeline. 
    
    The information produced by this function should be ready for plotting.
    TODO:
    This function in development. Needs a proper info string    
    """    
    # The pipeline_information dictionary will contain all the parsed information       
    pipeline_information = {}
        
    # Get the toplevel pipeline information
    # TODO: It might be easier to first collect all stats and then calculate
    # attempt to extract as much as possible for now to get a proof of concept
    # I do not realy like the dict keys. It would be better to use the index?
    # works when itterating tru the information
    pipeline_information[0] = collect_toplevel_information(stats_xml)
    
    for idx, dom in enumerate(stats_xml.childNodes):
        node_name = dom.nodeName.encode("ascii", 'ignore')
        
        # First check if we are in the active_stack node.
        # this node should be empty. print warning if not!              
        if (node_name == 'active_stack'):
            if (len(dom.childNodes) != 0):        
                print "The active stack contained leftover nodes" 
                print "This probably means that the pipeline failed in a step"
                
            # TODO: The mem size could be of interest: might point into
            # the direction of an issue with the config.
            continue  # do not parse information in this node
        
        # We have a pipeline node. Send it to function to get parsed
        recipe_dict = collect_recipe_information(dom)

        pipeline_information[idx] = recipe_dict
    
    return pipeline_information


def create_recipe_duration_lists(pipeline_information):
    """
    Collects the duration and the name of the steps in the pipeline.
    """
    duration_list = []
    step_name_list = []
    for idx in range(1, len(pipeline_information.items())):
        duration_list.append(pipeline_information[idx]["duration"])
        step_name_list.append( pipeline_information[idx]["node_name"])
    
    return duration_list, step_name_list
    
    
if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()
    
    xml_stats_path = sys.argv[1]
    stats_xml = open_file_and_parse(xml_stats_path)
    
    # From here the information is extracted from the xml 
    # This implementation is tightly coupled with the way it is represented
    # in the file. The filling of this xml is done in the pipeline framework
    #
    # It might contain a large amount of information that is not of
    # interest at this level. Search for specific tree information and entries    
    # first step is parsing and formatting the information in the xml
    pipeline_information = get_pipeline_information(stats_xml)
    
    # Create the roughest stat possible: A duration list.
    duration_list, step_name_list = create_recipe_duration_lists(pipeline_information)
    print duration_list
    print step_name_list
    
    # 
    # for entry in pipeline_information.items():
        # print entry
        # print "\n"
        
    #print pipeline_information
    
    
    
        
        