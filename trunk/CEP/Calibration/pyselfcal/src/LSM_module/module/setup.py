from __future__ import print_function
from setuptools import setup, Command
import os
import lsmtool._version


description = 'The LOFAR Local Sky Model Tool'
long_description = description
if os.path.exists('README.md'):
    with open('README.md') as f:
        long_description=f.read()


class PyTest(Command):
    user_options = []
    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        import sys,subprocess
        errno = subprocess.call([sys.executable, 'runtests.py'])
        raise SystemExit(errno)


setup(
    name='lsmtool',
    version=lsmtool._version.__version__,
    url='http://github.com/darafferty/lsmtool/',
    author='David Rafferty',
    author_email='drafferty@hs.uni-hamburg.de',
    description=description,
    long_description=long_description,
    platforms='any',
    classifiers = [
        'Programming Language :: Python',
        'Development Status :: 4 - Beta',
        'Natural Language :: English',
        'Intended Audience :: Science/Research',
        'Operating System :: POSIX :: Linux',
        'Topic :: Scientific/Engineering :: Astronomy',
        'Topic :: Software Development :: Libraries :: Python Modules',
        ],
    install_requires=['numpy','astropy >= 0.4'],
    scripts = ['bin/lsmtool'],
    packages=['lsmtool','lsmtool.operations'],
    cmdclass = {'test': PyTest},
    )
