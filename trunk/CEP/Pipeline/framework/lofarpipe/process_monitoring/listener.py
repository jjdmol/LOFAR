from threading import Thread
import socket
import os

class Listener(Thread):
    def __init__(self, config):
        """This class listens to a socket and keeps a list of the PIDs sent to
         that socket in the config object. The config object should contain
         a parentpid and sock_fname parameter. This class adds a set of the
         provided pids to the config under the parameter obspids"""
        self.config = config
        parentpid = self.config['parentpid']
        sock_fname  = self.config['sock_fname']
        self.sockname = sock_fname.format(parentpid)
        self.config.add_item('obspids',set())
        self.config.add_item('pidnames', dict())
        self.running = True
        
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
            self.s.listen(10)
            con, addr = self.s.accept()
            cmd = con.recv(80)
            self.__command(cmd)
            con.close()
            
            
    def __stop(self):
        """Stop the thread"""
        self.running = False
        

    def __command(self, cmd):
        """Execute the command. To add the pid to the list, it should be a
        executable name followed by an int (the pid). If 'del' 
        and int, remove it from list. Send stop to stop the listener theread
        No exception handling yet"""
        try:
            scmd = cmd.split()
            ccmd = int(scmd[1])
        except ValueError:
            cs = cmd.split()
            if cs[0].strip().lower() == "del":
                self.config['obspids'].remove(int(cs[1].strip()))
                del(self.config['pidnames'][int(cs[1].strip())])
        except IndexError:
            if "stop" in cmd.lower():
                self.config['stop'] = True
                self.__stop()
        else:
            self.config['obspids'].add(ccmd)
            self.config['pidnames'][ccmd] = scmd[0]