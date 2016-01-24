#                                                      LOFAR PIPELINE FRAMEWORK
#
#                                                          aggregate stats 
#                                                           Wouter Klijn, 2014
#                                                               klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import xml.dom.minidom as xml

from matplotlib import pyplot as plt
import numpy as np


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

    
def convert_xml_attributes_to_dict(attributes, clean_empty_values=True):
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
    toplevel_dict = convert_xml_attributes_to_dict(stats_xml.attributes,
                                          clean_empty_values=False)
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
        if ((single_data_point_dict['mem'] == "") or
           (single_data_point_dict['cpu'] == "") or
           (single_data_point_dict['read_bytes'] == "")):
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
    traces_dict = {}
    processes_xml = job_xml.getElementsByTagName('process')
    for idx, single_process_xml in enumerate(processes_xml):
        # First grab information about the executable
        single_process_dict =  convert_xml_attributes_to_dict(single_process_xml.attributes)
        
        # Then create the time line
        data_points_list = single_process_xml.getElementsByTagName('data_point')
        resource_traces_dict = create_timeline_dict(data_points_list)
        single_process_dict['trace'] = resource_traces_dict
        
        # we have no idea what is the order of the indiv jobs, store them with an idx
        traces_dict[idx] = single_process_dict
    
    jobs_dict['traces'] = traces_dict
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


def create_and_display_pie_chart(fracs, labels, title_string):
    """
    Make a pie chart - see
    http://matplotlib.sf.net/matplotlib.pylab.html#-pie for the docstring.

    This example shows a basic pie chart with labels optional features,
    like autolabeling the percentage, offsetting a slice with "explode",
    adding a shadow, and changing the starting angle.

    The version of pylab installed is old. Most advanced functionality is not 
    working: it is not possible to use startangle=90, or explode

    """
    # make a square figure and axes
    plt.figure(1, figsize=(6,6))
    ax = plt.axes([0.1, 0.1, 0.8, 0.8])

    # The slices will be ordered and plotted counter-clockwise.
    plt.pie(fracs, labels=labels, autopct='%1.1f%%', shadow=True)

    plt.title(title_string, bbox={'facecolor':'0.8', 'pad':5})

    plt.show()    


def create_and_display_horizontal_bar(title_string,
           dataset, data_orders, colors=["r","g","b","y"]):
    """
    Create a horizontal bar chart
    https://stackoverflow.com/questions/11273196/stacked-bar-chart-with-differently-ordered-colors-using-matplotlib
    """
    #dataset = [{'A':19, 'B':39, 'C':61, 'D':70},
    #       {'A':34, 'B':68, 'C':32, 'D':38},
    #       {'A':35, 'B':45, 'C':66, 'D':50},
    #       {'A':23, 'B':23, 'C':21, 'D':16},
    #       {'A':35, 'B':45, 'C':66, 'D':50}]
    #data_orders = [['A', 'B', 'C', 'D'], 
    #           ['B', 'A', 'C', 'D'], 
    #           ['A', 'B', 'D', 'C'], 
    #           ['B', 'A', 'C', 'D'],
    #           ['A', 'B', 'C', 'D']]

    names = sorted(dataset[0].keys())
    values = np.array([[data[name] for name in order] for data,order in zip(dataset, data_orders)])
    lefts = np.insert(np.cumsum(values, axis=1),0,0, axis=1)[:, :-1]
    orders = np.array(data_orders)
    bottoms = np.arange(len(data_orders))

    for name, color in zip(names, colors):
        idx = np.where(orders == name)
        value = values[idx]
        left = lefts[idx]
        plt.bar(left=left, height=0.8, width=value, bottom=bottoms, 
                color=color, orientation="horizontal", label=name)

    plt.yticks(bottoms+0.4, ["data %d" % (t+1) for t in bottoms])
    plt.legend(loc="best", bbox_to_anchor=(1.0, 1.00))
    plt.subplots_adjust(right=0.75)
    plt.title(title_string, bbox={'facecolor':'0.8', 'pad':5})
    plt.show()

def create_toplevel_pipeline_visualizations(duration_list, step_name_list):
    """
    Some examples of visualizations possible using the stats collected from the 
    pipeline
    """

    # Create a label list for plotting:
    formatted_step_name_list = []
    for duration, step_name in zip(duration_list, step_name_list):
        formatted_step_name_list.append(step_name[1:] + 
                                        "\n({0}sec.)".format(format(duration, '.3g')))

    if False:
      create_and_display_pie_chart(duration_list[::-1],  # reverse the list to get
                                 formatted_step_name_list[::-1], # clockwise charts
                                 "Pipeline steps")

    # The barchart function has been pulled from the internet
    # the interface is fragile    
    data_dict = {}
    formatted_step_name_list = []
    for duration, step_name in zip(duration_list, step_name_list):
        formatted_step_name = step_name[1:]
        data_dict[formatted_step_name]=float(format(duration, '.3g'))
        formatted_step_name_list.append(formatted_step_name)

    dataset = [data_dict]
    data_orders = [formatted_step_name_list]
    colors = ["r","g","b","y", "c", "m"]
    if False:
      create_and_display_horizontal_bar( "Pipeline steps", 
                                 dataset, data_orders, colors)


