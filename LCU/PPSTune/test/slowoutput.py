#!/usr/bin/env python2

import time
import sys

output = ' '.join(sys.argv[1:])

for ch in output:
    sys.stdout.write(ch)
    time.sleep(0.2)
    sys.stdout.flush()
