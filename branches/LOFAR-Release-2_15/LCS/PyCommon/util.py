# util.py: utils for lofar software
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: util.py 1584 2015-10-02 12:10:14Z loose $
#
"""
This package contains different utilities that are common for LOFAR software
"""

import sys
import time


def check_bit(value, bit):
    """
        function to check if a given bit is set
        :param value: value to check
        :param bit: bit to be checked
        :return: true or false
    """
    return bool(value & (1 << bit))


def set_bit(value, bit):
    """
        function to set a bit in a given value
        :param value: value to set bit in
        :param bit: bit to set
        :return: value after the bit is set
    """
    return value | (1 << bit)


def clear_bit(value, bit):
    """
        function to clear a given bit
        :param value: value to clear the bit in
        :param bit: bit to clear
        :return: value after the bit is cleared
    """
    return value & ~(1 << bit)


def feq(val_a, val_b, rtol=1e-05, atol=1e-08):
    """

    :param val_a: float a to compare with
    :param val_b: float b
    :param rtol: The relative tolerance parameter
    :param atol: The absolute tolerance parameter
    :return:
    """
    return abs(val_a - val_b) <= rtol * (abs(val_a) + abs(val_b)) + atol


def chunker(seq, size):
    """
    function to divide a list into equal chunks
    :param seq: initial list
    :param size: size of the chunks
    :return:
    """
    return (seq[pos:pos + size] for pos in xrange(0, len(seq), size))


def raise_exception(cls, msg):
    """
    Raise an exception of type `cls`. Augment the exception message `msg` with
    the type and value of the last active exception if any.
    :param cls: type of exception that will be raised
    :param msg: exception message
    """
    exc_type, exc_val = sys.exc_info()[:2]
    if exc_type is not None:
        msg = "%s [%s: %s]" % (msg, exc_type.__name__, exc_val)
    raise cls(msg)


def isIntList(lst):
    """
    function to see if a value is a list of ints
    :param lst: the list that needs to be examined
    :return:  True if it is a list of ints otherwise False
    """
    if not isinstance(lst, list):
        return False
    return all(isinstance(x, int) for x in lst)


def isFloatList(lst):
    """
    function to see if a value is a list of floats
    :param lst: the list that needs to be examined
    :return:  True if it is a list of ints otherwise False
    """
    if not isinstance(lst, list):
        return False
    return all(isinstance(x, float) for x in lst)


def waitForInterrupt():
    """
    Useful (low cpu load) loop that waits for keyboard interrupt.
    """
    while True:
        try:
            time.sleep(10)
        except KeyboardInterrupt:
            break


def humanreadablesize(num, suffix='B', base=1000):
    """ converts the given size (number) to a human readable string in powers of 'base'"""
    try:
        for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
            if abs(num) < float(base):
                return "%3.1f%s%s" % (num, unit, suffix)
            num /= float(base)
        return "%.2f%s%s" % (num, 'Y', suffix)
    except TypeError:
        return str(num)

