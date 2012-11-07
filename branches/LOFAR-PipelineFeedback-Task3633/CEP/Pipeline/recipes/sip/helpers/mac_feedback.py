#                                                         LOFAR IMAGING PIPELINE
#
#                                                   MAC Status Feedback QuickFix
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

"""
This module implements the quick fix (Redmine issue #3633) for the pipeline
status feedback to MAC. 
"""

import random
import socket
import time

def __try_connect(host, port, tries=5, min_timeout=1.0, max_timeout=5.0):
    """
    Try to connect to `host`:`port` up time `tries` times, using a random
    timeout interval ([`min_timeout` .. `max_timeout`) seconds ) between
    retries.
    Return a socket object.
    Raises `socket.error` if all connect tries fail.
    """
    # Create a socket (SOCK_STREAM means a TCP socket)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        tries -= 1
        try:
            sock.connect((host, port))
        except socket.error, err:
            print("Could not connect to %s:%s (got %s)" %
                  (host, str(port), str(err)))
            if tries > 0:
                timeout = random.uniform(min_timeout, max_timeout)
                print("Retrying in %f seconds (%d more %s)." %
                      (timeout, tries, "try" if tries==1 else "tries"))
                time.sleep(timeout)
            else:
                raise
        else:
            return sock


def send_status(host, port, status):
    """
    Send the pipeline status to a TCP listener at `host`:`port`. If `status` is
    non-zero, send the string 'ABORT'; otherwise send the string 'FINISHED'.
    """
    message = "FINISHED" if status == 0 else "ABORT"
    sock = __try_connect(host, port)
    sock.sendall(message)
    sock.close()


if __name__ == "__main__":
    """
    Simple command line test.
    Usage: python mac_feedback.py [<host>] [<port>] [<status>]
    """
    import sys
    host = str(sys.argv[1]) if len(sys.argv) > 1 else "localhost"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 9999
    stat = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    send_status(host, port, stat)


