import xml.dom.minidom as _xml

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
