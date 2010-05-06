#
# Test scripts for LOFAR station regression, production and operation tests.
#

################################################################################
1) Introduction

a) Purpose
This test suite uses scripts that call rspctl and tbbctl to control a LOFAR
station. Most scripts are written in Python. The test suite offers modules
written in Python that allow easy access to rspctl and tbbctl, and that offer
pass/fail handling and logging. The test scripts allow focussing on a typical
function or interface of the station, therefore they can be used for:

- regression tests (e.g. functional coverage tests)
- production tests (e.g. chip interface, board interface tests)
- stress tests (e.g. many HBA write, read accesses)
- operation sanity tests

b) Why Python?
For tool interfacing TCL is the common script language, that is why the VHDL
developent using Modelsim uses TCL scripts. The problem with the remote station
TCL scripts is that they run under Windows using WinPCap to interface with the
Ethernet card. TCL is platform independent, but there is no easy equivalent for
WinPCAP under Linux. Hence it is not straigthforward to run the TCL testcases
environment also under Linux on the LCU.
Instead of using something like WinPCap the low level register access functions
--readblock and --writeblock were added to rspctl . This allows controlling the
RSP board via scripts.
Python was choosen as test script language, because it is a popular script
language and adopted at various levels within the LOFAR software development,
including operations. Based on my experience in translating TCL scripts into
Python equivalents I think that Python is a more natural and nicer language than
TCL.


################################################################################
2) Test setup

a) Directory structure
  ./ : README.txt, Python scripts, bash scripts. Run from this directory.
  modules/ : Python modules
  gold/ : Golden result files for corresponding scripts, 'diff *.log *.gold'
          should yield no difference for the test to pass
  tc/ : Testcase scripts to be used with verify.py
  
b) This test suite uses different types of test scripting:

  - Testcase Python scripts in tc/ called via verify.py
    . e.g.: tc/status.py, tc/bist.py, tc/prsg.py
    . verify.py offers argument parsing, test result logging
    . Resembles TCL testcase structure used for gateware development tests
    . Most scripts in tc/ are direct (manual) translations of the TCL test
      scripts that were used for the gateware (VHDL) development, see
      LOFAR-ASTRON-MEM-186. The Python / rspctl test scripts run rather slow,
      about a factor 2 slower than a comparable TCL / C test script.
  - Stand alone Python scripts
    . e.g.: python i2c_spu.py, i2c_td.py
    . suitable for simple tests without argument passing
    . test pass/fail and logging avaiable via testlog.py 
  - BASH shell scripts
    . e.g.: ./rsp_version.sh
  
  All these individual test scripts can be combined:
  . e.g. for production test, regression test
  . e.g. subracktest.sh
   
c) Environment setting

  The scripts run from ./ The search path needs to be set:
    export PYTHONPATH=./modules
  or absolute path
    export PYTHONPATH=/home/lofartest/ptest/modules
  or to add the path
    export PYTHONPATH=$PYTHONPATH:./modules
  or on older systems
    PYTHONPATH=$PYTHONPATH:./modules
    export PYTHONPATH
      
  If rspctl is called then there must be a directory ../log/ for rspctl.log.
  This ../log/ can be a symbolic link.
  

d) RSPDriver.conf

To allow the 'rspctl --readblock' and 'rspctl --writeblock' low level access
commands the RSP driver in /opt/lofar/etc/ must have:

  RSPDriver.READ_RAW_DATA = 1
  RSPDriver.WRITE_RAW_DATA = 1

For tests that access the SPU or TDS, the RSP driver regular accesses via TDSH
must be stopped via:

  RSPDriver.READWRITE_TDSSTATUS=0


################################################################################
3) Test modules:

cli.py      - Command line interface access
verify.py   - Run one or more test scripts after parsing the arguments from the
              command line
testcase.py - Testcase pass/fail control, timing and logging for testcases that
              run with verify.py
testlog.py  - Similar to testcase.py, provides pass/fail control, timing and
              logging for standalone test scripts like subrack_production.py
mep.py      - MEP interface for RSP board access via 'rspctl --readblock/
              rspctl --writeblock'
rsp.py      - RSP board register access functions
smbus.py    - SMBus (I2C) access functions


