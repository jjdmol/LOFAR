"""Command line interface access
"""

################################################################################
# System imports
import commands

################################################################################
# Functions

def command(arg, p=False):
  if p:
    print arg
  return commands.getoutput(arg)