def create_trace_plot_information(step_dict, plot_debug):   
    """
    Creates statistics timelines for all executables on all jobs
    For this the min and max timestep are found
    THen all the traces are binnen to 10 second bins.
    Missing traces are padded in with zero untill all traces have the same lenght
    This allows for a dull scatter plot of all the executables

    next all the traces for indiv nodes are added to get the 'total' load for
    a dataproduct

    A lot of information is lost in this parse step.
    This is an example what you could do with the data
    """
    poll_step_ms = 10000 * 60
    # structures to be filled in this function
    aggregate_traces = {}
    aggregate_traces['cpu'] = []
    aggregate_traces['mem'] = []
    aggregate_traces['read_bytes'] = []
    aggregate_traces['write_bytes'] = []
    aggregate_traces['cancelled_bytes'] = []
    all_traces = {}
    all_traces['cpu'] = []
    all_traces['mem'] = []
    all_traces['read_bytes'] = []
    all_traces['write_bytes'] = []
    all_traces['cancelled_bytes'] = []

    # We first need to get the min and max timestamp
    max_time_stamp = 0
    min_time_stamp = 9999999999999 # insanely large timestamp know larger then in the stats (somewhere aound 300 years in the future)

    if not step_dict.has_key('jobs'):
        time_stamps = []
        return time_stamps, all_traces, aggregate_traces

    for id, node_dict in step_dict['jobs'].items():      # the node level information        
        for id, pid_dict in node_dict['traces'].items(): # traces of the actual executables
             if len(pid_dict['trace']['timestamp']) == 0:
                continue
                
             # Timestamp is in milli seconds
             if pid_dict['trace']['timestamp'][-1] > max_time_stamp:  # check last item in the array
                  max_time_stamp = pid_dict['trace']['timestamp'][-1]

             if pid_dict['trace']['timestamp'][0] < min_time_stamp:
                  min_time_stamp = pid_dict['trace']['timestamp'][0]


    # floor all the timestamps to the nearest whole 10 min  mark 
    #( the exact time does not matter for the stats)
    # this allows us to create additional step inserts if needed
    # first the min and max to allow us to pad
    min_time_stamp = (min_time_stamp / poll_step_ms) * poll_step_ms   
    max_time_stamp = (max_time_stamp / poll_step_ms) * poll_step_ms

    # the time line for all data traces returned by this function
    time_stamps = [x for x in range(min_time_stamp, max_time_stamp + poll_step_ms, poll_step_ms)] # we also need the last bin range() is exclusive

    # loop the data, clean and pad.     
    for id, node_dict in step_dict['jobs'].items():      # the nodes
        # list needed to calculate the total load on a node: aggregate        
        cpu_job = [0] * len(time_stamps)
        mem_job = [0] * len(time_stamps)
        read_bytes_job = [0] * len(time_stamps)
        write_bytes_job = [0] * len(time_stamps)
        cancelled_bytes_job = [0] * len(time_stamps)

        for id2, pid_dict in node_dict['traces'].items(): # traces of the actual executables running on the node
            # 'rounding' errors might cause the binning to be non continues
            # therefore floor the first entry in the timestamp list and complete
            # the array 
            binned_time_stamps = []
            # Not all traces contain entries for the whole traced timje
            n_missing_stamps_start = 0
            n_missing_stamps_end = 0

            # A executable call might be that short that not a single trace was recorded
            if len(pid_dict['trace']['timestamp']) == 0: 
                n_missing_stamps_start = (max_time_stamp - min_time_stamp ) / poll_step_ms + 1
            else:
                # only take the first timestamp and expand after. Rounding might
                # put neighbouring samples with an additional bin between them
                first_timestamp_floored = (pid_dict['trace']['timestamp'][0] / poll_step_ms) * poll_step_ms
                binned_time_stamps = [first_timestamp_floored + idx * poll_step_ms  for idx 
                                  in range(len(pid_dict['trace']['timestamp']))]

                n_missing_stamps_start = (binned_time_stamps[0] - min_time_stamp) / poll_step_ms
                n_missing_stamps_end = (max_time_stamp - binned_time_stamps[-1]) / poll_step_ms
             
            # Now we create the time lines with padding
            cpu = []    
            mem = []
            read_bytes = []
            write_bytes = []
            cancelled_bytes = []

            # add missing entries at begin
            for idx in range(n_missing_stamps_start):
                cpu.append(0)
                mem.append(0)
                read_bytes.append(0)
                write_bytes.append(0)
                cancelled_bytes.append(0)
             
            # add the recorded timelines   
            for cpu_value in pid_dict['trace']['cpu']:
                if cpu_value > 10000:    # TODO: Why this if statement?
                    print  pid_dict['trace']['cpu']
                    raise Exception

            cpu = cpu + pid_dict['trace']['cpu']
            mem = mem + pid_dict['trace']['mem']

            # Convert the read and write to deltas, remember that the delta
            # means the length is minus 1 so add extra 0 at the start
            temp_list = pid_dict['trace']['read_bytes']  # use temp to prevent double
            # dict access
            if len(temp_list) >= 2:            # if there are enough entries to de delta ( 2 or more)
                read_bytes = read_bytes + [0] + [y-x for x, y in zip(temp_list[:-1], temp_list[1:])] 
                temp_list = pid_dict['trace']['write_bytes']  # use temp to prevent double
                write_bytes = write_bytes + [0] + [y-x for x, y in zip(temp_list[:-1], temp_list[1:])] 
                temp_list = pid_dict['trace']['cancelled_bytes']  # use temp to prevent double
                cancelled_bytes = cancelled_bytes + [0] + [y-x for x, y in zip(temp_list[:-1], temp_list[1:])] 

            else:  # just add the recorded data point (its 1 or np point)
                # TODO: it might be best to add these as deltas!!!
                read_bytes = read_bytes + pid_dict['trace']['read_bytes']
                write_bytes = write_bytes + pid_dict['trace']['write_bytes']
                cancelled_bytes = cancelled_bytes + pid_dict['trace']['cancelled_bytes']

            # add the missing bins at the end
            for idx in range(n_missing_stamps_end):
                cpu.append(0)
                mem.append(0)
                # use the last value for the read and write bites
                read_bytes.append(0)
                write_bytes.append(0)
                cancelled_bytes.append(cancelled_bytes[-1])

            # save a unique trace
            all_traces['cpu'].append(cpu)
            all_traces['mem'].append(mem)
            all_traces['read_bytes'].append(read_bytes)
            all_traces['write_bytes'].append(write_bytes)
            all_traces['cancelled_bytes'].append(cancelled_bytes)

            #check if the data parced is complete

            if len(cpu) != len(time_stamps):

                raise Exception("Length of the traces is incorrect, error parsing")
                

            # Now we aggregate all the executables on a node
            for idx,(cpu_entrie, mem_entrie, read_entrie, write_entrie, cancelled_entrie) in enumerate(zip(cpu, mem,read_bytes, write_bytes, cancelled_bytes)):
                  cpu_job[idx] += cpu_entrie
                  mem_job[idx] += mem_entrie

                  try:
                    read_bytes_job[idx] += read_entrie
                    write_bytes_job[idx] += write_entrie
                    cancelled_bytes_job[idx] += cancelled_entrie
                  except:
                    print pid_dict
                    raise BaseException


        # save the aggregate for the job trace
        aggregate_traces['cpu'].append(cpu_job)
        aggregate_traces['mem'].append(mem_job)
        aggregate_traces['read_bytes'].append(read_bytes_job)
        aggregate_traces['write_bytes'].append(write_bytes_job)
        aggregate_traces['cancelled_bytes'].append(cancelled_bytes_job)

    return time_stamps, all_traces, aggregate_traces

