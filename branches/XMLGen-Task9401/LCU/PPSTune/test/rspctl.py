#!/usr/bin/env python2

r'''
rspctl dummy program used for unit testing of the ppstune module. The
precise functionality is determined by the name by which the program
is called. The following names are supported:

- rspctl            : behave normally and locked to 200 MHz clock nl
- rspctl-intl       : behave as a normal intl station
- rspctl-no-output  : print nothing and exit
- rspctl-not-locked : behave as if not yet locked to 10 MHz

'''

from optparse import OptionParser
import os, sys, time


def parse_command_line(argv):
    r'''
    Command line parser.
    '''

    parser = OptionParser(usage = '%prog [options]')

    parser.add_option('--tdstatus',
                      dest   = 'command',
                      action = 'append_const',
                      const  = 'tdstatus')

    parser.add_option('--status',
                      dest   = 'command',
                      action = 'append_const',
                      const  = 'status')

    parser.add_option('--clock', type = 'int',
                      dest   = 'clock',
                      default = 0)


    prog_name = os.path.split(argv[0])[-1]
    (options, args) = parser.parse_args(argv[1:])
    command = ''
    if options.command is not None:
        command = options.command[0]
    else:
        if options.clock in [160, 200]:
            command = 'clock'
    return prog_name, command



def rspctl_clock(prog_name):
    r'''
    '''
    if prog_name == 'rspctl-160':
        print('''Sample frequency: clock=160MHz
''')
    else:
        print('''Sample frequency: clock=200MHz
''')
    return 0



def rspctl_tdstatus(prog_name):
    r'''
    '''
    if prog_name == 'rspctl':
        print('''RSP |   10MHz input | output clock | PPS input | PLL 160MHZ | PLL 200MHz | V3.3 | V5.0 | Temperature
  0 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.2 |  4.4 | 21
  1 | Not controlling the TD board
  2 | Not controlling the TD board
  3 | Not controlling the TD board
  4 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.2 |  4.3 | 21
  5 | Not controlling the TD board
  6 | Not controlling the TD board
  7 | Not controlling the TD board
  8 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.2 |  4.3 | 20
  9 | Not controlling the TD board
 10 | Not controlling the TD board
 11 | Not controlling the TD board
''')
    elif prog_name == 'rspctl-intl':
        print('''RSP |   10MHz input | output clock | PPS input | PLL 160MHZ | PLL 200MHz | V3.3 | V5.0 | Temperature
  0 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 29
  1 | Not controlling the TD board
  2 | Not controlling the TD board
  3 | Not controlling the TD board
  4 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 32
  5 | Not controlling the TD board
  6 | Not controlling the TD board
  7 | Not controlling the TD board
  8 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 30
  9 | Not controlling the TD board
 10 | Not controlling the TD board
 11 | Not controlling the TD board
 12 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 32
 13 | Not controlling the TD board
 14 | Not controlling the TD board
 15 | Not controlling the TD board
 16 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 29
 17 | Not controlling the TD board
 18 | Not controlling the TD board
 19 | Not controlling the TD board
 20 |      SMA      |      200     |    SMA    | not locked |     LOCKED |  3.3 |  0.0 | 33
 21 | Not controlling the TD board
 22 | Not controlling the TD board
 23 | Not controlling the TD board
''')
    elif prog_name == 'rspctl-160':
        print('''RSP |   10MHz input | output clock | PPS input | PLL 160MHZ | PLL 200MHz | V3.3 | V5.0 | Temperature
  0 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.2 |  4.3 | 22
  1 | Not controlling the TD board
  2 | Not controlling the TD board
  3 | Not controlling the TD board
  4 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.2 |  4.3 | 22
  5 | Not controlling the TD board
  6 | Not controlling the TD board
  7 | Not controlling the TD board
  8 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.2 |  4.3 | 22
  9 | Not controlling the TD board
 10 | Not controlling the TD board
 11 | Not controlling the TD board
''')
    elif prog_name == 'rspctl-not-locked':
        print('''RSP |   10MHz input | output clock | PPS input | PLL 160MHZ | PLL 200MHz | V3.3 | V5.0 | Temperature
  0 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  1 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  2 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  3 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  4 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  5 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  6 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  7 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  8 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
  9 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
 10 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
 11 |        ?      |        ?     |      ?    |          ? |          ? |  0.0 |  0.0 | 167223960
''')
    else:
        print('Unknown name '+prog_name)
    return 0


