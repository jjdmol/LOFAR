#!/usr/bin/env python
"""
Program to test the RPC and Service class of the Messaging package.
It defines 5 functions and first calls those functions directly to check
that the functions are OK. Next the same tests are done with the RPC and
Service classes in between. This should give the same results.
"""
import logging
import sys
import time
from lofar.messaging import *

logging.basicConfig(stream=sys.stdout, level=logging.WARNING)

class UserException(Exception):
    "Always thrown in one of the functions"
    pass
class InvalidArgType(Exception):
    "Thrown when the input is wrong for one of the functions"
    pass

# create several function:
def ErrorFunc(input_value):
    " Always thrown a predefined exception"
    raise UserException("Exception thrown by the user")

def ExceptionFunc(input_value):
    "Generate a exception not caught by the function"
    a = "aap"
    b = a[23]

def StringFunc(input_value):
    "Convert the string to uppercase."
    if not isinstance(input_value, str) and not isinstance(input_value, unicode):
        raise InvalidArgType("Input value must be of the type 'string'")
    return input_value.upper()

class OnlyMessageHandling(MessageHandlerInterface):
    def __init__(self, arg_dict):
        MessageHandlerInterface.__init__(self)
        print "Creation of OnlyMessageHandling class: %s" % arg_dict
        self.handle_message = arg_dict.pop("function")
        self.args = arg_dict

class FullMessageHandling(MessageHandlerInterface):
    def __init__(self, arg_dict):
        MessageHandlerInterface.__init__(self)
        print "Creation of FullMessageHandling class: %s" % arg_dict
        self.handle_message = arg_dict.pop("function")
        self.args = arg_dict
    def before_main_loop(self):
        print "FullMessageHandling before_main_loop: %s" % self.args
    def loop_before_receive(self):
        print "FullMessageHandling loop_before_receive: %s" % self.args
    def loop_after_handling(self):
        print "FullMessageHandling loop_after_handling: %s" % self.args
    def after_main_loop(self):
        print "FullMessageHandling after_main_loop: %s" % self.args

class FailingMessageHandling(MessageHandlerInterface):
    def __init__(self, arg_dict):
        MessageHandlerInterface.__init__(self)
        print "Creation of FailingMessageHandling class: %s" % arg_dict
        self.handle_message = arg_dict.pop("function")
        self.args = arg_dict
        self.counter = 0
    def before_main_loop(self):
        print "FailingMessageHandling before_main_loop: %s" % self.args
        raise Exception("oops in before_main_loop()")
    def loop_before_receive(self):
        # allow one succesfull call otherwise the main loop never accepts the message :-)
        print "FailingMessageHandling loop_before_receive: %s" % self.args
        if self.counter:
            time.sleep(1)  # Prevent running around too fast
            raise Exception("oops in loop_before_receive(%d)" % self.counter)
        else:
            self.counter = self.counter + 1
    def loop_after_handling(self):
        print "FailingMessageHandling loop_after_handling: %s" % self.args
        raise Exception("oops in loop_after_handling()")
    def after_main_loop(self):
        print "FailingMessageHandling after_main_loop: %s" % self.args
        raise Exception("oops in after_main_loop()")

if __name__ == '__main__':
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

    # Register functs as a service handler listening at busname and ServiceName
    serv1_plain         = Service(busname, "Error1Service", ErrorFunc,              numthreads=1, startonwith=True)
    serv1_minimal_class = Service(busname, "Error2Service", OnlyMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : ErrorFunc})
    serv1_full_class    = Service(busname, "Error3Service", FullMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : ErrorFunc})
    serv1_failing_class = Service(busname, "Error4Service", FailingMessageHandling, numthreads=1, startonwith=True, 
                                  handler_args={"function" : ErrorFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv1_plain, serv1_minimal_class, serv1_full_class, serv1_failing_class:
        # Redo Error tests via RPC
        with RPC(busname, "Error1Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Caught expected exception"

        with RPC(busname, "Error2Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Caught expected exception"

        with RPC(busname, "Error3Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Caught expected exception"

        with RPC(busname, "Error4Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except Exception as e:
                print "Caught expected exception"

    # Register functs as a service handler listening at busname and ServiceName
    serv2_plain         = Service(busname, "Except1Service", ExceptionFunc,          numthreads=1, startonwith=True)
    serv2_minimal_class = Service(busname, "Except2Service", OnlyMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : ExceptionFunc})
    serv2_full_class    = Service(busname, "Except3Service", FullMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : ExceptionFunc})
    serv2_failing_class = Service(busname, "Except4Service", FailingMessageHandling, numthreads=1, startonwith=True, 
                                  handler_args={"function" : ExceptionFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv2_plain, serv2_minimal_class, serv2_full_class, serv2_failing_class:
        # Redo exception tests via RPC
        with RPC(busname, "Except1Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Caught expected exception"

        with RPC(busname, "Except2Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Caught expected exception"

        with RPC(busname, "Except3Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Caught expected exception"

        with RPC(busname, "Except4Service", ForwardExceptions=True) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Caught expected exception"

    # Register functs as a service handler listening at busname and ServiceName
    serv3_plain         = Service(busname, "String1Service", StringFunc,             numthreads=1, startonwith=True)
    serv3_minimal_class = Service(busname, "String2Service", OnlyMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : StringFunc})
    serv3_full_class    = Service(busname, "String3Service", FullMessageHandling,    numthreads=1, startonwith=True, 
                                  handler_args={"function" : StringFunc})
    serv3_failing_class = Service(busname, "String4Service", FailingMessageHandling, numthreads=1, startonwith=True, 
                                  handler_args={"function" : StringFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv3_plain, serv3_minimal_class, serv3_full_class, serv3_failing_class:
        # Redo string tests via RPC
        with RPC(busname, "String1Service", ForwardExceptions=True) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String1Service:{}".format(result))

        with RPC(busname, "String2Service", ForwardExceptions=True) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String2Service:{}".format(result))

        with RPC(busname, "String3Service", ForwardExceptions=True) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String3Service:{}".format(result))

        with RPC(busname, "String4Service", ForwardExceptions=True) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String4Service:{}".format(result))

        print "Functions tested with RPC: All OK"
