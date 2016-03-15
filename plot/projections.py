import math
import numpy
import sqlite3
import sys

conn = sqlite3.connect(':memory:')

assert len(sys.argv) == 3, 'usage: {} <datafile>.csv <noisefile>.csv'.format(sys.argv[0])

'example use: python bigexperimentplot.csv servers_100.csv'

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
    25: -10000,
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

# ms = (1000, 500, 100, 50)
ms = sorted(list(range(10, 1001, 10)) + [75])
dots = [50, 75, 100]
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
            print m, epsilon, noise
            conn.execute('insert into noise values (?, ?, ?)', (m, epsilon, noise))

# plotting stuff
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

colors = ['#1b9e77','#d95f02','#7570b3']
colors = ['#fef0d9','#fdcc8a','#fc8d59','#d7301f']
colors = ['#fef0d9','#fdcc8a','#fc8d59','#e34a33','#b30000'][1:]
colors = ['#f0f9e8','#bae4bc','#7bccc4','#43a2ca','#0868ac'][1:]
colors = ['#ffffcc','#a1dab4','#41b6c4','#2c7fb8','#253494'][2:]
colors = ['#fef0d9','#fdcc8a','#fc8d59','#d7301f'][1:]

symbols = ['-', '--']

s = plot.gcf().get_size_inches()
plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))


# clen: chain length
# servers: ??? (in 50, 75, 100)
# load: total load on system
def interpolant(clen, servers, load):
    # hacky: interpolate twice if we're not 50, 75, 100
    xs = [50, 75, 100]
    if servers not in xs:
        # TODO use log interpolation
        ys = [interpolant(clen, x, load) for x in xs]
        ys = [numpy.log(y) for y in ys]
        #   print clen, servers, ys
        if servers > 100:
            # we only have data up to 100, so:
            # ignore distribution time and
            # reduce total load by server-number increase
            scaled = load * 100 / servers
            y = interpolant(clen, 100, scaled)
        elif servers < 50:
            # TODO invalid
            y = ys[0]+(servers-xs[0])*(ys[1]-ys[0])/(xs[1]-xs[0])
        else:
            # TODO verify this log-interpolation works
            y = numpy.interp(servers, xs, ys, -1, -1)
            y = pow(numpy.e, y)
        # return pow(numpy.e, y)
        return y

    cur = conn.execute('''
    select users, avg(latency)
    from experiments
    where clen=? and servers=?
    group by users''', (clen, servers))
    entries = []
    for row in cur:
        entries.append((row[0], row[1]))

    # xs -> load (experiment sample points), with units being 10K
    # ys -> latency (observed experimentally for that load)
    xs, ys = zip(*entries)

    result = numpy.interp(load, xs, ys, -1, -1)
    if result < 0:
        # https://stackoverflow.com/questions/2745329/
        if load < xs[0]:
            return ys[0]+(load-xs[0])*(ys[1]-ys[0])/(xs[1]-xs[0])
        elif load > xs[-1]:
            return ys[-1]+(load-xs[-1])*(ys[-1]-ys[-2])/(xs[-1]-xs[-2])
    else:
        return result

edges = [x/10.0 for x in range(10, 50, 1) + range(50, 80, 2) + range(80, 140, 5) + [140, 150, 160]]
totalusers = 300

# vuvuzela
def vz(users):
    latency = 8 + ((45 - 8) / 2.0) * users
    return latency
def vzinv(latency):
    users = latency / (8 + ((45 - 8) / 2.0))
    return users

vzcolor = '#01665e'

################################################################################

# eps v latency, varying servers, chain length = 6

# find boundaries and plot
#   clen = 6
#   ms = (1000, 500, 100, 50)
#   for i, m in enumerate(sorted(ms)):
#       epses = []
#       lats = []

#       for e in edges:
#           cur = conn.execute('''
#       select b.epsilon, b.noise
#       from noise a, noise b
#       where a.servers=? and b.servers=?
#       and a.noise+1 = b.noise
#       and a.epsilon > ?
#       and b.epsilon < ?''', (m, m, e, e))
#           for row in cur:
#               eps, noise = row
#               load = int(noise * m / 100) + totalusers
#               latency = interpolant(clen, m, load)
#               if latency < 0:
#                   continue
#               epses.append(eps)
#               lats.append(latency)
#       #   if max(epses) < 15:
#       #       epses.append(16.0)
#       #       lats.append(min(lats))
#       plot.plot(epses, lats, symbols[i/2], color=colors[i], label='$m={}$'.format(m))

