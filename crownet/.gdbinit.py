import sys
import os
import gdb
import tempfile
from os.path import dirname
"""
Structure used from {opp_root}/misc/gdb
"""

# set unwind signal so an exeption caused by the pretty printer will not stop the debugger
gdb.execute('set unwindonsignal on')
# settings taken from https://stackoverflow.com/a/5713387/12079469
gdb.execute('set print pretty on')
gdb.execute('set print object on')
gdb.execute('set print static-members on')
gdb.execute('set print vtbl on')
gdb.execute('set print demangle on')
gdb.execute('set demangle-style gnu-v3')
gdb.execute('set print sevenbit-strings off')

# add the pretty printer classes to the system class path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# My own
if 'register_rover_printers' in dir():
    print('rover pretty printers already initialized.')
else:
    from gdb_rover.printers import register_rover_printers
    register_rover_printers(None)
    print('Pretty printers initialized: roVer')
