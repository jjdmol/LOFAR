#!/usr/bin/python
import sys
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
parset = GSMParset(sys.argv[1])
pip = GSMPipeline(use_monet=False, database='test')
pip.run_parset(parset)