#   # projections

#   plot.legend(loc='upper left', prop={'size':8},framealpha=1.0)
#   plot.title('30 million users', fontsize=8)
#   plot.xlabel('$e^\epsilon$', fontsize=10)
#   plot.ylabel('round latency (s)', fontsize=8)
#   plot.yticks(fontsize=8)
#   plot.xlim([1, 11])
#   plot.ylim([0, 900])
#   plot.xticks(range(1, 11), fontsize=8)

#   plot.grid()
#   plot.savefig('epsvlatm.pdf')
#   plot.clf()

################

# eps v latency, varying chain length, m = 100
clens = [3, 6, 9]
m = 100
for i, clen in enumerate(clens):
    epses = []
    lats = []

    for e in edges:
        cur = conn.execute('''
    select b.epsilon, b.noise
    from noise a, noise b
    where a.servers=? and b.servers=?
    and a.noise+1 = b.noise
    and a.epsilon > ?
    and b.epsilon < ?''', (m, m, e, e))
        for row in cur:
            eps, noise = row
            load = int(noise * m / 100) + totalusers
            latency = interpolant(clen, m, load)
            if latency < 0:
                continue
            epses.append(numpy.log(eps))
            lats.append(latency)
    if max(epses) < 15:
        epses.append(16.0)
        lats.append(min(lats))
    plot.plot(epses, lats, '-', color=colors[i], label='$\ell={}$'.format(clen))

vzcompare = vz(30)
plot.plot(numpy.log(3), vzcompare, '.', color=vzcolor)
xshift = 0.01
yshift = -100; yshift = 0
plot.text(numpy.log(3)+xshift, vzcompare+yshift, 'Vuvuzela, $\ell=3$', fontsize=8)

# projections

plot.legend(loc='upper right', prop={'size':8},framealpha=1.0)
plot.title('30 million users, $m=100$ servers', fontsize=8)
plot.xlabel('$\epsilon$', fontsize=10)
plot.ylabel('round latency (s)', fontsize=8)
plot.yticks(fontsize=8)
plot.xlim([0.6, 2])
#   plot.ylim([0, 1200])
plot.xticks([0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], fontsize=8)

plot.gca().set_axisbelow(True)
plot.grid(color='gray')
s = plot.gcf().get_size_inches()
#   plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
plot.savefig('epsvlat.pdf')
plot.clf()

################

# latency v servers, varying chain length, e^eps = 3
clens = [3, 6, 9]
ms = sorted(list(range(40, 1001, 10)) + [75])
e = 3
for projected in [False, True]:
    for i, clen in enumerate(clens):
        srvs = []
        lats = []
        dotvs = []

        for m in ms:
            if projected and m < 100:
                continue
            if not projected and m > 100:
                continue

            cur = conn.execute('''
        select b.epsilon, b.noise
        from noise a, noise b
        where a.servers=? and b.servers=?
        and a.noise+1 = b.noise
        and a.epsilon > ?
        and b.epsilon < ?''', (m, m, e, e))
            for row in cur:
                eps, noise = row
                load = int(noise * m / 100) + totalusers
                latency = interpolant(clen, m, load)
                #   if latency < 0:
                #       continue
                srvs.append(m)
                lats.append(latency)
                if m in dots:
                    dotvs.append(latency)
        if projected:
            plot.plot(srvs, lats, '--', color=colors[i])
        else:
            #   plot.plot(srvs, lats, '-', color=colors[i], label='$\ell={}$'.format(clen))
            plot.plot(dots, dotvs, '+', color=colors[i], ms=6, mew=1.5)
            plot.plot(srvs, lats, '-', color=colors[i], label='$\ell={}$'.format(clen))

highestx = 500

vzcompare = vz(30)
plot.axhline(vzcompare, linestyle='--', color=vzcolor)
xshift = highestx
yshift = -50; yshift = 0
plot.text(xshift, vzcompare+yshift, 'Vuvuzela, $\ell=3$', horizontalalignment='right', fontsize=8)

