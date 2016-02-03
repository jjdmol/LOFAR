from distutils.core import setup

url="http://www.analytics.washington.edu/statcomp/projects/rzope/fpconst/"

import fpconst

setup(name="fpconst",
      version=fpconst.__version__,
      description="Utilities for handling IEEE 754 floating point special values",
      author="Gregory Warnes",
      author_email="gregory_r_warnes@groton.pfizer.com",
      url = url,
      long_description=fpconst.__doc__,
      py_modules=['fpconst']
     )

