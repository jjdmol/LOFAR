import copy
import os

import xml.dom.minidom as _xml


from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory

# Minidom does not allow creation of xml nodes without an owning Document
# Therefore a single Document is created which owns all 'unbound' xml nodes
_node_factory_document = None

def open_parset_as_xml_node(parset):
    """
    Open the parset supplied at path and convert it to a fill xml_document
    """
    parset_as_dict = "test"

    if isinstance(parset, basestring):
        try:
            # Small wrapper of parset parsing: If parameter set cannot be found 
            # use an limmited stand alone parser
            from lofar.parameterset import parameterset  #@UnresolvedImport
            parset = parameterset(parset)
            parset_as_dict = parset.dict()
        except:
            # use standalone parser if import failed
            parset_as_dict = _read_parset_to_dict(parset)
    else:
        message = "open_parset_as_xml_node received an unknown argument. Only"\
            " a path to a parset is allowed."
        raise PipelineException(message)

    return _convert_dict_to_xml_node(parset_as_dict)


def write_xml_as_parset(xml, parset_path):
    """
    Writes the supplied xml_document or xml_node to the supplied path
    
    """
    create_directory(os.path.dirname(parset_path))
    fp = open(parset_path, "w")

    dicted_xml = _convert_xml_to_dict(xml)
    # parset is key = value  on each line
    for (key, value) in dicted_xml:
        line = "{}={}\n".format(key, value)
        fp.write(line)

    fp.close()


def add_child(node, name):
    """
    Create a node with name. And append it to the supplied node.
    This function might create duplicate node names.
    """
    local_document = _xml.Document()
    created_node = local_document.createElement(name)
    node.appendChild(created_node)
    return created_node


def _read_parset_to_dict(parset_path):
    """
    Open the parset at parset_path. Parse the entries and return the a dict with
    all key value pairs.
    """
    fp = open(parset_path, 'r')
    parset_as_dict = {}

    # lastline is used for multi line parsing
    lastline = ''
    for line in fp.readlines():
        #parset_as_dict[idx] = line
        # ignore end of line comments
        current_line_no_comment = line.split('#')[0]
        lastline = lastline + current_line_no_comment
        # remove the whitespace
        lastline = lastline.rstrip()

        # If the currentline is a multiline line (end with a '\')
        if len(lastline) > 0 and lastline[-1] == '\\':
            # remove the \ and update the lastline value
            lastline = lastline[:-1]
            # and continue with the next line in the file
            continue

        # If there is a = we have a key value pair
        if '=' in lastline:
            # split at the = (maximum of 1: allows usage of = in string values
            key, value = lastline.split('=', 1)
            # strip whitespace and create key value entries
            parset_as_dict[key.strip()] = value.strip()
            # finished line parsing: clear the lastline and continue
            lastline = ''
        #else skip the line

    fp.close()

    return parset_as_dict


def _convert_dict_to_xml_node(par_dict, top_level_name="default name"):
    """
    _convert_dict_to_xml_node receives an dicted parset, parses the (implicit)
    node names, dot seperated names. Inserts new nodes into an xml node.
    leaves are added as attributes. No duplicate nodes are created on the same level
    Iff the par_dict contains multiple toplevel  names (nodes or leaves), a root
    xml node with the name supplied in top_level_name (default='default name') is created. 
    TODO: deze functie wil te veel doen. Mischien moet de validatie ergens anders. 
    """
    xml_document = _xml.Document()
    # start by creating a top_level_node
    root = xml_document.createElement(top_level_name)
    # for all the items in the dicted parameterset
    for (key, value) in par_dict.items():
        # Get a list version of all the nodes
        node_names = key.strip().split('.')

        current_parent = root
        # for all the node names exept the last one (the leaf)
        for name in node_names[:-1]:
            # get all nodes matching with the current node name
            matching_nodes = current_parent.getElementsByTagName(name)
            # If we already have this node name
            if len(matching_nodes) == 1:
                # update the current_parent and continue parsing the node name
                current_parent = matching_nodes[0]
            else:
                # Create a child with that name
                child = xml_document.createElement(name)
                current_parent.appendChild(child)

                # update the current_parent and continue parsing the node name
                current_parent = child

        # we have now parsed all the node names in the key except the leaf
        # insert the leafs as an Attribute 
        current_parent.setAttribute(node_names[-1], value)

    # test if there is a single node toplevel child
    if len(root.childNodes) == 1 and (not root.hasAttributes()):
        # return this child as the xml node
        return root.childNodes[0]
    # else return the root xml node
    return root


def _convert_xml_to_dict(xml_node):
    """
    Convert an parset xml document to a dictionary with key value pairs
    as used in the parset: keys are point seperated node indentifiers with
    the final id the leaf name.
    """
    parset_dict = {}
    if isinstance(xml_node, _xml.Document):
        # Get the toplevel xml element (parset)
        root_node = xml_node.documentElement
        if root_node.hasChildNodes():
            # if there are children
            for child in root_node.childNodes:
                # convert these to dict entries and insert into dict
                _convert_xml_node_to_dict(child, [], parset_dict)

    elif isinstance(xml_node, _xml.Node):
        _convert_xml_node_to_dict(xml_node, [], parset_dict)

    return parset_dict


def _convert_xml_node_to_dict(node, prefix_list, target_dict):
    """
    Recursive function receiving an xml node a prefix string with the parent
    node names to prepent to the key and the dict to which the values must
    be placed
    """
    # Push the current node name in the prefix stack
    prefix_list.append(node.nodeName)

    # Recursive call of all child nodes
    for child in node.childNodes:
        # stop criteria: if no nodes left stop recursion
        _convert_xml_node_to_dict(child, prefix_list, target_dict)

    # For each attribute get the name and the value
    if node.hasAttributes():
        node_prefix_string = ".".join(prefix_list)
        for (attr_name, value) in node.attributes.items():
            # add the allement to the dict use string manipulate
            target_dict['{0}.{1}'.format(node_prefix_string, attr_name)] = value

    # remove the current node name
    prefix_list.pop()


def _enter_active_stack_node(calling_object, active_stack_node_name,
                             child_name):
    stack_name = "active_stack"
    active_stack_node = None
    stack_node = None
    if not hasattr(calling_object, active_stack_node_name): # specifiek
        # Create the xml node if it not exists
        _throw_away_document = _xml.Document()
        stack_node = \
            _throw_away_document.createElement(active_stack_node_name)
        stack_node.setAttribute("Name", calling_object.__class__.__name__)
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
        active_stack_node = calling_object.__getattribute__(active_stack_node_name)
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
    classes: It adds info  to the (xml node) class attribute timing_info. Creating the 
    attribute if it not exists, allowing for fire and forget usage.
    Subsequent usage of this logger decorator in nested function will result
    in a nested xml structure.
    The created data well need to be processed separately: it is not displayed 
    standalone
    """

    def wrapper(*args, **argsw):
        import time # alows stand alone usage
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