# projections

plot.legend(loc='center right', prop={'size':8},framealpha=1.0)
plot.title('30 million users, $e^\epsilon = 3$', fontsize=8)
plot.xlabel('number of servers', fontsize=8)
plot.ylabel('round latency (s)', fontsize=8)
#   plot.yticks(fontsize=8)
plot.xlim([50, highestx])
plot.ylim([0, 800])
plot.xticks([50] + list(range(100, highestx+100, 100)), fontsize=8)

plot.gca().set_axisbelow(True)
plot.grid(color='gray')
s = plot.gcf().get_size_inches()
#   plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
plot.savefig('latvsrv.pdf')
plot.clf()

################

#   # eps v servers, varying chain length, latency = 120s
#   clens = [3, 6, 9]
#   #   ms = list(range(50, 1001, 10))
#   ms = list(range(50, 501, 10))
#   lat = 120
#   #   totalusers = 240
#   totalusers = vzinv(lat) * 10
#   maxnoise = 400
#   for projected in [False, True]:
#       for i, clen in enumerate(clens):
#           epses = []
#           srvs = []

#           for m in ms:
#               if projected and m < 100:
#                   continue
#               if not projected and m > 100:
#                   continue
            
#               minlat = interpolant(clen, m, math.ceil(totalusers))
#               if minlat > lat:
#                   continue

#               # binary search for right amount of noise which puts us right under
#               highnoise = maxnoise
#               lownoise = 0
#               noise = None
#               propnoise = -1
#               lastnoise = -2
#               while lastnoise != propnoise:
#                   lastnoise = propnoise
#                   propnoise = lownoise + ((highnoise - lownoise) / 2)
#                   propnoise = int(propnoise)
#                   thisload = int(propnoise * m / 100) + math.ceil(totalusers)
#                   nextload = int((propnoise+1) * m / 100) + math.ceil(totalusers)
#                   thislat = interpolant(clen, m, thisload)
#                   nextlat = interpolant(clen, m, nextload)

#                   if thislat <= lat and nextlat > lat:
#                       noise = propnoise
#                   elif nextlat <= lat: # can add more noise
#                       lownoise = propnoise
#                   elif thislat > lat: # too much noise added
#                       highnoise = propnoise

#               if noise is None:
#                   # set epsilon to largest computed value (corresponding to maxnoise)
#                   print 'propnoise out of bounds', propnoise
#                   noise = maxnoise

#               print 'l={}: noise {} for m={}'.format(clen, noise, m)

#               cur = conn.execute('''
#           select a.epsilon, a.noise
#           from noise a
#           where a.servers=?
#           and a.noise = ?''', (m, int(noise)))
#               for row in cur: # should have at most one
#                   eps, noise = row
#                   srvs.append(m)
#                   epses.append(numpy.log(eps))
#                   break

#           if projected:
#               plot.plot(srvs, epses, '--', color=colors[i])
#           else:
#               plot.plot(srvs, epses, '-', color=colors[i], label='$\ell={}$'.format(clen))

#   highestx = 500

#   vzcompare = vzinv(lat)
#   print 'vzinv', lat, vzcompare
#   plot.axhline(numpy.log(3), linestyle='--', color=vzcolor)
#   xshift = highestx
#   yshift = -0.1
#   plot.text(xshift, numpy.log(3)+yshift, 'Vuvuzela, $\ell=3$', horizontalalignment='right', fontsize=8)

#   # projections

#   plot.legend(loc='upper right', prop={'size':8},framealpha=1.0)
#   plot.title('{} million users, {}s latency'.format(math.ceil(totalusers)/10, lat), fontsize=8)
#   plot.xlabel('number of servers', fontsize=8)
#   plot.xlim([50, highestx])
#   plot.xticks([50] + list(range(100, highestx+100, 100)), fontsize=8)
#   plot.ylabel('$\epsilon$', fontsize=10)
#   plot.ylim([0.2, 2])
#   plot.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], fontsize=8)

#   plot.grid()
#   s = plot.gcf().get_size_inches()
#   #   plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
#   plot.savefig('epsvsrv120.pdf')
#   plot.clf()

