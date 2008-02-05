#!/usr/bin/python
'''
This is a prototype for a Finite State Machine class modelled after
the ideas found in "Practical Statecharts in C/C++", by M.Samek.
This may be extended to a HSM in the future.
'''
class fsm:
    def __init__(self, initial):
        self.state=initial
    def tran(self, newstate):
        self.state=newstate
    def state(self, signal):
        return 0
    def dispatch(self, signal):
        self.state(signal)

class myfsm(fsm):
    def __init__(self):
        fsm.__init__(self, self.initial_state)
    def initial_state(self, signal):
        print 'initial_state', signal
        if signal == 2:
            self.tran(self.second_state)
        return 0
    def second_state(self, signal):
        print 'second_state', signal
        if signal == 0:
            self.tran(self.initial_state)
        return 0

def main():
    m = myfsm()
    print m.initial_state
    print m.second_state
    print m.state
    m.dispatch(0)
    m.dispatch(1)
    m.dispatch(2)
    m.dispatch(2)
    m.dispatch(4)
    m.dispatch(0)
    m.dispatch(1)
    m.dispatch(1)
    m.dispatch('test')

if __name__ == "__main__":
    main()
