import numpy as np
import matplotlib.pyplot as plot

from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})
#   rcParams['pdf.fonttype'] = 42
#   rcParams['ps.fonttype'] = 42
rcParams['ps.useafm'] = True
rcParams['pdf.use14corefonts'] = True
rcParams['text.usetex'] = True

l = np.arange(3, 15)

shades = ['#ffffcc','#c2e699','#78c679','#31a354','#006837'][::-1]
#   shades = ['#ffffcc','#d9f0a3','#addd8e','#78c679','#31a354','#006837'][::-1]
#   groups = ['#006837','#7570b3','#d95f02','#e7298a']
groups = ['#e41a1c','#377eb8','#4daf4a','#984ea3']
groups = ['#c2e699','#78c679','#ffffcc','#238443']
groups[2] = '#000000'
symbols = ['-', '--', ':']

# varying m
f = 0.25
for i, m in enumerate([100, 500, 1000]):
    inv = int(1/f)
    plot.plot(l, m*(f**l), symbols[i], color=groups[2], label='$({},1/{})$'.format(m, inv))
    #   if m == 100:
        
    #   else:
    #       plot.plot(l, m*(f**l), symbols[i], color=groups[2], label='$m={}$'.format(m))
    plot.plot(l, m*(f**l), '+', color=groups[2], ms=6, mew=1.5)
#   m, f = (500, 0.25)
#   plot.plot(l, m*(f**l), '.--', color=shades[0], label='$m=500$')
#   m, f = (1000, 0.25)
#   plot.plot(l, m*(f**l), '.:', color=shades[0], label='$m=1000$')
#   m, f = (100, 0.25)
#   plot.plot(l, m*(f**l), '.-', color=groups[0], label='$f=0.25$')

m = 100
for i, f in enumerate([0.1, 0.2, 0.25, 0.5]):
    if f == 0.25:
        continue

    inv = int(1/f)
    plot.plot(l, m*(f**l), '-', color=groups[i], label='$({},1/{})$'.format(m, inv))
    #   plot.plot(l, m*(f**l), '-', color=groups[i], label='$f=1/{}$'.format(inv))
    plot.plot(l, m*(f**l), '+', color=groups[i], ms=6, mew=1.5)
#   m, f = (100, 0.1)

#   m, f = (100, 0.33)
#   plot.plot(l, m*(f**l), '.-', color=shades[2], label='$f=1/3$')
#   m, f = (100, 0.5)
#   plot.plot(l, m*(f**l), '.-',  color=shades[3], label='$f=1/2$')

#   xt = range(3, 15)
#   xt = [x for x in xt if x % 2 == 0]
xt = [3, 6, 9, 12, 14]
# xt = [xt[0], xt[3], xt[5]] + xt[6:]
plot.xlim([0, 15])
plot.xticks(xt, fontsize=8)
plot.ylim([10e-6, 0.5])
plot.yticks(fontsize=8)

#   plot.title('30 million users, $m=100$ servers', fontsize=8)
plot.xlabel('chain length', fontsize=8)
plot.ylabel('probability of compromised chain', fontsize=8)
plot.yscale('log')
plot.gca().set_axisbelow(True)
plot.grid(color='gray')
l = plot.legend(loc='lower left', prop={'size':8})
s = plot.gcf().get_size_inches()
plot.gcf().set_size_inches((s[0] / 2, s[1] / 2.4))
plot.savefig('chainsec.pdf', bbox_inches='tight')

#   plot.xlabel('chain length')
#   plot.ylabel('probability of compromised chain')
#   plot.yscale('log')
#   plot.ylim([10e-6, 0.5])
#   plot.legend()
#   plot.show()
