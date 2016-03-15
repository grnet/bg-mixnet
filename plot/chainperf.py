import json
import os
import sqlite3
import sys

from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})
#   rcParams['pdf.fonttype'] = 42
#   rcParams['ps.fonttype'] = 42
rcParams['ps.useafm'] = True
rcParams['pdf.use14corefonts'] = True
rcParams['text.usetex'] = True


assert len(sys.argv) == 2, 'usage: {} <file>.csv'.format(sys.argv[0])

points = []

conn = sqlite3.connect(':memory:')

conn.execute('''
create table experiments (
clen int,
load int,
latency float
)''')

for row in open(sys.argv[1]):
    stuff = row.strip().split(',')
    stuff = [x.strip() for x in stuff]
    load, clen, latency = int(stuff[0]), int(stuff[1]), float(stuff[2])
    conn.execute('insert into experiments values (?, ?, ?)', (clen, load, latency))

#   print 1 / 0

#   for n in d:
#       parts = n.split('-')
#       _, msgsperserver, chainlen, totalservers, _ = parts
#       msgsperserver = int(msgsperserver)
#       chainlen = int(chainlen)
#       totalservers = int(totalservers)

#       with open(sys.argv[1] + '/' + n) as f:
#           data = json.load(f)
#           for x in data:
#               totaltime = int(x['Total'])
#               points.append((msgsperserver, chainlen, totaltime))

#   print points[-5:]

import matplotlib.pyplot as plot

colors = ['#a6cee3','#1f78b4','#b2df8a','#33a02c','#fb9a99','#e31a1c','#fdbf6f','#ff7f00','#cab2d6','#6a3d9a','#ffff99','#b15928']

colors = ['#543005','#8c510a','#bf812d','#dfc27d','#f6e8c3','#f5f5f5','#c7eae5','#80cdc1','#35978f','#01665e','#003c30', '#a6cee3']

colors = ['#e41a1c','#377eb8','#4daf4a']
colors = ['#edf8fb','#b3cde3','#8c96c6','#88419d'][1:]

configs = []
cur = conn.execute('select distinct load from experiments')
for row in cur:
    configs.append(row[0])

from collections import defaultdict

colored = {}
for i, c in enumerate(configs):
    colored[c] = colors[i]

    cur = conn.execute('select clen, avg(latency), min(latency), max(latency) from experiments where load=? group by clen', (c,))
    xs = []
    ys = []
    ynegs = []
    yposs = []
    for row in cur:
        xs.append(row[0])
        ys.append(row[1])
        avg = row[1]
        ynegs.append(avg - row[2])
        yposs.append(row[3] - avg)

    plot.plot(xs, ys, '-', color=colors[i], label='{}K msgs/server'.format(c/ 1000))
    #   plot.errorbar(xs, ys, fmt='.', yerr=[ynegs, yposs], color=colors[i])
    plot.plot(xs, ys, marker='+', color=colors[i], ms=6, mew=1.5)

xt = range(3, 10)
# xt = [xt[0], xt[3], xt[5]] + xt[6:]
plot.xlim([2, 10])
plot.xlabel('chain length', fontsize=8)
plot.ylabel('round latency (s)', fontsize=8)
plot.yticks(fontsize=8)
plot.xticks(xt, fontsize=8)
plot.gca().set_axisbelow(True)
plot.grid(color='gray')

print colored
plot.legend(loc='upper left', prop={'size':8},)
s = plot.gcf().get_size_inches()
#   cms = s * 2.54
#   to75 = cms[0] / 7.5
#   print s / to75
#   plot.gcf().set_size_inches(s / to75)
plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
plot.savefig('chainplot.pdf', bbox_inches='tight')
