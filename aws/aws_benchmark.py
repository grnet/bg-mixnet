import os
import sys

# need args
assert len(sys.argv) == 3, "usage: {} <config-filepath> <stadium-server-id>".format(sys.argv[0])

print 'called with args', sys.argv[1], sys.argv[2]

def execute(cmd):
    print 'execute:', cmd
    os.system(cmd)

execute('collectl -sn -oTm -i.1 > ~/benchmark.collectl &')
execute('~/go/bin/server -conf {} -id {}'.format(sys.argv[1], sys.argv[2]))
execute('pkill collectl')
