import boto3
import paramiko
import json
import os
import random
import sys
import time

client = boto3.client('ec2')

#   if len(sys.argv) != 2:
#       print 'usage: {} <number_of_instances>'.format(sys.argv[0])
#       sys.exit(1)

# BEGIN CONFIGURABLE

# config stuff
port = 2720
# num_instances = int(sys.argv[1])
chain_len = 3
payload_filename = 'benchmark.conf'
git_key_filename = 'derek-git-token.txt'
key_filename = 'derek-stadium.pem'

# END CONFIGURABLE

outstanding_requests = client.describe_spot_fleet_requests()['SpotFleetRequestConfigs']

active = [x for x in outstanding_requests if x['SpotFleetRequestState'] == 'active']
assert len(active) == 1
rid = active[0]['SpotFleetRequestId']

response = client.describe_spot_fleet_instances(
    SpotFleetRequestId = rid
)
instances = response['ActiveInstances']
iids = [x['InstanceId'] for x in instances]

response = client.describe_instances(
    InstanceIds = iids
)
reservations = response['Reservations']

instances = []

for r in reservations:
    i = r['Instances']
    instances = instances + i
    print len(instances)


# assert len(reservations) == 1

# print 1 / 0
# instances = response['Reservations'][0]['Instances']
# assert len(instances) == num_instances

print 'instance IPs:'
ips = [x['PublicIpAddress'] for x in instances]
for ip in ips:
    print ip
        
from multiprocessing.dummy import Pool
from threading import Lock

blacklist = ['34.201.12.176']

def benchmark_one_stadium_round(ips, chain_len, num_machines, num_msgs, result_filename):
    if num_machines > len(ips):
        raise Exception('not enough machines for test! given {} but need {}'.format(len(ips), num_machines))

    ips = [x for x in ips if x not in blacklist]
    ips = random.sample(ips, num_machines)
    listen_ips = [x + ':2720' for x in ips]


    config = {
        "ListenAddrs": listen_ips,
        "ChainLen": chain_len,
        "NumMsgs": num_msgs,
    }
    sys.stderr.write('running round with config {}\n'.format(config))

    with open(payload_filename, 'w') as f:
        json.dump(config, f)

    with open(git_key_filename) as f:
        token = f.read().strip()

    server_commands = ''' # paramiko: remote server commands
    export PATH=$PATH:$HOME/.local/bin:$HOME/bin:/usr/local/go/bin;
    export GOPATH=$HOME/go;
    export LD_LIBRARY_PATH=/usr/local/lib/:$HOME/go/src/stadium/groth;
    cd ~/go/src/stadium/groth;
    git config remote.origin.url https://derbear:{}@github.com/nirvantyagi/stadium.git
    git fetch;
    git checkout -- ..;
    git checkout master;
    git branch -D benchmark;
    git reset --hard origin/benchmark;
    git checkout benchmark;
    git checkout -- ..;
    # make &> output.log;
    go install stadium/groth stadium/stadium stadium/coordinator stadium/server;
    cd ~/go/src/stadium/groth;
    '''.format(token)
    k = paramiko.RSAKey.from_private_key_file(key_filename)

    def copyover(args):
        i, ip = args
        try:
            c = paramiko.SSHClient()
            c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            c.connect(hostname = ip, username = 'ec2-user', pkey = k)
            c.exec_command('pkill server')
            c.exec_command('pkill python')
            c.exec_command('pkill collectl')
            c.exec_command('rm {}'.format(payload_filename))
            c.exec_command('rm round.out')

            cmd = 'scp -i {} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null {} ec2-user@{}:~'.format(key_filename, payload_filename, ip)
            print cmd
            os.system(cmd)

            run_cmd = 'nohup python ~/go/src/stadium/aws/aws_benchmark.py ~/{} {} > ~/benchmark.out 2> ~/benchmark.err'.format(payload_filename, i)
            cmd = server_commands + run_cmd
            #   print cmd
            c.exec_command(cmd)
            time.sleep(5)
            c.close()
            print 'execute remote command (omitted from print) on server', i, ":", ip
        except Exception as e:
            print 'failure on ', i, ':', ip
            raise e

    p = Pool(len(ips))
    p.map(copyover, enumerate(ips))

    print 'remote servers (hopefully) set up; launching stadium round now!'

    cmd = 'cp {} {}.conf'.format(payload_filename, result_filename)
    print cmd
    os.system(cmd)

    cmd = '~/go/bin/coordinator -conf {} > {}'.format(payload_filename, result_filename)
    print cmd
    os.system(cmd)

    cmd = 'cp roundstats.json {}.roundstats.json'.format(result_filename)
    print cmd
    os.system(cmd)

    #   for i, ip in enumerate(ips):
    #       cmd = 'scp -i {} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ec2-user@{}:~/benchmark.collectl net_{}.dat'.format(key_filename, ip, ip)
    #       print cmd
        # collectl is huge and not really useful
        # os.system(cmd)

    # os.system('tail -n +1 net*.dat > {}.net.dat'.format(result_filename))
    # os.system('rm net*.dat')

    time.sleep(2)

