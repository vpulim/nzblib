# setup.py
import distutils
import os
from distutils.core import setup, Extension

os.system("rm -rf build")
setup(name = "Simple example from the SWIG website",
      version = "2.2",
      include_dirs=["../../include"],
      ext_modules = [Extension("nzbfetch",
                               [ "nzbfetchmodule.c"],
                               library_dirs=['/Users/mvtellingen/Projects/prive/libnzbfetch/src/.libs'],
                               libraries=['nzb_fetch', 'expat'],
                               runtime_library_dirs=['/Users/mvtellingen/Projects/prive/libnzbfetch/src/.libs']
                               )
                     ]
     )
                


os.system("mv build/lib.darwin-8.10.1-i386-2.4/nzbfetch.so .")

