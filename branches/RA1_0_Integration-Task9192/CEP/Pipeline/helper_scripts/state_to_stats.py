#                                                      LOFAR PIPELINE FRAMEWORK
#
#                                                          state_to_stats
#                                                           Wouter Klijn, 2014
#                                                               klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import xml.dom.minidom as xml
import pickle

def usage():
    usage_string = """
    state_to_stats converts a statefile for a partially succesfull run into
    a valid resource usage stats file to be visualized

    usage: python state_to_stats.py <path_of_state_file> <output_path_of_stats>
    """

    print usage_string


def open_file_and_parse_to_python_data(path):
    """
    Opens the state file and parses it to a valid xml dom

    """
    try:

        f = open(path)
        data = pickle.load(f)

    except:
        print "failed opening statefile: "
        print path
        exit(1)

    return data



if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage()
        exit(1)

    state_path     = sys.argv[1]  # where to find the state file
    xml_stats_path = sys.argv[2]  # where to output the created stats file

    data = open_file_and_parse_to_python_data(state_path)

    # mirror the normal statefile
    local_document = xml.Document()
    xml_node = local_document.createElement("active_stack")  
    xml_node.setAttribute("duration", "0")
    xml_node.setAttribute("type", "Manual_Created_statistics")   
    
    # add a fake active stack (needed for later processing steps)
    # This allows us to use the visualization script without
    # aditional arguments
    local_document = xml.Document()                
    step_node = local_document.createElement("active_stack")
    step_node.setAttribute("type", "active_stack") 
    xml_node.appendChild(step_node)

    for entry in data[1]:
        # parse the name and create a xml_node with this name
        step_name = entry[0]

        print "processing step: {0}".format(step_name)
        local_document = xml.Document()                
        step_node = local_document.createElement(step_name)
        step_node.setAttribute("duration", "0")
        step_node.setAttribute("type", "active_stack")

        # collect the statistics
        step_xml_string = xml.parseString(entry[1]['return_xml']).documentElement  #

        # append to the newly create node
        step_node.appendChild(step_xml_string)

        # Save to the 'large' xml tree
        xml_node.appendChild(step_node)
        #print step_xml_string.toprettyxml()

    
    f = open(xml_stats_path, 'w')
    f.write(xml_node.toxml())
    print "wrote file: "
    print xml_stats_path 


    