def experimentM():
    'Server upper bound probe'
    ips0 = [x for x in ips if x not in blacklist]
    M = len(ips0)
    chain_len = 8
    trial_num = 0
    num_msgs = 300000
    benchmark_one_stadium_round(ips0, chain_len, M, num_msgs, "results-{:02}-{:02}-{:02}.txt".format(chain_len, M, trial_num))

def experimentT():
    'Cheap dry run to build blacklist'
    ips0 = [x for x in ips if x not in blacklist]
    M = len(ips0)
    chain_len = 3
    trial_num = 0
    num_msgs = 10000
    benchmark_one_stadium_round(ips0, chain_len, M, num_msgs, "results-{:02}-{:02}-{:02}.txt".format(chain_len, M, trial_num))

def experiment1():
    'Varying chain lengths with 12 machines'
    num_machines = 12
    num_msgs = 100000
    for chain_len in range(12, 2, -1):
        for trial_num in range(1, 4):
            benchmark_one_stadium_round(ips, chain_len, num_machines, num_msgs, "results-{:08}-{:02}-{:02}-{:02}.txt".format(num_msgs, chain_len, num_machines, trial_num))
    try:
        os.system('mkdir experiment/1')
    except:
        pass
    os.system('mv results* experiment/1')

def experiment2():
    'Varying number of machines with chain lengths 3, 8, and 12'
    num_msgs = 200000
    for chain_len in [3, 8, 12]:
        for num_machines in range(chain_len, 13):
            for trial_num in range(1, 4):
                benchmark_one_stadium_round(ips, chain_len, num_machines, num_msgs, "results-{:08}-{:02}-{:02}-{:02}.txt".format(num_msgs, chain_len, num_machines, trial_num))

    try:
        os.system('mkdir experiment/2')
    except:
        pass
    os.system('mv results* experiment/2')


def experiment3():
    '''
    Varying total number of messages (100K, 150K, 200K, 500K, 750K, 1M, 2M, 5M, 7.5M, 10M),
    fixed chain length of 8,
    fixed number of servers 10, 20, 25, 30, 40, 50, 60, 70, 75, 80, 90, 100
    '''
    num_msgs = [100, 150, 200, 500, 750, 1000, 2000, 5000, 7500, 10000]
    num_msgs = [x * 1000 for x in num_msgs]
    num_msgs = num_msgs[::-1]
    # num_machines = [10, 20, 25, 30, 40, 50, 60, 70, 75, 80, 90, 100]
    num_machines = [10, 20, 25, 30]
    num_machines = num_machines[::-1]
    chain_len = 8
    trial_num = 0
    for m in num_machines:
        for n0 in num_msgs:
            n = n0 / m
            if n > 330000:
                continue
            print m, n
            benchmark_one_stadium_round(ips, chain_len, m, n, "results-{:08}-{:02}-{:02}-{:02}.txt".format(n0, chain_len, m, trial_num))

    try:
        os.system('mkdir experiment/3')
    except:
        pass
    os.system('mv results* experiment/3')

def experiment4():
    'Varying number of cores for M messages'
    pass


import datetime
now = datetime.datetime.now
print 'starting experiments. time is', now()

try:
    os.system('mkdir experiment')
except:
    pass

print 'starting experiment 1. time is', now()
experiment1()

#   print 'starting experiment 2. time is', now()
#   experiment2()

#   print 'starting experiment 3. time is', now()
#   experiment3()

#   print 'starting light experiment. time is', now()
#   experimentT()

#   print 'starting probe experiment. time is', now()
#   experimentM()

print 'finished experiments. time is', now()
