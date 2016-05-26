from threading import Thread
import socket
import os


# deadlocks on the same PID??
class Listener(Thread):
    def __init__(self, config):
        """This class listens to a socket and keeps a list of the PIDs sent to
         that socket in the config object. The config object should contain
         a parentpid and sock_fname parameter. This class adds a set of the
         provided pids to the config under the parameter obspids"""
        self.config = config
        parentpid = self.config['parentpid']
        sock_fname  = self.config['sock_fname']  
        # the default socked name, it can be formatted with the pid
        # retrieved from the defaults.cfg


        self.sockname = sock_fname.format(parentpid)
        self.config.add_item('obspids', set())
        self.config.add_item('pidnames', dict())
        self.config.add_item('stoppedpids', set())
        self.config.add_item('startpids', set())
        self.running = True
        
        # is this a save construct. Error but no action on fail?
        # what happens when called twice?
        try:
            os.remove(self.sockname)
        except OSError:
            pass
        
        super(Listener, self).__init__()

    def run(self):
        """ Listen untill the stop routine is called. You can send a pid to this
        socket, after which this pid is added to the observation list object 
        (of type set). You can send a 'del' PID command, which removes the
        PID from the observation list. The list is kept unique."""
        self.s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.s.bind(self.sockname)

        while self.running:
            self.s.listen(10)  # magic numbers
            con, addr = self.s.accept()
            cmd = con.recv(80)
            print cmd
            self.__command(cmd)
            con.close()
            
            
    def __stop(self):
        """Stop the thread"""
        # should there be an additional action to do?
        # only setting to false enough?
        self.running = False

        

    def __command(self, cmd):
        """Execute the command. To add the pid to the list, it should be a
        executable name followed by an int (the pid). If 'del' 
        and int, remove it from list. Send stop to stop the listener theread
        No exception handling yet"""
        scmd = cmd.split()
        # I would like a more structured command loop.
        try:
            ccmd = int(scmd[1])
        except IndexError:
            if "stop" in cmd.lower():
                self.config['stop'] = True
                self.__stop()
        else:
            if scmd[0].strip().lower() == "del":
                self.config['stoppedpids'].add(ccmd)
            else:
                print ccmd
                self.config['startpids'].add(ccmd)               
                self.config['pidnames'][ccmd] = scmd[0]