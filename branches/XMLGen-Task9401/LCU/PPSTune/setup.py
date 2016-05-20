#!/usr/bin/env python

from distutils.core import setup
from ppstune.ppstune import version_string

setup(name='ppstune',
      version     = version_string(),
      description = 'A python tool to tune PPS delays at LOFAR stations',
      author      = 'Michiel Brentjens',
      author_email= 'brentjens@astron.nl',
      url         = '',
      packages    = ['ppstune'],
      scripts     = [],
      requires    = ['numpy','matplotlib','nose', 'coverage', 'IPython(>=0.11)', 'numpydoc', 'sphinx'],
     )
