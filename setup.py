from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import os

pwd = os.path.dirname(os.path.abspath(__file__))

setup(ext_modules = cythonize(Extension(
           "pybgmix",
           sources=["pybgmix.pyx"],
           include_dirs=[r"./src"],
           library_dirs=[r"."],
           runtime_library_dirs=[pwd],
           extra_compile_args=["-std=c++0x", "-fopenmp"],
           libraries=["bgmix"],
           language="c++",
)))
