#!/usr/bin/env python
"""
Program to test the RPC and Service class of the Messaging package.
It defines 5 functions and first calls those functions directly to check
that the functions are OK. Next the same tests are done with the RPC and
Service classes in between. This should give the same results.
"""
import sys
from contextlib import nested

from lofar.messaging import Service, RPC

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

def ListFunc(input_value):
    "Convert the list to uppercase."
    if not isinstance(input_value, list):
        raise InvalidArgType("Input value must be of the type 'list'")
    result = []
    for item in input_value:
        if isinstance(item, str) or isinstance(item, unicode):
            result.append(item.upper())
        elif isinstance(item, list):
            result.append(ListFunc(item))
        elif isinstance(item, dict):
            result.append(DictFunc(item))
        else:
            result.append(item)
    return result

def DictFunc(input_value):
    "Convert the dict to uppercase."
    if not isinstance(input_value, dict):
        raise InvalidArgType("Input value must be of the type 'dict'")
    result = {}
    for key, value in input_value.items():
        if isinstance(value, str) or isinstance(value, unicode):
            result[key] = str(value).upper()
        elif isinstance(value, list):
            result[key] = ListFunc(value)
        elif isinstance(value, dict):
            result[key] = DictFunc(value)
        else:
            result[key] = value
    return result

if __name__ == '__main__':
    # First do basic test for the functions
    # ErrorFunc
    try:
        result = ErrorFunc("aap noot mies")
    except UserException as e:
        pass

    # ExceptionFunc
    try:
        result = ExceptionFunc("aap noot mies")
    except IndexError as e:
        pass

    # StringFunc
    try:
        result = StringFunc(25)
    except InvalidArgType as e:
        pass
    result = StringFunc("aap noot mies")
    if result != "AAP NOOT MIES":
        raise Exception("String function failed:{}".format(result))

    # ListFunc
    try:
        result = ListFunc(25)
    except InvalidArgType as e:
        pass
    result = ListFunc(["aap", 25, [1, 2], {'mies' : "meisje"}])
    if result != ["AAP", 25, [1, 2], {'mies' : "MEISJE"}]:
        raise Exception("List function failed:{}".format(result))

    # DictFunc
    try:
        result = DictFunc(25)
    except InvalidArgType as e:
        pass
    result = DictFunc({'mies' : "meisje", "aap" : 125, "noot" : [2, 3]})
    if result != {'mies' : "MEISJE", "aap" : 125, "noot" : [2, 3]}:
        raise Exception("Dict function failed:{}".format(result))

    print "Functions tested outside RPC: All OK"

    # Used settings
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

    # Register functs as a service handler listening at busname and ServiceName
    serv1 = Service("ErrorService",     ErrorFunc,     busname=busname, numthreads=1)
    serv2 = Service("ExceptionService", ExceptionFunc, busname=busname, numthreads=1)
    serv3 = Service("StringService",    StringFunc,    busname=busname, numthreads=1)
    serv4 = Service("ListService",      ListFunc,      busname=busname, numthreads=1)
    serv5 = Service("DictService",      DictFunc,      busname=busname, numthreads=1)

    # 'with' sets up the connection context and defines the scope of the service.
    with nested(serv1, serv2, serv3, serv4, serv5):
        # Start listening in the background. This will start as many threads as defined by the instance
        serv1.start_listening()
        serv2.start_listening()
        serv3.start_listening()
        serv4.start_listening()
        serv5.start_listening()

        # Redo all tests but via through RPC
        # ErrorFunc
        with RPC("ErrorService", busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except UserException as e:
                pass

        # ExceptionFunc
        with RPC("ExceptionService", busname=busname) as rpc:
            try:
                result = rpc("aap noot mies")
            except IndexError as e:
                pass

        # StringFunc
        with RPC("StringService", busname=busname) as rpc:
            try:
                result = rpc([25])
            except InvalidArgType as e:
                pass
            result = rpc("aap noot mies")
            if result[0] != "AAP NOOT MIES":
                raise Exception("String function failed:{}".format(result))

        # ListFunc
        with RPC("ListService", busname=busname) as rpc:
            try:
                result = rpc("25")
            except InvalidArgType as e:
                pass
            result = rpc(["aap", 25, [1, 2], {'mies' : "meisje"}])
            if result[0] != ["AAP", 25, [1, 2], {'mies' : "MEISJE"}]:
                raise Exception("List function failed:{}".format(result))

        # DictFunc
        with RPC("DictService", busname=busname) as rpc:
            try:
                result = rpc([25])
            except InvalidArgType as e:
                pass
            result = rpc({'mies' : "meisje", "aap" : 125, "noot" : [2, 3]})
            if result[0] != {'mies' : "MEISJE", "aap" : 125, "noot" : [2, 3]}:
                raise Exception("Dict function failed:{}".format(result))

        print "Functions tested with RPC: All OK"

        # Tell all background listener threads to stop and wait for them to finish.
        serv1.stop_listening()
        serv2.stop_listening()
        serv3.stop_listening()
        serv4.stop_listening()
        serv5.stop_listening()
