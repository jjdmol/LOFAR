__version__ = "1.1"

class Error(Exception):
    """Generalization of all System Health Management errors"""
    pass

class DatabaseError(Error):
    def __init__(self, str):
        Error.__init__(self)
        self.str = str
    
    def __str__(self):
        return "DatabaseError: " + self.str

