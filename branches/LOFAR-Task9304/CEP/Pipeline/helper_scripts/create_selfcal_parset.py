import sys

def open_and_parse_config_file(file_location):
    """
    This script uses the most simple parameter parsing possible.
    If you enter incorrect settings it will raise an exception, tough luck!!
    """    
    config_dict = {}
    # *********************
    # Open the file containing parameters, remove comment, empty lines
    # and end of line comments
    raw_lines = open(file_location, 'r').read().splitlines()

    lines = []
    for line in raw_lines:
        if len(line) == 0:   # empty lines
            continue

        if line[0] == '#':  # skip lines starting with a #
            continue

        # use strip to remove oel and trailing white space
        line_without_comment = line.split('#')[0].strip()     
        lines.append(line_without_comment)
  

    # *********************
    # new_obs_name, first entry
    new_obs_name = ""
    try:

        new_obs_name = lines[0]  
    except:
        print "Tried parsing:"
        print  lines[0]
        print "this failed"
        raise ImportError

    config_dict["new_obs_name"] = new_obs_name

    # *********************
    # second line, output path 
    output_path = ""
    try:
        output_path = lines[1]  # just read as a python list
    except:
        print "Tried parsing:"
        print  lines[1]
        print "this failed"
        raise ImportError

    config_dict["output_path"] = output_path

    # *********************
    # second line, list of the nodes to run the pipeline on 
    node_list = []
    try:
        node_list = eval(lines[2])  # just read as a python list
    except:
        print "Tried parsing:"
        print  lines[2]
        print "this failed"
        raise ImportError

    if len(node_list) == 0:
        print "opened node list did not contain any entries!\n\n"  
        raise ImportError

    config_dict["node_list"] = node_list



    # *********************
    # number of major cycles
    number_of_major_cycles = ""
    try:
        number_of_major_cycles = int(eval(lines[3]))  
    except:
        print "tried parsing:"
        print  lines[3]
        print "this failed"
        raise importerror

    config_dict["number_of_major_cycles"] = number_of_major_cycles

    return config_dict


def open_and_convert_parset(parset_file):
    """
    Open the parset file. Parse all the entries, clean up comments and
    then file a dict with the key values pairs
    """
    parset_as_dict = {}
    lines = open(parset_file, 'r')

    for idx, line in enumerate(lines):
        if len(line) == 0:
            continue

        if line[0] == '#':  # skip lines starting with a #
            continue

        line_without_comment = line.split('#')[0]
        key, value = line_without_comment.split("=")                 
        parset_as_dict[key.strip()] = value.strip()  # use strip to remove oel

    return parset_as_dict


def create_output_lists(config_dict):
    """
    Based on the information in the parst config_dict use string foo to 
    create the correct location and filename string lists
    """
    filenames_h5 = []
    filenames_ms = []
    locations    = []
    for idx, node in enumerate(config_dict["node_list"]):
        # LIst of file names
        filename_single = (config_dict["new_obs_name"] + "_" + str(idx) + ".H5")
        filenames_h5.append(filename_single)

        filename_single = (config_dict["new_obs_name"] + "_" + str(idx) + ".MS")
        filenames_ms.append(filename_single)

        location_single = (node + ":"  + config_dict["output_path"] + config_dict["new_obs_name"] + "/")
        locations.append(location_single)
        

    return filenames_h5, filenames_ms, locations


def add_output_lists_to_parset(parset_as_dict, filenames_h5, 
                               filenames_ms, locations, number_of_major_cycles):
    """
    Insert the newly create lists into the parset dict using correct
    HARD CODED key vlues
    """
    parset_as_dict["ObsSW.Observation.DataProducts.Output_SkyImage.filenames"] =  repr(filenames_h5)

    parset_as_dict["ObsSW.Observation.DataProducts.Output_SkyImage.locations"] =  repr(locations)

    parset_as_dict["ObsSW.Observation.DataProducts.Output_Correlated.filenames"] = repr(filenames_ms)
    
    parset_as_dict["ObsSW.Observation.DataProducts.Output_Correlated.locations"] = repr(locations)

    # Grab the skip list from the output skyimage and add it with the correct c
    # correlated key
    parset_as_dict["ObsSW.Observation.DataProducts.Output_Correlated.skip"] = parset_as_dict["ObsSW.Observation.DataProducts.Output_SkyImage.skip"]

    parset_as_dict["ObsSW.Observation.ObservationControl.PythonControl.Imaging.number_of_major_cycles"] = str( number_of_major_cycles)

    




def output_parset_to_file(parset_as_dict_of_string_to_string, 
                          output_parset_path):
    """
    Print the parset dict to file.
    Use a sorted version of the keys for itterating in the dict. This results
    in the same order of the entries as produced by SAS/MAC
    """   
    sorted_keys = sorted(parset_as_dict_of_string_to_string.keys())

    file_ptr = open(output_parset_path, 'w')
    for key in sorted_keys:
        file_ptr.write(key + "=" + parset_as_dict_of_string_to_string[key] + "\n")

    file_ptr.close()


def basic_validity_ok(locations, parset_as_dict_of_string_to_string):
    """
    Performs a very basic (set of) test (s) to check if the created output 
    parset would make any sense 
    """
    skip_list_as_string = parset_as_dict_of_string_to_string["ObsSW.Observation.DataProducts.Output_SkyImage.skip"]
    skip_list = eval(skip_list_as_string)

    # now check if the lenght is the same, if not the config does not match the parset
    if len(skip_list) != len(locations):
        print "The length of the skip list in the provided parset"
        print "is not the same as the number of dataproduct specified in the config\n"
        print "aborting, NO output parset written!"

        return False

    return True


def usage():
    print """"***************************
    usage: python create_selfcal_parset.py <config_file> <parset_file> <output_parset_path>

    create_selfcal_parset is a script which creates a 'new' selfcal parset based on 
    a valid imaging pipeline parset. 
    It takes a config file and applies the information contained in there on
    the parset_file data outputting it to the output_parset_path

    The config file allows the controling of the locus nodes and the output
    observation ID. It also allows the settings of the number of major cycles.   
    See config file for syntax

    output_parset_path will be overwritten without any prompth
    ****************************************
    """

if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
        exit(1)

    # Quick and dirty parsing of the parameters could use tooling but it is a 
    # one of script
    config_file        = sys.argv[1]
    parset_file        = sys.argv[2]
    output_parset_path = sys.argv[3]

    # Single function attempting to parse the parameters minimal error checking
    config_dict = open_and_parse_config_file(config_file)
    # get the parset in a format we can easyly change
    parset_as_dict_of_string_to_string = open_and_convert_parset(parset_file)

    # Create the new list with output location
    filenames_h5, filenames_ms, locations = \
      create_output_lists(config_dict)

    # Very basic check if what was specified is correct

    if not basic_validity_ok(locations, parset_as_dict_of_string_to_string):
        exit(1)     

    # Add them to the parset, 
    add_output_lists_to_parset(parset_as_dict_of_string_to_string, filenames_h5, 
                               filenames_ms, locations, 
                               config_dict["number_of_major_cycles"])

    # print as valid parset
    output_parset_to_file(parset_as_dict_of_string_to_string, 
                          output_parset_path)

    exit(0)    
