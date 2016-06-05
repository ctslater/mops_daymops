import os, sys
from pip import locations
from setuptools import setup, Extension
from distutils import sysconfig

kwds = dict(
    extra_compile_args=['-std=c++11'],
    extra_link_args=['-Llib -lmops_daymops'],
    include_dirs=[
        os.path.join('..', 'include'),
        os.path.join('include'),
        os.path.dirname(locations.distutils_scheme('pybind11')['headers'])
    ],
)
if sys.platform == 'darwin':
    kwds["extra_compile_args"].append('-mmacosx-version-min=10.7')
    kwds["extra_compile_args"].append('-stdlib=libc++')

ext_modules = [
    Extension(
        'lsst.mops.daymops',
        sources=[
            os.path.join('src', 'wrappers', 'daymops.cc'),
        ], **kwds
    ),
]

setup(
    name='mops_daymops',
    version='0.0.1',
    author='LSST/Aura',
    test_suite="tests",
    description='LSST Mops',
    ext_modules=ext_modules,
)