Remarks:

a) To see all script options (both general and test case specific) do:
  
  python verify.py --help

   Remarks:
   - via --brd a test can be ran for one RSP or multiple
   - via --fpga a test can be ran on the BP, AP0, AP1, AP2, and/or AP3 in any
     combination
   - via --pol a test can be ran on RCU x and/or RCU y of the AP.
   - There are 2 RCU per AP and 4 AP per RSP. In a station the RCUs are
     numbered starting from 0:
     
       . RCU[0]  = (rsp0, blp0, x)
       . RCU[1]  = (rsp0, blp0, y)
       . RCU[2]  = (rsp0, blp1, x)
       . RCU[3]  = (rsp0, blp1, y)
       . RCU[4]  = (rsp0, blp2, x)
       . RCU[5]  = (rsp0, blp2, y)
       . RCU[6]  = (rsp0, blp3, x)
       . RCU[7]  = (rsp0, blp3, y)
       . RCU[8]  = (rsp1, blp0, x)
       . RCU[9]  = (rsp1, blp0, y)
       .
       . etc
       .
       . RCU[64] = (rsp8, blp0, x)
       . RCU[65] = (rsp8, blp0, y)
       . RCU[66] = (rsp8, blp1, x)
       . RCU[67] = (rsp8, blp1, y)
       . RCU[68] = (rsp8, blp2, x)
       . RCU[69] = (rsp8, blp2, y)
       . RCU[70] = (rsp8, blp3, x)
       . RCU[71] = (rsp8, blp3, y)
       . RCU[72] = (rsp9, blp0, x)
       . RCU[73] = (rsp9, blp0, y)
       .
       . etc
       .
  
b) Classes:
   - mep.py defines class MepMessage
   - testcase.py defines class Testcase
   - testlog.py defines class Testlog
  The other modules cli.py, rsp.py and smbus.py define plain functions. Classes
  are a nice way of grouping constants and functions to an object, for mep.py
  the object it the message string that can be manipulated and access through
  the class functions. Similar smbus.py could have been written as a class to
  with the protocol_list and protocol_result as objects. I do not know what is
  the best approach and why. With smbus.py as a plain set of functions it is 
  also clear that the functions belong together, because they are called using
  the module name as prefix.
   
c) The 'rspctl --readblock' and 'rspctl --writeblock' are quite slow, due to:
   - the double buffering in the RSP driver
   - maybe rspctl excepts only one access per pps interval
   - the hex translations in mep.py to adhere to the format ot rspctl
  For scripts that only use these low level peek and poke rspctl commands it
  would be a great improvement to have a dedicated driver program that takes 
  care of these low level commands. This may be a driver program or some C
  functions that can be used directly in Python. The scripts can then remain as
  they are, only a different mep.py module needs to be added.
   
d) Some scripts also use higher level rspctl commands, e.g. to set the RCU in
   PSRG mode.
   
e) Typically all RSP access goes via functions in rsp.py and smbus.py. Hence in
   a test script it should never be necessary to call the low level rspctl
   commands directly.
   
f) I tried to follow the Python documentation rules. For example to read the
   documentation in the module rsp.py do:

   python
   >>> import rsp
   >>> dir(rsp)
   >>> print rsp.__doc__
   >>> print rsp.write_mem.__doc__

g) When command line options are needed I use the Python option parser. For the
   test scripts all command line options need to be defined in testcase.py.
   There are some general test script options (e.g. --brd) but there are also
   test script specific options (e.g. --pid). The general testcase options are
   passed on via the Testcase class, the specific options are passed on as
   directly and all get prefix 'arg_' in verify.py to easily recognize them.
   
h) For logging I followed the logging approach that was used for the RSP test
   environment written in TCL. Python also has modules for logging thay may be
   useful in future.
   
   
################################################################################
4) Test scripts in tc/