def create_plots_of_traces(time_stamps, all_traces, aggregate_traces,
                           super_title):

    # Clean the time stamps: start at zero and convert to seconds
    first_time_stamp = time_stamps[0]
    time_stamps = [(x - first_time_stamp) / 1000 for x in time_stamps]
    plt.suptitle(super_title, fontsize=20)
    plt.subplot(2,5,1)
    
    plt.title("CPU (% of core)")
    plt.ylabel('Traces of all processes')


    for trace in all_traces['cpu']:
        plt.plot(time_stamps, trace) 
    
    plt.subplot(2,5,2)
    plt.title("MEM (% total)")     
    for trace in all_traces['mem']:
        plt.plot(time_stamps, trace)  

    
    plt.subplot(2,5,3)
    plt.title("Read (bytes)")
    for trace in all_traces['read_bytes']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,4)
    plt.title("Write (bytes)")
    for trace in all_traces['write_bytes']:
        plt.plot(time_stamps, trace)  


    plt.subplot(2,5,5)
    plt.title("cancelled (bytes)")
    for trace in all_traces['cancelled_bytes']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,6)
    plt.ylabel('Traces of aggregate per node ')
    for trace in aggregate_traces['cpu']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,7)
    for trace in aggregate_traces['mem']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,8)
    for trace in aggregate_traces['read_bytes']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,9)
    for trace in aggregate_traces['write_bytes']:
        plt.plot(time_stamps, trace)  

    plt.subplot(2,5,10)
    for trace in aggregate_traces['cancelled_bytes']:
        plt.plot(time_stamps, trace)  
    plt.show()
   
