import time   # allows stand alone usage
import xml.dom.minidom as _xml
from pipelinexml import add_child, get_child
import lofaringredient as ingredient
from lofarexceptions import PipelineException
from utilities import create_directory
import traceback
import sys
import inspect
import types
import os


def _enter_active_stack_node(calling_object,
            active_stack_node_name, child):
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

    # if child is a string add a xml node with this name
    stacked_child = None
    if isinstance(child, basestring):
        stacked_child = add_child(active_stack_node, child)
    # else try adding it as a node
    elif isinstance(child, _xml.Node):
        active_stack_node.appendChild(child)
        stacked_child = child
    return stacked_child


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

def xml_node(target):
    """
    function decorator to be used on member functions of the pipeline
    classes: It adds info  to the (xml node) class attribute active_xml.
    Creating this attribute if it not exist, allowing fire and forget
    usage.
    Subsequent usage of this logger decorator in nested function will result
    in a nested xml structure.
    The created data well need to be processed separately:
    displaying is not part of this logger
    """
    def wrapper(*args, **argsw):
        calling_object = args[0]

        xml_current_node = _enter_active_stack_node(
                    calling_object, 'active_xml', target.__name__)

        t1 = time.time()
        # call the actual function

        return_value = target(*args, **argsw)
        # end time
        t2 = time.time()
        # add the duration
        xml_current_node.setAttribute("duration", str(t2 - t1))

        _exit_active_stack_node(calling_object, 'active_xml')

        return return_value

    return wrapper


def add_node_to_current_active_stack_node(calling_object,
                             name_of_stack, xml_node):
    _enter_active_stack_node(
                    calling_object, name_of_stack, xml_node)

    _exit_active_stack_node(calling_object, name_of_stack)



def _add_error_info_to_node(node, locals=None, exception_string=None):
    local_document = _xml.Document()
    error_node = add_child(node, "Debug_info")

    #add local variables
    if exception_string:
        except_node = local_document.createElement("Traceback")
        #raise Exception(' '.join(exception_string))
        except_textnode = local_document.createTextNode(
            ' '.join(exception_string))
        except_node.appendChild(except_textnode)
        error_node.appendChild(except_node)

    #add local variables
    if locals:
        local_node = local_document.createElement("locals")
        locals_textnode = local_document.createTextNode(str(locals))
        local_node.appendChild(locals_textnode)
        error_node.appendChild(local_node)

    #add global variables
    global_node = local_document.createElement("globals")
    globals_textnode = local_document.createTextNode(str(globals()))
    global_node.appendChild(globals_textnode)

    error_node.appendChild(global_node)


