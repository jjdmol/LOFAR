#!/usr/bin/env python2

import string

if __name__=='__main__':

  for i in range(0, 128):
      print '%s%s%s' % ('{"power', string.zfill(i, 3), '_x", GCFPValue::LPT_DOUBLE, GCF_READABLE_PROP, "0"},')
      print '%s%s%s' % ('{"power', string.zfill(i, 3), '_y", GCFPValue::LPT_DOUBLE, GCF_READABLE_PROP, "0"},')
  



