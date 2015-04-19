class Error(Exception):
    """
    Exception returned by sql.connect()
    """
    def __str__(self):
        return "Error"

def connect(hostname, database, username, password, port):
    """
    Muck sql.connect: returns the string connection upon a call of connect with
    an int as port and the string 'hostname' as hostname.
    Will return an Error of the type Exception when the hostname is 'except'.
    Otherwise the function return the string 'connection Failed'
    """
    if hostname == "hostname" and isinstance(port, int):
        return "connection"
    if hostname == "except":
        raise Error()

    return "connection Failed"
