import xml.dom.minidom as _xml
from pipelinexml import add_child, get_child
import lofaringredient as ingredient
from lofarexceptions import PipelineException

def _enter_active_stack_node(calling_object, active_stack_node_name,
                             child_name):
    """
    """

    stack_name = "active_stack"
    active_stack_node = None
    stack_node = None
    # Check if the calling object has a active stack node with
    # name ==  active_stack_node_name
    if not hasattr(calling_object, active_stack_node_name):
        # Create the xml node if it not exists
        _throw_away_document = _xml.Document()
        stack_node = \
            _throw_away_document.createElement(active_stack_node_name)

        # The xml name of the object is the calling object
        stack_node.setAttribute("Name", calling_object.__class__.__name__)
        stack_node.setAttribute("type", "active_stack_node")
        # assign the node to the calling class as an attribute
        calling_object.__setattr__(active_stack_node_name, stack_node)

        # add the 'call stack'
        active_stack_node = add_child(stack_node, stack_name)  # generiek
    else:
        stack_node = calling_object.__getattribute__(active_stack_node_name)
        # Find the active stack
        for child_node in stack_node.childNodes:
            if child_node.nodeName == stack_name:
                active_stack_node = child_node
                break
        if active_stack_node == None:
            active_stack_node = add_child(stack_node, stack_name)

    named_stacked_child = add_child(active_stack_node, child_name)
    return named_stacked_child


def _exit_active_stack_node(calling_object, active_stack_node_name):
    # get the named active stack node
    active_stack_node = calling_object.__getattribute__(
                                             active_stack_node_name)
    stack_name = "active_stack"

    # get the active stack
    active_stack = None
    for child_node in active_stack_node.childNodes:
        if child_node.nodeName == stack_name:
            active_stack = child_node
            break

    # Get the current last item in the stack
    last_child = active_stack.lastChild
    # remove it
    active_stack.removeChild(last_child)

    # Now 'log' the now 'finished' step
    if active_stack.lastChild == None:
        # add to the main time_logger node
        active_stack_node.appendChild(last_child)
    else:
        # add to the calling node info
        active_stack.lastChild.appendChild(last_child)

def timing_logger(target):
    """
    function decorator to be used on member functions of the pipeline
    classes: It adds info  to the (xml node) class attribute timing_info.
    Creating thos attribute if it not exist, allowing fire and forget
    usage.
    Subsequent usage of this logger decorator in nested function will result
    in a nested xml structure.
    The created data well need to be processed separately:
    displaying is not part of this logger
    """
    def wrapper(*args, **argsw):
        import time  #allows stand alone usage
        calling_object = args[0]

        time_info_current_node = _enter_active_stack_node(
                    calling_object, 'timing_info', target.__name__)

        t1 = time.time()
        # call the actual function
        return_value = target(*args, **argsw)
        # end time
        t2 = time.time()
        # add the duration
        time_info_current_node.setAttribute("duration", str(t2 - t1))

        _exit_active_stack_node(calling_object, 'timing_info')

        return return_value

    return wrapper

def add_node_to_current_active_stack_node(calling_object, name_of_stack, xml_node):
    time_info_current_node = _enter_active_stack_node(
                    calling_object, name_of_stack, "External_source")

    time_info_current_node.appendChild(xml_node)

    _exit_active_stack_node(calling_object, name_of_stack)


### EXPERIMENTAL CODE!!!
def node_xml_decorator(target):
    """
    Decorator for a single function (run) in the node recipe:
    It adds xml logging capability without influencing the current
    recipe structure
    TODO: This decorator needs to be more general: There is more information
    passing here: detailed parset info, AND, maybee the actual parameters
    """
    def wrapper(*args, **argsw):
        import time   #allows stand alone usage
        calling_object = args[0]

        time_info_current_node = _enter_active_stack_node(
                    calling_object, 'timing_info', target.__name__)

        t1 = time.time()
        # call the actual function
        return_value = target(*args, **argsw)
        # end time
        t2 = time.time()
        # add the duration
        time_info_current_node.setAttribute("duration", str(t2 - t1))

        _exit_active_stack_node(calling_object, 'timing_info')

        # Write away the complete 'xml stack'
        local_document = _xml.Document()
        created_node = local_document.createElement("output")
        created_node.appendChild(calling_object.timing_info)
        calling_object.outputs['xml'] = created_node.toxml(encoding='ascii')


        return return_value

    return wrapper


