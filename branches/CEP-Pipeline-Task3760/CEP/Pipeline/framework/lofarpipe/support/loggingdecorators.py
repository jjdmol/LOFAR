"""
A collection of function and class decorators used to add structured logging 
functionality based on xml
"""

import time

from lofarpipe.support.xmllogging import enter_active_stack, \
                                          exit_active_stack


def xml_node(target):
    """
    function decorator to be used on member functions of (pipeline)
    classes:
    It creates an active xml stack and adds timing info  to the (xml node)
    Creating this stack if it not exist, allowing fire and forget
    usage.
    Subsequent usage of this logger decorator in nested function will result
    in a nested xml structure.
    """
    def wrapper(*args, **argsw):
        """
        Decorator construct, receives arguments to the decorated function
        """
        # Get the calling object (first argument supplied to this decorator)
        calling_object = args[0]

        # Add a node with the name of the current function to the active stack
        # if stack does not exist the stack will be created
        xml_current_node = enter_active_stack(
                    calling_object, target.__name__)

        # log time
        time_info1 = time.time()

        # call the actual function
        return_value = target(*args, **argsw)

        # end time
        time_info2 = time.time()
        # add the duration
        xml_current_node.setAttribute("duration", str(time_info2 - time_info1))

        # leave the stack
        exit_active_stack(calling_object)

        # return the actual value of the function
        return return_value

    return wrapper
