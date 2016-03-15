import csv
import os
import sys

import matplotlib.pyplot as plot
from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})
#   rcParams['pdf.fonttype'] = 42
#   rcParams['ps.fonttype'] = 42
rcParams['ps.useafm'] = True
rcParams['pdf.use14corefonts'] = True
rcParams['text.usetex'] = True

assert len(sys.argv) == 2, 'usage: {} <dir>'.format(sys.argv[0])

d = os.listdir(sys.argv[1])
d = sorted([int(n.replace('servers_', '').replace('.csv', '')) for n in d])

numservers = d
numservers = [25, 50, 100, 200, 400, 800]
fnames = ['servers_' + str(x) + '.csv' for x in numservers]

target_k = ' 10^4.0'
target_delta = '0.0001'

data = []
for i, fname in enumerate(fnames):
    with open(sys.argv[1] + '/' + fname) as f:
        reader = csv.reader(f)
        #   for row in reader:
        #       print row
        matches = [row for row in reader if row[1] == target_k and row[3] == target_delta]
        for m in matches:
            noise, k, eps, delta = m
            data.append((numservers[i], int(noise) * 1000, float(eps)))

print data
points = data

colors = ['#543005','#8c510a','#bf812d','#dfc27d','#f6e8c3','#f5f5f5','#c7eae5','#80cdc1','#35978f','#01665e','#003c30', '#a6cee3']

colors = ['#f7fcfd','#e5f5f9','#ccece6','#99d8c9','#66c2a4','#41ae76','#238b45','#006d2c','#00441b', '#000000'][::-1]

colors = ['#feebe2','#fcc5c0','#fa9fb5','#f768a1','#dd3497','#ae017e','#7a0177']

configs = numservers

for i, c in enumerate(configs):
    xs = [x[1] / 1e3 for x in points if x[0] == c]
    ys = [x[2] for x in points if x[0] == c]
    plot.plot(xs, ys, '-', color=colors[i+1], label='{} servers'.format(c))

# xt = sorted(list(set([x[1] / 1e6 for x in points])))
xt = range(0, 211, 20)
xt = xt[1:-1]
# xt = [xt[0], xt[3], xt[5]] + xt[6:]
plot.xlim([0, 200])
plot.ylim([0, 20])
plot.xlabel('noise (thousands of messages per server)', fontsize=8)
# plot.ylabel(u'e^\u03b5', fontsize=8)
plot.ylabel(r'$e^\epsilon$', fontsize=10)
plot.yticks(fontsize=8)
plot.xticks(xt, fontsize=8)
plot.grid(color='gray')

plot.legend(loc='upper right', prop={'size':8})
s = plot.gcf().get_size_inches()
plot.gcf().set_size_inches((s[0] / 2, s[1] / 2))
plot.savefig('servers_privacy.pdf')

