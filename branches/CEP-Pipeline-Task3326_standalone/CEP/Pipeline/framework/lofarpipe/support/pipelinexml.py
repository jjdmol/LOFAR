import os
import xml.dom.minidom as _xml

from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory

# Minidom does not allow creation of xml nodes without an owning Document
# Therefore a single Document is created which owns all 'unbound' xml nodes
_node_factory_document = None


def open_parset_as_xml_node(parset):
    """
    Open the parset supplied at path and convert it to a full xml_document
    """
    parset_as_dict = None
    if isinstance(parset, basestring):
        try:
            # Small wrapper of parset parsing: If parameter set cannot
            # be found use an limited stand alone parser
            from lofar.parameterset import parameterset  #@UnresolvedImport
            parset = parameterset(parset)
            parset_as_dict = parset.dict()

        except:
            # use standalone parser if import failed
            print "lofar.parameterset not found. Using standalone parser."
            parset_as_dict = _read_parset_to_dict(parset)
    else:
        message = "open_parset_as_xml_node received an unknown argument. Only"\
            " a path to a parset is allowed."
        raise PipelineException(message)

    return _convert_dict_to_xml_node(parset_as_dict)


def write_xml_as_parset(xml, parset_path, prefix_remove=""):
    """
    Writes the supplied xml_document or xml_node to the supplied path
    as a lofar parameterset
    prefix_remove allows the removal of a possible document name
    before writing to file. It removes <prefix_remove.> from each
    key name
    """
    # assure existence of target path
    create_directory(os.path.dirname(parset_path))

    dicted_xml = _convert_xml_to_dict(xml)
    fp = open(parset_path, "w")
    # parset is key = value  on each line
    for (key, value) in dicted_xml.items():
        if not (prefix_remove == ""):
            key = key[len(prefix_remove) + 1:]

        # Needed for correct conversion of dict to parset:
        line = "{0}={1}\n".format(key, value)

        fp.write(line)

    fp.close()


def add_child(node, name):
    """
    Create a node with name. And append it to the supplied node.
    (This function allows duplicate node names as specified by xml)
    """
    local_document = _xml.Document()
    created_node = local_document.createElement(name)
    node.appendChild(created_node)
    return created_node


def _read_parset_to_dict(parset_path):
    """
    Open the parset at parset_path. Parse the entries and return the a dict
    with all key value pairs.
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


def _convert_dict_to_xml_node(par_dict, top_level_name="dict"):
    """
    _convert_dict_to_xml_node receives an dicted parset, parses the (implicit)
    node names, dot seperated names. Inserts new nodes into an xml node.
    leaves are added as attributes. No duplicate nodes are created on the same
    level
    Iff the par_dict contains multiple toplevel  names (nodes or leaves), a
    root xml node with the name supplied in top_level_name (default='default
    name') is created.
    TODO: deze functie wil te veel doen. Mischien moet de validatie ergens
    anders.
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
            # get all nodes matching with the current node name, INCLUDING grandchil.
            matching_nodes = current_parent.getElementsByTagName(name)
            # If we already have this node name as a CHILD
            if len(matching_nodes) == 1 and \
               (matching_nodes[0] in current_parent.childNodes):
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
        # return this child as the xml node: this results in expected behaviour
        # that the xml contains only dict information
        # In other instances we need a node to assign the leaf information to
        return root.childNodes[0]
    # else return the root xml node: T
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
            target_dict['{0}.{1}'.format(node_prefix_string, attr_name)] = \
                value

    # remove the current node name
    prefix_list.pop()