def master_xml_decorator(target):
    """
    Decorator for a single function (go) in the node recipe:
    It adds xml logging capability without influencing the current
    recipe structure

    TODO: This decorator needs to be more general: There is more information
    passing here: detailed parset info, AND, maybee the actual parameters
    ATM. this is a proof of concept xml tree/ decorator mechanism allowing
    detailed master node level communcation!!!!
    """
    def wrapper(*args, **argsw):
        calling_object = args[0]

        # call the actual function and save the return argument
        return_value = target(*args, **argsw)

        # Create containing doc to store node xml data
        xml_document = _xml.Document()
        root_node = xml_document.createElement("Nodes")

        # Get the actual information from the nodes
        for job in calling_object.jobs:
            if "xml" in job.results:
                #raise Exception(job.results['xml'])
                node_xml = get_child(_xml.parseString(
                        job.results['xml']).documentElement,
                        'timing_info')
                node_timeinfo = add_child(root_node, job.host)
                run_xml = get_child(node_xml, 'run')

                # remove 'noise': active stack and 'expected' info
                node_timeinfo.setAttribute("duration",
                                    run_xml.getAttribute('duration'))

                for child_node in run_xml.childNodes:
                    node_timeinfo.appendChild(
                                    child_node.cloneNode(deep=True))

        # DANGER!! HERE BE DRAGONS:
        # directly update the output internal dict with return_xml entry
        calling_object.outputs._fields["return_xml"] = ingredient.StringField(
        help="Full path of mapfile; containing the succesfull generated"
            )

        # now assign the actual return xml
        calling_object.outputs["return_xml"] = root_node.toxml(
                                                    encoding='ascii')

        return return_value

    return wrapper


def _process_time_log(timer_output):
    """
    To much at once smell:
    regex matching and xml creation. Dont so a real easier way though
    """
    import re
    # First create the reg ex matcher that should
    # be able to competely match all lines in the received timer_output
    white_line_matcher = re.compile("^\s*$")

    # example: Total NDPPP time      59.48 real      214.52 user       10.92 system 
    log_header_matcher = re.compile(".*Total (.+) time"   #any char + space+ 'Total' + space + 'time' 
                                    ".+?([0-9]+\.[0-9]*)"  # nongready chars + numbers + dot + numbers
                                    ".+?([0-9]+\.[0-9]*)"
                                    ".+?([0-9]+\.[0-9]*)")

    # example: 13.4% AOFlagger aoflag.
    head_entry_matcher = re.compile("^\s*"  # white space
                                    "([0-9]+\.[0-9]*)%"  # a float + %
                                    "\s*(.*)")  # whitespace + 'name'

    # example: 22.2% of it spent in making quality statistics
    sub_entry_matcher = re.compile("^\s*"  # white space
                                    "([0-9]+\.[0-9]*)% of it spent in " # a float + %
                                    "(.*)")  # 'name'

    executable_log_node = None
    latest_child_node = None  # latest child node used for subsystem info
    # now process the lines
    for line in timer_output:
        #if the current line contains only whitespace or is empty, continue
        match_object = white_line_matcher.match(line)
        if match_object != None:
            continue

        # try matching with the log header
        match_object = log_header_matcher.match(line)
        if match_object != None:
            xml_document = _xml.Document()
            executable_log_node = xml_document.createElement(
                            match_object.group(1).replace (" ", "_"))
            executable_log_node.setAttribute("real_time",
                                             match_object.group(2))
            executable_log_node.setAttribute("user_time",
                                             match_object.group(3))
            executable_log_node.setAttribute("system_time",
                                             match_object.group(4))
            continue

        # If we get here with out reading the header line: abort 
        if executable_log_node == None:
            raise PipelineException(
                "encountered non whitespace line before the header node."
                " Invalid log")

        # sub-entries, before the head entry cause head entry matches with subentry
        match_object = sub_entry_matcher.match(line)
        if match_object != None:
            child_node = add_child(latest_child_node,
                        match_object.group(2).replace (" ", "_"))
            child_node.setAttribute("percentage", match_object.group(1))
            continue

        # match with a head entry
        match_object = head_entry_matcher.match(line)
        if match_object != None:
            child_node = add_child(executable_log_node,
                        match_object.group(2).replace (" ", "_"))
            latest_child_node = child_node
            child_node.setAttribute("percentage", match_object.group(1))
            continue

        # if a line ends up here it could not be parsed: raise
        # exception with un parsed line
        raise PipelineException(
            "process time log, received a line which could not"
            " be parsed:\n {0}".format(line))

    return executable_log_node


def scan_file_for_timing_info(in_file):
    import re
    # regular expressions matcher for different elements in the log
    start_matcher = re.compile("^{0}.*".format("Start timer output"))
    end_matcher = re.compile("^{0}.*".format("End timer output"))

    timer_output = []
    in_timer_output = False
    for line in in_file.readlines():
        # if we have previously found the start of a timelog
        if in_timer_output:
            # if the endblock is found
            if end_matcher.match(line):

                # process all the lines in the logtextblok
                # return the xml for logblock
                return _process_time_log(timer_output)
                in_timer_output = False

            # save the current line as part of the log block
            timer_output.append(line)

        # regex for the start of log block
        elif start_matcher.match(line):
            in_timer_output = True
        #else: Not in timelog, just keep consuming lines

    # raise exception if eof while still in timelog block
    if in_timer_output:
        raise PipelineException("Found the start of an timer output blok, but no end")

