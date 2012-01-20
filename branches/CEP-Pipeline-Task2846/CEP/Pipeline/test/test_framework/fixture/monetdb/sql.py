class Error(Exception):
    def __str__(self):
        return "Error"

def connect(hostname, database, username, password, port):
    if hostname == "hostname" and isinstance(port, int):
        return "connection"
    if hostname == "except":
        raise Error()

    return "connection Failed"