def node_xml_decorator(*args_decorator, **argsw_decorator):
    """
    The node_xml_decorator enables the xml mechanism for the pipeline
    frame work. 
    It adds timeing logging.
    Parsing of log files for time logging
    Arguments are passed using xml
    optional decorator arguments allow the streaming if parameterset files 
    """
    class internal_class(object):
        """
        ***Gory Implementation details***
        This decorator class allows the retention of decorator arguments.
        On initialization it gets the decorator arguments and retains these

        This class contains two important functions:
        1. internal_node_xml_decorator is the "actual" runtime decorator that
        is assigned to run() explanation:
        node_xml_decorator is a decorator with arguments:
        it is a function that creates a class based on the retrieved arguments
        and it returns the actual runtime decorator function.
        internal_node_xml_decorator has as argument function (run for nodes)
        It saves the function and return the internal_class member function
        wrapper

        2. The actual runtime decorator function is wrapper.
        wrapper is a member function and  self == internal_class
        This is problematic because this means that we do not have
        access to the actual node recipe object. But we need this if we
        want to call run as a member function of the node recipe.
        This problem is solved in LOFARnode.run_with_logging
        It is at this place that either run() is called -or- the 
        decorated run function.
        If run is not decorated the normal functionality is used.
        If we know that it is decorate: the name of 'run' is than
        actually 'wrapper'. We add the node_recipe object as args[0]
        Now it is possible to call the saved run function on wrapper
        with self as first argument: mimiking member functionality:
        The run function does not know that run was decorated while
        ***Gory Implementation details*** 
        """
        def __init__(self, *args_decorator, **argsw_decorator):
            # save the decorator arguments
            self.args_decorator = args_decorator
            self.argsw_decorator = argsw_decorator

        def internal_node_xml_decorator(self, function):
            # Save the decorated function
            self.function = function
            return self.wrapper

        def wrapper(self, *args, **argsw):
            input_xml_string = args[-1]
            input_node = _xml.parseString(input_xml_string).documentElement
            args = args[:-1]
            calling_object = args[0]

            # Add the input_node to the node recipe object
            calling_object.input_xml = input_node

            # ***************************************************
            # convert the parameter node to an argument dict
            # if it is a streamed (config) file: write the
            # the config file and insert the new location
            argument_node = get_child(input_node, "run_arguments")
            arg_dict = {}
            arg_files = []
            for child in argument_node.childNodes:
                config_file_node = get_child(
                    get_child(input_node, "config_files"), str(child.nodeName))
                # Try casting to python value: Used for internal defaults: 
                # int, float, etc.
                try:
                    value_at_master_script = eval(str(
                                     child.getAttribute("value")))
                except:
                    value_at_master_script = str(
                                     child.getAttribute("value"))

                # If the argument is not in the streamed list
                if config_file_node == None:
                    # cast the unicode strings to strings
                    arg_dict[str(child.nodeName)] = value_at_master_script
                else:
                    # open new file to write the config data to
                    config_file_as_string = config_file_node.firstChild.data
                    # TODO fixme: We need the working directory on all recipe
                    # scripts
                    working_dir = "/data/scratch/klijn"
                    recipe_name = calling_object.__class__.__name__
                    new_path_for_config_file = os.path.join(working_dir,
                            recipe_name,
                            os.path.basename(value_at_master_script))
                    # Check for possible duplicate parset names,
                    # if found use the whole original filename
                    if new_path_for_config_file in arg_files:
                        new_path_for_config_file = os.path.join(
                            working_dir, recipe_name,
                            value_at_master_script.replace('/', "_") #.replace('\',"_")  # Allow windows usage!!
                            )
                    arg_files.append(new_path_for_config_file)

                    # assure existence of the directory to write the files
                    create_directory(os.path.join(working_dir,
                            recipe_name))

                    # open the file and write the parset file
                    fp = open(new_path_for_config_file, "w")
                    fp.write(config_file_as_string)
                    fp.close()
                    config_file_node.setAttribute("path",
                                                   new_path_for_config_file)

                    # procide this new file to the run function
                    arg_dict[str(child.nodeName)] = new_path_for_config_file
            # ***************************************************



            # Create an output node and enter stacked timing logging
            local_document = _xml.Document()
            output_xml = local_document.createElement("output")
            xml_current_node = _enter_active_stack_node(
                        args[0], 'active_xml', self.function.__name__)
            output_xml.appendChild(calling_object.active_xml)
            t1 = time.time()

            return_value = None
            exception = False
            try:
                # call the actual function: PYTHON FOO
                # function is a ref to run on the node script.
                # calling_object is the instantiated object: provide as the
                # first argument: we now have normal member behaviour
                return_value = self.function(calling_object, **arg_dict)

            except PipelineException, e:
                # if we have an exception: collect stack trace information,
                type_, value_, traceback_ = sys.exc_info()
                trace_back = traceback.format_exception(type_, value_, traceback_)

                _add_error_info_to_node(output_xml, e._locals, trace_back)
                exception = True
                return_value = 1

            # finalize the duration
            t2 = time.time()
            xml_current_node.setAttribute("duration", str(t2 - t1))
            xml_current_node.appendChild(input_node)
            _exit_active_stack_node(args[0], 'active_xml')

            # on error add additional debugging info, exception already does this
            if return_value == 1 and not exception:
                _add_error_info_to_node(output_xml)

            # return the full output as xml in return argument
            calling_object.outputs['xml'] = output_xml.toxml(
                                            encoding='ascii')
            return return_value

    local_wrapper = internal_class(*args_decorator, **argsw_decorator)
    # else return the actual decorator ( which will receive function)
    return local_wrapper.internal_node_xml_decorator


def _import_path(path_filename):
    """
    Import a file with full path specification. Allows one to
    import from anywhere, something __import__ does not do.

    !!!DANGER: HERE BE DRAGONS!!!
    """
    path, filename = os.path.split(path_filename)
    filename_noext, ext = os.path.splitext(filename)
    sys.path.append(path)
    module = __import__(filename_noext)
    reload(module)  # Might be out of date
    del sys.path[-1]
    return module, filename_noext


