#!/usr/bin/env python

import string


if __name__=='__main__':

  for i in range(0, 128):
      print '<power%s>\n\t<x><MACType>MACDouble</MACType></x>\n\t<y><MACType>MACDouble</MACType></y></power%s>' \
            % (string.zfill(i, 3), string.zfill(i,3))
  



