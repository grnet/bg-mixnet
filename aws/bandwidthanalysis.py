import json
import os
import sys

assert len(sys.argv) == 2, 'usage: {} <dir>'.format(sys.argv[0])

d = os.listdir(sys.argv[1])
d = [x for x in d if 'net' in x and 'dat' in x]

def tryread(s):
    try:
        return int(s)
    except:
        if 'K' in s:
            return int(s[:-1]) * 1000

stats = {}

for fname in d:
    _, _, chainlen, _, _ = fname.split('-')
    chainlen = int(chainlen)

    altname = fname.replace('.net.dat', '.roundstats.json')
    with open(sys.argv[1] + '/' + altname) as f:
        data = json.load(f)
        totaltime = int(data[0]['Total'])

    with open(sys.argv[1] + '/' + fname) as f:
        totkbin, totkbout, totpktin, totpktout = 0, 0, 0, 0
        for row in f:
            if 'waiting' in row:
                continue
            if row[0] == '#':
                continue
            row = row.strip()
            entries = row.split(' ')
            entries = [x for x in entries if len(x) > 0]
            time, kbin, pktin, kbout, pktout = entries

            kbin = tryread(kbin)
            kbout = tryread(kbout)
            pktin = tryread(pktin)
            pktout = tryread(pktout)

            totkbin += kbin
            totkbout += kbout
            totpktin += pktin
            totpktout += pktout

        stats[chainlen] = (totkbin, totkbout, totpktin, totpktout, totaltime)

print stats

for k in sorted(stats.keys()):
    v = stats[k]
    # chain length, gb out (collectl), time (s), gb/s
    gbout = v[1] / 1e7 # extra factor of 10 to denormalize against .1s collectl interval
    time = v[-1] / 1e9
    print k, gbout, time, gbout/time

for fname in d:
    _, _, chainlen, _, _ = fname.split('-')
    chainlen = int(chainlen)
    proofcost = 0
    ctextcost = 0
    mdcost = 0
    totcost = 0
    altname = fname.replace('.net.dat', '.info.dat')
    with open(sys.argv[1] + '/' + altname) as f:
        for row in f:
            if 'RPC' not in row:
                continue

            if '(proof)' in row:
                contents = row.split('proof=')
                proofcost += int(contents[1]) * (chainlen-1)
                contents = contents[0].split(' output=')
                proofcost += int(contents[1]) * (chainlen-1)
                contents = contents[0].split(' input=')
                proofcost += int(contents[1]) * (chainlen-1)

            if '(message)' in row:
                contents = row.split(' metadata=')
                mdcost += int(contents[1])
                contents = contents[0].split('content=')
                ctextcost += int(contents[1])

            if 'RPC send' in row:
                contents = row.split('size=')
                totcost += int(contents[1])
            #   print row

    # chain length, ciphertext, metadata, proof, total (Go)
    print chainlen, ctextcost / 1e9, mdcost / 1e9, proofcost / 1e9, totcost / 1e9
