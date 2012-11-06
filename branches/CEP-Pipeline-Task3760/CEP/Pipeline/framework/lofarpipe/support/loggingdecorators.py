"""
A collection of function and class decorators used to add structured logging 
functionality based on xml
"""

import time

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

def mail_log_on_exception(target):
    """
    Simple decorator, it tests if the any exceptions are throw in the wrapped
    function. It results in an email send on error 
    """
    def wrapper(*args, **argsw):
        """
        Decorator construct, receives arguments to the decorated function
        """
        # Get the calling object (first argument supplied to this decorator)
        calling_object = args[0]

        try:
            # call the actual function
            return_value = target(*args, **argsw)
        except Exception, message:
            calling_object.logger.error("*******************************************")
            calling_object.logger.error(message)
            calling_object.logger.error("Failed pipeline run")
            mail_list = ["klijn@astron.nl", "nonoice@gmail.com"]

            active_stack_data = get_active_stack(calling_object
                                        ).toprettyxml(encoding='ascii')

            for entry in mail_list:
                _mail_msg_to("lce072@astron.nl", entry,
                         "Fail pipeline run", active_stack_data)
        # return the actual value of the function
        return return_value

    return wrapper

def _mail_msg_to(adr_from, adr_to, subject, msg):
    # Import smtplib for the actual sending function
    import smtplib

    # Import the email modules we'll need
    from email.mime.text import MIMEText

    # Create a text/plain message
    msg = MIMEText(msg)

    msg['Subject'] = subject
    msg['From'] = adr_from
    msg['To'] = adr_to

    # Send the message via our own SMTP server, but don't include the
    # envelope header.
    s = smtplib.SMTP('smtp.lofar.eu')
    s.sendmail(adr_from, [adr_to], msg.as_string())
    s.quit()