################

# eps v servers, varying chain length, latency = 120s, 300s
clens = [3, 6, 9]
#   ms = list(range(50, 1001, 10))
ms = sorted(list(range(50, 501, 10)) + [75])
lats = [120, 300]
#   totalusers = 240
maxnoise = 400
f, axarr = plot.subplots(1, 2, sharey=True)
for j, lat in enumerate(lats):
    totalusers = vzinv(lat) * 10
    for projected in [False, True]:
        for i, clen in enumerate(clens):
            epses = []
            srvs = []
            dotms = []
            dotvs = []

            for m in ms:
                if projected and m < 100:
                    continue
                if not projected and m > 100:
                    continue

                minlat = interpolant(clen, m, math.ceil(totalusers))
                if minlat > lat:
                    continue

                # binary search for right amount of noise which puts us right under
                highnoise = maxnoise
                lownoise = 0
                noise = None
                propnoise = -1
                lastnoise = -2
                while lastnoise != propnoise:
                    lastnoise = propnoise
                    propnoise = lownoise + ((highnoise - lownoise) / 2)
                    propnoise = int(propnoise)
                    thisload = int(propnoise * m / 100) + math.ceil(totalusers)
                    nextload = int((propnoise+1) * m / 100) + math.ceil(totalusers)
                    thislat = interpolant(clen, m, thisload)
                    nextlat = interpolant(clen, m, nextload)

                    if thislat <= lat and nextlat > lat:
                        noise = propnoise
                    elif nextlat <= lat: # can add more noise
                        lownoise = propnoise
                    elif thislat > lat: # too much noise added
                        highnoise = propnoise

                if noise is None:
                    # set epsilon to largest computed value (corresponding to maxnoise)
                    print 'propnoise out of bounds', propnoise
                    noise = maxnoise

                print 'l={}: noise {} for m={}'.format(clen, noise, m)

                cur = conn.execute('''
            select a.epsilon, a.noise
            from noise a
            where a.servers=?
            and a.noise = ?''', (m, int(noise)))
                for row in cur: # should have at most one
                    eps, noise = row
                    srvs.append(m)
                    epses.append(numpy.log(eps))
                    if m in dots:
                        print clen, m, lat, eps
                        dotms.append(m)
                        dotvs.append(numpy.log(eps))
                    break

            if projected:
                axarr[j].plot(srvs, epses, '--', color=colors[i])
            else:
                axarr[j].plot(srvs, epses, '-', color=colors[i], label='$\ell={}$'.format(clen))
                axarr[j].plot(dotms, dotvs, '+', color=colors[i], ms=6, mew=1.5)

    highestx = 500

    vzcompare = vzinv(lat)
    print 'vzinv', lat, vzcompare
    axarr[j].axhline(numpy.log(3), linestyle='--', color=vzcolor)
    xshift = highestx
    yshift = -0.15; yshift = 0
    axarr[j].text(xshift, numpy.log(3)+yshift, 'Vuvuzela, $\ell=3$', horizontalalignment='right', fontsize=8)

    axarr[j].set_title('{}M users, {}s latency'.format(math.ceil(totalusers)/10, lat), fontsize=8)

    axarr[j].set_xlim([50, highestx])
    axarr[j].set_xlabel('number of servers')
    axarr[j].set_xticks(list(range(100, highestx+100, 100)))
    axarr[j].set_axisbelow(True)
    axarr[j].grid(color='gray')
    #   axarr[j].set_xlabel('number of servers', fontsize=8)
    #   axarr[j].set_xticks([50] + list(range(100, highestx+100, 100)), fontsize=8)

# projections

plot.legend(loc='upper right', prop={'size':8},framealpha=1.0)
axarr[0].set_ylabel('$\epsilon$', fontsize=10)
axarr[0].set_ylim([0.2, 2])
#   plot.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], fontsize=8)
axarr[0].set_yticks([0.4, 0.8, 1.2, 1.6, 2.0])

#   plot.grid()
s = plot.gcf().get_size_inches()
plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
#   plot.gcf().set_size_inches((s[0] / 1.1, s[1] / 1.1))
plot.savefig('epsvsrv.pdf')
plot.clf()
