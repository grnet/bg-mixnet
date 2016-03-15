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

font = {'size'   : 8}
import matplotlib
matplotlib.rc('font', **font)

import matplotlib.pyplot as plot

conn = sqlite3.connect(':memory:')

assert len(sys.argv) == 3, 'usage: {} <datafile>.csv <noisefile>.csv'.format(sys.argv[0])

conn.execute('''
create table experiments (
clen int,
servers int,
users int,
latency float
)''')

conn.execute('''
create table noise (
servers int,
epsilon float,
noise int
)''')

offsets = {
    100: 37,
    75: 37.5,
    50: 38.5,
    25: 40.5,
}

# read data

with open(sys.argv[1]) as f:
    for line in f:
        clen, m, users, t = line.split(',')
        clen = int(clen)
        m = int(m)
        users = int(float(users) * 10 + offsets[m])
        t = float(t)
        conn.execute('insert into experiments values (?, ?, ?, ?)', (clen, m, users, t))

ms = [25, 50, 75, 100]
for m in ms:
    fname = sys.argv[2].replace('100', str(m))
    with open(fname) as f:
        skip = False
        for line in f:
            if not skip:
                skip = True
                continue
            noise, _, epsilon, _ = line.split(',')
            noise = int(noise)
            epsilon = float(epsilon)
            conn.execute('insert into noise values (?, ?, ?)', (m, epsilon, noise))

# find noise


# plotting

f, axarr = plot.subplots(1, 3, sharey=True)
vzcolor = '#01665e'
vzcolor2 = '#c51b7d'

colors = [
    ['#543005','#8c510a','#bf812d','#dfc27d'][::-1],
    ['#80cdc1','#35978f','#01665e','#003c30'],
    ['#8e0152','#c51b7d','#de77ae','#f1b6da'][::-1],
]

def vzeqn(users):
    latency = 8 + ((45 - 8) / 2.0) * users
    return latency

# query error statistics

clens = [3, 6, 9]
ms = [25, 50, 75, 100]
eeps = 10

for i, clen in enumerate(clens):
    for j, m in enumerate(ms):
        xs = []
        ys = []
        ynegs = []
        yposs = []
        plotcolor = colors[0][j]

        cur = conn.execute('''
        select b.epsilon, b.noise
        from noise a, noise b
        where a.servers=? and b.servers=?
        and a.noise+1 = b.noise
        and a.epsilon > ?
        and b.epsilon < ?''', (m, m, eeps, eeps))
        noise = -1
        for row in cur:
            _, noise = row
            noise = noise / 10.0
            #   print clen, m, noise

        cur = conn.execute(
            '''select users, min(latency), max(latency), avg(latency)
            from experiments
            where clen=? and servers=?
            group by users
            order by users''', (clen, m))
        for row in cur:
            spread = row[2] - row[1]
            users = (row[0] / 10.0) - (noise * m / 100.0)
            avg = row[3]
            print spread/avg
            #   print avg, spread, users, clen, m

            if noise != -1:
                xs.append(users)
                ys.append(avg)
                ynegs.append(avg - row[1])
                yposs.append(row[2] - avg)

        axarr[i].plot(xs, ys, '-', label='$m = {}$'.format(m), color=plotcolor)
        #   axarr[i].errorbar(xs, ys, fmt='+', yerr=[ynegs, yposs], color=plotcolor, ms=6, mew=1.5)
        axarr[i].plot(xs, ys, marker='+', color=plotcolor, ms=6, mew=1.5)
        
    # vz line
    xt = [0, 52]
    yt = [vzeqn(x) for x in xt]
    axarr[i].plot(xt, yt, '--', color=vzcolor)

    axarr[i].set_xlim([0, 50])
    axarr[i].set_title('chain length={}'.format(clen), fontsize=8)

    axarr[i].set_yticks([0, 200, 400, 600, 800, 1000])
    axarr[i].set_ylim([0, 1200])

    if i == 1:
        axarr[i].set_xlabel('millions of users', fontsize=8)
    if i == 0:
        axarr[i].set_ylabel('round latency (s)', fontsize=8)
        axarr[i].legend(loc='upper center', prop={'size':8})
    #   axarr[i].legend(loc='upper center', prop={'size':8}, framealpha=1.0, ncol=3, bbox_to_anchor=(0.5, 1.033))

    axarr[i].set_axisbelow(True)
    axarr[i].grid(color='gray')


s = plot.gcf().get_size_inches()
#   plot.gcf().set_size_inches((s[0] / 2, s[1] / 1.5))
# s0 = 1.35
s0 = 1.25
plot.gcf().set_size_inches((s[0] / (1.5 * s0), s[1] / (2 * s0)))
plot.savefig('scaling.pdf')
