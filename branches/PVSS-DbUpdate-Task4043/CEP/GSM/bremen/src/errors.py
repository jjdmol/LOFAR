#!/usr/bin/env python
class GSMException(Exception):
    """
    Basic GSM exception class.
    """
    pass

class SourceException(GSMException):
    """
    No source or source reading error.
    """
    pass

class SourcePartMissingException(GSMException):
    """
    Required part of the source information is missing.
    """
    pass

class ParsetContentError(GSMException):
    """
    Missing part of the parset file or parset file is wrong.
    """
    pass

class ImageStateError(GSMException):
    """
    Image cannot be processed: wrong state.
    """
    pass

class SQLError(GSMException):
    """
    Some error in SQL
    """
    pass