def rspctl_status(prog_name):
    if prog_name == 'rspctl':
        if int(time.time()) %2 == 0:
            return rspctl_status('rspctl-even')
        else:
            return rspctl_status('rspctl-odd')
    elif prog_name == 'rspctl-not-locked':
        print('''RSP[ 0] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 1] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 2] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 3] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 4] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 5] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 6] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 7] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 8] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 9] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[10] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[11] 1.2 V: 0.00 , 2.5 V: 0.00, 3.3 V: 0.00
RSP[ 0] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 1] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 2] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 3] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 4] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 5] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 6] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 7] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 8] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 9] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[10] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[11] PCB_temp:  0 , BP_temp:  0, Temp AP0:   0 , AP1:   0 , AP2:   0 , AP3:   0
RSP[ 0] BP_clock:   0
RSP[ 1] BP_clock:   0
RSP[ 2] BP_clock:   0
RSP[ 3] BP_clock:   0
RSP[ 4] BP_clock:   0
RSP[ 5] BP_clock:   0
RSP[ 6] BP_clock:   0
RSP[ 7] BP_clock:   0
RSP[ 8] BP_clock:   0
RSP[ 9] BP_clock:   0
RSP[10] BP_clock:   0
RSP[11] BP_clock:   0
RSP[ 0] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 1] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 2] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 3] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 4] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 5] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 6] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 7] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 8] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 9] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[10] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[11] Ethernet nr frames: 0 , nr errors: 0 , last error: 0
RSP[ 0] MEP sequencenr: 0 , error: 0
RSP[ 1] MEP sequencenr: 0 , error: 0
RSP[ 2] MEP sequencenr: 0 , error: 0
RSP[ 3] MEP sequencenr: 0 , error: 0
RSP[ 4] MEP sequencenr: 0 , error: 0
RSP[ 5] MEP sequencenr: 0 , error: 0
RSP[ 6] MEP sequencenr: 0 , error: 0
RSP[ 7] MEP sequencenr: 0 , error: 0
RSP[ 8] MEP sequencenr: 0 , error: 0
RSP[ 9] MEP sequencenr: 0 , error: 0
RSP[10] MEP sequencenr: 0 , error: 0
RSP[11] MEP sequencenr: 0 , error: 0
RSP[ 0] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 0]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 1] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 1]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 2] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 2]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 3] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 3]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 4] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 4]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 5] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 5]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 6] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 6]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 7] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 7]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 8] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 8]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 9] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[ 9]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[10] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[10]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[11] Errors ri:     0 ,  rcuX:     0 ,  rcuY:     0,   lcu:     0,    cep:     0
RSP[11]    serdes:     0 , ap0ri:     0 , ap1ri:     0, ap2ri:     0 , ap3ri:     0
RSP[ 0] Sync         diff      count    samples     slices
RSP[ 0]     0:          0          0          0          0
RSP[ 0]     1:          0          0          0          0
RSP[ 0]     2:          0          0          0          0
RSP[ 0]     3:          0          0          0          0
RSP[ 1] Sync         diff      count    samples     slices
RSP[ 1]     0:          0          0          0          0
RSP[ 1]     1:          0          0          0          0
RSP[ 1]     2:          0          0          0          0
RSP[ 1]     3:          0          0          0          0
RSP[ 2] Sync         diff      count    samples     slices
RSP[ 2]     0:          0          0          0          0
RSP[ 2]     1:          0          0          0          0
RSP[ 2]     2:          0          0          0          0
RSP[ 2]     3:          0          0          0          0
RSP[ 3] Sync         diff      count    samples     slices
RSP[ 3]     0:          0          0          0          0
RSP[ 3]     1:          0          0          0          0
RSP[ 3]     2:          0          0          0          0
RSP[ 3]     3:          0          0          0          0
RSP[ 4] Sync         diff      count    samples     slices
RSP[ 4]     0:          0          0          0          0
RSP[ 4]     1:          0          0          0          0
RSP[ 4]     2:          0          0          0          0
RSP[ 4]     3:          0          0          0          0
RSP[ 5] Sync         diff      count    samples     slices
RSP[ 5]     0:          0          0          0          0
RSP[ 5]     1:          0          0          0          0
RSP[ 5]     2:          0          0          0          0
RSP[ 5]     3:          0          0          0          0
RSP[ 6] Sync         diff      count    samples     slices
RSP[ 6]     0:          0          0          0          0
RSP[ 6]     1:          0          0          0          0
RSP[ 6]     2:          0          0          0          0
RSP[ 6]     3:          0          0          0          0
RSP[ 7] Sync         diff      count    samples     slices
RSP[ 7]     0:          0          0          0          0
RSP[ 7]     1:          0          0          0          0
RSP[ 7]     2:          0          0          0          0
RSP[ 7]     3:          0          0          0          0
RSP[ 8] Sync         diff      count    samples     slices
RSP[ 8]     0:          0          0          0          0
RSP[ 8]     1:          0          0          0          0
RSP[ 8]     2:          0          0          0          0
RSP[ 8]     3:          0          0          0          0
RSP[ 9] Sync         diff      count    samples     slices
RSP[ 9]     0:          0          0          0          0
RSP[ 9]     1:          0          0          0          0
RSP[ 9]     2:          0          0          0          0
RSP[ 9]     3:          0          0          0          0
RSP[10] Sync         diff      count    samples     slices
RSP[10]     0:          0          0          0          0
RSP[10]     1:          0          0          0          0
RSP[10]     2:          0          0          0          0
RSP[10]     3:          0          0          0          0
RSP[11] Sync         diff      count    samples     slices
RSP[11]     0:          0          0          0          0
RSP[11]     1:          0          0          0          0
RSP[11]     2:          0          0          0          0
RSP[11]     3:          0          0          0          0
RSP[ 0] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 0]     0:          0          0          0          0
RSP[ 0]     1:          0          0          0          0
RSP[ 0]     2:          0          0          0          0
RSP[ 0]     3:          0          0          0          0
RSP[ 1] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 1]     0:          0          0          0          0
RSP[ 1]     1:          0          0          0          0
RSP[ 1]     2:          0          0          0          0
RSP[ 1]     3:          0          0          0          0
RSP[ 2] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 2]     0:          0          0          0          0
RSP[ 2]     1:          0          0          0          0
RSP[ 2]     2:          0          0          0          0
RSP[ 2]     3:          0          0          0          0
RSP[ 3] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 3]     0:          0          0          0          0
RSP[ 3]     1:          0          0          0          0
RSP[ 3]     2:          0          0          0          0
RSP[ 3]     3:          0          0          0          0
RSP[ 4] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 4]     0:          0          0          0          0
RSP[ 4]     1:          0          0          0          0
RSP[ 4]     2:          0          0          0          0
RSP[ 4]     3:          0          0          0          0
RSP[ 5] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 5]     0:          0          0          0          0
RSP[ 5]     1:          0          0          0          0
RSP[ 5]     2:          0          0          0          0
RSP[ 5]     3:          0          0          0          0
RSP[ 6] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 6]     0:          0          0          0          0
RSP[ 6]     1:          0          0          0          0
RSP[ 6]     2:          0          0          0          0
RSP[ 6]     3:          0          0          0          0
RSP[ 7] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 7]     0:          0          0          0          0
RSP[ 7]     1:          0          0          0          0
RSP[ 7]     2:          0          0          0          0
RSP[ 7]     3:          0          0          0          0
RSP[ 8] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 8]     0:          0          0          0          0
RSP[ 8]     1:          0          0          0          0
RSP[ 8]     2:          0          0          0          0
RSP[ 8]     3:          0          0          0          0
RSP[ 9] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 9]     0:          0          0          0          0
RSP[ 9]     1:          0          0          0          0
RSP[ 9]     2:          0          0          0          0
RSP[ 9]     3:          0          0          0          0
RSP[10] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[10]     0:          0          0          0          0
RSP[10]     1:          0          0          0          0
RSP[10]     2:          0          0          0          0
RSP[10]     3:          0          0          0          0
RSP[11] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[11]     0:          0          0          0          0
RSP[11]     1:          0          0          0          0
RSP[11]     2:          0          0          0          0
RSP[11]     3:          0          0          0          0
RSP[ 0] RSU Status:
RSP[ 0]     Trigger: Board Reset
RSP[ 0]     Image  : Factory image
RSP[ 0]     FPGA   : BP was reconfigured
RSP[ 0]     Result : OK
RSP[ 0]     Status : IN PROGRESS
RSP[ 1] RSU Status:
RSP[ 1]     Trigger: Board Reset
RSP[ 1]     Image  : Factory image
RSP[ 1]     FPGA   : BP was reconfigured
RSP[ 1]     Result : OK
RSP[ 1]     Status : IN PROGRESS
RSP[ 2] RSU Status:
RSP[ 2]     Trigger: Board Reset
RSP[ 2]     Image  : Factory image
RSP[ 2]     FPGA   : BP was reconfigured
RSP[ 2]     Result : OK
RSP[ 2]     Status : IN PROGRESS
RSP[ 3] RSU Status:
RSP[ 3]     Trigger: Board Reset
RSP[ 3]     Image  : Factory image
RSP[ 3]     FPGA   : BP was reconfigured
RSP[ 3]     Result : OK
RSP[ 3]     Status : IN PROGRESS
RSP[ 4] RSU Status:
RSP[ 4]     Trigger: Board Reset
RSP[ 4]     Image  : Factory image
RSP[ 4]     FPGA   : BP was reconfigured
RSP[ 4]     Result : OK
RSP[ 4]     Status : IN PROGRESS
RSP[ 5] RSU Status:
RSP[ 5]     Trigger: Board Reset
RSP[ 5]     Image  : Factory image
RSP[ 5]     FPGA   : BP was reconfigured
RSP[ 5]     Result : OK
RSP[ 5]     Status : IN PROGRESS
RSP[ 6] RSU Status:
RSP[ 6]     Trigger: Board Reset
RSP[ 6]     Image  : Factory image
RSP[ 6]     FPGA   : BP was reconfigured
RSP[ 6]     Result : OK
RSP[ 6]     Status : IN PROGRESS
RSP[ 7] RSU Status:
RSP[ 7]     Trigger: Board Reset
RSP[ 7]     Image  : Factory image
RSP[ 7]     FPGA   : BP was reconfigured
RSP[ 7]     Result : OK
RSP[ 7]     Status : IN PROGRESS
RSP[ 8] RSU Status:
RSP[ 8]     Trigger: Board Reset
RSP[ 8]     Image  : Factory image
RSP[ 8]     FPGA   : BP was reconfigured
RSP[ 8]     Result : OK
RSP[ 8]     Status : IN PROGRESS
RSP[ 9] RSU Status:
RSP[ 9]     Trigger: Board Reset
RSP[ 9]     Image  : Factory image
RSP[ 9]     FPGA   : BP was reconfigured
RSP[ 9]     Result : OK
RSP[ 9]     Status : IN PROGRESS
RSP[10] RSU Status:
RSP[10]     Trigger: Board Reset
RSP[10]     Image  : Factory image
RSP[10]     FPGA   : BP was reconfigured
RSP[10]     Result : OK
RSP[10]     Status : IN PROGRESS
RSP[11] RSU Status:
RSP[11]     Trigger: Board Reset
RSP[11]     Image  : Factory image
RSP[11]     FPGA   : BP was reconfigured
RSP[11]     Result : OK
RSP[11]     Status : IN PROGRESS
RSP[ 0] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 0]     0:                  0             0
RSP[ 0]     1:                  0             0
RSP[ 0]     2:                  0             0
RSP[ 0]     3:                  0             0
RSP[ 1] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 1]     0:                  0             0
RSP[ 1]     1:                  0             0
RSP[ 1]     2:                  0             0
RSP[ 1]     3:                  0             0
RSP[ 2] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 2]     0:                  0             0
RSP[ 2]     1:                  0             0
RSP[ 2]     2:                  0             0
RSP[ 2]     3:                  0             0
RSP[ 3] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 3]     0:                  0             0
RSP[ 3]     1:                  0             0
RSP[ 3]     2:                  0             0
RSP[ 3]     3:                  0             0
RSP[ 4] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 4]     0:                  0             0
RSP[ 4]     1:                  0             0
RSP[ 4]     2:                  0             0
RSP[ 4]     3:                  0             0
RSP[ 5] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 5]     0:                  0             0
RSP[ 5]     1:                  0             0
RSP[ 5]     2:                  0             0
RSP[ 5]     3:                  0             0
RSP[ 6] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 6]     0:                  0             0
RSP[ 6]     1:                  0             0
RSP[ 6]     2:                  0             0
RSP[ 6]     3:                  0             0
RSP[ 7] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 7]     0:                  0             0
RSP[ 7]     1:                  0             0
RSP[ 7]     2:                  0             0
RSP[ 7]     3:                  0             0
RSP[ 8] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 8]     0:                  0             0
RSP[ 8]     1:                  0             0
RSP[ 8]     2:                  0             0
RSP[ 8]     3:                  0             0
RSP[ 9] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 9]     0:                  0             0
RSP[ 9]     1:                  0             0
RSP[ 9]     2:                  0             0
RSP[ 9]     3:                  0             0
RSP[10] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[10]     0:                  0             0
RSP[10]     1:                  0             0
RSP[10]     2:                  0             0
RSP[10]     3:                  0             0
RSP[11] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[11]     0:                  0             0
RSP[11]     1:                  0             0
RSP[11]     2:                  0             0
RSP[11]     3:                  0             0
RSP[ 0] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 0]              ri:      _   ERROR      OK             0
RSP[ 0] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 0] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 0] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 0] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 0] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 0] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 0] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 0] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 1] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 1]              ri:      _   ERROR      OK             0
RSP[ 1] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 1] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 1] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 1] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 1] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 1] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 1] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 1] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 2] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 2]              ri:      _   ERROR      OK             0
RSP[ 2] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 2] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 2] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 2] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 2] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 2] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 2] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 2] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 3] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 3]              ri:      _   ERROR      OK             0
RSP[ 3] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 3] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 3] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 3] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 3] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 3] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 3] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 3] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 4] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 4]              ri:      _   ERROR      OK             0
RSP[ 4] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 4] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 4] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 4] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 4] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 4] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 4] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 4] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 5] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 5]              ri:      _   ERROR      OK             0
RSP[ 5] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 5] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 5] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 5] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 5] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 5] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 5] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 5] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 6] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 6]              ri:      _   ERROR      OK             0
RSP[ 6] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 6] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 6] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 6] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 6] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 6] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 6] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 6] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 7] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 7]              ri:      _   ERROR      OK             0
RSP[ 7] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 7] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 7] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 7] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 7] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 7] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 7] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 7] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 8] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 8]              ri:      _   ERROR      OK             0
RSP[ 8] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 8] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 8] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 8] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 8] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 8] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 8] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 8] lane3  beamlets:     OK   ERROR      OK             0
RSP[ 9] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 9]              ri:      _   ERROR      OK             0
RSP[ 9] lane0 crosslets:     OK   ERROR      OK             0
RSP[ 9] lane0  beamlets:     OK   ERROR      OK             0
RSP[ 9] lane1 crosslets:     OK   ERROR      OK             0
RSP[ 9] lane1  beamlets:     OK   ERROR      OK             0
RSP[ 9] lane2 crosslets:     OK   ERROR      OK             0
RSP[ 9] lane2  beamlets:     OK   ERROR      OK             0
RSP[ 9] lane3 crosslets:     OK   ERROR      OK             0
RSP[ 9] lane3  beamlets:     OK   ERROR      OK             0
RSP[10] RAD Status        Align    Sync     CRC     Frame cnt
RSP[10]              ri:      _   ERROR      OK             0
RSP[10] lane0 crosslets:     OK   ERROR      OK             0
RSP[10] lane0  beamlets:     OK   ERROR      OK             0
RSP[10] lane1 crosslets:     OK   ERROR      OK             0
RSP[10] lane1  beamlets:     OK   ERROR      OK             0
RSP[10] lane2 crosslets:     OK   ERROR      OK             0
RSP[10] lane2  beamlets:     OK   ERROR      OK             0
RSP[10] lane3 crosslets:     OK   ERROR      OK             0
RSP[10] lane3  beamlets:     OK   ERROR      OK             0
RSP[11] RAD Status        Align    Sync     CRC     Frame cnt
RSP[11]              ri:      _   ERROR      OK             0
RSP[11] lane0 crosslets:     OK   ERROR      OK             0
RSP[11] lane0  beamlets:     OK   ERROR      OK             0
RSP[11] lane1 crosslets:     OK   ERROR      OK             0
RSP[11] lane1  beamlets:     OK   ERROR      OK             0
RSP[11] lane2 crosslets:     OK   ERROR      OK             0
RSP[11] lane2  beamlets:     OK   ERROR      OK             0
RSP[11] lane3 crosslets:     OK   ERROR      OK             0
RSP[11] lane3  beamlets:     OK   ERROR      OK             0
''')
    elif prog_name == 'rspctl-even':
        print('''RSP[ 0] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 1] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.26
RSP[ 2] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.20
RSP[ 3] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.20
RSP[ 4] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 5] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 6] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 7] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 8] 1.2 V: 1.18 , 2.5 V: 2.51, 3.3 V: 3.23
RSP[ 9] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[10] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[11] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 0] PCB_temp: 20 , BP_temp: 47, Temp AP0:  28 , AP1:  39 , AP2:  36 , AP3:  38
RSP[ 1] PCB_temp: 21 , BP_temp: 40, Temp AP0:  29 , AP1:  37 , AP2:  35 , AP3:  38
RSP[ 2] PCB_temp: 21 , BP_temp: 44, Temp AP0:  28 , AP1:  35 , AP2:  33 , AP3:  36
RSP[ 3] PCB_temp: 20 , BP_temp: 40, Temp AP0:  29 , AP1:  36 , AP2:  36 , AP3:  38
RSP[ 4] PCB_temp: 19 , BP_temp: 48, Temp AP0:  29 , AP1:  37 , AP2:  34 , AP3:  36
RSP[ 5] PCB_temp: 19 , BP_temp: 43, Temp AP0:  29 , AP1:  35 , AP2:  33 , AP3:  37
RSP[ 6] PCB_temp: 20 , BP_temp: 43, Temp AP0:  31 , AP1:  36 , AP2:  34 , AP3:  38
RSP[ 7] PCB_temp: 20 , BP_temp: 44, Temp AP0:  32 , AP1:  36 , AP2:  35 , AP3:  38
RSP[ 8] PCB_temp: 20 , BP_temp: 53, Temp AP0:  29 , AP1:  38 , AP2:  34 , AP3:  38
RSP[ 9] PCB_temp: 20 , BP_temp: 43, Temp AP0:  28 , AP1:  36 , AP2:  34 , AP3:  36
RSP[10] PCB_temp: 20 , BP_temp: 44, Temp AP0:  29 , AP1:  34 , AP2:  33 , AP3:  36
RSP[11] PCB_temp: 19 , BP_temp: 43, Temp AP0:  29 , AP1:  35 , AP2:  34 , AP3:  37
RSP[ 0] BP_clock: 200
RSP[ 1] BP_clock: 200
RSP[ 2] BP_clock: 200
RSP[ 3] BP_clock: 200
RSP[ 4] BP_clock: 200
RSP[ 5] BP_clock: 200
RSP[ 6] BP_clock: 200
RSP[ 7] BP_clock: 200
RSP[ 8] BP_clock: 200
RSP[ 9] BP_clock: 200
RSP[10] BP_clock: 200
RSP[11] BP_clock: 200
RSP[ 0] Ethernet nr frames: 33139 , nr errors: 0 , last error: 0
RSP[ 1] Ethernet nr frames: 32560 , nr errors: 0 , last error: 0
RSP[ 2] Ethernet nr frames: 32560 , nr errors: 0 , last error: 0
RSP[ 3] Ethernet nr frames: 32560 , nr errors: 0 , last error: 0
RSP[ 4] Ethernet nr frames: 33131 , nr errors: 0 , last error: 0
RSP[ 5] Ethernet nr frames: 32552 , nr errors: 0 , last error: 0
RSP[ 6] Ethernet nr frames: 32548 , nr errors: 0 , last error: 0
RSP[ 7] Ethernet nr frames: 32546 , nr errors: 0 , last error: 0
RSP[ 8] Ethernet nr frames: 33135 , nr errors: 0 , last error: 0
RSP[ 9] Ethernet nr frames: 32550 , nr errors: 0 , last error: 0
RSP[10] Ethernet nr frames: 32551 , nr errors: 0 , last error: 0
RSP[11] Ethernet nr frames: 32554 , nr errors: 0 , last error: 0
RSP[ 0] MEP sequencenr: 0 , error: 0
RSP[ 1] MEP sequencenr: 0 , error: 0
RSP[ 2] MEP sequencenr: 0 , error: 0
RSP[ 3] MEP sequencenr: 0 , error: 0
RSP[ 4] MEP sequencenr: 0 , error: 0
RSP[ 5] MEP sequencenr: 0 , error: 0
RSP[ 6] MEP sequencenr: 0 , error: 0
RSP[ 7] MEP sequencenr: 0 , error: 0
RSP[ 8] MEP sequencenr: 0 , error: 0
RSP[ 9] MEP sequencenr: 0 , error: 0
RSP[10] MEP sequencenr: 0 , error: 0
RSP[11] MEP sequencenr: 0 , error: 0
RSP[ 0] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 0]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 1] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 1]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 2] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 2]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 3] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 3]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 4] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 4]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 5] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 5]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 6] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 6]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 7] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 7]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 8] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 8]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 9] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 9]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[10] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[10]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[11] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[11]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 0] Sync         diff      count    samples     slices
RSP[ 0]     0:          0        294 2965425152     195312
RSP[ 0]     1:          0        294 2965425152     195312
RSP[ 0]     2:          0        294 2965425152     195312
RSP[ 0]     3:          0        294 2965425152     195312
RSP[ 1] Sync         diff      count    samples     slices
RSP[ 1]     0:          1        294 2965425152     195312
RSP[ 1]     1:          0        294 2965425152     195312
RSP[ 1]     2:          0        294 2965425152     195312
RSP[ 1]     3:          0        294 2965425152     195312
RSP[ 2] Sync         diff      count    samples     slices
RSP[ 2]     0:          0        294 2965425152     195312
RSP[ 2]     1:          1        294 2965425152     195312
RSP[ 2]     2:          0        294 2965425152     195312
RSP[ 2]     3:          0        294 2965425152     195312
RSP[ 3] Sync         diff      count    samples     slices
RSP[ 3]     0:          0        294 2965425152     195312
RSP[ 3]     1:          0        294 2965425152     195312
RSP[ 3]     2:          1        294 2965425152     195312
RSP[ 3]     3:          0        294 2965425152     195312
RSP[ 4] Sync         diff      count    samples     slices
RSP[ 4]     0:          0        294 2965425152     195312
RSP[ 4]     1:          0        294 2965425152     195312
RSP[ 4]     2:          0        294 2965425152     195312
RSP[ 4]     3:          1        294 2965425152     195312
RSP[ 5] Sync         diff      count    samples     slices
RSP[ 5]     0:          0        294 2965425152     195312
RSP[ 5]     1:          0        294 2965425152     195312
RSP[ 5]     2:          0        294 2965425152     195312
RSP[ 5]     3:          0        294 2965425152     195312
RSP[ 6] Sync         diff      count    samples     slices
RSP[ 6]     0:          0        294 2965425152     195312
RSP[ 6]     1:          0        294 2965425152     195312
RSP[ 6]     2:          0        294 2965425152     195312
RSP[ 6]     3:          0        294 2965425152     195312
RSP[ 7] Sync         diff      count    samples     slices
RSP[ 7]     0:          0        294 2965425152     195312
RSP[ 7]     1:          0        294 2965425152     195312
RSP[ 7]     2:          0        294 2965425152     195312
RSP[ 7]     3:          0        294 2965425152     195312
RSP[ 8] Sync         diff      count    samples     slices
RSP[ 8]     0:          0        294 2965425152     195312
RSP[ 8]     1:          0        294 2965425152     195312
RSP[ 8]     2:          0        294 2965425152     195312
RSP[ 8]     3:          0        294 2965425152     195312
RSP[ 9] Sync         diff      count    samples     slices
RSP[ 9]     0:          0        294 2965425152     195312
RSP[ 9]     1:          0        294 2965425152     195312
RSP[ 9]     2:          0        294 2965425152     195312
RSP[ 9]     3:          0        294 2965425152     195312
RSP[10] Sync         diff      count    samples     slices
RSP[10]     0:          0        294 2965425152     195312
RSP[10]     1:          0        294 2965425152     195312
RSP[10]     2:          0        294 2965425152     195312
RSP[10]     3:          0        294 2965425152     195312
RSP[11] Sync         diff      count    samples     slices
RSP[11]     0:          0        294 2965425152     195312
RSP[11]     1:          0        294 2965425152     195312
RSP[11]     2:          0        294 2965425152     195312
RSP[11]     3:          0        294 2965425152     195312
RSP[ 0] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 0]     0:          0          0          0          0
RSP[ 0]     1:          0          0          0          0
RSP[ 0]     2:          0          0          0          0
RSP[ 0]     3:          0          0          0          0
RSP[ 1] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 1]     0:          0          0          0          0
RSP[ 1]     1:          0          0          0          0
RSP[ 1]     2:          0          0          0          0
RSP[ 1]     3:          0          0          0          0
RSP[ 2] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 2]     0:          0          0          0          0
RSP[ 2]     1:          0          0          0          0
RSP[ 2]     2:          0          0          0          0
RSP[ 2]     3:          0          0          0          0
RSP[ 3] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 3]     0:          0          0          0          0
RSP[ 3]     1:          0          0          0          0
RSP[ 3]     2:          0          0          0          0
RSP[ 3]     3:          0          0          0          0
RSP[ 4] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 4]     0:          0          0          0          0
RSP[ 4]     1:          0          0          0          0
RSP[ 4]     2:          0          0          0          0
RSP[ 4]     3:          0          0          0          0
RSP[ 5] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 5]     0:          0          0          0          0
RSP[ 5]     1:          0          0          0          0
RSP[ 5]     2:          0          0          0          0
RSP[ 5]     3:          0          0          0          0
RSP[ 6] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 6]     0:          0          0          0          0
RSP[ 6]     1:          0          0          0          0
RSP[ 6]     2:          0          0          0          0
RSP[ 6]     3:          0          0          0          0
RSP[ 7] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 7]     0:          0          0          0          0
RSP[ 7]     1:          0          0          0          0
RSP[ 7]     2:          0          0          0          0
RSP[ 7]     3:          0          0          0          0
RSP[ 8] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 8]     0:          0          0          0          0
RSP[ 8]     1:          0          0          0          0
RSP[ 8]     2:          0          0          0          0
RSP[ 8]     3:          0          0          0          0
RSP[ 9] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 9]     0:          0          0          0          0
RSP[ 9]     1:          0          0          0          0
RSP[ 9]     2:          0          0          0          0
RSP[ 9]     3:          0          0          0          0
RSP[10] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[10]     0:          0          0          0          0
RSP[10]     1:          0          0          0          0
RSP[10]     2:          0          0          0          0
RSP[10]     3:          0          0          0          0
RSP[11] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[11]     0:          0          0          0          0
RSP[11]     1:          0          0          0          0
RSP[11]     2:          0          0          0          0
RSP[11]     3:          0          0          0          0
RSP[ 0] RSU Status:
RSP[ 0]     Trigger: User reset request
RSP[ 0]     Image  : Application image
RSP[ 0]     FPGA   : BP was reconfigured
RSP[ 0]     Result : OK
RSP[ 0]     Status : DONE
RSP[ 1] RSU Status:
RSP[ 1]     Trigger: User reset request
RSP[ 1]     Image  : Application image
RSP[ 1]     FPGA   : BP was reconfigured
RSP[ 1]     Result : OK
RSP[ 1]     Status : DONE
RSP[ 2] RSU Status:
RSP[ 2]     Trigger: User reset request
RSP[ 2]     Image  : Application image
RSP[ 2]     FPGA   : BP was reconfigured
RSP[ 2]     Result : OK
RSP[ 2]     Status : DONE
RSP[ 3] RSU Status:
RSP[ 3]     Trigger: User reset request
RSP[ 3]     Image  : Application image
RSP[ 3]     FPGA   : BP was reconfigured
RSP[ 3]     Result : OK
RSP[ 3]     Status : DONE
RSP[ 4] RSU Status:
RSP[ 4]     Trigger: User reset request
RSP[ 4]     Image  : Application image
RSP[ 4]     FPGA   : BP was reconfigured
RSP[ 4]     Result : OK
RSP[ 4]     Status : DONE
RSP[ 5] RSU Status:
RSP[ 5]     Trigger: User reset request
RSP[ 5]     Image  : Application image
RSP[ 5]     FPGA   : BP was reconfigured
RSP[ 5]     Result : OK
RSP[ 5]     Status : DONE
RSP[ 6] RSU Status:
RSP[ 6]     Trigger: User reset request
RSP[ 6]     Image  : Application image
RSP[ 6]     FPGA   : BP was reconfigured
RSP[ 6]     Result : OK
RSP[ 6]     Status : DONE
RSP[ 7] RSU Status:
RSP[ 7]     Trigger: User reset request
RSP[ 7]     Image  : Application image
RSP[ 7]     FPGA   : BP was reconfigured
RSP[ 7]     Result : OK
RSP[ 7]     Status : DONE
RSP[ 8] RSU Status:
RSP[ 8]     Trigger: User reset request
RSP[ 8]     Image  : Application image
RSP[ 8]     FPGA   : BP was reconfigured
RSP[ 8]     Result : OK
RSP[ 8]     Status : DONE
RSP[ 9] RSU Status:
RSP[ 9]     Trigger: User reset request
RSP[ 9]     Image  : Application image
RSP[ 9]     FPGA   : BP was reconfigured
RSP[ 9]     Result : OK
RSP[ 9]     Status : DONE
RSP[10] RSU Status:
RSP[10]     Trigger: User reset request
RSP[10]     Image  : Application image
RSP[10]     FPGA   : BP was reconfigured
RSP[10]     Result : OK
RSP[10]     Status : DONE
RSP[11] RSU Status:
RSP[11]     Trigger: User reset request
RSP[11]     Image  : Application image
RSP[11]     FPGA   : BP was reconfigured
RSP[11]     Result : OK
RSP[11]     Status : DONE
RSP[ 0] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 0]     0:                  0             0
RSP[ 0]     1:                  0             0
RSP[ 0]     2:                  0             0
RSP[ 0]     3:                  0             0
RSP[ 1] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 1]     0:                  0             0
RSP[ 1]     1:                  0             0
RSP[ 1]     2:                  0             0
RSP[ 1]     3:                  0             0
RSP[ 2] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 2]     0:                  0             0
RSP[ 2]     1:                  0             0
RSP[ 2]     2:                  0             0
RSP[ 2]     3:                  0             0
RSP[ 3] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 3]     0:                  0             0
RSP[ 3]     1:                  0             0
RSP[ 3]     2:                  0             0
RSP[ 3]     3:                  0             0
RSP[ 4] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 4]     0:                  0             0
RSP[ 4]     1:                  0             0
RSP[ 4]     2:                  0             0
RSP[ 4]     3:                  0             0
RSP[ 5] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 5]     0:                  0             0
RSP[ 5]     1:                  0             0
RSP[ 5]     2:                  0             0
RSP[ 5]     3:                  0             0
RSP[ 6] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 6]     0:                  0             0
RSP[ 6]     1:                  0             0
RSP[ 6]     2:                  0             0
RSP[ 6]     3:                  0             0
RSP[ 7] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 7]     0:                  0             0
RSP[ 7]     1:                  0             0
RSP[ 7]     2:                  0             0
RSP[ 7]     3:                  0             0
RSP[ 8] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 8]     0:                  0             0
RSP[ 8]     1:                  0             0
RSP[ 8]     2:                  0             0
RSP[ 8]     3:                  0             0
RSP[ 9] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 9]     0:                  0             0
RSP[ 9]     1:                  0             0
RSP[ 9]     2:                  0             0
RSP[ 9]     3:                  0             0
RSP[10] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[10]     0:                  0             0
RSP[10]     1:                  0             0
RSP[10]     2:                  0             0
RSP[10]     3:                  0             0
RSP[11] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[11]     0:                  0             0
RSP[11]     1:                  0             0
RSP[11]     2:                  0             0
RSP[11]     3:                  0             0
RSP[ 0] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 0]              ri:      _      OK      OK        195313
RSP[ 0] lane0 crosslets:     OK      OK      OK        195313
RSP[ 0] lane0  beamlets:     OK      OK      OK        195313
RSP[ 0] lane1 crosslets:     OK      OK      OK        195313
RSP[ 0] lane1  beamlets:     OK      OK      OK        195313
RSP[ 0] lane2 crosslets:     OK      OK      OK        195313
RSP[ 0] lane2  beamlets:     OK      OK      OK        195313
RSP[ 0] lane3 crosslets:     OK      OK      OK        195313
RSP[ 0] lane3  beamlets:     OK      OK      OK        195313
RSP[ 1] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 1]              ri:      _      OK      OK        195313
RSP[ 1] lane0 crosslets:     OK      OK      OK        195313
RSP[ 1] lane0  beamlets:     OK      OK      OK        195313
RSP[ 1] lane1 crosslets:     OK      OK      OK        195313
RSP[ 1] lane1  beamlets:     OK      OK      OK        195313
RSP[ 1] lane2 crosslets:     OK      OK      OK        195313
RSP[ 1] lane2  beamlets:     OK      OK      OK        195313
RSP[ 1] lane3 crosslets:     OK      OK      OK        195313
RSP[ 1] lane3  beamlets:     OK      OK      OK        195313
RSP[ 2] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 2]              ri:      _      OK      OK        195313
RSP[ 2] lane0 crosslets:     OK      OK      OK        195313
RSP[ 2] lane0  beamlets:     OK      OK      OK        195313
RSP[ 2] lane1 crosslets:     OK      OK      OK        195313
RSP[ 2] lane1  beamlets:     OK      OK      OK        195313
RSP[ 2] lane2 crosslets:     OK      OK      OK        195313
RSP[ 2] lane2  beamlets:     OK      OK      OK        195313
RSP[ 2] lane3 crosslets:     OK      OK      OK        195313
RSP[ 2] lane3  beamlets:     OK      OK      OK        195313
RSP[ 3] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 3]              ri:      _      OK      OK        195313
RSP[ 3] lane0 crosslets:     OK      OK      OK        195313
RSP[ 3] lane0  beamlets:     OK      OK      OK        195313
RSP[ 3] lane1 crosslets:     OK      OK      OK        195313
RSP[ 3] lane1  beamlets:     OK      OK      OK        195313
RSP[ 3] lane2 crosslets:     OK      OK      OK        195313
RSP[ 3] lane2  beamlets:     OK      OK      OK        195313
RSP[ 3] lane3 crosslets:     OK      OK      OK        195313
RSP[ 3] lane3  beamlets:     OK      OK      OK        195313
RSP[ 4] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 4]              ri:      _      OK      OK        195313
RSP[ 4] lane0 crosslets:     OK      OK      OK        195313
RSP[ 4] lane0  beamlets:     OK      OK      OK        195313
RSP[ 4] lane1 crosslets:     OK      OK      OK        195313
RSP[ 4] lane1  beamlets:     OK      OK      OK        195313
RSP[ 4] lane2 crosslets:     OK      OK      OK        195313
RSP[ 4] lane2  beamlets:     OK      OK      OK        195313
RSP[ 4] lane3 crosslets:     OK      OK      OK        195313
RSP[ 4] lane3  beamlets:     OK      OK      OK        195313
RSP[ 5] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 5]              ri:      _      OK      OK        195313
RSP[ 5] lane0 crosslets:     OK      OK      OK        195313
RSP[ 5] lane0  beamlets:     OK      OK      OK        195313
RSP[ 5] lane1 crosslets:     OK      OK      OK        195313
RSP[ 5] lane1  beamlets:     OK      OK      OK        195313
RSP[ 5] lane2 crosslets:     OK      OK      OK        195313
RSP[ 5] lane2  beamlets:     OK      OK      OK        195313
RSP[ 5] lane3 crosslets:     OK      OK      OK        195313
RSP[ 5] lane3  beamlets:     OK      OK      OK        195313
RSP[ 6] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 6]              ri:      _      OK      OK        195313
RSP[ 6] lane0 crosslets:     OK      OK      OK        195313
RSP[ 6] lane0  beamlets:     OK      OK      OK        195313
RSP[ 6] lane1 crosslets:     OK      OK      OK        195313
RSP[ 6] lane1  beamlets:     OK      OK      OK        195313
RSP[ 6] lane2 crosslets:     OK      OK      OK        195313
RSP[ 6] lane2  beamlets:     OK      OK      OK        195313
RSP[ 6] lane3 crosslets:     OK      OK      OK        195313
RSP[ 6] lane3  beamlets:     OK      OK      OK        195313
RSP[ 7] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 7]              ri:      _      OK      OK        195313
RSP[ 7] lane0 crosslets:     OK      OK      OK        195313
RSP[ 7] lane0  beamlets:     OK      OK      OK        195313
RSP[ 7] lane1 crosslets:     OK      OK      OK        195313
RSP[ 7] lane1  beamlets:     OK      OK      OK        195313
RSP[ 7] lane2 crosslets:     OK      OK      OK        195313
RSP[ 7] lane2  beamlets:     OK      OK      OK        195313
RSP[ 7] lane3 crosslets:     OK      OK      OK        195313
RSP[ 7] lane3  beamlets:     OK      OK      OK        195313
RSP[ 8] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 8]              ri:      _      OK      OK        195313
RSP[ 8] lane0 crosslets:     OK      OK      OK        195313
RSP[ 8] lane0  beamlets:     OK      OK      OK        195313
RSP[ 8] lane1 crosslets:     OK      OK      OK        195313
RSP[ 8] lane1  beamlets:     OK      OK      OK        195313
RSP[ 8] lane2 crosslets:     OK      OK      OK        195313
RSP[ 8] lane2  beamlets:     OK      OK      OK        195313
RSP[ 8] lane3 crosslets:     OK      OK      OK        195313
RSP[ 8] lane3  beamlets:     OK      OK      OK        195313
RSP[ 9] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 9]              ri:      _      OK      OK        195313
RSP[ 9] lane0 crosslets:     OK      OK      OK        195313
RSP[ 9] lane0  beamlets:     OK      OK      OK        195313
RSP[ 9] lane1 crosslets:     OK      OK      OK        195313
RSP[ 9] lane1  beamlets:     OK      OK      OK        195313
RSP[ 9] lane2 crosslets:     OK      OK      OK        195313
RSP[ 9] lane2  beamlets:     OK      OK      OK        195313
RSP[ 9] lane3 crosslets:     OK      OK      OK        195313
RSP[ 9] lane3  beamlets:     OK      OK      OK        195313
RSP[10] RAD Status        Align    Sync     CRC     Frame cnt
RSP[10]              ri:      _      OK      OK        195313
RSP[10] lane0 crosslets:     OK      OK      OK        195313
RSP[10] lane0  beamlets:     OK      OK      OK        195313
RSP[10] lane1 crosslets:     OK      OK      OK        195313
RSP[10] lane1  beamlets:     OK      OK      OK        195313
RSP[10] lane2 crosslets:     OK      OK      OK        195313
RSP[10] lane2  beamlets:     OK      OK      OK        195313
RSP[10] lane3 crosslets:     OK      OK      OK        195313
RSP[10] lane3  beamlets:     OK      OK      OK        195313
RSP[11] RAD Status        Align    Sync     CRC     Frame cnt
RSP[11]              ri:      _      OK      OK        195313
RSP[11] lane0 crosslets:     OK      OK      OK        195313
RSP[11] lane0  beamlets:     OK      OK      OK        195313
RSP[11] lane1 crosslets:     OK      OK      OK        195313
RSP[11] lane1  beamlets:     OK      OK      OK        195313
RSP[11] lane2 crosslets:     OK      OK      OK        195313
RSP[11] lane2  beamlets:     OK      OK      OK        195313
RSP[11] lane3 crosslets:     OK      OK      OK        195313
RSP[11] lane3  beamlets:     OK      OK      OK        195313
''')
    elif prog_name == 'rspctl-odd':
        print('''RSP[ 0] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 1] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.26
RSP[ 2] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.20
RSP[ 3] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 4] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 5] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 6] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 7] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 8] 1.2 V: 1.18 , 2.5 V: 2.51, 3.3 V: 3.23
RSP[ 9] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[10] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[11] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 0] PCB_temp: 20 , BP_temp: 47, Temp AP0:  28 , AP1:  39 , AP2:  37 , AP3:  38
RSP[ 1] PCB_temp: 20 , BP_temp: 40, Temp AP0:  29 , AP1:  37 , AP2:  34 , AP3:  38
RSP[ 2] PCB_temp: 21 , BP_temp: 44, Temp AP0:  27 , AP1:  35 , AP2:  34 , AP3:  36
RSP[ 3] PCB_temp: 19 , BP_temp: 40, Temp AP0:  28 , AP1:  36 , AP2:  36 , AP3:  38
RSP[ 4] PCB_temp: 19 , BP_temp: 48, Temp AP0:  29 , AP1:  37 , AP2:  34 , AP3:  36
RSP[ 5] PCB_temp: 19 , BP_temp: 42, Temp AP0:  28 , AP1:  35 , AP2:  33 , AP3:  36
RSP[ 6] PCB_temp: 20 , BP_temp: 42, Temp AP0:  30 , AP1:  36 , AP2:  34 , AP3:  37
RSP[ 7] PCB_temp: 20 , BP_temp: 44, Temp AP0:  32 , AP1:  36 , AP2:  35 , AP3:  37
RSP[ 8] PCB_temp: 20 , BP_temp: 53, Temp AP0:  30 , AP1:  38 , AP2:  34 , AP3:  38
RSP[ 9] PCB_temp: 20 , BP_temp: 43, Temp AP0:  28 , AP1:  36 , AP2:  34 , AP3:  36
RSP[10] PCB_temp: 20 , BP_temp: 44, Temp AP0:  29 , AP1:  34 , AP2:  33 , AP3:  36
RSP[11] PCB_temp: 20 , BP_temp: 43, Temp AP0:  29 , AP1:  35 , AP2:  34 , AP3:  37
RSP[ 0] BP_clock: 200
RSP[ 1] BP_clock: 200
RSP[ 2] BP_clock: 200
RSP[ 3] BP_clock: 200
RSP[ 4] BP_clock: 200
RSP[ 5] BP_clock: 200
RSP[ 6] BP_clock: 200
RSP[ 7] BP_clock: 200
RSP[ 8] BP_clock: 200
RSP[ 9] BP_clock: 200
RSP[10] BP_clock: 200
RSP[11] BP_clock: 200
RSP[ 0] Ethernet nr frames: 38953 , nr errors: 0 , last error: 0
RSP[ 1] Ethernet nr frames: 38272 , nr errors: 0 , last error: 0
RSP[ 2] Ethernet nr frames: 38272 , nr errors: 0 , last error: 0
RSP[ 3] Ethernet nr frames: 38272 , nr errors: 0 , last error: 0
RSP[ 4] Ethernet nr frames: 38945 , nr errors: 0 , last error: 0
RSP[ 5] Ethernet nr frames: 38264 , nr errors: 0 , last error: 0
RSP[ 6] Ethernet nr frames: 38260 , nr errors: 0 , last error: 0
RSP[ 7] Ethernet nr frames: 38258 , nr errors: 0 , last error: 0
RSP[ 8] Ethernet nr frames: 38949 , nr errors: 0 , last error: 0
RSP[ 9] Ethernet nr frames: 38262 , nr errors: 0 , last error: 0
RSP[10] Ethernet nr frames: 38263 , nr errors: 0 , last error: 0
RSP[11] Ethernet nr frames: 38266 , nr errors: 0 , last error: 0
RSP[ 0] MEP sequencenr: 0 , error: 0
RSP[ 1] MEP sequencenr: 0 , error: 0
RSP[ 2] MEP sequencenr: 0 , error: 0
RSP[ 3] MEP sequencenr: 0 , error: 0
RSP[ 4] MEP sequencenr: 0 , error: 0
RSP[ 5] MEP sequencenr: 0 , error: 0
RSP[ 6] MEP sequencenr: 0 , error: 0
RSP[ 7] MEP sequencenr: 0 , error: 0
RSP[ 8] MEP sequencenr: 0 , error: 0
RSP[ 9] MEP sequencenr: 0 , error: 0
RSP[10] MEP sequencenr: 0 , error: 0
RSP[11] MEP sequencenr: 0 , error: 0
RSP[ 0] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 0]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 1] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 1]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 2] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 2]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 3] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 3]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 4] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 4]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 5] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 5]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 6] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 6]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 7] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 7]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 8] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 8]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 9] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 9]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[10] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[10]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[11] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[11]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 0] Sync         diff      count    samples     slices
RSP[ 0]     0:        512        345  280523776     195313
RSP[ 0]     1:        512        345  280523776     195313
RSP[ 0]     2:        512        345  280523776     195313
RSP[ 0]     3:        512        345  280523776     195313
RSP[ 1] Sync         diff      count    samples     slices
RSP[ 1]     0:        512        345  280523776     195313
RSP[ 1]     1:        512        345  280523776     195313
RSP[ 1]     2:        512        345  280523776     195313
RSP[ 1]     3:        512        345  280523776     195313
RSP[ 2] Sync         diff      count    samples     slices
RSP[ 2]     0:        512        345  280523776     195313
RSP[ 2]     1:        512        345  280523776     195313
RSP[ 2]     2:        512        345  280523776     195313
RSP[ 2]     3:        512        345  280523776     195313
RSP[ 3] Sync         diff      count    samples     slices
RSP[ 3]     0:        512        345  280523776     195313
RSP[ 3]     1:        512        345  280523776     195313
RSP[ 3]     2:        512        345  280523776     195313
RSP[ 3]     3:        512        345  280523776     195313
RSP[ 4] Sync         diff      count    samples     slices
RSP[ 4]     0:        512        345  280523776     195313
RSP[ 4]     1:        512        345  280523776     195313
RSP[ 4]     2:        512        345  280523776     195313
RSP[ 4]     3:        512        345  280523776     195313
RSP[ 5] Sync         diff      count    samples     slices
RSP[ 5]     0:        512        345  280523776     195313
RSP[ 5]     1:        512        345  280523776     195313
RSP[ 5]     2:        512        345  280523776     195313
RSP[ 5]     3:        512        345  280523776     195313
RSP[ 6] Sync         diff      count    samples     slices
RSP[ 6]     0:        512        345  280523776     195313
RSP[ 6]     1:        512        345  280523776     195313
RSP[ 6]     2:        512        345  280523776     195313
RSP[ 6]     3:        512        345  280523776     195313
RSP[ 7] Sync         diff      count    samples     slices
RSP[ 7]     0:        512        345  280523776     195313
RSP[ 7]     1:        512        345  280523776     195313
RSP[ 7]     2:        512        345  280523776     195313
RSP[ 7]     3:        512        345  280523776     195313
RSP[ 8] Sync         diff      count    samples     slices
RSP[ 8]     0:        511        345  280523776     195313
RSP[ 8]     1:        512        345  280523776     195313
RSP[ 8]     2:        512        345  280523776     195313
RSP[ 8]     3:        512        345  280523776     195313
RSP[ 9] Sync         diff      count    samples     slices
RSP[ 9]     0:        512        345  280523776     195313
RSP[ 9]     1:        511        345  280523776     195313
RSP[ 9]     2:        512        345  280523776     195313
RSP[ 9]     3:        512        345  280523776     195313
RSP[10] Sync         diff      count    samples     slices
RSP[10]     0:        512        345  280523776     195313
RSP[10]     1:        512        345  280523776     195313
RSP[10]     2:        513        345  280523776     195313
RSP[10]     3:        512        345  280523776     195313
RSP[11] Sync         diff      count    samples     slices
RSP[11]     0:        512        345  280523776     195313
RSP[11]     1:        512        345  280523776     195313
RSP[11]     2:        512        345  280523776     195313
RSP[11]     3:        513        345  280523776     195313
RSP[ 0] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 0]     0:          0          0          0          0
RSP[ 0]     1:          0          0          0          0
RSP[ 0]     2:          0          0          0          0
RSP[ 0]     3:          0          0          0          0
RSP[ 1] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 1]     0:          0          0          0          0
RSP[ 1]     1:          0          0          0          0
RSP[ 1]     2:          0          0          0          0
RSP[ 1]     3:          0          0          0          0
RSP[ 2] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 2]     0:          0          0          0          0
RSP[ 2]     1:          0          0          0          0
RSP[ 2]     2:          0          0          0          0
RSP[ 2]     3:          0          0          0          0
RSP[ 3] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 3]     0:          0          0          0          0
RSP[ 3]     1:          0          0          0          0
RSP[ 3]     2:          0          0          0          0
RSP[ 3]     3:          0          0          0          0
RSP[ 4] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 4]     0:          0          0          0          0
RSP[ 4]     1:          0          0          0          0
RSP[ 4]     2:          0          0          0          0
RSP[ 4]     3:          0          0          0          0
RSP[ 5] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 5]     0:          0          0          0          0
RSP[ 5]     1:          0          0          0          0
RSP[ 5]     2:          0          0          0          0
RSP[ 5]     3:          0          0          0          0
RSP[ 6] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 6]     0:          0          0          0          0
RSP[ 6]     1:          0          0          0          0
RSP[ 6]     2:          0          0          0          0
RSP[ 6]     3:          0          0          0          0
RSP[ 7] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 7]     0:          0          0          0          0
RSP[ 7]     1:          0          0          0          0
RSP[ 7]     2:          0          0          0          0
RSP[ 7]     3:          0          0          0          0
RSP[ 8] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 8]     0:          0          0          0          0
RSP[ 8]     1:          0          0          0          0
RSP[ 8]     2:          0          0          0          0
RSP[ 8]     3:          0          0          0          0
RSP[ 9] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 9]     0:          0          0          0          0
RSP[ 9]     1:          0          0          0          0
RSP[ 9]     2:          0          0          0          0
RSP[ 9]     3:          0          0          0          0
RSP[10] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[10]     0:          0          0          0          0
RSP[10]     1:          0          0          0          0
RSP[10]     2:          0          0          0          0
RSP[10]     3:          0          0          0          0
RSP[11] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[11]     0:          0          0          0          0
RSP[11]     1:          0          0          0          0
RSP[11]     2:          0          0          0          0
RSP[11]     3:          0          0          0          0
RSP[ 0] RSU Status:
RSP[ 0]     Trigger: User reset request
RSP[ 0]     Image  : Application image
RSP[ 0]     FPGA   : BP was reconfigured
RSP[ 0]     Result : OK
RSP[ 0]     Status : DONE
RSP[ 1] RSU Status:
RSP[ 1]     Trigger: User reset request
RSP[ 1]     Image  : Application image
RSP[ 1]     FPGA   : BP was reconfigured
RSP[ 1]     Result : OK
RSP[ 1]     Status : DONE
RSP[ 2] RSU Status:
RSP[ 2]     Trigger: User reset request
RSP[ 2]     Image  : Application image
RSP[ 2]     FPGA   : BP was reconfigured
RSP[ 2]     Result : OK
RSP[ 2]     Status : DONE
RSP[ 3] RSU Status:
RSP[ 3]     Trigger: User reset request
RSP[ 3]     Image  : Application image
RSP[ 3]     FPGA   : BP was reconfigured
RSP[ 3]     Result : OK
RSP[ 3]     Status : DONE
RSP[ 4] RSU Status:
RSP[ 4]     Trigger: User reset request
RSP[ 4]     Image  : Application image
RSP[ 4]     FPGA   : BP was reconfigured
RSP[ 4]     Result : OK
RSP[ 4]     Status : DONE
RSP[ 5] RSU Status:
RSP[ 5]     Trigger: User reset request
RSP[ 5]     Image  : Application image
RSP[ 5]     FPGA   : BP was reconfigured
RSP[ 5]     Result : OK
RSP[ 5]     Status : DONE
RSP[ 6] RSU Status:
RSP[ 6]     Trigger: User reset request
RSP[ 6]     Image  : Application image
RSP[ 6]     FPGA   : BP was reconfigured
RSP[ 6]     Result : OK
RSP[ 6]     Status : DONE
RSP[ 7] RSU Status:
RSP[ 7]     Trigger: User reset request
RSP[ 7]     Image  : Application image
RSP[ 7]     FPGA   : BP was reconfigured
RSP[ 7]     Result : OK
RSP[ 7]     Status : DONE
RSP[ 8] RSU Status:
RSP[ 8]     Trigger: User reset request
RSP[ 8]     Image  : Application image
RSP[ 8]     FPGA   : BP was reconfigured
RSP[ 8]     Result : OK
RSP[ 8]     Status : DONE
RSP[ 9] RSU Status:
RSP[ 9]     Trigger: User reset request
RSP[ 9]     Image  : Application image
RSP[ 9]     FPGA   : BP was reconfigured
RSP[ 9]     Result : OK
RSP[ 9]     Status : DONE
RSP[10] RSU Status:
RSP[10]     Trigger: User reset request
RSP[10]     Image  : Application image
RSP[10]     FPGA   : BP was reconfigured
RSP[10]     Result : OK
RSP[10]     Status : DONE
RSP[11] RSU Status:
RSP[11]     Trigger: User reset request
RSP[11]     Image  : Application image
RSP[11]     FPGA   : BP was reconfigured
RSP[11]     Result : OK
RSP[11]     Status : DONE
RSP[ 0] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 0]     0:                  0             0
RSP[ 0]     1:                  0             0
RSP[ 0]     2:                  0             0
RSP[ 0]     3:                  0             0
RSP[ 1] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 1]     0:                  0             0
RSP[ 1]     1:                  0             0
RSP[ 1]     2:                  0             0
RSP[ 1]     3:                  0             0
RSP[ 2] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 2]     0:                  0             0
RSP[ 2]     1:                  0             0
RSP[ 2]     2:                  0             0
RSP[ 2]     3:                  0             0
RSP[ 3] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 3]     0:                  0             0
RSP[ 3]     1:                  0             0
RSP[ 3]     2:                  0             0
RSP[ 3]     3:                  0             0
RSP[ 4] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 4]     0:                  0             0
RSP[ 4]     1:                  0             0
RSP[ 4]     2:                  0             0
RSP[ 4]     3:                  0             0
RSP[ 5] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 5]     0:                  0             0
RSP[ 5]     1:                  0             0
RSP[ 5]     2:                  0             0
RSP[ 5]     3:                  0             0
RSP[ 6] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 6]     0:                  0             0
RSP[ 6]     1:                  0             0
RSP[ 6]     2:                  0             0
RSP[ 6]     3:                  0             0
RSP[ 7] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 7]     0:                  0             0
RSP[ 7]     1:                  0             0
RSP[ 7]     2:                  0             0
RSP[ 7]     3:                  0             0
RSP[ 8] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 8]     0:                  0             0
RSP[ 8]     1:                  0             0
RSP[ 8]     2:                  0             0
RSP[ 8]     3:                  0             0
RSP[ 9] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 9]     0:                  0             0
RSP[ 9]     1:                  0             0
RSP[ 9]     2:                  0             0
RSP[ 9]     3:                  0             0
RSP[10] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[10]     0:                  0             0
RSP[10]     1:                  0             0
RSP[10]     2:                  0             0
RSP[10]     3:                  0             0
RSP[11] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[11]     0:                  0             0
RSP[11]     1:                  0             0
RSP[11]     2:                  0             0
RSP[11]     3:                  0             0
RSP[ 0] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 0]              ri:      _      OK      OK        195312
RSP[ 0] lane0 crosslets:     OK      OK      OK        195312
RSP[ 0] lane0  beamlets:     OK      OK      OK        195312
RSP[ 0] lane1 crosslets:     OK      OK      OK        195312
RSP[ 0] lane1  beamlets:     OK      OK      OK        195312
RSP[ 0] lane2 crosslets:     OK      OK      OK        195312
RSP[ 0] lane2  beamlets:     OK      OK      OK        195312
RSP[ 0] lane3 crosslets:     OK      OK      OK        195312
RSP[ 0] lane3  beamlets:     OK      OK      OK        195312
RSP[ 1] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 1]              ri:      _      OK      OK        195312
RSP[ 1] lane0 crosslets:     OK      OK      OK        195312
RSP[ 1] lane0  beamlets:     OK      OK      OK        195312
RSP[ 1] lane1 crosslets:     OK      OK      OK        195312
RSP[ 1] lane1  beamlets:     OK      OK      OK        195312
RSP[ 1] lane2 crosslets:     OK      OK      OK        195312
RSP[ 1] lane2  beamlets:     OK      OK      OK        195312
RSP[ 1] lane3 crosslets:     OK      OK      OK        195312
RSP[ 1] lane3  beamlets:     OK      OK      OK        195312
RSP[ 2] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 2]              ri:      _      OK      OK        195312
RSP[ 2] lane0 crosslets:     OK      OK      OK        195312
RSP[ 2] lane0  beamlets:     OK      OK      OK        195312
RSP[ 2] lane1 crosslets:     OK      OK      OK        195312
RSP[ 2] lane1  beamlets:     OK      OK      OK        195312
RSP[ 2] lane2 crosslets:     OK      OK      OK        195312
RSP[ 2] lane2  beamlets:     OK      OK      OK        195312
RSP[ 2] lane3 crosslets:     OK      OK      OK        195312
RSP[ 2] lane3  beamlets:     OK      OK      OK        195312
RSP[ 3] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 3]              ri:      _      OK      OK        195312
RSP[ 3] lane0 crosslets:     OK      OK      OK        195312
RSP[ 3] lane0  beamlets:     OK      OK      OK        195312
RSP[ 3] lane1 crosslets:     OK      OK      OK        195312
RSP[ 3] lane1  beamlets:     OK      OK      OK        195312
RSP[ 3] lane2 crosslets:     OK      OK      OK        195312
RSP[ 3] lane2  beamlets:     OK      OK      OK        195312
RSP[ 3] lane3 crosslets:     OK      OK      OK        195312
RSP[ 3] lane3  beamlets:     OK      OK      OK        195312
RSP[ 4] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 4]              ri:      _      OK      OK        195312
RSP[ 4] lane0 crosslets:     OK      OK      OK        195312
RSP[ 4] lane0  beamlets:     OK      OK      OK        195312
RSP[ 4] lane1 crosslets:     OK      OK      OK        195312
RSP[ 4] lane1  beamlets:     OK      OK      OK        195312
RSP[ 4] lane2 crosslets:     OK      OK      OK        195312
RSP[ 4] lane2  beamlets:     OK      OK      OK        195312
RSP[ 4] lane3 crosslets:     OK      OK      OK        195312
RSP[ 4] lane3  beamlets:     OK      OK      OK        195312
RSP[ 5] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 5]              ri:      _      OK      OK        195312
RSP[ 5] lane0 crosslets:     OK      OK      OK        195312
RSP[ 5] lane0  beamlets:     OK      OK      OK        195312
RSP[ 5] lane1 crosslets:     OK      OK      OK        195312
RSP[ 5] lane1  beamlets:     OK      OK      OK        195312
RSP[ 5] lane2 crosslets:     OK      OK      OK        195312
RSP[ 5] lane2  beamlets:     OK      OK      OK        195312
RSP[ 5] lane3 crosslets:     OK      OK      OK        195312
RSP[ 5] lane3  beamlets:     OK      OK      OK        195312
RSP[ 6] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 6]              ri:      _      OK      OK        195312
RSP[ 6] lane0 crosslets:     OK      OK      OK        195312
RSP[ 6] lane0  beamlets:     OK      OK      OK        195312
RSP[ 6] lane1 crosslets:     OK      OK      OK        195312
RSP[ 6] lane1  beamlets:     OK      OK      OK        195312
RSP[ 6] lane2 crosslets:     OK      OK      OK        195312
RSP[ 6] lane2  beamlets:     OK      OK      OK        195312
RSP[ 6] lane3 crosslets:     OK      OK      OK        195312
RSP[ 6] lane3  beamlets:     OK      OK      OK        195312
RSP[ 7] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 7]              ri:      _      OK      OK        195312
RSP[ 7] lane0 crosslets:     OK      OK      OK        195312
RSP[ 7] lane0  beamlets:     OK      OK      OK        195312
RSP[ 7] lane1 crosslets:     OK      OK      OK        195312
RSP[ 7] lane1  beamlets:     OK      OK      OK        195312
RSP[ 7] lane2 crosslets:     OK      OK      OK        195312
RSP[ 7] lane2  beamlets:     OK      OK      OK        195312
RSP[ 7] lane3 crosslets:     OK      OK      OK        195312
RSP[ 7] lane3  beamlets:     OK      OK      OK        195312
RSP[ 8] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 8]              ri:      _      OK      OK        195312
RSP[ 8] lane0 crosslets:     OK      OK      OK        195312
RSP[ 8] lane0  beamlets:     OK      OK      OK        195312
RSP[ 8] lane1 crosslets:     OK      OK      OK        195312
RSP[ 8] lane1  beamlets:     OK      OK      OK        195312
RSP[ 8] lane2 crosslets:     OK      OK      OK        195312
RSP[ 8] lane2  beamlets:     OK      OK      OK        195312
RSP[ 8] lane3 crosslets:     OK      OK      OK        195312
RSP[ 8] lane3  beamlets:     OK      OK      OK        195312
RSP[ 9] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 9]              ri:      _      OK      OK        195312
RSP[ 9] lane0 crosslets:     OK      OK      OK        195312
RSP[ 9] lane0  beamlets:     OK      OK      OK        195312
RSP[ 9] lane1 crosslets:     OK      OK      OK        195312
RSP[ 9] lane1  beamlets:     OK      OK      OK        195312
RSP[ 9] lane2 crosslets:     OK      OK      OK        195312
RSP[ 9] lane2  beamlets:     OK      OK      OK        195312
RSP[ 9] lane3 crosslets:     OK      OK      OK        195312
RSP[ 9] lane3  beamlets:     OK      OK      OK        195312
RSP[10] RAD Status        Align    Sync     CRC     Frame cnt
RSP[10]              ri:      _      OK      OK        195312
RSP[10] lane0 crosslets:     OK      OK      OK        195312
RSP[10] lane0  beamlets:     OK      OK      OK        195312
RSP[10] lane1 crosslets:     OK      OK      OK        195312
RSP[10] lane1  beamlets:     OK      OK      OK        195312
RSP[10] lane2 crosslets:     OK      OK      OK        195312
RSP[10] lane2  beamlets:     OK      OK      OK        195312
RSP[10] lane3 crosslets:     OK      OK      OK        195312
RSP[10] lane3  beamlets:     OK      OK      OK        195312
RSP[11] RAD Status        Align    Sync     CRC     Frame cnt
RSP[11]              ri:      _      OK      OK        195312
RSP[11] lane0 crosslets:     OK      OK      OK        195312
RSP[11] lane0  beamlets:     OK      OK      OK        195312
RSP[11] lane1 crosslets:     OK      OK      OK        195312
RSP[11] lane1  beamlets:     OK      OK      OK        195312
RSP[11] lane2 crosslets:     OK      OK      OK        195312
RSP[11] lane2  beamlets:     OK      OK      OK        195312
RSP[11] lane3 crosslets:     OK      OK      OK        195312
RSP[11] lane3  beamlets:     OK      OK      OK        195312
''')
    elif prog_name == 'rspctl-160':
        print('''RSP[ 0] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 1] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.26
RSP[ 2] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 3] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 4] 1.2 V: 1.18 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[ 5] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 6] 1.2 V: 1.20 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 7] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 8] 1.2 V: 1.18 , 2.5 V: 2.51, 3.3 V: 3.23
RSP[ 9] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[10] 1.2 V: 1.20 , 2.5 V: 2.47, 3.3 V: 3.23
RSP[11] 1.2 V: 1.18 , 2.5 V: 2.49, 3.3 V: 3.23
RSP[ 0] PCB_temp: 20 , BP_temp: 45, Temp AP0:  27 , AP1:  37 , AP2:  35 , AP3:  37
RSP[ 1] PCB_temp: 20 , BP_temp: 39, Temp AP0:  28 , AP1:  36 , AP2:  33 , AP3:  37
RSP[ 2] PCB_temp: 21 , BP_temp: 42, Temp AP0:  26 , AP1:  33 , AP2:  32 , AP3:  35
RSP[ 3] PCB_temp: 19 , BP_temp: 39, Temp AP0:  27 , AP1:  34 , AP2:  35 , AP3:  36
RSP[ 4] PCB_temp: 19 , BP_temp: 46, Temp AP0:  28 , AP1:  36 , AP2:  33 , AP3:  34
RSP[ 5] PCB_temp: 19 , BP_temp: 41, Temp AP0:  27 , AP1:  34 , AP2:  32 , AP3:  35
RSP[ 6] PCB_temp: 20 , BP_temp: 41, Temp AP0:  29 , AP1:  34 , AP2:  33 , AP3:  36
RSP[ 7] PCB_temp: 20 , BP_temp: 43, Temp AP0:  30 , AP1:  35 , AP2:  34 , AP3:  36
RSP[ 8] PCB_temp: 20 , BP_temp: 52, Temp AP0:  28 , AP1:  36 , AP2:  33 , AP3:  37
RSP[ 9] PCB_temp: 20 , BP_temp: 41, Temp AP0:  27 , AP1:  35 , AP2:  33 , AP3:  34
RSP[10] PCB_temp: 20 , BP_temp: 42, Temp AP0:  28 , AP1:  33 , AP2:  32 , AP3:  35
RSP[11] PCB_temp: 19 , BP_temp: 41, Temp AP0:  28 , AP1:  34 , AP2:  33 , AP3:  36
RSP[ 0] BP_clock: 160
RSP[ 1] BP_clock: 160
RSP[ 2] BP_clock: 160
RSP[ 3] BP_clock: 160
RSP[ 4] BP_clock: 160
RSP[ 5] BP_clock: 160
RSP[ 6] BP_clock: 160
RSP[ 7] BP_clock: 160
RSP[ 8] BP_clock: 160
RSP[ 9] BP_clock: 160
RSP[10] BP_clock: 160
RSP[11] BP_clock: 160
RSP[ 0] Ethernet nr frames: 7033 , nr errors: 0 , last error: 0
RSP[ 1] Ethernet nr frames: 6912 , nr errors: 0 , last error: 0
RSP[ 2] Ethernet nr frames: 6912 , nr errors: 0 , last error: 0
RSP[ 3] Ethernet nr frames: 6912 , nr errors: 0 , last error: 0
RSP[ 4] Ethernet nr frames: 7025 , nr errors: 0 , last error: 0
RSP[ 5] Ethernet nr frames: 6904 , nr errors: 0 , last error: 0
RSP[ 6] Ethernet nr frames: 6900 , nr errors: 0 , last error: 0
RSP[ 7] Ethernet nr frames: 6898 , nr errors: 0 , last error: 0
RSP[ 8] Ethernet nr frames: 7029 , nr errors: 0 , last error: 0
RSP[ 9] Ethernet nr frames: 6902 , nr errors: 0 , last error: 0
RSP[10] Ethernet nr frames: 6903 , nr errors: 0 , last error: 0
RSP[11] Ethernet nr frames: 6906 , nr errors: 0 , last error: 0
RSP[ 0] MEP sequencenr: 0 , error: 0
RSP[ 1] MEP sequencenr: 0 , error: 0
RSP[ 2] MEP sequencenr: 0 , error: 0
RSP[ 3] MEP sequencenr: 0 , error: 0
RSP[ 4] MEP sequencenr: 0 , error: 0
RSP[ 5] MEP sequencenr: 0 , error: 0
RSP[ 6] MEP sequencenr: 0 , error: 0
RSP[ 7] MEP sequencenr: 0 , error: 0
RSP[ 8] MEP sequencenr: 0 , error: 0
RSP[ 9] MEP sequencenr: 0 , error: 0
RSP[10] MEP sequencenr: 0 , error: 0
RSP[11] MEP sequencenr: 0 , error: 0
RSP[ 0] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 0]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 1] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 1]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 2] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 2]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 3] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 3]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 4] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 4]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 5] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 5]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 6] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 6]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 7] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 7]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 8] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 8]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 9] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[ 9]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[10] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[10]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[11] Errors ri: 65535 ,  rcuX: 65535 ,  rcuY: 65535,   lcu: 65535,    cep: 65535
RSP[11]    serdes: 65535 , ap0ri: 65535 , ap1ri: 65535, ap2ri: 65535 , ap3ri: 65535
RSP[ 0] Sync         diff      count    samples     slices
RSP[ 0]     0:          0         65 1810065408     156250
RSP[ 0]     1:          0         65 1810065408     156250
RSP[ 0]     2:          1         65 1810065408     156250
RSP[ 0]     3:          0         65 1810065408     156250
RSP[ 1] Sync         diff      count    samples     slices
RSP[ 1]     0:          0         65 1810065408     156250
RSP[ 1]     1:          0         65 1810065408     156250
RSP[ 1]     2:          0         65 1810065408     156250
RSP[ 1]     3:          0         65 1810065408     156250
RSP[ 2] Sync         diff      count    samples     slices
RSP[ 2]     0:          0         65 1810065408     156250
RSP[ 2]     1:          0         65 1810065408     156250
RSP[ 2]     2:          0         65 1810065408     156250
RSP[ 2]     3:          0         65 1810065408     156250
RSP[ 3] Sync         diff      count    samples     slices
RSP[ 3]     0:          0         65 1810065408     156250
RSP[ 3]     1:          0         65 1810065408     156250
RSP[ 3]     2:          0         65 1810065408     156250
RSP[ 3]     3:          1         65 1810065408     156250
RSP[ 4] Sync         diff      count    samples     slices
RSP[ 4]     0:          0         65 1810065408     156250
RSP[ 4]     1:          0         65 1810065408     156250
RSP[ 4]     2:          0         65 1810065408     156250
RSP[ 4]     3:          0         65 1810065408     156250
RSP[ 5] Sync         diff      count    samples     slices
RSP[ 5]     0:          0         65 1810065408     156250
RSP[ 5]     1:          0         65 1810065408     156250
RSP[ 5]     2:          0         65 1810065408     156250
RSP[ 5]     3:          0         65 1810065408     156250
RSP[ 6] Sync         diff      count    samples     slices
RSP[ 6]     0:          0         65 1810065408     156250
RSP[ 6]     1:          0         65 1810065408     156250
RSP[ 6]     2:          0         65 1810065408     156250
RSP[ 6]     3:          0         65 1810065408     156250
RSP[ 7] Sync         diff      count    samples     slices
RSP[ 7]     0:          0         65 1810065408     156250
RSP[ 7]     1:          0         65 1810065408     156250
RSP[ 7]     2:          0         65 1810065408     156250
RSP[ 7]     3:          0         65 1810065408     156250
RSP[ 8] Sync         diff      count    samples     slices
RSP[ 8]     0:          0         65 1810065408     156250
RSP[ 8]     1:          0         65 1810065408     156250
RSP[ 8]     2:          0         65 1810065408     156250
RSP[ 8]     3:          0         65 1810065408     156250
RSP[ 9] Sync         diff      count    samples     slices
RSP[ 9]     0:          0         65 1810065408     156250
RSP[ 9]     1:          0         65 1810065408     156250
RSP[ 9]     2:          0         65 1810065408     156250
RSP[ 9]     3:          0         65 1810065408     156250
RSP[10] Sync         diff      count    samples     slices
RSP[10]     0:          0         65 1810065408     156250
RSP[10]     1:          0         65 1810065408     156250
RSP[10]     2:          0         65 1810065408     156250
RSP[10]     3:          0         65 1810065408     156250
RSP[11] Sync         diff      count    samples     slices
RSP[11]     0:          0         65 1810065408     156250
RSP[11]     1:          1         65 1810065408     156250
RSP[11]     2:          0         65 1810065408     156250
RSP[11]     3:          0         65 1810065408     156250
RSP[ 0] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 0]     0:          0          0          0          0
RSP[ 0]     1:          0          0          0          0
RSP[ 0]     2:          0          0          0          0
RSP[ 0]     3:          0          0          0          0
RSP[ 1] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 1]     0:          0          0          0          0
RSP[ 1]     1:          0          0          0          0
RSP[ 1]     2:          0          0          0          0
RSP[ 1]     3:          0          0          0          0
RSP[ 2] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 2]     0:          0          0          0          0
RSP[ 2]     1:          0          0          0          0
RSP[ 2]     2:          0          0          0          0
RSP[ 2]     3:          0          0          0          0
RSP[ 3] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 3]     0:          0          0          0          0
RSP[ 3]     1:          0          0          0          0
RSP[ 3]     2:          0          0          0          0
RSP[ 3]     3:          0          0          0          0
RSP[ 4] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 4]     0:          0          0          0          0
RSP[ 4]     1:          0          0          0          0
RSP[ 4]     2:          0          0          0          0
RSP[ 4]     3:          0          0          0          0
RSP[ 5] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 5]     0:          0          0          0          0
RSP[ 5]     1:          0          0          0          0
RSP[ 5]     2:          0          0          0          0
RSP[ 5]     3:          0          0          0          0
RSP[ 6] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 6]     0:          0          0          0          0
RSP[ 6]     1:          0          0          0          0
RSP[ 6]     2:          0          0          0          0
RSP[ 6]     3:          0          0          0          0
RSP[ 7] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 7]     0:          0          0          0          0
RSP[ 7]     1:          0          0          0          0
RSP[ 7]     2:          0          0          0          0
RSP[ 7]     3:          0          0          0          0
RSP[ 8] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 8]     0:          0          0          0          0
RSP[ 8]     1:          0          0          0          0
RSP[ 8]     2:          0          0          0          0
RSP[ 8]     3:          0          0          0          0
RSP[ 9] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[ 9]     0:          0          0          0          0
RSP[ 9]     1:          0          0          0          0
RSP[ 9]     2:          0          0          0          0
RSP[ 9]     3:          0          0          0          0
RSP[10] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[10]     0:          0          0          0          0
RSP[10]     1:          0          0          0          0
RSP[10]     2:          0          0          0          0
RSP[10]     3:          0          0          0          0
RSP[11] RCUStatus   pllX       pllY  overflowX  overflowY
RSP[11]     0:          0          0          0          0
RSP[11]     1:          0          0          0          0
RSP[11]     2:          0          0          0          0
RSP[11]     3:          0          0          0          0
RSP[ 0] RSU Status:
RSP[ 0]     Trigger: User reset request
RSP[ 0]     Image  : Application image
RSP[ 0]     FPGA   : BP was reconfigured
RSP[ 0]     Result : OK
RSP[ 0]     Status : DONE
RSP[ 1] RSU Status:
RSP[ 1]     Trigger: User reset request
RSP[ 1]     Image  : Application image
RSP[ 1]     FPGA   : BP was reconfigured
RSP[ 1]     Result : OK
RSP[ 1]     Status : DONE
RSP[ 2] RSU Status:
RSP[ 2]     Trigger: User reset request
RSP[ 2]     Image  : Application image
RSP[ 2]     FPGA   : BP was reconfigured
RSP[ 2]     Result : OK
RSP[ 2]     Status : DONE
RSP[ 3] RSU Status:
RSP[ 3]     Trigger: User reset request
RSP[ 3]     Image  : Application image
RSP[ 3]     FPGA   : BP was reconfigured
RSP[ 3]     Result : OK
RSP[ 3]     Status : DONE
RSP[ 4] RSU Status:
RSP[ 4]     Trigger: User reset request
RSP[ 4]     Image  : Application image
RSP[ 4]     FPGA   : BP was reconfigured
RSP[ 4]     Result : OK
RSP[ 4]     Status : DONE
RSP[ 5] RSU Status:
RSP[ 5]     Trigger: User reset request
RSP[ 5]     Image  : Application image
RSP[ 5]     FPGA   : BP was reconfigured
RSP[ 5]     Result : OK
RSP[ 5]     Status : DONE
RSP[ 6] RSU Status:
RSP[ 6]     Trigger: User reset request
RSP[ 6]     Image  : Application image
RSP[ 6]     FPGA   : BP was reconfigured
RSP[ 6]     Result : OK
RSP[ 6]     Status : DONE
RSP[ 7] RSU Status:
RSP[ 7]     Trigger: User reset request
RSP[ 7]     Image  : Application image
RSP[ 7]     FPGA   : BP was reconfigured
RSP[ 7]     Result : OK
RSP[ 7]     Status : DONE
RSP[ 8] RSU Status:
RSP[ 8]     Trigger: User reset request
RSP[ 8]     Image  : Application image
RSP[ 8]     FPGA   : BP was reconfigured
RSP[ 8]     Result : OK
RSP[ 8]     Status : DONE
RSP[ 9] RSU Status:
RSP[ 9]     Trigger: User reset request
RSP[ 9]     Image  : Application image
RSP[ 9]     FPGA   : BP was reconfigured
RSP[ 9]     Result : OK
RSP[ 9]     Status : DONE
RSP[10] RSU Status:
RSP[10]     Trigger: User reset request
RSP[10]     Image  : Application image
RSP[10]     FPGA   : BP was reconfigured
RSP[10]     Result : OK
RSP[10]     Status : DONE
RSP[11] RSU Status:
RSP[11]     Trigger: User reset request
RSP[11]     Image  : Application image
RSP[11]     FPGA   : BP was reconfigured
RSP[11]     Result : OK
RSP[11]     Status : DONE
RSP[ 0] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 0]     0:                  0             0
RSP[ 0]     1:                  0             0
RSP[ 0]     2:                  0             0
RSP[ 0]     3:                  0             0
RSP[ 1] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 1]     0:                  0             0
RSP[ 1]     1:                  0             0
RSP[ 1]     2:                  0             0
RSP[ 1]     3:                  0             0
RSP[ 2] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 2]     0:                  0             0
RSP[ 2]     1:                  0             0
RSP[ 2]     2:                  0             0
RSP[ 2]     3:                  0             0
RSP[ 3] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 3]     0:                  0             0
RSP[ 3]     1:                  0             0
RSP[ 3]     2:                  0             0
RSP[ 3]     3:                  0             0
RSP[ 4] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 4]     0:                  0             0
RSP[ 4]     1:                  0             0
RSP[ 4]     2:                  0             0
RSP[ 4]     3:                  0             0
RSP[ 5] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 5]     0:                  0             0
RSP[ 5]     1:                  0             0
RSP[ 5]     2:                  0             0
RSP[ 5]     3:                  0             0
RSP[ 6] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 6]     0:                  0             0
RSP[ 6]     1:                  0             0
RSP[ 6]     2:                  0             0
RSP[ 6]     3:                  0             0
RSP[ 7] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 7]     0:                  0             0
RSP[ 7]     1:                  0             0
RSP[ 7]     2:                  0             0
RSP[ 7]     3:                  0             0
RSP[ 8] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 8]     0:                  0             0
RSP[ 8]     1:                  0             0
RSP[ 8]     2:                  0             0
RSP[ 8]     3:                  0             0
RSP[ 9] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[ 9]     0:                  0             0
RSP[ 9]     1:                  0             0
RSP[ 9]     2:                  0             0
RSP[ 9]     3:                  0             0
RSP[10] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[10]     0:                  0             0
RSP[10]     1:                  0             0
RSP[10]     2:                  0             0
RSP[10]     3:                  0             0
RSP[11] ADO Status adc_offset_x  adc_offset_y (in LS bits)
RSP[11]     0:                  0             0
RSP[11]     1:                  0             0
RSP[11]     2:                  0             0
RSP[11]     3:                  0             0
RSP[ 0] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 0]              ri:      _      OK      OK        156250
RSP[ 0] lane0 crosslets:     OK      OK      OK        156250
RSP[ 0] lane0  beamlets:     OK      OK      OK        156250
RSP[ 0] lane1 crosslets:     OK      OK      OK        156250
RSP[ 0] lane1  beamlets:     OK      OK      OK        156250
RSP[ 0] lane2 crosslets:     OK      OK      OK        156250
RSP[ 0] lane2  beamlets:     OK      OK      OK        156250
RSP[ 0] lane3 crosslets:     OK      OK      OK        156250
RSP[ 0] lane3  beamlets:     OK      OK      OK        156250
RSP[ 1] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 1]              ri:      _      OK      OK        156250
RSP[ 1] lane0 crosslets:     OK      OK      OK        156250
RSP[ 1] lane0  beamlets:     OK      OK      OK        156250
RSP[ 1] lane1 crosslets:     OK      OK      OK        156250
RSP[ 1] lane1  beamlets:     OK      OK      OK        156250
RSP[ 1] lane2 crosslets:     OK      OK      OK        156250
RSP[ 1] lane2  beamlets:     OK      OK      OK        156250
RSP[ 1] lane3 crosslets:     OK      OK      OK        156250
RSP[ 1] lane3  beamlets:     OK      OK      OK        156250
RSP[ 2] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 2]              ri:      _      OK      OK        156250
RSP[ 2] lane0 crosslets:     OK      OK      OK        156250
RSP[ 2] lane0  beamlets:     OK      OK      OK        156250
RSP[ 2] lane1 crosslets:     OK      OK      OK        156250
RSP[ 2] lane1  beamlets:     OK      OK      OK        156250
RSP[ 2] lane2 crosslets:     OK      OK      OK        156250
RSP[ 2] lane2  beamlets:     OK      OK      OK        156250
RSP[ 2] lane3 crosslets:     OK      OK      OK        156250
RSP[ 2] lane3  beamlets:     OK      OK      OK        156250
RSP[ 3] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 3]              ri:      _      OK      OK        156250
RSP[ 3] lane0 crosslets:     OK      OK      OK        156250
RSP[ 3] lane0  beamlets:     OK      OK      OK        156250
RSP[ 3] lane1 crosslets:     OK      OK      OK        156250
RSP[ 3] lane1  beamlets:     OK      OK      OK        156250
RSP[ 3] lane2 crosslets:     OK      OK      OK        156250
RSP[ 3] lane2  beamlets:     OK      OK      OK        156250
RSP[ 3] lane3 crosslets:     OK      OK      OK        156250
RSP[ 3] lane3  beamlets:     OK      OK      OK        156250
RSP[ 4] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 4]              ri:      _      OK      OK        156250
RSP[ 4] lane0 crosslets:     OK      OK      OK        156250
RSP[ 4] lane0  beamlets:     OK      OK      OK        156250
RSP[ 4] lane1 crosslets:     OK      OK      OK        156250
RSP[ 4] lane1  beamlets:     OK      OK      OK        156250
RSP[ 4] lane2 crosslets:     OK      OK      OK        156250
RSP[ 4] lane2  beamlets:     OK      OK      OK        156250
RSP[ 4] lane3 crosslets:     OK      OK      OK        156250
RSP[ 4] lane3  beamlets:     OK      OK      OK        156250
RSP[ 5] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 5]              ri:      _      OK      OK        156250
RSP[ 5] lane0 crosslets:     OK      OK      OK        156250
RSP[ 5] lane0  beamlets:     OK      OK      OK        156250
RSP[ 5] lane1 crosslets:     OK      OK      OK        156250
RSP[ 5] lane1  beamlets:     OK      OK      OK        156250
RSP[ 5] lane2 crosslets:     OK      OK      OK        156250
RSP[ 5] lane2  beamlets:     OK      OK      OK        156250
RSP[ 5] lane3 crosslets:     OK      OK      OK        156250
RSP[ 5] lane3  beamlets:     OK      OK      OK        156250
RSP[ 6] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 6]              ri:      _      OK      OK        156250
RSP[ 6] lane0 crosslets:     OK      OK      OK        156250
RSP[ 6] lane0  beamlets:     OK      OK      OK        156250
RSP[ 6] lane1 crosslets:     OK      OK      OK        156250
RSP[ 6] lane1  beamlets:     OK      OK      OK        156250
RSP[ 6] lane2 crosslets:     OK      OK      OK        156250
RSP[ 6] lane2  beamlets:     OK      OK      OK        156250
RSP[ 6] lane3 crosslets:     OK      OK      OK        156250
RSP[ 6] lane3  beamlets:     OK      OK      OK        156250
RSP[ 7] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 7]              ri:      _      OK      OK        156250
RSP[ 7] lane0 crosslets:     OK      OK      OK        156250
RSP[ 7] lane0  beamlets:     OK      OK      OK        156250
RSP[ 7] lane1 crosslets:     OK      OK      OK        156250
RSP[ 7] lane1  beamlets:     OK      OK      OK        156250
RSP[ 7] lane2 crosslets:     OK      OK      OK        156250
RSP[ 7] lane2  beamlets:     OK      OK      OK        156250
RSP[ 7] lane3 crosslets:     OK      OK      OK        156250
RSP[ 7] lane3  beamlets:     OK      OK      OK        156250
RSP[ 8] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 8]              ri:      _      OK      OK        156250
RSP[ 8] lane0 crosslets:     OK      OK      OK        156250
RSP[ 8] lane0  beamlets:     OK      OK      OK        156250
RSP[ 8] lane1 crosslets:     OK      OK      OK        156250
RSP[ 8] lane1  beamlets:     OK      OK      OK        156250
RSP[ 8] lane2 crosslets:     OK      OK      OK        156250
RSP[ 8] lane2  beamlets:     OK      OK      OK        156250
RSP[ 8] lane3 crosslets:     OK      OK      OK        156250
RSP[ 8] lane3  beamlets:     OK      OK      OK        156250
RSP[ 9] RAD Status        Align    Sync     CRC     Frame cnt
RSP[ 9]              ri:      _      OK      OK        156250
RSP[ 9] lane0 crosslets:     OK      OK      OK        156250
RSP[ 9] lane0  beamlets:     OK      OK      OK        156250
RSP[ 9] lane1 crosslets:     OK      OK      OK        156250
RSP[ 9] lane1  beamlets:     OK      OK      OK        156250
RSP[ 9] lane2 crosslets:     OK      OK      OK        156250
RSP[ 9] lane2  beamlets:     OK      OK      OK        156250
RSP[ 9] lane3 crosslets:     OK      OK      OK        156250
RSP[ 9] lane3  beamlets:     OK      OK      OK        156250
RSP[10] RAD Status        Align    Sync     CRC     Frame cnt
RSP[10]              ri:      _      OK      OK        156250
RSP[10] lane0 crosslets:     OK      OK      OK        156250
RSP[10] lane0  beamlets:     OK      OK      OK        156250
RSP[10] lane1 crosslets:     OK      OK      OK        156250
RSP[10] lane1  beamlets:     OK      OK      OK        156250
RSP[10] lane2 crosslets:     OK      OK      OK        156250
RSP[10] lane2  beamlets:     OK      OK      OK        156250
RSP[10] lane3 crosslets:     OK      OK      OK        156250
RSP[10] lane3  beamlets:     OK      OK      OK        156250
RSP[11] RAD Status        Align    Sync     CRC     Frame cnt
RSP[11]              ri:      _      OK      OK        156250
RSP[11] lane0 crosslets:     OK      OK      OK        156250
RSP[11] lane0  beamlets:     OK      OK      OK        156250
RSP[11] lane1 crosslets:     OK      OK      OK        156250
RSP[11] lane1  beamlets:     OK      OK      OK        156250
RSP[11] lane2 crosslets:     OK      OK      OK        156250
RSP[11] lane2  beamlets:     OK      OK      OK        156250
RSP[11] lane3 crosslets:     OK      OK      OK        156250
RSP[11] lane3  beamlets:     OK      OK      OK        156250
''')
    else:
        print('Unknown name '+prog_name)
    return 0

def rspctl_main(argv):
    r'''
    '''

    prog_name, command = parse_command_line(argv)

    if prog_name == 'rspctl-no-output':
        print('\n')
        return 0

    if command == 'clock':
        return rspctl_clock(prog_name)
    elif command == 'tdstatus':
        return rspctl_tdstatus(prog_name)
    elif command == 'status':
        return rspctl_status(prog_name)
    else:
       return 0

if __name__ == '__main__':
    sys.exit(rspctl_main(sys.argv))
