"""
xml based logging constructs and helpers functions
"""
import xml.dom.minidom as _xml


def add_child(head, name):
    """
    Create a node with name. And append it to the node list of the supplied
    node. (This function allows duplicate head names as specified by xml)
    return the create node.
    """
    local_document = _xml.Document()
    created_node = local_document.createElement(name)
    head.appendChild(created_node)
    return created_node


def get_child(node, name):
    """
    Return the first direct descendant (child) of the supplied node with
    the tagname name. The default xml getchild also looks in child nodes.
    Return None if no match is found
    """
    for child in node.childNodes:
        if child.nodeName == name:
            return child

    return None


def get_active_stack(calling_object, stack_name="active_stack"):
    """
    returns the active stack on the current class
    return None of it is not present
    """
    if hasattr(calling_object, stack_name):
        stack_node = calling_object.__getattribute__(stack_name)
        if stack_node.getAttribute("type") == "active_stack":
            return stack_node

    return None


def add_child_to_active_stack_head(calling_object, child,
                              stack_name="active_stack"):
    """
    Add the supplied child to the current active node in the active stack.
    returns the added child on succes, None if not active stack was found.
    Selection between active stacks can be done with the stack_name argument
    """
    active_stack = get_active_stack(calling_object, stack_name="active_stack")
    if not active_stack == None:
        active_stack_node = get_child(active_stack, stack_name)
        last_child = active_stack_node.lastChild
        if last_child != None:
            last_child.appendChild(child)
            return child

    return None


def enter_active_stack(calling_object, child,
             stack_name="active_stack", comment=None):
    """
    This function adds stack-like behaviour to an object:
    On a 'fresh' class an xml node is added as a class attribute. This node
    performs stack functionality and allows nested adding of nodes to track
    functionality.
    If the function is called on a class with an active_stack already present
    a nested node is added.
    The current nesting is book kept in the active stack. Past calls are
    saved for logging purposes.
    The comment argument allows adding extra info to a node
    """
    active_stack_node = None
    stack_node = None
    # Check if the calling object has a active stack node with
    # name ==  stack_name
    if not hasattr(calling_object, stack_name):
        # Create the xml node if it not exists
        _throw_away_document = _xml.Document()
        stack_node = \
            _throw_away_document.createElement(stack_name)

        # The xml name of the object is the calling object
        stack_node.setAttribute("Name", calling_object.__class__.__name__)
        stack_node.setAttribute("type", "active_stack")
        # assign the node to the calling class as an attribute
        calling_object.__setattr__(stack_name, stack_node)

        # add the 'call stack'
        active_stack_node = add_child(stack_node, stack_name)  # generiek
    else:
        stack_node = calling_object.__getattribute__(stack_name)
        # Find the active stack
        active_stack_node = get_child(stack_node, stack_name)
        if active_stack_node == None:
            active_stack_node = add_child(stack_node, stack_name)

    if comment != None:
        stack_node.setAttribute("comment", comment)

    active_stack_node.setAttribute("info",
                             "Contains functions not left with a return")
    # if child is a string add a xml node with this name
    stacked_child = None
    if isinstance(child, basestring):
        stacked_child = add_child(active_stack_node, child)
    # else try adding it as a node
    elif isinstance(child, _xml.Node):
        active_stack_node.appendChild(child)
        stacked_child = child
    return stacked_child


def exit_active_stack(calling_object, stack_name="active_stack"):
    """
    Mirror function to enter_active_stack.
    Performs bookkeeping after leaving a stack:
    Add the left node a child of the current active node.
    If this is the last active node move it to the 'inactive node' list
    """
    # get the named active stack node
    if not hasattr(calling_object, stack_name):
        raise ValueError(
            "Tried leaving an active-stack which"
            " has not been entered: stack_name={0} does not exist".format(
                                stack_name))
    active_stack_node = calling_object.__getattribute__(
                                             stack_name)

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
