#!/usr/bin/env python

import lofar.ObservationStartListener as osl

print('program version: ' + osl.__version__)  # test get program version; --version ends calling sys.exit(0)
print('')

from sys import argv
osl.main(argv[1:])
# Don't add anything here, since the above invocation will be terminated using SIGTERM
