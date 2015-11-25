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
    def __init__(self, **kwargs):
        MessageHandlerInterface.__init__(self)
        print "Creation of OnlyMessageHandling class: %s" % kwargs
        self.handle_message = kwargs.pop("function")
        self.args = kwargs

class FullMessageHandling(MessageHandlerInterface):
    def __init__(self, **kwargs):
        MessageHandlerInterface.__init__(self)
        print "Creation of FullMessageHandling class: %s" % kwargs
        self.handle_message = kwargs.pop("function")
        self.args = kwargs
    def prepare_loop(self):
        print "FullMessageHandling prepare_loop: %s" % self.args
    def prepare_receive(self):
        print "FullMessageHandling prepare_receive: %s" % self.args
    def finalize_handling(self, successful):
        print "FullMessageHandling finalize_handling: %s" % self.args
    def finalize_loop(self):
        print "FullMessageHandling finalize_loop: %s" % self.args

class FailingMessageHandling(MessageHandlerInterface):
    def __init__(self, **kwargs):
        MessageHandlerInterface.__init__(self)
        print "Creation of FailingMessageHandling class: %s" % kwargs
        self.handle_message = kwargs.pop("function")
        self.args = kwargs
        self.counter = 0
    def prepare_loop(self):
        print "FailingMessageHandling prepare_loop: %s" % self.args
        raise UserException("oops in prepare_loop()")
    def prepare_receive(self):
        # allow one succesfull call otherwise the main loop never accepts the message :-)
        print "FailingMessageHandling prepare_receive: %s" % self.args
        if self.counter:
            time.sleep(1)  # Prevent running around too fast
            raise UserException("oops in prepare_receive(%d)" % self.counter)
        else:
            self.counter = self.counter + 1
    def finalize_handling(self, successful):
        print "FailingMessageHandling finalize_handling: %s, %s" % (self.args, successful)
        raise UserException("oops in finalize_handling()")
    def finalize_loop(self):
        print "FailingMessageHandling finalize_loop: %s" % self.args
        raise UserException("oops in finalize_loop()")

if __name__ == '__main__':
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

    # Register functs as a service handler listening at busname and ServiceName
    serv1_plain         = Service("String1Service", StringFunc,             busname=busname, numthreads=1)
    serv1_minimal_class = Service("String2Service", OnlyMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : StringFunc})
    serv1_full_class    = Service("String3Service", FullMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : StringFunc})
    serv1_failing_class = Service("String4Service", FailingMessageHandling, busname=busname, numthreads=1,
                                  handler_args={"function" : StringFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv1_plain, serv1_minimal_class, serv1_full_class, serv1_failing_class:
        # Redo string tests via RPC
        with RPC("String1Service", ForwardExceptions=True, busname=busname) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String1Service:{}".format(result))
            print "string1Service is OK"

        with RPC("String2Service", ForwardExceptions=True, busname=busname) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String2Service:{}".format(result))
            print "string2Service is OK"

        with RPC("String3Service", ForwardExceptions=True, busname=busname) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String3Service:{}".format(result))
            print "string3Service is OK"

        with RPC("String4Service", ForwardExceptions=True, busname=busname) as rpc:
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed of String4Service:{}".format(result))
            print "string4Service is OK"

    # Register functs as a service handler listening at busname and ServiceName
    serv2_plain         = Service("Error1Service", ErrorFunc,              busname=busname, numthreads=1)
    serv2_minimal_class = Service("Error2Service", OnlyMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : ErrorFunc})
    serv2_full_class    = Service("Error3Service", FullMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : ErrorFunc})
    serv2_failing_class = Service("Error4Service", FailingMessageHandling, busname=busname, numthreads=1,
                                  handler_args={"function" : ErrorFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv2_plain, serv2_minimal_class, serv2_full_class, serv2_failing_class:
        # Redo Error tests via RPC
        with RPC("Error1Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Error1Service is OK"

        with RPC("Error2Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Error2Service is OK"

        with RPC("Error3Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except RPCException as e:
                print "Error3Service is OK"

        with RPC("Error4Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except Exception as e:
                print "Error4Service is OK"

    # Register functs as a service handler listening at busname and ServiceName
    serv3_plain         = Service("Except1Service", ExceptionFunc,          busname=busname, numthreads=1)
    serv3_minimal_class = Service("Except2Service", OnlyMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : ExceptionFunc})
    serv3_full_class    = Service("Except3Service", FullMessageHandling,    busname=busname, numthreads=1,
                                  handler_args={"function" : ExceptionFunc})
    serv3_failing_class = Service("Except4Service", FailingMessageHandling, busname=busname, numthreads=1,
                                  handler_args={"function" : ExceptionFunc})

    # 'with' sets up the connection context and defines the scope of the service.
    with serv3_plain, serv3_minimal_class, serv3_full_class, serv3_failing_class:
        # Redo exception tests via RPC
        with RPC("Except1Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Except1Service is OK"

        with RPC("Except2Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Except2Service is OK"

        with RPC("Except3Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Except3Service is OK"

        with RPC("Except4Service", ForwardExceptions=True, busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                print "Except4Service is OK"

    print "Functions tested with RPC: All OK"
