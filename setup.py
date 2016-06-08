import os, sys
from pip import locations
from setuptools import setup, Extension
from distutils import sysconfig
import eups

def get_dependencies(product):
    # tuples are: (productName, VersionName, optional, recursionDepth)
    dependecy_tuples = eups.getDependencies(product)
    required_dependencies = filter(lambda x: x[2] is False, dependecy_tuples)
    unique_product_names = set(map(lambda x: x[0], required_dependencies))

    # I don't think we want swig to include itself?
    excluded_dependencies = set(['scons', 'sconsUtils', 'swig'])
    return list(unique_product_names - excluded_dependencies)

dependencies = get_dependencies('mops_daymops')
print dependencies
dependency_dirs = [eups.productDir(dep, version=None) for dep in dependencies]
print dependency_dirs

kwds = dict(
    extra_compile_args=['-std=c++11'],
    extra_objects=['-lmops_daymops'],
    extra_link_args=['-Llib'],
    include_dirs=[
        os.path.join('..', 'include'),
        os.path.join('include'),
        os.path.join('/Users/ctslater/lsstsw/stack/DarwinX86/pex_exceptions/2016_01.0+3/include'),
        os.path.join('/Users/ctslater/lsstsw/stack/DarwinX86/eigen/3.2.5-1-g70497dd/include'),
        os.path.dirname(locations.distutils_scheme('pybind11')['headers'])
    ],
)

for dep_dir in dependency_dirs:
    kwds['include_dirs'].append(dep_dir + "/include")

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
    # package_dir={'': "python/"},
    ext_modules=ext_modules,
)

