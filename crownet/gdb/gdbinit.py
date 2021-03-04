import sys
import os
import gdb
import tempfile
from os.path import dirname
"""
Structure used from {opp_root}/misc/gdb
"""

# skip standard C++ headers when stepping
global skippedstdcxxheaders
skippedstdcxxheaders = False

def skipAllIn(root):
    import os
    for root, dirs, files in os.walk(root, topdown=False):
        for name in files:
            path = os.path.join(root, name)
            gdb.execute('skip file %s' % path, to_string=True)

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


# gdb.execute('source %s' % os.path.join(os.path.dirname(__file__), 'gdbinit'))

# add the pretty printer classes to the system class path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# register libstdc++ pretty printers
if 'register_libstdcxx_printers' in dir():
    print('libstdc++ pretty printers already initialized.')
else:
    from libstdcxx.v6.printers import register_libstdcxx_printers
    register_libstdcxx_printers(None)
    print('Pretty printers initialized: libstdc++')

# register OMNeT++-specific pretty printers
if 'register_omnetpp_printers' in dir():
    print('omnetpp pretty printers already initialized.')
else:
    from omnetpp.printers import register_omnetpp_printers
    register_omnetpp_printers(None)
    print('Pretty printers initialized: omnetpp')

# My own
if 'register_crownet_printers' in dir():
    print('crownet printers already initialized.')
else:
    from crownet.printers import register_crownet_printers
    register_crownet_printers(None)
    print('Pretty printers initialized: Crownet')


with open("/tmp/foo", "w") as f :
    f.write("foobar\n")