a) The testcases in tc/ are ran using verify.py. The test results are reported
   to the screen and also stored in tc/ in a *.log file with the same name. The
   ammount of logging depends on the verbosity level set by option -v.
   
   To run a testcase with verify.py in ./ do:

  python verify.py --brd rsp0 --fp blp0 --te tc/prsg.py -v 21
  
   To run a testcase for one subrack with 4 RSP do:
 
  python verify.py --brd rsp0,rsp1,rsp2,rsp3 --rep 1 -v 21 --te tc/serdes.py

   To run a testcase for a station with three subracks, so 12 RSP do:

  python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,
                         rsp10,rsp11 --te tc/serdes.py
   
  Remarks:
  - The default option parameter values are defined in verify.py
  - For options with multiple parameters the parameters must form one continuous
    string seperated by commas's. 
  
b) The following test scripts are available in tc/, they are more or less plain,
   manual translations from TCL test cases:

  - empty.py           = empty, can be used to try verify.py
  - cdo_ctrl.py        = Read or write the CTRL field in the CDO settings
                         register
  - prsg.py            = TC 5.10, capture RCU PSRG data
  - serdes.py          = TC 3.8, serdes ring test
  - read_serdes_phy.py = TC 8.3, read serdes PHY registers
  - status.py          = TC 11.1, read RSP status register
  - spustat.py         = TC 9.6, read SPU sensor status <=> 'rspctl --spustat'
  - tdstat.py          = TC 9.1, read TD sensor status <=> 'rspctl --tdstat' 
  - hba_client         = TC 5.42, read or write to a HBA client register at the
                         RCU
  - hba_server         = TC 5.43, read or write to a HBA server register at the
                         tile
  - rad_lanemode       = TC 5.24, write or read the lane mode for the SERDES
                         lanes
  - rad_latency        = TC 5.49, show latency of data frames on the SERDES
                         lanes

c) 'rspctl --readblock' and 'rspctl --writeblock'

  If available high level rspctl options are used to set up things, e.g. like
  the PRSG on an RCU. For other low level board control use the direct access
  '--readblock' and '--writeblock' options of rspctl. The modules mep.py and
  rsp.py provide easy interfacing with theses options.
  
d) For help on the testcase usage and options do:

  python verify.py --help

e) To delete temporary files do:

  ./rmfiles.sh
  
f) Verbosity level (-v)
  The testcases all adhere to the following verbosity level convention:
   
  -v  0 : show PASSED or FAILED
  -v  1 : show testcase title
  -v  2 : show testcase time
  -v 11 : show '... went wrong' for each step or section in a testcase, plus
          some more info like expected result and read result
  -v 21 : show '... went OK' for each step or section in a testcase
  -v 22 : show rspctl commands
  -v 23 : show rspctl command return results
	   
g) Pass/fail
  Default a testcase run using verify.py/testcase.py (or using testlog.py) gets
  pass/fail status 'RUNONLY'. This is useful for test scripts that are
  utilities. If the testcase checks for expected results then it one of the it 
  should first initialized the tc result to 'PASSED'. Subsequently each step in
  the testcase that fails should also set the tc result to 'FAILED'. Once the
  tc status is 'FAILED' it can not change anymore.
   
h) Test scripts and the RSP driver
  The test scripts use rspctl. Some test scripts do not affect the functional
  behaviour of the RSP driver, however many scripts do. Due to the slow and 
  asynchronlous behaviour of teh test scripts it is often handy to disable the
  external sync (pps). The scripts also enable the external sync again, but 
  if that does not recover normal rspctl behaviour then it may be necessary to
  do a 'rspctl --rspclear' or restart the RSP driver via swlevel 1, swlevel 2.
   
   
################################################################################
5) More examples

a) HBA client access
  To read the speed register of HBA client on RCU[65] do:
   
  python verify.py --brd rsp8 --fp blp0 --rep 1 -v 21 --te tc/hba_client.py
                      --client_access r --client_reg speed --data 1

  The test will only signal pass if the --data value equals the read speed
  value, hence for read the --data option is used as expected result.

