#
# Test scripts for LOFAR station regression and production tests.
#

################################################################################
1) Purpose

This test suite uses scripts that call rsptcl and tbbctl to control a LOFAR
station. Most scripts are written in Python. The test suite offers modules
written in Python that allow easy access to rspctl and tbbctl, and that offer
pass/fail handling and logging. The test scripts allow focussing on a typical
function or interface of the station, therefore they can be used for:

- regression tests (e.g. functional coverage tests)
- production tests (e.g. chip interface, board interface tests)
- stress tests (e.g. many HBA write, read accesses)
- operation sanity tests


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
    . e.g.: python i2c_spy.py
    . suitable for simple tests without argument passing
  - BASH shell scripts
    . e.g.: ./rsp_version.sh
  
  All these individual test scripts can be combined:
  . e.g. for production test, regression test
  . e.g. subracktest.sh
   
c) Environment setting

  The scripts run from ./. The search path needs to be set:
    export PYTHONPATH=../modules
  or absolute path
    export PYTHONPATH=/home/lofartest/ptest/modules
  or to add the path
    export PYTHONPATH=$PYTHONPATH:../modules
  or on older systems
    PYTHONPATH=$PYTHONPATH:../modules
    export PYTHONPATH
      
  If rspctl is called then there must be a directory ../log/ for rspctl.log.
  This ../log/ can be a symbolic link.
  

################################################################################
3) Test modules:

cli.py      - Command line interface access
verify.py   - Run test script after parsing the arguments from the command line
testcase.py - Testcase pass/fail control and logging
mep.py      - MEP interface for RSP board access via 'rspctl --readblock/
              rspctl --writeblock'
rsp.py      - RSP board register access functions


################################################################################
4) Test scripts in tc/

a) To run a testcase with verify.py in ./ do:

  python verify.py --brd rsp0,rsp1 --fp blp0 --te tc/prsg.py -v 21
  
b) 'rspctl --readblock' and 'rspctl --writeblock'

  If available high level rspctl options are used to set up things, e.g. like
  the PRSG on an RCU. For other low level board control use the direct access
  '--readblock' and '--writeblock' options of rspctl. The modules mep.py and
  rsp.py provide easy interfacing with theses options.
  
c) For help on the testcase usage and options do:

  python verify.py --help

d) To delete temporary files do:

  ./rmfiles.sh  
