"""
A collection of function and class decorators used to add structured logging
functionality based on xml
"""
import smtplib
from email.mime.text import MIMEText
import time
import os

from lofarpipe.support.xmllogging import enter_active_stack, \
        exit_active_stack, get_active_stack

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

class duration:
    """
    context manager for logging duration of a code block:
    1. Add an xml active stack member on the object if not present
    2. Add a new active stack entry for current context
    3. On exit add the duration of the code block to the now deactivate stack
       member
    """
    def __init__(self, containing_object, name):
        """
        On creation of the contect manager provide the object instance to add
        the xml stack to and the name for in the loggin tree.
        """
        self._containing_object = containing_object
        self._name = name
        self._xml_current_node = None
        self._time_info_start = None

    def __enter__(self):
        """
        The duration context should be initialized with the calling object self
        pointer. This allows adding the duration xml to the object
        """
        # Get or create an active stack (default name)
        self._xml_current_node = enter_active_stack(
            self._containing_object, self._name)
        # Get and save the current time
        self._time_info_start = time.time()

        return self # return self, the context manager

    def __exit__(self, exc_type, exc_value, exc_tb):
        """
        upon leaving the context log the duration and leave the current
        Xml node
        """
        time_info_end = time.time()
        self._xml_current_node.setAttribute(
                    "duration", str(time_info_end - self._time_info_start))
        if exc_type == None:

            exit_active_stack(self._containing_object)
        else:
            # Exception thrown in the context: Return False here reraises it
            # automatically.
            False


def mail_log_on_exception(target):
    """
    Simple decorator, it tests if the any exceptions are throw in the wrapped
    function. It results in an email send on error.
    """
    def wrapper(*args, **argsw):
        """
        Decorator construct, receives arguments to the decorated function
        """
        # Get the calling object (first argument supplied to this decorator)
        calling_object = args[0]

        try:
            # call the actual function
            time_info_start = time.time()
            return_value = target(*args, **argsw)
            time_info_end = time.time()
            # Force exception on non zero output
            if return_value != 0:
                raise Exception("Non zero pipeline output")
            # Mail main dev on succesfull run
            stack = get_active_stack(calling_object)
            duration_recipe = str(time_info_end - time_info_start)
            if stack != None:
                stack.setAttribute(
                    "duration", duration_recipe)
                msg_string = stack.toprettyxml(encoding='ascii')
            else:
                msg_string = "duration: {0} \n "\
                 "No additional pipeline data available".format(duration_recipe
                        )

            _mail_msg_to("pipeline_finished", "klijn@astron.nl",
                         "pipeline finished: {0}: {1}".format(
                                os.path.basename(calling_object.__file__),
                                calling_object.inputs['job_name']),
                         msg_string)

        except Exception, message:
            # Static list of mail to be send (could be made configurable,
            # but yeah temp mail functionality so...)
            mail_list = ["klijn@astron.nl", "frieswijk@astron.nl"
                         "pizzo@astron.nl", "orru@astron.nl"
                         ]

            # get the active stack
            stack = get_active_stack(calling_object)
            active_stack_data = ""
            if stack != None:
                active_stack_data = stack.toprettyxml(encoding='ascii')
            # get the Obsid and pipeline name add to subjecy title
            subject = "Failed pipeline run {0}: {1}".format(
                        os.path.basename(calling_object.__file__),
                        calling_object.inputs['job_name'])

            # construct the message
            msg = "Error ({0}): {1} \n information: \n {2}".format(
                    type(message), message, active_stack_data)

            # mail all recipients
            for entry in mail_list:
                _mail_msg_to("pipeline_error", entry,
                         subject, msg)

            raise

        calling_object.logger.info("pipeline_finished" + " xml summary:")
        calling_object.logger.info("\n" + msg_string)

        # return the actual value of the function
        return return_value

    return wrapper


def _mail_msg_to(adr_from, adr_to, subject, msg):
    """
    Fire and forget wrapper from lofar stmp mail access.
    sends an email with a from adress to an adress with a subject and message.
    """
    # Create a text/plain message
    msg = MIMEText(msg)

    msg['Subject'] = subject
    msg['From'] = adr_from
    msg['To'] = adr_to

    # Send the message via our own SMTP server, but don't include the
    # envelope header.
    try:
        s = smtplib.SMTP('smtp.lofar.eu')
        s.sendmail(adr_from, [adr_to], msg.as_string())
        s.quit()
    except:
        # Nothing: This is additional functionality.
        # If the smtp server is down we kan nothing else here
        print "Could not establish a connection with smtp.lofar.eu"