def create_pipeline_traces_and_stat(pipeline_information):
    """

    """
    stats = {'cpu':{'max_max':0.0}, 
             'mem':{'max_max':0.0},
             'read_bytes':{'max_max':0.0},
             'write_bytes':{'max_max':0.0},
             'cancelled_bytes':{'max_max':0.0}}
    traces = {}
    idx = 0
    for key, entrie in pipeline_information.items()[1:]:  # skip first entry not a step
        # First create the traces

        if idx == 2:
            plot_debug = True
        else:
            plot_debug = False

        idx = idx + 1


        time_stamps, all_traces, aggregate_traces = create_trace_plot_information(entrie, plot_debug) 
        traces[key] = {'time_stamps':time_stamps, 
                       'all_traces':all_traces,
                       'aggregate_traces':aggregate_traces}

        statistical_traces = {}
        # use numpy to calculate some statistics
        for metric_key, node_traces in aggregate_traces.items():
            statistical_traces[metric_key] =  {}
            # TODO: The current statistical properties have a problem:
            # They are calculated on all traces, they might start delayed, due to node congestion
            # only none zero instances should be take in account.
            # possible solution is a search and replace of zero to Nan. and use the Nan save
            # statistical method for calculating the stats.
            median = np.median(node_traces, axis=0)
            statistical_traces[metric_key]['median'] = \
                median.tolist()

            min = np.amin(node_traces, axis=0)
            statistical_traces[metric_key]['min'] = \
                min.tolist()

            max = np.amax(node_traces, axis=0)
            max_max =  float(np.amax(max))
            if max_max > stats[metric_key]['max_max']:
                stats[metric_key]['max_max']= max_max

            statistical_traces[metric_key]['max'] = \
                max.tolist()

            mean = np.mean(node_traces, axis=0)
            statistical_traces[metric_key]['mean'] = \
                mean.tolist()

            # The std is not that interesting on it self, we need it to be 
            # as traces around the mean
            std =  np.std(node_traces, axis=0)
            statistical_traces[metric_key]['std_top'] = \
                np.add(mean, std).tolist()

            statistical_traces[metric_key]['std_bot'] = \
                np.subtract(mean, std).tolist()
        

        traces[key]['statistical_traces'] = statistical_traces

    return traces, stats