b) HBA server access
  To manage the HBA server addresses use:
  
  test/hbatest/hba_read_all.sh            # to read all servers in a HBA tile
  test/hbatest/hba_new_address.sh         # to change a HBA server address
  
  These shell scripts use tc/hba_server.py. More examples:
  
  To read the delay settings for polarization X and Y of HBA server 2 on the 
  HBA tile of RCU[64]=X (power via X) and RCU[65]=Y (control via Y) do:
  
 python verify.py --brd rsp8 --fp blp0 --rep 1 -v 21 --te tc/hba_server.py
                  --server 2 --server_access uc --server_function gw
		  --server_reg delay_x --data 50,51
	
  To change the default server address 127 to operational server address # in
  1-16 range, first be sure to set a free address, otherwise two servers in the
  tile will have the same address and can not be controlled remotely anymore.
  Use the following command to check whether server # = 1, 2, ... 16, or 127
  exists in the tile on RSP 8, BLP 0:

 python verify.py --brd rsp8 --fp blp0 -v 21 --te tc/hba_server.py
                  --server # --server_access uc --server_function gb
                  --server_reg address --data #

  Then use the following command to change the default server address into #:
  
 python verify.py --brd rsp8 --fp blp0 -v 21 --te tc/hba_server.py
                  --server 127 --server_access uc --server_function sb
		              --server_reg address --data #
		              
		  
c) Read version nummers of station hardware, firmware and software:
  - The RSP board version and BP and AP firmware versions: 'rspctl --version'.
  - The RSP CPLD CP firmware version: 'tc/status.py --pid rsu'.
    Should be version 3 (although 2 probably also works). Version 3 reveals
    what caused the RSP board to reset (e.g. user, watchdog, overtemperature).
  - The RCU firmware version: 'rspctl --rcu'.
    The version number is given by the most significant nibble in the control
    word "RCU[ 0].control=0x10057980". Shouls be 1 for RCUs with the I2C
    problem of bug 1111 solved. This fix also implies that the I2C to the RCU
    requires an active system clock (160 or 200 MHz). For I2C access to the
    HBA client the system clock is not used.
  - HBA client software version (old HBA software does not have a version
    register): 'tc/hba_client.py --client_access r --client_reg version
    --data 10' --> should be version 10.
  - HBA server software version (old HBA software does not have a version
    register) for all 1-16 servers: 'tc/hba_server.py --server 1
    --server_access uc --server_reg version --server_func gb --data 10'
    --> should be version 10.

d) Useful tests for monitoring the data on the serdes ring between the RSP
   boards (carries the crosslets for the XST and the beamlets for CDO):
   
   - 'rspctl --rcumode=3', 'rspctl --xcsubband=256', 'rspctl --xcstat' and/or
     'rspctl --xcstat --xcangle' --> Should yield an smooth, noisy XST matrix
     with autocorrelation diagonal.
   - 'tc/rad_latency.py' --> Shows how the arrival delay in number of samples
     for the crosslet and beamlet packets at each subsequent RSP board.
   - 'tc/rad_lanemode.py --read' --> Reads the lane mode settings, this 
     corresponds to the beamlet and crosslet out settings in RSP_Driver.conf.
   - 'tc/status.py --pid rad' --> Shows whether no data packets go lost on the
     serdes ring and whether the data streams from the local AP and the
     preceding RSP are aligned.
   - 'tc/read_serdes_phy.py --data 16,18,20,21,22,23' --> for multiple RSP read
     the redundant ring registers to diagnose in case serdes.py fails.

e) Clock and PPS can also affect the XST:
   - 'tc/serdes.py --diag_sync 0' --> Uses the PPS to start and stop the
     test, so implicitely it verifies that the PPS from the Rubidium is OK.
   - The diff in 'rspctl --status' should be fixed (or +0, +512 at 200 MHz). 
     Same as ext_cnt in 'tc/status.py --pid bs'. If it drifts then the XO
     on the TDS clock board is free running, i.e. the PLL on the TDS clock
     board is not (always) in lock.
   - 'rspctl --tdstat' shows whether the PLL on the clock board is in lock.
     However, this is based on a single measurement read via I2C. The PLL could
     in fact loose and reaquire lock regular basis.


################################################################################
6) Standalone test scripts

a) Subrack production tests

The Python script subrack_production.py runs the subrack production test, it 
requires a batch nr and a serial nr that will be used to name the log file.

  python subrack_production.py --help
  python subrack_production.py -b 3 -s 2

This subrack_production.py replaces subracktest.sh.

b) Various

  i2c_spu.py = uses 'rspctl --spustat' to verify SPU sensor status
  i2c_td.py  = uses 'rspctl --spustat' to verify TDS sensor status