def master_node_xml_decorator(target):
    """
    This class is used to decorate RemoteCommandRecipeMixIn._schedule_jobs
    It does a 'look ahead' in the sourcecode of the called node recipe and
    scans for a node_xml_decorator.
    If this is found:
    1. The argument names of the run function in the node recipe
    are collected and used to transfer the arguments as xml data.
    If the wrapper is not found the compute job is left as is.
    2. 
    """
    def wrapper(*args, **argsw):
        # Runtime add the jobs for this recipe to the master object
        # Noded for later parsing of the output data
        calling_master_recipe_object = args[0]
        calling_master_recipe_object._jobs_for_xml = args[1]

        # WARNING MAJOR PYTHON FOO AHEAD:
        for compute_job in args[1]:
            command = compute_job.command
            # Get the node recipe and the filename
            node_recipe_as_module, filename_noext = _import_path(
                            command.strip().split(" ")[1])  # python <node.py>

            # Get the class description ( this is still 'source code')
            node_recipe_class = getattr(node_recipe_as_module, filename_noext)

            # PYTHON FOO: node_recipe_class.run returns the run function
            # If this is None, we got a source code object which means
            # it is a non decorated function. In this case do nothing
            node_recipe_wrap_object = node_recipe_class.run.im_self
            if node_recipe_wrap_object != None:
                #raise Exception(dir(node_recipe_wrap_object))
                master_recipe_object = args[0]
                # We are in the wrapper object: the attribute function
                # now contains the run function 
                # Get the arguments for the run function
                function_argspec = inspect.getargspec(
                                node_recipe_wrap_object.function)
                # the argument names are on the first array index
                # skip the self argument (it is a member function)
                function_argument_names = function_argspec[0][1:]

                # xml_node: will be send to the node recipe
                xml_document = _xml.Document()
                input_xml = xml_document.createElement("input_xml")

                # ********************************************************
                # Now create a node with all the arguments as value: string
                run_arguments_node = add_child(input_xml, "run_arguments")
                for name, value in zip(function_argument_names,
                            compute_job.arguments):
                    argument_node = add_child(run_arguments_node, name)
                    argument_node.setAttribute("type", str(type(value)))
                    argument_node.setAttribute("value", str(value))
                # ********************************************************

                # ********************************************************
                # Create xml versions of the stream-able parameters:
                # config files, these are streamed and recreated at the
                # node side
                decorator_arguments = node_recipe_wrap_object.argsw_decorator
                streamed_node = add_child(input_xml, "config_files")
                for config_file_argument in decorator_arguments['config_files']:
                    # open the file: value is part of run_arguments_node
                    config_file_path = get_child(run_arguments_node,
                             config_file_argument).getAttribute("value")
                    fp = open(config_file_path)
                    file_as_string = fp.read()
                    # convert to xml text node
                    file_as_text_node = xml_document.createTextNode(
                                                file_as_string)
                    # add the text node
                    add_child(streamed_node, config_file_argument).appendChild(
                                        file_as_text_node)
                # *************************************************


                # add the now created argument xml as variable to the
                # compute job
                compute_job.arguments.append(
                            input_xml.toxml(encoding="ascii"))

        # call the actual function. the jobs in args are unchanged
        # if no decorator is found. And the decorator does nothing
        return_value = target(*args, **argsw)
        return return_value

    # return the wrapped function
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

        # Create containing document to store node xml data
        xml_document = _xml.Document()
        root_node = xml_document.createElement("Nodes")

        # Collect and add timing info information from the nodes
        for job in calling_object._jobs_for_xml:

            if "xml" in job.results:

                full_xml = _xml.parseString(job.results['xml']).documentElement
                #raise Exception(job.results['xml'])
                node_xml = get_child(full_xml,
                        'active_xml')
                node_job_info = add_child(root_node, job.host)
                run_xml = get_child(node_xml, 'run')

                # remove 'noise': active stack and 'expected' info:
                # parse specific data objects
                node_job_info.setAttribute("duration",
                                    run_xml.getAttribute('duration'))

                node_job_info.appendChild(
                        get_child(
                            get_child(node_xml, 'run')
                            , 'input_xml'))

                for child_node in run_xml.childNodes:
                    node_job_info.appendChild(
                                    child_node.cloneNode(deep=True))

                if (not return_value == 0):
                    debug_info = get_child(full_xml, 'Debug_info')
                    node_job_info.appendChild(debug_info.cloneNode(deep=True))
                    #raise Exception(debug_info.toprettyxml())

        # DANGER!! HERE BE DRAGONS:
        # directly update the output internal dict with return_xml entry
        calling_object.outputs._fields["return_xml"] = ingredient.StringField(
        help="XML return data.")

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