def create_plot_of_full_pipeline(pipeline_information, stats, 
                                 step_name_list, label_list):


    first_loop = True
    first_time_stamp = 0
    create_legend = True
    # when printing the labels for steps, they need to be ofset in hight to each other
    # to prevent that they are plotted on each other when running a short stop
    # this bool flips each step 
    label_flip_flop = True 
                
    f = plt.figure()

    # step 1, add all the information to the plots
    for (key, entrie), step_name in zip(pipeline_information.items(), step_name_list):
        if first_loop:
            first_time_stamp = entrie['time_stamps'][0]            
            first_loop = False
        
        # Clean the time stamps: take the first timestamp as the zero point
        time_stamps = [(x - first_time_stamp) / 1000 for x in entrie['time_stamps']]
  
        aggregate_traces = entrie['aggregate_traces']
        statistical_traces = entrie['statistical_traces']

        stat_list = ['cpu', 'mem', 'read_bytes', 'write_bytes', 'cancelled_bytes']
        unit_list = [' (% core)', ' (% total)', ' (bytes)', ' (bytes_)', ' (bytes)']
        for idx, (key, unit) in enumerate(zip(stat_list,
                                      unit_list)):
            aggregate = aggregate_traces[key]
            statistical = statistical_traces[key]
            max_max = stats[key]['max_max']

            # plot all the node traces
            ax = plt.subplot(5,1,idx + 1) # subplot index starts at 1       
                     
            plt.ylabel(key + unit)

            # Plit the traces for each each node (not individual pids)
            for trace in aggregate:
                line1 = ax.plot(time_stamps, trace, color='y') 

            #plot the max in red, the mean in black and the std in black dotted
            #line2 = ax.plot(time_stamps, statistical['max'], color='r') 
            #line3 = ax.plot(time_stamps, statistical['min'], color='g') 
            line4 = ax.plot(time_stamps, statistical['mean'], color='k', linewidth=2.0) 
            #line5 = ax.plot(time_stamps, statistical['std_top'], color='k', linestyle='--' ) 
            #line5 = ax.plot(time_stamps, statistical['std_bot'], color='k', linestyle='--' ) 
            

            # plot lines demarking the start of the individual step names
            plt.plot([time_stamps[0], time_stamps[0]],[0, max_max], color='k', linestyle='--')
            if idx == 0:                    # only print labels above the top trace
                if step_name in label_list:  # Only print names in the list  
                    if label_flip_flop:
                        ax.text(time_stamps[0], max_max, '\n\n' + step_name)     
                        label_flip_flop = not label_flip_flop
                    else:
                        ax.text(time_stamps[0], max_max,  step_name + '\n\n')     
                        label_flip_flop = not label_flip_flop
            
            # print the legend only once
            if create_legend and idx == 4:  # 4 is the botom plot. Used for cancelled bytes. Should not happen that often. Is the least important plot
                line1[0].set_label('Node traces')
               # line2[0].set_label('Max utilization')
               # line3[0].set_label('Min utilization')
                line4[0].set_label('Mean utilization')
                #line5[0].set_label('Mean +- 1 std')
                create_legend = False

    # make up the plot 
    plt.suptitle("Aggregated performance plots for a full pipeline", fontsize=20)
    plt.subplots_adjust(hspace=0.001)   # Set the spacing between the plot to almost zero
    ax1 = plt.subplot(5,1,1)
    ax1.yaxis.get_major_formatter().set_powerlimits((1, 4))
    ax2 = plt.subplot(5,1,2)
    ax2.yaxis.tick_right()
    ax2.yaxis.get_major_formatter().set_powerlimits((1, 4))
    ax3 = plt.subplot(5,1,3)
    ax3.yaxis.get_major_formatter().set_powerlimits((1, 4))
    ax4 = plt.subplot(5,1,4)
    ax4.yaxis.get_major_formatter().set_powerlimits((1, 4))
    ax4.yaxis.tick_right()
    
    ax5 = plt.subplot(5,1,5)
    ax5.yaxis.get_major_formatter().set_powerlimits((1, 4))
    ax5.xaxis.get_major_formatter().set_powerlimits((1, 4))
    min_max_x = ax5.xaxis.get_data_interval()
    # plot ticks at hours
    #print min_max_x
    #print type(min_max_x)
    #print min_max_x.shape

    #print min_max_x[0]
    #print min_max_x[1]
    #raise Exception

    # create x-axis labels: print each hours mark
    tick_step = 7200.0
    hr_multi =  round(7200.0 / 3600)
    xtick_range = np.arange(min_max_x[0], min_max_x[1] + 1, tick_step)
    xtick_labels = []
    for tick_value in xtick_range:
        xtick_labels.append(str((tick_value / tick_step) * hr_multi))

    ax5.xaxis.set_ticks(xtick_range)
    ax5.xaxis.set_ticklabels(xtick_labels)

    plt.xlabel('Time (Hrs)')
    #ax5.legend()  # The legend is only created for the bottom plot
    ax5.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15),
          fancybox=True, shadow=True, ncol=5)



    # remove xlabel of the first 4 plots
    xticklabels = plt.subplot(5,1,1).get_xticklabels()+plt.subplot(5,1,2).get_xticklabels()+ \
                 plt.subplot(5,1,3).get_xticklabels()+plt.subplot(5,1,4).get_xticklabels()
    plt.setp(xticklabels, visible=False)

    plt.show()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()
        exit(1)
    
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

    # Create all the traces including statistical traces of all the staps
    traces, stats = create_pipeline_traces_and_stat(pipeline_information)

    
    # list of labels to print: THe label is cluttered, only plot
    # the interesting ones
    label_list = ['selfcal_prepare', 'selfcal_prepare', 'selfcal_bbs', 'selfcal_awimager']
    # create and plot
    create_plot_of_full_pipeline(traces, stats, step_name_list, label_list)
    exit(0)

    
    #print step specific_traces
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[1]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "prepare")
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[2]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "db create")
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[3]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "bbs")
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[4]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "awimager")
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[5]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "sourcefinding")
    #time_stamps, all_traces, aggregate_traces = create_trace_plot_information(pipeline_information[6]) 
    #create_plots_of_traces(time_stamps, all_traces, aggregate_traces, "finalize")


        